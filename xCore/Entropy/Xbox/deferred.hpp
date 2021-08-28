#ifndef _DEFERRED_HPP_
#define _DEFERRED_HPP_

#ifndef TARGET_XBOX
#   error This is not for this target platform. Check dependancy rules.
#endif



///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#ifdef bhapgood
//#define SWITCH_USE_DEFERRED_RENDERING 0
#endif

// Included this header only one time
#pragma once

#ifdef CONFIG_RETAIL
#   define D3DCOMPILE_PUREDEVICE 1
#endif

#ifndef X_RETAIL
#define D3DCOMPILE_NOTINLINE 1
#endif


#include <xtl.h>
#include <xonline.h>



///////////////////////////////////////////////////////////////////////////////
// SUBVERSIVE D3D INTERFACE: AHHH, THE PRETENSE
///////////////////////////////////////////////////////////////////////////////

class IDeferred3DDevice8
{
    ///////////////////////////////////////////////////////////////////////////
    // IMPLEMENTATION OF IDEFERRED3DDEVICE8: IUNKNOWN
    ///////////////////////////////////////////////////////////////////////////

public:

    static ULONG WINAPI AddRef ();
    static ULONG WINAPI Release();



    ///////////////////////////////////////////////////////////////////////////
    // IMPLEMENTATION OF IDeferred3DDevice8: DEFERRED FUNCTIONALITY
    // ------------------------------------------------------------------------
    // All of these member functions construct a stack image and the D3D method
    // to call. For example, Reset() would setup a packet that has a function *
    // to D3DDevice::Reset(), one parameter, and the D3DPRESENT_PARAMETERS obj.
    ///////////////////////////////////////////////////////////////////////////



    static HRESULT  WINAPI Reset                        (D3DPRESENT_PARAMETERS *pPresentationParameters);
    static HRESULT  WINAPI Present                      (CONST RECT *pSourceRect, CONST RECT *pDestRect, void *pUnused, void *pUnused2);
    static HRESULT  WINAPI SetRenderTarget              (D3DSurface *pRenderTarget, D3DSurface *pNewZStencil);
    static HRESULT  WINAPI BeginScene                   ();
    static HRESULT  WINAPI EndScene                     ();
    static HRESULT  WINAPI Clear                        (DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
    static HRESULT  WINAPI SetTransform                 (D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
    static HRESULT  WINAPI MultiplyTransform            (D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
    static HRESULT  WINAPI SetViewport                  (CONST D3DVIEWPORT8 *pViewport);
    static HRESULT  WINAPI SetMaterial                  (CONST D3DMATERIAL8 *pMaterial);
    static HRESULT  WINAPI SetLight                     (DWORD Index, CONST D3DLIGHT8 *pLight);
    static HRESULT  WINAPI LightEnable                  (DWORD Index, BOOL Enable);
    static HRESULT  WINAPI SetRenderState               (D3DRENDERSTATETYPE State, DWORD Value);
    static HRESULT  WINAPI ApplyStateBlock              (DWORD Token);
    static HRESULT  WINAPI CaptureStateBlock            (DWORD Token);
    static HRESULT  WINAPI DeleteStateBlock             (DWORD Token);
    static HRESULT  WINAPI CreateStateBlock             (D3DSTATEBLOCKTYPE Type,DWORD *pToken);
    static HRESULT  WINAPI SetTexture                   (DWORD Stage, D3DBaseTexture *pTexture);
    static HRESULT  WINAPI SetTextureStageState         (DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
    static HRESULT  WINAPI DrawPrimitive                (D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
    static HRESULT  WINAPI DrawIndexedPrimitive         (D3DPRIMITIVETYPE, UINT UnusedMinIndex, UINT UnusedNumIndices, UINT StartIndex, UINT PrimitiveCount);
    static HRESULT  WINAPI DrawPrimitiveUP              (D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
    static HRESULT  WINAPI DrawIndexedPrimitiveUP       (D3DPRIMITIVETYPE PrimitiveType, UINT UnusedMinIndex, UINT UnusedNumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT UnusedIndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
    static HRESULT  WINAPI SetVertexShader              (DWORD Handle);
    static HRESULT  WINAPI SetVertexShaderConstant      (INT Register, CONST void *pConstantData, DWORD ConstantCount);
    static HRESULT  WINAPI SetVertexShaderConstantFast  (INT Register, CONST void *pConstantData, DWORD ConstantCount);
    static HRESULT  WINAPI SetStreamSource              (UINT StreamNumber, D3DVertexBuffer *pStreamData, UINT Stride);
    static HRESULT  WINAPI SetIndices                   (D3DIndexBuffer *pIndexData, UINT BaseVertexIndex);
    static HRESULT  WINAPI SetPixelShader               (DWORD Handle);
    static HRESULT  WINAPI SetPixelShaderProgram        (CONST D3DPIXELSHADERDEF *pPSDef);
    static HRESULT  WINAPI SetPixelShaderConstant       (DWORD Register, CONST void *pConstantData, DWORD ConstantCount);
    static HRESULT  WINAPI SetShaderConstantMode        (D3DSHADERCONSTANTMODE Mode);
    static HRESULT  WINAPI LoadVertexShader             (DWORD Handle, DWORD Address);
    static HRESULT  WINAPI LoadVertexShaderProgram      (CONST DWORD *pFunction, DWORD Address);
    static HRESULT  WINAPI SelectVertexShader           (DWORD Handle, DWORD Address);
    static HRESULT  WINAPI SelectVertexShaderDirect     (D3DVERTEXATTRIBUTEFORMAT *pVAF, DWORD Address);
    static HRESULT  WINAPI GetVertexShaderSize          (DWORD Handle, UINT *pSize);
    static HRESULT  WINAPI GetVertexShaderType          (DWORD Handle, DWORD *pType);
    static HRESULT  WINAPI DrawVertices                 (D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT VertexCount);
    static HRESULT  WINAPI DrawIndexedVertices          (D3DPRIMITIVETYPE, UINT VertexCount, CONST WORD *pIndexData);
    static HRESULT  WINAPI DrawVerticesUP               (D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
    static HRESULT  WINAPI PrimeVertexCache             (UINT VertexCount, CONST WORD *pIndexData);
    static HRESULT  WINAPI SetPalette                   (DWORD Stage, D3DPalette *pPalette);
    static HRESULT  WINAPI SetTextureStageStateNotInline(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
    static HRESULT  WINAPI SetRenderStateNotInline      (D3DRENDERSTATETYPE State, DWORD Value);
    static HRESULT  WINAPI SetBackMaterial              (CONST D3DMATERIAL8 *pMaterial);
    static HRESULT  WINAPI UpdateOverlay                (D3DSurface *pSurface, CONST RECT *SrcRect, CONST RECT *DstRect, BOOL EnableColorKey, D3DCOLOR ColorKey);
    static HRESULT  WINAPI EnableOverlay                (BOOL Enable);
    static HRESULT  WINAPI SetVertexData2f              (INT Register, float a, float b);
    static HRESULT  WINAPI SetVertexData4f              (INT Register, float a, float b, float c, float d);
    static HRESULT  WINAPI SetVertexData2s              (INT Register, SHORT a, SHORT b);
    static HRESULT  WINAPI SetVertexData4s              (INT Register, SHORT a, SHORT b, SHORT c, SHORT d);
    static HRESULT  WINAPI SetVertexData4ub             (INT Register, BYTE a, BYTE b, BYTE c, BYTE d);
    static HRESULT  WINAPI SetVertexDataColor           (INT Register, D3DCOLOR Color);
    static HRESULT  WINAPI Begin                        (D3DPRIMITIVETYPE PrimitiveType);
    static HRESULT  WINAPI End                          ();
    static HRESULT  WINAPI RunPushBuffer                (D3DPushBuffer* pPushBuffer, D3DFixup *pFixup);
    static HRESULT  WINAPI Nop                          ();
    static HRESULT  WINAPI SetModelView                 (CONST D3DMATRIX* pModelView, CONST D3DMATRIX* pInverseModelView, CONST D3DMATRIX* pComposite);
    static HRESULT  WINAPI SetVertexBlendModelView      (UINT Count, CONST D3DMATRIX* pModelViews, CONST D3DMATRIX* pInverseModelViews, CONST D3DMATRIX* pProjectionViewport);
    static HRESULT  WINAPI SetVertexShaderInput         (DWORD Handle, UINT StreamCount, CONST D3DSTREAM_INPUT* pStreamInputs);
    static HRESULT  WINAPI SetVertexShaderInputDirect   (D3DVERTEXATTRIBUTEFORMAT *pVAF, UINT StreamCount, CONST D3DSTREAM_INPUT* pStreamInputs);
    static HRESULT  WINAPI GetVertexShaderInput         (DWORD* pHandle, UINT* pStreamCount, D3DSTREAM_INPUT* pStreamInputs);
    static HRESULT  WINAPI SwitchTexture                (DWORD Stage, D3DBaseTexture *pTexture);
    static HRESULT  WINAPI SetScissors                  (DWORD Count, BOOL Exclusive, CONST D3DRECT *pRects);
    static VOID     WINAPI InsertCallback               (D3DCALLBACKTYPE Type, D3DCALLBACK pCallback, DWORD Context);
    static void     WINAPI SetRenderTargetFast          (D3DSurface *pRenderTarget, D3DSurface *pNewZStencil, DWORD Flags);



    ///////////////////////////////////////////////////////////////////////////
    // IMPLEMENTATION OF IDeferred3DDevice8: PASS THRU - EVAPORATES DURING LINK
    ///////////////////////////////////////////////////////////////////////////



    static HRESULT  WINAPI GetDirect3D                  (Direct3D **ppD3D8);
    static HRESULT  WINAPI GetDeviceCaps                (D3DCAPS8 *pCaps);
    static HRESULT  WINAPI GetDisplayMode               (D3DDISPLAYMODE *pMode);
    static HRESULT  WINAPI GetCreationParameters        (D3DDEVICE_CREATION_PARAMETERS *pParameters);
    static HRESULT  WINAPI GetBackBuffer                (INT BackBuffer, D3DBACKBUFFER_TYPE UnusedType, D3DSurface **ppBackBuffer);
    static HRESULT  WINAPI GetRasterStatus              (D3DRASTER_STATUS *pRasterStatus);
    static void     WINAPI SetGammaRamp                 (DWORD Flags, CONST D3DGAMMARAMP *pRamp);
    static void     WINAPI GetGammaRamp                 (D3DGAMMARAMP *pRamp);
    static HRESULT  WINAPI CreateTexture                (UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DTexture **ppTexture);
    static HRESULT  WINAPI CreateVolumeTexture          (UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DVolumeTexture **ppVolumeTexture);
    static HRESULT  WINAPI CreateCubeTexture            (UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DCubeTexture **ppCubeTexture);
    static HRESULT  WINAPI CreateVertexBuffer           (UINT Length, DWORD UnusedUsage, DWORD UnusedFVF, D3DPOOL UnusedPool, D3DVertexBuffer **ppVertexBuffer);
    static HRESULT  WINAPI CreateIndexBuffer            (UINT Length, DWORD UnusedUsage, D3DFORMAT UnusedFormat, D3DPOOL UnusedPool, D3DIndexBuffer **ppIndexBuffer);
    static HRESULT  WINAPI CreateRenderTarget           (UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE UnusedMultiSample, BOOL UnusedLockable, D3DSurface **ppSurface);
    static HRESULT  WINAPI CreateDepthStencilSurface    (UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE UnusedMultiSample, D3DSurface **ppSurface);
    static HRESULT  WINAPI CreateImageSurface           (UINT Width, UINT Height, D3DFORMAT Format, D3DSurface **ppSurface);
    static HRESULT  WINAPI CopyRects                    (D3DSurface *pSourceSurface, CONST RECT *pSourceRectsArray, UINT cRects, D3DSurface *pDestinationSurface, CONST POINT *pDestPointsArray);
    static HRESULT  WINAPI GetDepthStencilSurface       (D3DSurface **ppZStencilSurface);
    static HRESULT  WINAPI CreateVertexShader           (CONST DWORD *pDeclaration, CONST DWORD *pFunction, DWORD *pHandle, DWORD Usage);
    static HRESULT  WINAPI DeleteVertexShader           (DWORD Handle);
    static HRESULT  WINAPI GetVertexShaderFunction      (DWORD Handle,void *pData,DWORD *pSizeOfData);
    static HRESULT  WINAPI GetVertexShaderDeclaration   (DWORD Handle,void *pData,DWORD *pSizeOfData);
    static HRESULT  WINAPI CreatePixelShader            (CONST D3DPIXELSHADERDEF *pPSDef, DWORD *pHandle);
    static HRESULT  WINAPI DeletePixelShader            (DWORD Handle);
    static HRESULT  WINAPI DeletePatch                  (UINT Handle);

    // The following APIs are all Xbox extensions:

    static HRESULT  WINAPI CreatePalette                (D3DPALETTESIZE Size, D3DPalette **ppPalette);
    static HRESULT  WINAPI BeginVisibilityTest          ();
    static HRESULT  WINAPI GetVisibilityTestResult      (DWORD Index,UINT* pResult,ULONGLONG* pTimeStamp);
    static HRESULT  WINAPI EndVisibilityTest            (DWORD Index);
    static HRESULT  WINAPI CreateFixup                  (UINT Size, D3DFixup **ppFixup);
    static HRESULT  WINAPI CreatePushBuffer             (UINT Size, BOOL RunUsingCpuCopy, D3DPushBuffer **ppPushBuffer);
    static HRESULT  WINAPI SetTile                      (DWORD Index, CONST D3DTILE* pTile);
    static HRESULT  WINAPI GetPixelShaderFunction       (DWORD Handle,D3DPIXELSHADERDEF *pData);
    static HRESULT  WINAPI GetTile                      (DWORD Index, D3DTILE* pTile);
    static DWORD    WINAPI GetTileCompressionTags       (DWORD ZStartTag, DWORD ZEndTag);
    static void     WINAPI SetTileCompressionTagBits    (DWORD Partition, DWORD Address, CONST DWORD *pData, DWORD Count);
    static void     WINAPI GetTileCompressionTagBits    (DWORD Partition, DWORD Address, DWORD *pData, DWORD Count);
    static BOOL     WINAPI IsBusy                       ();
    static void     WINAPI SetVerticalBlankCallback     (D3DVBLANKCALLBACK pCallback);
    static void     WINAPI SetSwapCallback              (D3DSWAPCALLBACK pCallback);
    static BOOL     WINAPI IsFencePending               (DWORD Fence);
    static VOID     WINAPI FlushVertexCache             ();
    static void     WINAPI SetFlickerFilter             (DWORD Filter);
    static void     WINAPI SetSoftDisplayFilter         (BOOL Enable);
    static HRESULT  WINAPI SetCopyRectsState            (CONST D3DCOPYRECTSTATE *pCopyRectState, CONST D3DCOPYRECTROPSTATE *pCopyRectRopState);
    static HRESULT  WINAPI PersistDisplay               ();
    static HRESULT  WINAPI GetPersistedSurface          (IDirect3DSurface8 **ppSurface);
    static DWORD    WINAPI Swap                         (DWORD Flags);
    static HRESULT  WINAPI SetBackBufferScale           (float x, float y);
    static HRESULT  WINAPI GetBackBufferScale           (float *pX, float *pY);
    static HRESULT  WINAPI SetScreenSpaceOffset         (float x, float Y);
    static HRESULT  WINAPI GetScreenSpaceOffset         (float *pX, float *pY);
    static void     WINAPI SetOverscanColor             (D3DCOLOR Color);
    static D3DCOLOR WINAPI GetOverscanColor             ();
    static void     WINAPI SetDepthClipPlanes           (float Near, float Far, DWORD Flags);
    static void     WINAPI GetDepthClipPlanes           (float *pNear, float *pFar, DWORD Flags);
    static void     WINAPI GetViewportOffsetAndScale    (D3DVECTOR4 *pOffset, D3DVECTOR4 *pScale);
    static void     WINAPI SetStipple                   (CONST DWORD *pPattern);
    static void     WINAPI SetWaitCallback              (D3DWAITCALLBACK pCallback);
    static DWORD    WINAPI GetPushDistance              (DWORD Handle);
    static HRESULT  WINAPI SetTimerCallback             (ULONGLONG Time, D3DCALLBACK pCallback, DWORD Context);
};

#endif
