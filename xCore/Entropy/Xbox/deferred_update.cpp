///////////////////////////////////////////////////////////////////////////////
// INCLUDES 
///////////////////////////////////////////////////////////////////////////////

#include "deferred_pipeline.hpp"



///////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF IDEFERRED3DDEVICE8: IUNKNOWN
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////

ULONG __stdcall IDeferred3DDevice8::AddRef( void )
{
    return g_pd3dInternal->AddRef();
}

///////////////////////////////////////////////////////////////////////////

ULONG __stdcall IDeferred3DDevice8::Release( void )
{
    return g_pd3dInternal->Release();
}



///////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////

static u32 CountVertices( D3DPRIMITIVETYPE Type,UINT Count )
{
    u32 Bytes;
    switch( Type )
    {
        case D3DPT_TRIANGLESTRIP:
            Bytes = Count*3;
            break;
        case D3DPT_TRIANGLELIST:
            Bytes = Count-2;
            break;
        case D3DPT_TRIANGLEFAN:
            Bytes = Count-1;
            break;
        case D3DPT_LINESTRIP:
            Bytes = Count*2;
            break;
        case D3DPT_POINTLIST:
            Bytes = Count;
            break;
        case D3DPT_LINELIST:
            Bytes = Count-1;
            break;
        case D3DPT_QUADLIST:
            Bytes = Count*4;
            break;

        default:
            BREAK;
    }
    return Bytes;
}

///////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF IDeferred3DDevice8: DEFERRED FUNCTIONALITY
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::DrawPrimitiveUP( D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void *pVertexStreamZeroData,UINT VertexStreamZeroStride )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"DrawPrimitiveUP" );
#endif

    g_Cmds.begin( D3DDevice::DrawPrimitiveUP );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( pVertexStreamZeroData,VertexStreamZeroStride * CountVertices( PrimitiveType,PrimitiveCount ));

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       VertexStreamZeroStride );
        g_Cmds.write( Arg0, pVertexStreamZeroData  );
        g_Cmds.write(       PrimitiveCount         );
        g_Cmds.write(       PrimitiveType          );
    }
    g_Cmds.end(4);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::DrawIndexedPrimitiveUP
(
    D3DPRIMITIVETYPE    PrimitiveType,
    UINT                UnusedMinIndex,
    UINT                UnusedNumVertices,
    UINT                PrimitiveCount,
    CONST void*         pIndexData,
    D3DFORMAT           IndexDataFormat,
    CONST void*         pVertexStreamZeroData,
    UINT                VertexStreamZeroStride
)
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"DrawIndexedPrimitiveUP" );
#endif

    g_Cmds.begin( D3DDevice::DrawIndexedPrimitiveUP );
    {
        // save all structures ................................................

        u32 IndexCount = CountVertices( PrimitiveType,PrimitiveCount );

        void* Arg0 = g_Cmds.write( pIndexData,(( IndexDataFormat==D3DFMT_INDEX16 ) ? 2:4 )*IndexCount );
        void* Arg1 = g_Cmds.write( pVertexStreamZeroData,VertexStreamZeroStride * IndexCount );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       VertexStreamZeroStride );
        g_Cmds.write( Arg1, pVertexStreamZeroData  );
        g_Cmds.write(       IndexDataFormat        );
        g_Cmds.write( Arg0, pIndexData             );
        g_Cmds.write(       PrimitiveCount         );
        g_Cmds.write(       UnusedNumVertices      );
        g_Cmds.write(       UnusedMinIndex         );
        g_Cmds.write(       PrimitiveType          );
    }
    g_Cmds.end(8);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::DrawIndexedVertices( D3DPRIMITIVETYPE PrimitiveType,UINT VertexCount,CONST WORD *pIndexData )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"DrawIndexedVertices" );
#endif

    g_Cmds.begin( D3DDevice::DrawIndexedVertices );
    {
        g_Cmds.write( pIndexData );
        g_Cmds.write( VertexCount );
        g_Cmds.write( PrimitiveType );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::DrawVerticesUP
(
    D3DPRIMITIVETYPE    PrimitiveType,
    UINT                VertexCount,
    CONST void*         pVertexStreamZeroData,
    UINT                VertexStreamZeroStride
)
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"DrawVerticesUP" );
#endif

    g_Cmds.begin( D3DDevice::DrawVerticesUP );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( pVertexStreamZeroData,VertexStreamZeroStride * VertexCount );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       VertexStreamZeroStride );
        g_Cmds.write( Arg0, pVertexStreamZeroData  );
        g_Cmds.write(       VertexCount            );
        g_Cmds.write(       PrimitiveType          );
    }
    g_Cmds.end(4);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetStreamSource( UINT StreamNumber,D3DVertexBuffer *pStreamData,UINT Stride )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetStreamSource" );
#endif

    g_Cmds.begin( D3DDevice::SetStreamSource );
    {
        // save all structures ................................................

        void* Arg0 = NULL; if( pStreamData ) Arg0 = g_Cmds.write( *pStreamData );

        // write arguments ....................................................

        g_Cmds.write(       Stride       );
        g_Cmds.write( Arg0, pStreamData  );
        g_Cmds.write(       StreamNumber );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::PrimeVertexCache( UINT VertexCount,CONST WORD *pIndexData )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"PrimeVertexCache" );
#endif

    g_Cmds.begin( D3DDevice::PrimeVertexCache );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( pIndexData,VertexCount*sizeof(WORD) );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg0, pIndexData  );
        g_Cmds.write(       VertexCount );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::Reset( D3DPRESENT_PARAMETERS *pPresentationParameters )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"Reset" );
#endif

    g_Cmds.begin( D3DDevice::Reset );
    {
        // save all structures ................................................

        void *Arg0 = g_Cmds.write( *pPresentationParameters );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg0, pPresentationParameters );
    }
    g_Cmds.end(1);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::Present( CONST RECT *pSourceRect,CONST RECT *pDestRect,void *pUnused,void *pUnused2 )
{
#if 0
    // Dispatch presentation packet *******************************************

    g_Cmds.begin( D3DDevice::Present );
    {
        // save all structures ................................................

        void* Arg0 = NULL; if( pSourceRect ) Arg0 = g_Cmds.write( *pSourceRect );
        void* Arg1 = NULL; if( pDestRect   ) Arg1 = g_Cmds.write( *pDestRect   );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       pUnused2    );
        g_Cmds.write(       pUnused     );
        g_Cmds.write( Arg1, pDestRect   );
        g_Cmds.write( Arg0, pSourceRect );
    }
    g_Cmds.end(4);
#endif
    // Now kick off the frame *************************************************

    g_Cmds.flip();

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

DWORD WINAPI IDeferred3DDevice8::Swap( DWORD Flags )
{
    HRESULT Result;
#if 0
    // Dispatch swap packet ***************************************************

    g_Cmds.begin( D3DDevice::Swap );
    g_Cmds.write( Flags );
    g_Cmds.end(1);

    // Now kick off the frame *************************************************

    if( Flags & D3DSWAP_FINISH )
        g_Cmds.flip();
    Result = S_OK;
#else
    Result = g_pd3dInternal->Swap( Flags );
    if( Flags & D3DSWAP_FINISH )
        g_Cmds.flip();
#endif
    return Result;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetRenderTarget( D3DSurface* pRenderTarget,D3DSurface *pNewZStencil )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetRenderTarget" );
#endif

    g_Cmds.begin( D3DDevice::SetRenderTarget );
    {
        g_Cmds.write( pNewZStencil  );
        g_Cmds.write( pRenderTarget );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::BeginScene( void )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"BeginScene" );
#endif

    g_Cmds.begin( D3DDevice::BeginScene );
    g_Cmds.end(0);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::EndScene( void )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"EndScene" );
#endif

    g_Cmds.begin( D3DDevice::EndScene );
    g_Cmds.end(0);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::Clear( DWORD Count,CONST D3DRECT *pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"Clear" );
#endif

    g_Cmds.begin( D3DDevice::Clear );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( pRects,sizeof(D3DRECT)*Count );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       Stencil );
        g_Cmds.write(       Z       );
        g_Cmds.write(       Color   );
        g_Cmds.write(       Flags   );
        g_Cmds.write( Arg0, pRects  );
        g_Cmds.write(       Count   );
    }
    g_Cmds.end(6);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetTransform( D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX *pMatrix )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetTransform" );
#endif

    g_Cmds.begin( D3DDevice::SetTransform );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( pMatrix,sizeof(D3DMATRIX) );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg0, pMatrix );
        g_Cmds.write(       State   );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::MultiplyTransform( D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX *pMatrix )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"MultiplyTransform" );
#endif

    g_Cmds.begin( D3DDevice::MultiplyTransform );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( *pMatrix );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg0, pMatrix );
        g_Cmds.write(       State   );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetViewport( CONST D3DVIEWPORT8* pViewport )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetViewport" );
#endif

    g_Cmds.begin( D3DDevice::SetViewport );
    {
        // save all structures ................................................

        void* Arg0 = NULL; if( pViewport ) Arg0 = g_Cmds.write( *pViewport );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg0, pViewport );
    }
    g_Cmds.end(1);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetMaterial( CONST D3DMATERIAL8* pMaterial )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetMaterial" );
#endif

    g_Cmds.begin( D3DDevice::SetMaterial );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( *pMaterial );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg0, pMaterial );
    }
    g_Cmds.end(1);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetLight( DWORD Index,CONST D3DLIGHT8* pLight )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetLight" );
#endif

    g_Cmds.begin( D3DDevice::SetLight );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( *pLight );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg0, pLight );
        g_Cmds.write(       Index  );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::LightEnable( DWORD Index,BOOL Enable )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"LightEnable" );
#endif

    g_Cmds.begin( D3DDevice::LightEnable );
    {
        g_Cmds.write( Enable );
        g_Cmds.write( Index  );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetRenderState( D3DRENDERSTATETYPE State,DWORD Value )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetRenderState" );
#endif

    g_Cmds.begin( D3DDevice::SetRenderState );
    {
        g_Cmds.write( Value );
        g_Cmds.write( State );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::ApplyStateBlock( DWORD Token )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"ApplyStateBlock" );
#endif

    g_Cmds.begin( D3DDevice::ApplyStateBlock );
    {
        g_Cmds.write( Token );
    }
    g_Cmds.end(1);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetTexture( DWORD Stage,D3DBaseTexture* pTexture )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetTexture" );
#endif

    g_Cmds.begin( D3DDevice::SetTexture );
    {
        g_Cmds.write( pTexture );
        g_Cmds.write( Stage    );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetTextureStageState( DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetTextureStageState" );
#endif

    g_Cmds.begin( D3DDevice::SetTextureStageState );
    {
        g_Cmds.write( Value );
        g_Cmds.write( Type  );
        g_Cmds.write( Stage );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::DrawPrimitive( D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"DrawPrimitive" );
#endif

    g_Cmds.begin( D3DDevice::DrawPrimitive );
    {
        g_Cmds.write( PrimitiveCount );
        g_Cmds.write( StartVertex    );
        g_Cmds.write( PrimitiveType  );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::DrawIndexedPrimitive( D3DPRIMITIVETYPE PrimitiveType,UINT UnusedMinIndex,UINT UnusedNumIndices,UINT StartIndex,UINT PrimitiveCount )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"DrawIndexedPrimitive" );
#endif

    g_Cmds.begin( D3DDevice::DrawIndexedPrimitive );
    {
        g_Cmds.write( PrimitiveCount   );
        g_Cmds.write( StartIndex       );
        g_Cmds.write( UnusedNumIndices );
        g_Cmds.write( UnusedMinIndex   );
        g_Cmds.write( PrimitiveType    );
    }
    g_Cmds.end(5);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexShader( DWORD Handle )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexShader" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexShader );
    {
        g_Cmds.write( Handle );
    }
    g_Cmds.end(1);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexShaderConstant( INT Register,CONST void *pConstantData,DWORD ConstantCount )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexShaderConstant" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexShaderConstant );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( pConstantData,sizeof(vector4)*ConstantCount );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       ConstantCount );
        g_Cmds.write( Arg0, pConstantData );
        g_Cmds.write(       Register      );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexShaderConstantFast( INT Register,CONST void *pConstantData,DWORD ConstantCount )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexShaderConstantFast" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexShaderConstantFast );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( pConstantData,sizeof(vector4)*ConstantCount );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       ConstantCount );
        g_Cmds.write( Arg0, pConstantData );
        g_Cmds.write(       Register      );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetIndices( D3DIndexBuffer *pIndexData,UINT BaseVertexIndex )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetIndices" );
#endif

    g_Cmds.begin( D3DDevice::SetIndices );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( *pIndexData );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       BaseVertexIndex );
        g_Cmds.write( Arg0, pIndexData );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetPixelShader( DWORD Handle )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetPixelShader" );
#endif

    g_Cmds.begin( D3DDevice::SetPixelShader );
    {
        g_Cmds.write( Handle );
    }
    g_Cmds.end(1);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetPixelShaderProgram( CONST D3DPIXELSHADERDEF *pPSDef )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetPixelShaderProgram" );
#endif

    g_Cmds.begin( D3DDevice::SetPixelShaderProgram );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( *pPSDef );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg0, pPSDef );
    }
    g_Cmds.end(1);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetPixelShaderConstant( DWORD Register,CONST void *pConstantData,DWORD ConstantCount )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetPixelShaderConstant" );
#endif

    g_Cmds.begin( D3DDevice::SetPixelShaderConstant );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( pConstantData,sizeof(vector4)*ConstantCount );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       ConstantCount );
        g_Cmds.write( Arg0, pConstantData );
        g_Cmds.write(       Register      );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetShaderConstantMode( D3DSHADERCONSTANTMODE Mode )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetShaderConstantMode" );
#endif

    g_Cmds.begin( D3DDevice::SetShaderConstantMode );
    {
        g_Cmds.write( Mode );
    }
    g_Cmds.end(1);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::LoadVertexShader( DWORD Handle,DWORD Address )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"LoadVertexShader" );
#endif

    g_Cmds.begin( D3DDevice::LoadVertexShader );
    {
        g_Cmds.write( Address );
        g_Cmds.write( Handle );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::LoadVertexShaderProgram( CONST DWORD *pFunction,DWORD Address )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"LoadVertexShaderProgram" );
#endif

    g_Cmds.begin( D3DDevice::LoadVertexShaderProgram );
    {
        g_Cmds.write( Address   );
        g_Cmds.write( pFunction );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SelectVertexShader( DWORD Handle,DWORD Address )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SelectVertexShader" );
#endif

    g_Cmds.begin( D3DDevice::SelectVertexShader );
    {
        g_Cmds.write( Address );
        g_Cmds.write( Handle );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SelectVertexShaderDirect( D3DVERTEXATTRIBUTEFORMAT *pVAF,DWORD Address )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SelectVertexShaderDirect" );
#endif

    g_Cmds.begin( D3DDevice::SelectVertexShaderDirect );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( *pVAF );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       Address );
        g_Cmds.write( Arg0, pVAF );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::DrawVertices( D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT VertexCount )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"DrawVertices" );
#endif

    g_Cmds.begin( D3DDevice::DrawVertices );
    {
        g_Cmds.write( VertexCount   );
        g_Cmds.write( StartVertex   );
        g_Cmds.write( PrimitiveType );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetPalette( DWORD Stage,D3DPalette *pPalette )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetPalette" );
#endif

    g_Cmds.begin( D3DDevice::SetPalette );
    {
        g_Cmds.write( pPalette );
        g_Cmds.write( Stage );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetTextureStageStateNotInline( DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetTextureStageStateNotInline" );
#endif

    g_Cmds.begin( D3DDevice::SetTextureStageStateNotInline );
    {
        g_Cmds.write( Value );
        g_Cmds.write( Type  );
        g_Cmds.write( Stage );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetRenderStateNotInline( D3DRENDERSTATETYPE State,DWORD Value )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetRenderStateNotInline" );
#endif

    g_Cmds.begin( D3DDevice::SetRenderStateNotInline );
    {
        g_Cmds.write( Value );
        g_Cmds.write( State );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetBackMaterial( CONST D3DMATERIAL8 *pMaterial )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetBackMaterial" );
#endif

    g_Cmds.begin( D3DDevice::SetBackMaterial );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( *pMaterial );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg0, pMaterial );
    }
    g_Cmds.end(1);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::UpdateOverlay( D3DSurface *pSurface,CONST RECT *SrcRect,CONST RECT *DstRect,BOOL EnableColorKey,D3DCOLOR ColorKey )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"UpdateOverlay" );
#endif

    g_Cmds.begin( D3DDevice::UpdateOverlay );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( *pSurface );
        void* Arg1 = g_Cmds.write( *SrcRect  );
        void* Arg2 = g_Cmds.write( *DstRect  );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       ColorKey       );
        g_Cmds.write(       EnableColorKey );
        g_Cmds.write( Arg2, DstRect        );
        g_Cmds.write( Arg1, SrcRect        );
        g_Cmds.write( Arg0, pSurface       );
    }
    g_Cmds.end(5);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::EnableOverlay( BOOL Enable )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"EnableOverlay" );
#endif

    g_Cmds.begin( D3DDevice::EnableOverlay );
    {
        g_Cmds.write( Enable );
    }
    g_Cmds.end(1);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexData2f( INT Register,float a,float b )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexData2f" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexData2f );
    {
        g_Cmds.write( b );
        g_Cmds.write( a );
        g_Cmds.write( Register );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexData4f( INT Register,float a,float b,float c,float d )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexData4f" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexData4f );
    {
        g_Cmds.write( d );
        g_Cmds.write( c );
        g_Cmds.write( b );
        g_Cmds.write( a );
        g_Cmds.write( Register );
    }
    g_Cmds.end(5);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexData2s( INT Register,SHORT a,SHORT b )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexData2s" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexData2s );
    {
        // todo: I'm not sure if BYTE is passed 32-bits or 8-bit
        // todo: I believe it's 32-bit for .NET
BREAK;
        g_Cmds.write( b );
        g_Cmds.write( a );
        g_Cmds.write( Register );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexData4s( INT Register,SHORT a,SHORT b,SHORT c,SHORT d )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexData4s" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexData4s );
    {
        // todo: I'm not sure if BYTE is passed 32-bits or 8-bit
        // todo: I believe it's 32-bit for .NET
BREAK;
        g_Cmds.write( d );
        g_Cmds.write( c );
        g_Cmds.write( b );
        g_Cmds.write( a );
        g_Cmds.write( Register );
    }
    g_Cmds.end(5);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexData4ub( INT Register,BYTE a,BYTE b,BYTE c,BYTE d )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexData4ub" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexData4ub );
    {
        // todo: I'm not sure if BYTE is passed 32-bits or 8-bit
        // todo: I believe it's 32-bit for .NET
BREAK;
        g_Cmds.write( d );
        g_Cmds.write( c );
        g_Cmds.write( b );
        g_Cmds.write( a );
        g_Cmds.write( Register );
    }
    g_Cmds.end(5);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexDataColor( INT Register,D3DCOLOR Color )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexDataColor" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexDataColor );
    {
        g_Cmds.write( Color );
        g_Cmds.write( Register );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::Begin( D3DPRIMITIVETYPE PrimitiveType )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"Begin" );
#endif

    g_Cmds.begin( D3DDevice::Begin );
    {
        g_Cmds.write( PrimitiveType );
    }
    g_Cmds.end(1);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::End( void )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"End" );
#endif

    g_Cmds.begin( D3DDevice::End );
    g_Cmds.end(0);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::RunPushBuffer( D3DPushBuffer* pPushBuffer,D3DFixup *pFixup )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"RunPushBuffer" );
#endif

    g_Cmds.begin( D3DDevice::RunPushBuffer );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( *pPushBuffer );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write(       pFixup      );
        g_Cmds.write( Arg0, pPushBuffer );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::Nop( void )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"Nop" );
#endif

    g_Cmds.begin( D3DDevice::Nop );
    g_Cmds.end(0);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetModelView( CONST D3DMATRIX* pModelView,CONST D3DMATRIX* pInverseModelView,CONST D3DMATRIX* pComposite )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetModelView" );
#endif

    g_Cmds.begin( D3DDevice::SetModelView );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( *pModelView        );
        void* Arg1 = g_Cmds.write( *pInverseModelView );
        void* Arg2 = g_Cmds.write( *pComposite        );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg2, pComposite  );
        g_Cmds.write( Arg1, pInverseModelView );
        g_Cmds.write( Arg0, pModelView );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexBlendModelView( UINT Count,CONST D3DMATRIX* pModelViews,CONST D3DMATRIX* pInverseModelViews,CONST D3DMATRIX* pProjectionViewport )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexBlendModelView" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexBlendModelView );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( pModelViews        ,sizeof(D3DMATRIX)*Count );
        void* Arg1 = g_Cmds.write( pInverseModelViews ,sizeof(D3DMATRIX)*Count );
        void* Arg2 = g_Cmds.write( pProjectionViewport,sizeof(D3DMATRIX)*Count );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg2, pProjectionViewport );
        g_Cmds.write( Arg1, pInverseModelViews  );
        g_Cmds.write( Arg0, pModelViews         );
        g_Cmds.write(       Count               );
    }
    g_Cmds.end(4);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexShaderInput( DWORD Handle,UINT StreamCount,CONST D3DSTREAM_INPUT* pStreamInputs )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexShaderInput" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexShaderInput );
    {
        g_Cmds.write( pStreamInputs );
        g_Cmds.write( StreamCount );
        g_Cmds.write( Handle );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetVertexShaderInputDirect( D3DVERTEXATTRIBUTEFORMAT *pVAF,UINT StreamCount,CONST D3DSTREAM_INPUT* pStreamInputs )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetVertexShaderInputDirect" );
#endif

    g_Cmds.begin( D3DDevice::SetVertexShaderInputDirect );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( *pVAF );
        void* Arg1 = g_Cmds.write( *pStreamInputs );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg1, pStreamInputs );
        g_Cmds.write(       StreamCount   );
        g_Cmds.write( Arg0, pVAF          );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SwitchTexture( DWORD Stage,D3DBaseTexture *pTexture )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SwitchTexture" );
#endif

    g_Cmds.begin( D3DDevice::SwitchTexture );
    {
        g_Cmds.write( pTexture );
        g_Cmds.write( Stage    );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetScissors( DWORD Count,BOOL Exclusive,CONST D3DRECT *pRects )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetScissors" );
#endif

    g_Cmds.begin( D3DDevice::SetScissors );
    {
        // save all structures ................................................

        void* Arg0 = g_Cmds.write( pRects,sizeof(D3DRECT)*Count );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg0, pRects    );
        g_Cmds.write(       Exclusive );
        g_Cmds.write(       Count     );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetTile( DWORD Index,CONST D3DTILE* pTile )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetTile" );
#endif

    g_Cmds.begin( D3DDevice::SetTile );
    {
        // save all structures ................................................

        void* Arg0 = NULL; if( pTile ) Arg0 = g_Cmds.write( *pTile );

        // write arguments ....................................................
        //
        // Must be declared in reverse argument order
        // All additional structures must be saved
        // before the arguments.

        g_Cmds.write( Arg0, pTile );
        g_Cmds.write(       Index );
    }
    g_Cmds.end(2);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////

VOID WINAPI IDeferred3DDevice8::InsertCallback( D3DCALLBACKTYPE Type,D3DCALLBACK pCallback,DWORD Context )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"InsertCallback" );
#endif

    g_Cmds.begin( D3DDevice::InsertCallback );
    {
        g_Cmds.write( Context   );
        g_Cmds.write( pCallback );
        g_Cmds.write( Type      );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif
}

///////////////////////////////////////////////////////////////////////////

VOID WINAPI IDeferred3DDevice8::FlushVertexCache( void )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"FlushVertexCache" );
#endif

    g_Cmds.begin( D3DDevice::FlushVertexCache );
    g_Cmds.end(0);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::SetRenderTargetFast( D3DSurface *pRenderTarget,D3DSurface *pNewZStencil,DWORD Flags )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 134,32,60,255 ),"SetRenderTargetFast" );
#endif

    g_Cmds.begin( D3DDevice::SetRenderTarget );
    {
        g_Cmds.write( Flags         );
        g_Cmds.write( pNewZStencil  );
        g_Cmds.write( pRenderTarget );
    }
    g_Cmds.end(3);

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif
}



///////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF IDeferred3DDevice8: PASS THROUGH -- EVAPORATES DURING LINK
///////////////////////////////////////////////////////////////////////////



// Standard D3D APIs:

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetDirect3D( Direct3D **ppD3D8 )
{
    return g_pd3dInternal->GetDirect3D( ppD3D8 );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetDeviceCaps( D3DCAPS8 *pCaps )
{
    return g_pd3dInternal->GetDeviceCaps( pCaps );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetDisplayMode( D3DDISPLAYMODE *pMode )
{
    return g_pd3dInternal->GetDisplayMode( pMode );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetCreationParameters( D3DDEVICE_CREATION_PARAMETERS *pParameters )
{
    return g_pd3dInternal->GetCreationParameters( pParameters );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetBackBuffer( INT BackBuffer,D3DBACKBUFFER_TYPE UnusedType,D3DSurface **ppBackBuffer )
{
    return g_pd3dInternal->GetBackBuffer( BackBuffer,UnusedType,ppBackBuffer );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetRasterStatus( D3DRASTER_STATUS *pRasterStatus )
{
    return g_pd3dInternal->GetRasterStatus( pRasterStatus );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::SetGammaRamp( DWORD Flags,CONST D3DGAMMARAMP *pRamp )
{
    return g_pd3dInternal->SetGammaRamp( Flags,pRamp );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::GetGammaRamp( D3DGAMMARAMP *pRamp )
{
    return g_pd3dInternal->GetGammaRamp( pRamp );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreateTexture( UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL UnusedPool,D3DTexture **ppTexture )
{
    return g_pd3dInternal->CreateTexture( Width,Height,Levels,Usage,Format,UnusedPool,ppTexture );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreateVolumeTexture( UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL UnusedPool,D3DVolumeTexture **ppVolumeTexture )
{
    return g_pd3dInternal->CreateVolumeTexture( Width,Height,Depth,Levels,Usage,Format,UnusedPool,ppVolumeTexture );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreateCubeTexture( UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL UnusedPool,D3DCubeTexture **ppCubeTexture )
{
    return g_pd3dInternal->CreateCubeTexture( EdgeLength,Levels,Usage,Format,UnusedPool,ppCubeTexture );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreateVertexBuffer( UINT Length,DWORD UnusedUsage,DWORD UnusedFVF,D3DPOOL UnusedPool,D3DVertexBuffer **ppVertexBuffer )
{
    return g_pd3dInternal->CreateVertexBuffer( Length,UnusedUsage,UnusedFVF,UnusedPool,ppVertexBuffer );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreateIndexBuffer( UINT Length,DWORD UnusedUsage,D3DFORMAT UnusedFormat,D3DPOOL UnusedPool,D3DIndexBuffer **ppIndexBuffer )
{
    return g_pd3dInternal->CreateIndexBuffer( Length,UnusedUsage,UnusedFormat,UnusedPool,ppIndexBuffer );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreateRenderTarget( UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE UnusedMultiSample,BOOL UnusedLockable,D3DSurface **ppSurface )
{
    return g_pd3dInternal->CreateRenderTarget( Width,Height,Format,UnusedMultiSample,UnusedLockable,ppSurface );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreateDepthStencilSurface( UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE UnusedMultiSample,D3DSurface **ppSurface )
{
    return g_pd3dInternal->CreateDepthStencilSurface( Width,Height,Format,UnusedMultiSample,ppSurface );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreateImageSurface( UINT Width,UINT Height,D3DFORMAT Format,D3DSurface **ppSurface )
{
    return g_pd3dInternal->CreateImageSurface( Width,Height,Format,ppSurface );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CopyRects( D3DSurface *pSourceSurface,CONST RECT *pSourceRectsArray,UINT cRects,D3DSurface *pDestinationSurface,CONST POINT *pDestPointsArray )
{
    return g_pd3dInternal->CopyRects( pSourceSurface,pSourceRectsArray,cRects,pDestinationSurface,pDestPointsArray );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetDepthStencilSurface( D3DSurface **ppZStencilSurface )
{
    return g_pd3dInternal->GetDepthStencilSurface( ppZStencilSurface );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CaptureStateBlock( DWORD Token )
{
    return g_pd3dInternal->CaptureStateBlock( Token );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::DeleteStateBlock( DWORD Token )
{
    return g_pd3dInternal->DeleteStateBlock( Token );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreateStateBlock( D3DSTATEBLOCKTYPE Type,DWORD *pToken )
{
    return g_pd3dInternal->CreateStateBlock( Type,pToken );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreateVertexShader( CONST DWORD *pDeclaration,CONST DWORD *pFunction,DWORD *pHandle,DWORD Usage )
{
    return g_pd3dInternal->CreateVertexShader( pDeclaration,pFunction,pHandle,Usage );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::DeleteVertexShader( DWORD Handle )
{
    return g_pd3dInternal->DeleteVertexShader( Handle );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetVertexShaderDeclaration( DWORD Handle,void *pData,DWORD *pSizeOfData )
{
    return g_pd3dInternal->GetVertexShaderDeclaration( Handle,pData,pSizeOfData );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetVertexShaderFunction( DWORD Handle,void *pData,DWORD *pSizeOfData )
{
    return g_pd3dInternal->GetVertexShaderFunction( Handle,pData,pSizeOfData );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreatePixelShader( CONST D3DPIXELSHADERDEF *pPSDef,DWORD *pHandle )
{
    return g_pd3dInternal->CreatePixelShader( pPSDef,pHandle );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::DeletePixelShader( DWORD Handle )
{
    return g_pd3dInternal->DeletePixelShader( Handle );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetPixelShaderFunction( DWORD Handle,D3DPIXELSHADERDEF *pData )
{
    return g_pd3dInternal->GetPixelShaderFunction( Handle,pData );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::DeletePatch( UINT Handle )
{
    return g_pd3dInternal->DeletePatch( Handle );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetVertexShaderSize( DWORD Handle,UINT *pSize )
{
    return g_pd3dInternal->GetVertexShaderSize( Handle,pSize );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetVertexShaderType( DWORD Handle,DWORD *pType )
{
    return g_pd3dInternal->GetVertexShaderType( Handle,pType );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreatePalette( D3DPALETTESIZE Size,D3DPalette **ppPalette )
{
    return g_pd3dInternal->CreatePalette( Size,ppPalette );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::BeginVisibilityTest( void )
{
    return g_pd3dInternal->BeginVisibilityTest();
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::EndVisibilityTest( DWORD Index )
{
    return g_pd3dInternal->EndVisibilityTest( Index );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetVisibilityTestResult( DWORD Index,UINT* pResult,ULONGLONG* pTimeStamp )
{
    return g_pd3dInternal->GetVisibilityTestResult( Index,pResult,pTimeStamp );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreateFixup( UINT Size,D3DFixup **ppFixup )
{
    return g_pd3dInternal->CreateFixup( Size,ppFixup );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::CreatePushBuffer( UINT Size,BOOL RunUsingCpuCopy,D3DPushBuffer **ppPushBuffer )
{
    return g_pd3dInternal->CreatePushBuffer( Size,RunUsingCpuCopy,ppPushBuffer );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetVertexShaderInput( DWORD* pHandle,UINT* pStreamCount,D3DSTREAM_INPUT* pStreamInputs )
{
    return g_pd3dInternal->GetVertexShaderInput( pHandle,pStreamCount,pStreamInputs );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetTile( DWORD Index,D3DTILE* pTile )
{
    return g_pd3dInternal->GetTile( Index,pTile );
}

///////////////////////////////////////////////////////////////////////////

DWORD WINAPI IDeferred3DDevice8::GetTileCompressionTags( DWORD ZStartTag,DWORD ZEndTag )
{
    return g_pd3dInternal->GetTileCompressionTags( ZStartTag,ZEndTag );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::SetTileCompressionTagBits( DWORD Partition,DWORD Address,CONST DWORD *pData,DWORD Count )
{
    return g_pd3dInternal->SetTileCompressionTagBits( Partition,Address,pData,Count );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::GetTileCompressionTagBits( DWORD Partition,DWORD Address,DWORD *pData,DWORD Count )
{
    return g_pd3dInternal->GetTileCompressionTagBits( Partition,Address,pData,Count );
}

///////////////////////////////////////////////////////////////////////////

BOOL WINAPI IDeferred3DDevice8::IsBusy( void )
{
    return g_pd3dInternal->IsBusy();
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::SetVerticalBlankCallback( D3DVBLANKCALLBACK pCallback )
{
    return g_pd3dInternal->SetVerticalBlankCallback( pCallback );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::SetSwapCallback( D3DSWAPCALLBACK pCallback )
{
    return g_pd3dInternal->SetSwapCallback( pCallback );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::SetFlickerFilter( DWORD Filter )
{
    return g_pd3dInternal->SetFlickerFilter( Filter );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::SetSoftDisplayFilter( BOOL Enable )
{
    return g_pd3dInternal->SetSoftDisplayFilter( Enable );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetCopyRectsState( CONST D3DCOPYRECTSTATE *pCopyRectState,CONST D3DCOPYRECTROPSTATE *pCopyRectRopState )
{
    return g_pd3dInternal->SetCopyRectsState( pCopyRectState,pCopyRectRopState );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::PersistDisplay( void )
{
    return g_pd3dInternal->PersistDisplay();
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetPersistedSurface( IDirect3DSurface8 **ppSurface )
{
    return g_pd3dInternal->GetPersistedSurface( ppSurface );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetBackBufferScale( float x,float y )
{
    return g_pd3dInternal->SetBackBufferScale( x,y );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetBackBufferScale( float *pX,float *pY )
{
    return g_pd3dInternal->GetBackBufferScale( pX,pY );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetScreenSpaceOffset( float x,float y )
{
    return g_pd3dInternal->SetScreenSpaceOffset( x,y );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::GetScreenSpaceOffset( float *pX,float *pY )
{
    return g_pd3dInternal->GetScreenSpaceOffset( pX,pY );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::SetOverscanColor( D3DCOLOR Color )
{
    return g_pd3dInternal->SetOverscanColor( Color );
}

///////////////////////////////////////////////////////////////////////////

D3DCOLOR WINAPI IDeferred3DDevice8::GetOverscanColor( void )
{
    return g_pd3dInternal->GetOverscanColor();
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::SetDepthClipPlanes( float Near,float Far,DWORD Flags )
{
    return g_pd3dInternal->SetDepthClipPlanes( Near,Far,Flags );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::GetDepthClipPlanes( float *pNear,float *pFar,DWORD Flags )
{
    return g_pd3dInternal->GetDepthClipPlanes( pNear,pFar,Flags );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::GetViewportOffsetAndScale( D3DVECTOR4 *pOffset,D3DVECTOR4 *pScale )
{
    return g_pd3dInternal->GetViewportOffsetAndScale( pOffset,pScale );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::SetStipple( CONST DWORD *pPattern )
{
    return g_pd3dInternal->SetStipple( pPattern );
}

///////////////////////////////////////////////////////////////////////////

void WINAPI IDeferred3DDevice8::SetWaitCallback( D3DWAITCALLBACK pCallback )
{
    return g_pd3dInternal->SetWaitCallback( pCallback );
}

///////////////////////////////////////////////////////////////////////////

DWORD WINAPI IDeferred3DDevice8::GetPushDistance( DWORD Handle )
{
    return g_pd3dInternal->GetPushDistance( Handle );
}

///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI IDeferred3DDevice8::SetTimerCallback( ULONGLONG Time,D3DCALLBACK pCallback,DWORD Context )
{
    return g_pd3dInternal->SetTimerCallback( Time,pCallback,Context );
}

///////////////////////////////////////////////////////////////////////////

BOOL WINAPI IDeferred3DDevice8::IsFencePending( DWORD Fence )
{
    return g_pd3dInternal->IsFencePending( Fence );
}
