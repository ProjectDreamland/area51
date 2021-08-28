/*==========================================================================;
 *
 *  Copyright (C) Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       d3d8.h
 *  Content:    Xbox Direct3D include file
 *
 ****************************************************************************/

#ifndef _D3D8_H_
#define _D3D8_H_

#pragma warning( push )
#pragma warning( disable : 4100 ) // unreferenced formal parameter

#ifndef DIRECT3D_VERSION
#define DIRECT3D_VERSION         0x0800
#endif  //DIRECT3D_VERSION

#ifndef D3DINLINE
#define D3DINLINE static __forceinline
#endif

#ifndef D3DMINLINE
#define D3DMINLINE __forceinline
#endif

#define D3DFASTCALL __fastcall

#ifndef DECLSPEC_SELECTANY
#define DECLSPEC_SELECTANY __declspec(selectany)
#endif

#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#ifdef __cplusplus
#define REFGUID const GUID &
#else
#define REFGUID const GUID * const
#endif
#endif

#define D3D_SDK_VERSION 0

#include <stdlib.h>

typedef struct Direct3D                  Direct3D;
typedef struct D3DDevice                 D3DDevice;
typedef struct D3DResource               D3DResource;
typedef struct D3DBaseTexture            D3DBaseTexture;
typedef struct D3DTexture                D3DTexture;
typedef struct D3DVolumeTexture          D3DVolumeTexture;
typedef struct D3DCubeTexture            D3DCubeTexture;
typedef struct D3DVertexBuffer           D3DVertexBuffer;
typedef struct D3DIndexBuffer            D3DIndexBuffer;
typedef struct D3DPalette                D3DPalette;
typedef struct D3DSurface                D3DSurface;
typedef struct D3DVolume                 D3DVolume;
typedef struct D3DPushBuffer             D3DPushBuffer;
typedef struct D3DFixup                  D3DFixup;

// Compatibility typedefs.

#define IDirect3D8                       Direct3D
#define IDirect3DDevice8                 D3DDevice
#define IDirect3DResource8               D3DResource
#define IDirect3DBaseTexture8            D3DBaseTexture
#define IDirect3DTexture8                D3DTexture
#define IDirect3DVolumeTexture8          D3DVolumeTexture
#define IDirect3DCubeTexture8            D3DCubeTexture
#define IDirect3DVertexBuffer8           D3DVertexBuffer
#define IDirect3DIndexBuffer8            D3DIndexBuffer
#define IDirect3DPalette8                D3DPalette
#define IDirect3DSurface8                D3DSurface
#define IDirect3DVolume8                 D3DVolume
#define IDirect3DPushBuffer8             D3DPushBuffer
#define IDirect3DFixup8                  D3DFixup

// Pointer typedefs.

typedef struct Direct3D                  *LPDIRECT3D8,              *PDIRECT3D8;
typedef struct D3DDevice                 *LPDIRECT3DDEVICE8,        *PDIRECT3DDEVICE8;
typedef struct D3DResource               *LPDIRECT3DRESOURCE8,      *PDIRECT3DRESOURCE8;
typedef struct D3DBaseTexture            *LPDIRECT3DBASETEXTURE8,   *PDIRECT3DBASETEXTURE8;
typedef struct D3DTexture                *LPDIRECT3DTEXTURE8,       *PDIRECT3DTEXTURE8;
typedef struct D3DVolumeTexture          *LPDIRECT3DVOLUMETEXTURE8, *PDIRECT3DVOLUMETEXTURE8;
typedef struct D3DCubeTexture            *LPDIRECT3DCUBETEXTURE8,   *PDIRECT3DCUBETEXTURE8;
typedef struct D3DVertexBuffer           *LPDIRECT3DVERTEXBUFFER8,  *PDIRECT3DVERTEXBUFFER8;
typedef struct D3DIndexBuffer            *LPDIRECT3DINDEXBUFFER8,   *PDIRECT3DINDEXBUFFER8;
typedef struct D3DPalette                *LPDIRECT3DPALETTE8,       *PDIRECT3DPALETTE8;
typedef struct D3DSurface                *LPDIRECT3DSURFACE8,       *PDIRECT3DSURFACE8;
typedef struct D3DVolume                 *LPDIRECT3DVOLUME8,        *PDIRECT3DVOLUME8;
typedef struct D3DPushBuffer             *LPDIRECT3DPUSHBUFFER8,    *PDIRECT3DPUSHBUFFER8;
typedef struct D3DFixup                  *LPDIRECT3DFIXUP8,         *PDIRECT3DFIXUP8;

#include "d3d8types.h"
#include "d3d8caps.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
 * The XBOX implementation of Direct3DCreate8 exists only for backwards
 * compatibility.
 *
 * There is no need to store the result of this function call.  You can
 * pass 'NULL' for the Direct3D 'this' pointer as it is completely
 * ignored.
 */

Direct3D * WINAPI Direct3DCreate8(UINT SDKVersion);

/****************************************************************************
 *
 * Parameter for Direct3D Enum and GetCaps8 functions to get the info for
 * the current mode only.
 *
 ****************************************************************************/

#define D3DCURRENT_DISPLAY_MODE                 0x00EFFFFFL

/****************************************************************************
 *
 * Flags for Direct3D::CreateDevice's BehaviorFlags
 *
 ****************************************************************************/

#define D3DCREATE_PUREDEVICE                    0x00000010L
#define D3DCREATE_SOFTWARE_VERTEXPROCESSING     0x00000020L
#define D3DCREATE_HARDWARE_VERTEXPROCESSING     0x00000040L
#define D3DCREATE_MIXED_VERTEXPROCESSING        0x00000080L

// D3DCREATE_FPU_PRESERVE is not supported on Xbox
// D3DCREATE_MULTITHREADED is not supported on Xbox

/****************************************************************************
 *
 * Parameter for Direct3D::CreateDevice's Adapter
 *
 ****************************************************************************/

#define D3DADAPTER_DEFAULT                     0

/****************************************************************************
 *
 * Flags for Direct3D::EnumAdapters
 *
 ****************************************************************************/

#define D3DENUM_NO_WHQL_LEVEL                   0x00000002L

/****************************************************************************
 *
 * Maximum number of back-buffers supported in DX8
 *
 ****************************************************************************/

#define D3DPRESENT_BACK_BUFFERS_MAX             2L

/****************************************************************************
 *
 * Flags for D3DDevice::SetGammaRamp
 *
 ****************************************************************************/

#define D3DSGR_NO_CALIBRATION                  0x00000000L
#define D3DSGR_CALIBRATE                       0x00000001L
#define D3DSGR_IMMEDIATE                       0x00000002L

/****************************************************************************
 * Flags for SetPrivateData method on all D3D8 interfaces
 *
 * The passed pointer is an IUnknown ptr. The SizeOfData argument to SetPrivateData
 * must be set to sizeof(IUnknown*). Direct3D will call AddRef through this
 * pointer and Release when the private data is destroyed. The data will be
 * destroyed when another SetPrivateData with the same GUID is set, when
 * FreePrivateData is called, or when the D3D8 object is freed.
 ****************************************************************************/

#define D3DSPD_IUNKNOWN                         0x00000001L

/*
 *  DirectDraw error codes
 */

#define _FACD3D  0x876
#define MAKE_D3DHRESULT( code )  MAKE_HRESULT( 1, _FACD3D, code )

/*
 * Direct3D Errors
 */

#define D3D_OK                              S_OK

#define D3DERR_WRONGTEXTUREFORMAT               MAKE_D3DHRESULT(2072)
#define D3DERR_UNSUPPORTEDCOLOROPERATION        MAKE_D3DHRESULT(2073)
#define D3DERR_UNSUPPORTEDCOLORARG              MAKE_D3DHRESULT(2074)
#define D3DERR_UNSUPPORTEDALPHAOPERATION        MAKE_D3DHRESULT(2075)
#define D3DERR_UNSUPPORTEDALPHAARG              MAKE_D3DHRESULT(2076)
#define D3DERR_TOOMANYOPERATIONS                MAKE_D3DHRESULT(2077)
#define D3DERR_CONFLICTINGTEXTUREFILTER         MAKE_D3DHRESULT(2078)
#define D3DERR_UNSUPPORTEDFACTORVALUE           MAKE_D3DHRESULT(2079)
#define D3DERR_CONFLICTINGRENDERSTATE           MAKE_D3DHRESULT(2081)
#define D3DERR_UNSUPPORTEDTEXTUREFILTER         MAKE_D3DHRESULT(2082)
#define D3DERR_CONFLICTINGTEXTUREPALETTE        MAKE_D3DHRESULT(2086)
#define D3DERR_DRIVERINTERNALERROR              MAKE_D3DHRESULT(2087)
#define D3DERR_TESTINCOMPLETE                   MAKE_D3DHRESULT(2088) // Xbox extension
#define D3DERR_BUFFERTOOSMALL                   MAKE_D3DHRESULT(2089) // Xbox extension
#define D3DERR_TIMEEXPIRED                      MAKE_D3DHRESULT(2090) // Xbox extension

#define D3DERR_NOTFOUND                         MAKE_D3DHRESULT(2150)
#define D3DERR_MOREDATA                         MAKE_D3DHRESULT(2151)
#define D3DERR_DEVICELOST                       MAKE_D3DHRESULT(2152)
#define D3DERR_DEVICENOTRESET                   MAKE_D3DHRESULT(2153)
#define D3DERR_NOTAVAILABLE                     MAKE_D3DHRESULT(2154)
#define D3DERR_OUTOFVIDEOMEMORY                 MAKE_D3DHRESULT(380)
#define D3DERR_INVALIDDEVICE                    MAKE_D3DHRESULT(2155)
#define D3DERR_INVALIDCALL                      MAKE_D3DHRESULT(2156)

/****************************************************************************
 *
 * BeginStateBlock/EndStateBlock semantics require that D3D have the ability
 * to record all API state-changing calls in a state block.  Rather than add
 * a time-consuming are-we-recording-a-state-block check to every SetRenderState
 * and SetTextureStageState call that would slow callers even if they never
 * used BeginStateBlock/EndStateBlock, we've placed that logic inline so that
 * it can be eliminated if not needed by the title.
 *
 * To enable BeginStateBlock/EndStateBlock, define the following before
 * including xtl.h.  This needs to be done for all modules that can change
 * state that should be recorded by BeginStateBlock.
 *
 *      #define D3DCOMPILE_BEGINSTATEBLOCK 1
 *
 ****************************************************************************/

typedef enum _D3DSTATEBLOCKDIRTYINDEX
{
    D3DSBD_TEXTURES              = 0,
    D3DSBD_PIXELSHADER           = D3DSBD_TEXTURES + D3DTSS_MAXSTAGES,
    D3DSBD_VERTEXSHADER          = D3DSBD_PIXELSHADER + 1,
    D3DSBD_INDICES               = D3DSBD_VERTEXSHADER + 1,
    D3DSBD_STREAMS               = D3DSBD_INDICES + 1,
    D3DSBD_PIXELSHADERCONSTANTS  = D3DSBD_STREAMS + D3DVS_STREAMS_MAX_V1_0,
    D3DSBD_VERTEXSHADERCONSTANTS = D3DSBD_PIXELSHADERCONSTANTS + D3DPS_CONSTREG_MAX_DX8,
    D3DSBD_RENDERSTATES          = D3DSBD_VERTEXSHADERCONSTANTS + D3DVS_CONSTREG_COUNT_XBOX,
    D3DSBD_TEXTURESTATES         = D3DSBD_RENDERSTATES + D3DRS_MAX,
    D3DSBD_TRANSFORMS            = D3DSBD_TEXTURESTATES + (D3DTSS_MAXSTAGES * D3DTSS_MAX),
    D3DSBD_VIEWPORT              = D3DSBD_TRANSFORMS + D3DTS_MAX,
    D3DSBD_MATERIAL              = D3DSBD_VIEWPORT + 1,
    D3DSBD_BACKMATERIAL          = D3DSBD_MATERIAL + 1,
    D3DSBD_MAX                   = D3DSBD_BACKMATERIAL + 1,
    D3DSBD_FORCE_DWORD           = 0x7fffffff, // force 32-bit size enum
} D3DSTATEBLOCKDIRTYINDEX;

#ifdef D3DCOMPILE_BEGINSTATEBLOCK

    #define D3DDIRTY_TEXTURE(stage)                                         \
        { D3D__StateBlockDirty[D3DSBD_TEXTURES + (stage)] = TRUE; }

    #define D3DDIRTY_PIXELSHADER()                                          \
        { D3D__StateBlockDirty[D3DSBD_PIXELSHADER] = TRUE; }

    #define D3DDIRTY_VERTEXSHADER()                                         \
        { D3D__StateBlockDirty[D3DSBD_VERTEXSHADER] = TRUE; }

    #define D3DDIRTY_INDICES()                                              \
        { D3D__StateBlockDirty[D3DSBD_INDICES] = TRUE; }

    #define D3DDIRTY_STREAM(stream)                                         \
        { D3D__StateBlockDirty[D3DSBD_STREAMS + (stream)] = TRUE; }

    #define D3DDIRTY_PIXELSHADERCONSTANT(index, count)                      \
        { memset(&D3D__StateBlockDirty[D3DSBD_PIXELSHADERCONSTANTS + (index)], TRUE, (count)); }

    #define D3DDIRTY_VERTEXSHADERCONSTANT(index, count)                     \
        { memset(&D3D__StateBlockDirty[D3DSBD_VERTEXSHADERCONSTANTS + (index) + 96], TRUE, (count)); }

    #define D3DDIRTY_RENDERSTATE(state)                                     \
        { D3D__StateBlockDirty[D3DSBD_RENDERSTATES + (state)] = TRUE; }

    #define D3DDIRTY_TEXTURESTATE(stage, state)                             \
        { D3D__StateBlockDirty[D3DSBD_TEXTURESTATES +                       \
                          ((state) * D3DTSS_MAXSTAGES) + (stage)] = TRUE; }

    #define D3DDIRTY_TRANSFORM(transform)                                   \
        { D3D__StateBlockDirty[D3DSBD_TRANSFORMS + (transform)] = TRUE; }

    #define D3DDIRTY_VIEWPORT()                                             \
        { D3D__StateBlockDirty[D3DSBD_VIEWPORT] = TRUE; }

    #define D3DDIRTY_MATERIAL()                                             \
        { D3D__StateBlockDirty[D3DSBD_MATERIAL] = TRUE; }

    #define D3DDIRTY_BACKMATERIAL()                                         \
        { D3D__StateBlockDirty[D3DSBD_BACKMATERIAL] = TRUE; }

#else

    #define D3DDIRTY_TEXTURE(stage)
    #define D3DDIRTY_PIXELSHADER()
    #define D3DDIRTY_VERTEXSHADER()
    #define D3DDIRTY_INDICES()
    #define D3DDIRTY_STREAM(stream)
    #define D3DDIRTY_PIXELSHADERCONSTANT(index, count)
    #define D3DDIRTY_VERTEXSHADERCONSTANT(index, count)
    #define D3DDIRTY_RENDERSTATE(state)
    #define D3DDIRTY_TEXTURESTATE(stage, state)
    #define D3DDIRTY_TRANSFORM(transform)
    #define D3DDIRTY_VIEWPORT()
    #define D3DDIRTY_MATERIAL()
    #define D3DDIRTY_BACKMATERIAL()

#endif

/****************************************************************************
 *
 * __declspec(selectany) has the lovely attribute that it allows the linker
 * to remove duplicate instantiations of global declarations, and to remove
 * the instantiation entirely if unreferenced.
 *
 ****************************************************************************/

#define D3DCONST extern CONST DECLSPEC_SELECTANY

/****************************************************************************
 *
 * Private internal data - Please don't access these directly, as they're
 *                         subject to change.
 *
 ****************************************************************************/

D3DCONST UINT D3DPRIMITIVETOVERTEXCOUNT[11][2] =
{
    {0, 0},         // Illegal
    {1, 0},         // D3DPT_POINTLIST     = 1,
    {2, 0},         // D3DPT_LINELIST      = 2,
    {1, 1},         // D3DPT_LINELOOP      = 3,
    {1, 1},         // D3DPT_LINESTRIP     = 4,
    {3, 0},         // D3DPT_TRIANGLELIST  = 5,
    {1, 2},         // D3DPT_TRIANGLESTRIP = 6,
    {1, 2},         // D3DPT_TRIANGLEFAN   = 7,
    {4, 0},         // D3DPT_QUADLIST      = 8,
    {2, 2},         // D3DPT_QUADSTRIP     = 9,
    {0, 0},         // Illegal (D3DPT_POLYGON)
};

D3DCONST DWORD D3DSIMPLERENDERSTATEENCODE[] =
{
    0x040260,    0x040264,    0x040268,    0x04026c,    // 0
    0x040270,    0x040274,    0x040278,    0x04027c,    // 4
    0x040288,    0x04028c,    0x040a60,    0x040a64,    // 8
    0x040a68,    0x040a6c,    0x040a70,    0x040a74,    // 12
    0x040a78,    0x040a7c,    0x040a80,    0x040a84,    // 16
    0x040a88,    0x040a8c,    0x040a90,    0x040a94,    // 20
    0x040a98,    0x040a9c,    0x040aa0,    0x040aa4,    // 24
    0x040aa8,    0x040aac,    0x040ab0,    0x040ab4,    // 28
    0x040ab8,    0x040abc,    0x040ac0,    0x040ac4,    // 32
    0x040ac8,    0x040acc,    0x040ad0,    0x040ad4,    // 36
    0x040ad8,    0x040adc,    0x0417f8,    0x041e20,    // 40
    0x041e24,    0x041e40,    0x041e44,    0x041e48,    // 44
    0x041e4c,    0x041e50,    0x041e54,    0x041e58,    // 48
    0x041e5c,    0x041e60,    0x041d90,    0x041e74,    // 52
    0x041e78,    0x040354,    0x04033c,    0x040304,    // 56
    0x040300,    0x040340,    0x040344,    0x040348,    // 60
    0x04035c,    0x040310,    0x04037c,    0x040358,    // 64
    0x040374,    0x040378,    0x040364,    0x040368,    // 68
    0x04036c,    0x040360,    0x040350,    0x04034c,    // 72
    0x0409f8,    0x040384,    0x040388,    0x040330,    // 76
    0x040334,    0x040338,    0x041d78,    0x04147c,    // 80
    0x041d90,    0x041d90,    0x041d90,    0x041d90,    // 84
    0x041d90,    0x041d90,    0x041d90,    0x041d90,    // 88
};

D3DCONST DWORD D3DTEXTUREDIRECTENCODE[] =
{
    0x081b00,    0x081b40,    0x081b80,    0x081bc0,
};

#define D3DDIRTYFLAG_TEXTURE_STATE                0x0000000f
#define D3DDIRTYFLAG_TEXTURE_STATE_0              0x00000001
#define D3DDIRTYFLAG_TEXTURE_STATE_1              0x00000002
#define D3DDIRTYFLAG_TEXTURE_STATE_2              0x00000004
#define D3DDIRTYFLAG_TEXTURE_STATE_3              0x00000008
#define D3DDIRTYFLAG_VERTEXFORMAT                 0x00000070
#define D3DDIRTYFLAG_VERTEXFORMAT_VB              0x00000010
#define D3DDIRTYFLAG_VERTEXFORMAT_UP              0x00000020
#define D3DDIRTYFLAG_VERTEXFORMAT_OFFSETS         0x00000040
#define D3DDIRTYFLAG_POINTPARAMS                  0x00000100
#define D3DDIRTYFLAG_TRANSFORM                    0x00000200
#define D3DDIRTYFLAG_TEXTURE_TRANSFORM            0x00000400
#define D3DDIRTYFLAG_COMBINERS                    0x00000800
#define D3DDIRTYFLAG_LIGHTS                       0x00001000
#define D3DDIRTYFLAG_SPECFOG_COMBINER             0x00002000
#define D3DDIRTYFLAG_SHADER_STAGE_PROGRAM         0x00004000
#define D3DDIRTYFLAG_LIGHT_MATERIAL               0x00008000
#define D3DDIRTYFLAG_LIGHT_SLOTS                  0x00ff0000
#define D3DDIRTYFLAG_LIGHT_SLOT_0                 0x00010000
#define D3DDIRTYFLAG_LIGHT_SLOT_1                 0x00020000
#define D3DDIRTYFLAG_LIGHT_SLOT_2                 0x00040000
#define D3DDIRTYFLAG_LIGHT_SLOT_3                 0x00080000
#define D3DDIRTYFLAG_LIGHT_SLOT_4                 0x00100000
#define D3DDIRTYFLAG_LIGHT_SLOT_5                 0x00200000
#define D3DDIRTYFLAG_LIGHT_SLOT_6                 0x00400000
#define D3DDIRTYFLAG_LIGHT_SLOT_7                 0x00800000

D3DCONST DWORD D3DDIRTYFROMRENDERSTATE[] =
{
    D3DDIRTYFLAG_SPECFOG_COMBINER,                      // D3DRS_FOGENABLE
    D3DDIRTYFLAG_SPECFOG_COMBINER,                      // D3DRS_FOGTABLEMODE
    D3DDIRTYFLAG_SPECFOG_COMBINER,                      // D3DRS_FOGSTART
    D3DDIRTYFLAG_SPECFOG_COMBINER,                      // D3DRS_FOGEND
    D3DDIRTYFLAG_SPECFOG_COMBINER,                      // D3DRS_FOGDENSITY
    D3DDIRTYFLAG_SPECFOG_COMBINER,                      // D3DRS_RANGEFOGENABLE
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DRS_WRAP0
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DRS_WRAP1
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DRS_WRAP2
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DRS_WRAP3
    // D3DRS_LIGHTING dirties the transform because of the inverse-model-
    // view optimization:
    (D3DDIRTYFLAG_LIGHTS | D3DDIRTYFLAG_TRANSFORM),     // D3DRS_LIGHTING
    (D3DDIRTYFLAG_LIGHTS | D3DDIRTYFLAG_SPECFOG_COMBINER),// D3DRS_SPECULARENABLE
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_LOCALVIEWER
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_COLORVERTEX
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_BACKSPECULARMATERIALSOURCE
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_BACKDIFFUSEMATERIALSOURCE
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_BACKAMBIENTMATERIALSOURCE
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_BACKEMISSIVEMATERIALSOURCE
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_SPECULARMATERIALSOURCE
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_DIFFUSEMATERIALSOURCE
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_AMBIENTMATERIALSOURCE
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_EMISSIVEMATERIALSOURCE
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_BACKAMBIENT
    D3DDIRTYFLAG_LIGHTS,                                // D3DRS_AMBIENT
    D3DDIRTYFLAG_POINTPARAMS,                           // D3DRS_POINTSIZE
    D3DDIRTYFLAG_POINTPARAMS,                           // D3DRS_POINTSIZE_MIN
    (D3DDIRTYFLAG_POINTPARAMS | D3DDIRTYFLAG_COMBINERS),// D3DRS_POINTSPRITEENABLE
    D3DDIRTYFLAG_POINTPARAMS,                           // D3DRS_POINTSCALEENABLE
    D3DDIRTYFLAG_POINTPARAMS,                           // D3DRS_POINTSCALE_A
    D3DDIRTYFLAG_POINTPARAMS,                           // D3DRS_POINTSCALE_B
    D3DDIRTYFLAG_POINTPARAMS,                           // D3DRS_POINTSCALE_C
    D3DDIRTYFLAG_POINTPARAMS,                           // D3DRS_POINTSIZE_MAX
    0,                                                  // D3DRS_PATCHEDGESTYLE
    0,                                                  // D3DRS_PATCHSEGMENTS
    0,                                                  // D3DRS_MULTISAMPLEFILTER
    0,                                                  // D3DRS_PRESENTATIONINTERVAL
    0,                                                  // D3DRS_DEFERRED_UNUSED8
    0,                                                  // D3DRS_DEFERRED_UNUSED7
    0,                                                  // D3DRS_DEFERRED_UNUSED6
    0,                                                  // D3DRS_DEFERRED_UNUSED5
    0,                                                  // D3DRS_DEFERRED_UNUSED4
    0,                                                  // D3DRS_DEFERRED_UNUSED3
    0,                                                  // D3DRS_DEFERRED_UNUSED2
    0,                                                  // D3DRS_DEFERRED_UNUSED1
};

D3DCONST DWORD D3DDIRTYFROMTEXTURESTATE[] =
{
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_ADDRESSU
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_ADDRESSV
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_ADDRESSW
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_MAGFILTER
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_MINFILTER
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_MIPFILTER
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_MIPMAPLODBIAS
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_MAXMIPLEVEL
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_MAXANISOTROPY
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_COLORKEYOP
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_COLORSIGN
    D3DDIRTYFLAG_TEXTURE_STATE,                         // D3DTSS_ALPHAKILL
    D3DDIRTYFLAG_COMBINERS,                             // D3DTSS_COLOROP (D3DTOP_BUMPENVMAP and D3DTOP_BUMPENVMAPLUMINANCE dirty more)
    D3DDIRTYFLAG_COMBINERS,                             // D3DTSS_COLORARG0
    D3DDIRTYFLAG_COMBINERS,                             // D3DTSS_COLORARG1
    D3DDIRTYFLAG_COMBINERS,                             // D3DTSS_COLORARG2
    D3DDIRTYFLAG_COMBINERS,                             // D3DTSS_ALPHAOP
    D3DDIRTYFLAG_COMBINERS,                             // D3DTSS_ALPHAARG0
    D3DDIRTYFLAG_COMBINERS,                             // D3DTSS_ALPHAARG1
    D3DDIRTYFLAG_COMBINERS,                             // D3DTSS_ALPHAARG2
    D3DDIRTYFLAG_COMBINERS,                             // D3DTSS_RESULTARG
    D3DDIRTYFLAG_TEXTURE_TRANSFORM,                     // D3DTSS_TEXTURETRANSFORMFLAGS
};

// Macro for converting from primitive count to number of vertices.
//
#define D3DVERTEXCOUNT(PrimitiveType, PrimitiveCount) \
    ((PrimitiveCount) * D3DPRIMITIVETOVERTEXCOUNT[PrimitiveType][0] + \
     + D3DPRIMITIVETOVERTEXCOUNT[PrimitiveType][1])

/****************************************************************************
 *
 * Publicly accessible globals - Feel free to modify the values.
 *
 ****************************************************************************/

// 'Extern' declaration that is both C and C++ friendly.
//
#ifdef __cplusplus
    #define D3DEXTERN extern "C"
#else
    #define D3DEXTERN extern
#endif

// Set D3D__NullHardware to TRUE to enable infinitely fast hardware (so fast
// you can't even see the results).  This is useful for determining how CPU-
// bound your program is.
//
D3DEXTERN BOOL D3D__NullHardware;

// Set D3D__SingleStepPusher to TRUE to cause D3D to do an implicit
// BlockUntilIdle after every D3D API call.  This is useful for tracking
// weird hardware crashes and hangs down to the problem API call.
//
D3DEXTERN BOOL D3D__SingleStepPusher;

// Points to the active IDirect3DDevice8 device (NULL if one hasn't been
// created yet).
//
D3DEXTERN IDirect3DDevice8 *D3D__pDevice;

// This is what D3D__pDevice actually points to.  Only the first 8 bytes
// are publicly accessible, for inline use by the BeginState/EndState APIs.
//
D3DEXTERN __declspec(align(16)) DWORD* D3D__Device[];

// Byte offsets into D3D__Device for useful stuff.
//
#define D3DDEVICE_PUT 0
#define D3DDEVICE_THRESHOLD 4

/****************************************************************************
 *
 * Private D3D globals - Please don't use these directly, as they're subject
 *                       subject to change.
 *
 ****************************************************************************/

// This array marks what APIs have been called when recording state blocks:
//
D3DEXTERN BYTE D3D__StateBlockDirty[];

// This array shadows the current render states:
//
D3DEXTERN DWORD D3D__RenderState[D3DRS_MAX];

// This array shadows the current texture stage states:
//
D3DEXTERN DWORD D3D__TextureState[D3DTSS_MAXSTAGES][D3DTSS_MAX];

// This array contains the dirty flags for deferred render and texture states:
//
D3DEXTERN DWORD D3D__DirtyFlags;

// This points to the data of the currently selected index buffer.  This is
// used for converting Draw[Indexed]Primitive calls to Draw[Indexed]Vertices
// calls inline.
//
D3DEXTERN WORD* D3D__IndexData;

/****************************************************************************
 *
 * Miscellaneous public defines
 *
 ****************************************************************************/

// The required alignment for any memory that is going to be
// rendered to from the hardware.
//
#define D3D_RENDER_MEMORY_ALIGNMENT           64

// The required alignment for any memory that is tiled, including the
// default frame buffers and depth buffer.
//
#define D3D_TILED_SURFACE_ALIGNMENT           0x4000

// Types of our DPC-level callback functions.
//
typedef void (__cdecl * D3DCALLBACK)(DWORD Context);
typedef void (__cdecl * D3DVBLANKCALLBACK)(D3DVBLANKDATA *pData);
typedef void (__cdecl * D3DSWAPCALLBACK)(D3DSWAPDATA *pData);
typedef void (__cdecl * D3DWAITCALLBACK)(DWORD Flags);

/*
 * NOTE: The C version of the methods for all of these interfaces
 *       are named "<interfacename>_<method name>" and have an
 *       explicit pointer to the interface as the first parameter.
 *       The actual definition of these methods is at the end
 *       of this file.
 */

/*
 * Direct3D, IDirect3D8 interface
 *
 */

#ifdef __cplusplus

struct Direct3D
{
    static ULONG WINAPI AddRef();
    static ULONG WINAPI Release();

    static UINT WINAPI GetAdapterCount();

    static HRESULT WINAPI GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier);
    static UINT    WINAPI GetAdapterModeCount(UINT Adapter);
    static HRESULT WINAPI EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE *pMode);
    static HRESULT WINAPI GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode);
    static HRESULT WINAPI CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed);
    static HRESULT WINAPI CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
    static HRESULT WINAPI CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType);
    static HRESULT WINAPI CheckDepthStencilMatch(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat);
    static HRESULT WINAPI GetDeviceCaps(UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS8 *pCaps);
    static HRESULT WINAPI CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, void *pUnused, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, D3DDevice **ppReturnedDeviceInterface);
    static HRESULT WINAPI SetPushBufferSize(DWORD PushBufferSize, DWORD KickOffSize); // Xbox extension
};

#endif __cplusplus

/*
 * D3DDevice, IDirect3DDevice8 interface
 */

#ifdef __cplusplus

struct D3DDevice
{
    // Standard D3D APIs:

    static ULONG WINAPI AddRef();
    static ULONG WINAPI Release();
    static HRESULT WINAPI GetDirect3D(Direct3D **ppD3D8);
    static HRESULT WINAPI GetDeviceCaps(D3DCAPS8 *pCaps);
    static HRESULT WINAPI GetDisplayMode(D3DDISPLAYMODE *pMode);
    static HRESULT WINAPI GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters);
    static HRESULT WINAPI Reset(D3DPRESENT_PARAMETERS *pPresentationParameters);
    static HRESULT WINAPI Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, void *pUnused, void *pUnused2);
    static HRESULT WINAPI GetBackBuffer(INT BackBuffer, D3DBACKBUFFER_TYPE UnusedType, D3DSurface **ppBackBuffer);
    static HRESULT WINAPI GetRasterStatus(D3DRASTER_STATUS *pRasterStatus);
    static void    WINAPI SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP *pRamp);
    static void    WINAPI GetGammaRamp(D3DGAMMARAMP *pRamp);
    static HRESULT WINAPI CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DTexture **ppTexture);
    static HRESULT WINAPI CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DVolumeTexture **ppVolumeTexture);
    static HRESULT WINAPI CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DCubeTexture **ppCubeTexture);
    static HRESULT WINAPI CreateVertexBuffer(UINT Length, DWORD UnusedUsage, DWORD UnusedFVF, D3DPOOL UnusedPool, D3DVertexBuffer **ppVertexBuffer);
    static HRESULT WINAPI CreateIndexBuffer(UINT Length, DWORD UnusedUsage, D3DFORMAT UnusedFormat, D3DPOOL UnusedPool, D3DIndexBuffer **ppIndexBuffer);
    static HRESULT WINAPI CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE UnusedMultiSample, BOOL UnusedLockable, D3DSurface **ppSurface);
    static HRESULT WINAPI CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE UnusedMultiSample, D3DSurface **ppSurface);
    static HRESULT WINAPI CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DSurface **ppSurface);
    static HRESULT WINAPI CopyRects(D3DSurface *pSourceSurface, CONST RECT *pSourceRectsArray, UINT cRects, D3DSurface *pDestinationSurface, CONST POINT *pDestPointsArray);
    static HRESULT WINAPI SetRenderTarget(D3DSurface *pRenderTarget, D3DSurface *pNewZStencil);
    static HRESULT WINAPI GetRenderTarget(D3DSurface **ppRenderTarget);
    static HRESULT WINAPI GetDepthStencilSurface(D3DSurface **ppZStencilSurface);
    static HRESULT WINAPI BeginScene();
    static HRESULT WINAPI EndScene();
    static HRESULT WINAPI Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
    static HRESULT WINAPI SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
    static HRESULT WINAPI GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix);
    static HRESULT WINAPI MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
    static HRESULT WINAPI SetViewport(CONST D3DVIEWPORT8 *pViewport);
    static HRESULT WINAPI GetViewport(D3DVIEWPORT8 *pViewport);
    static HRESULT WINAPI SetMaterial(CONST D3DMATERIAL8 *pMaterial);
    static HRESULT WINAPI GetMaterial(D3DMATERIAL8 *pMaterial);
    static HRESULT WINAPI SetLight(DWORD Index, CONST D3DLIGHT8 *pLight);
    static HRESULT WINAPI GetLight(DWORD Index, D3DLIGHT8 *pLight);
    static HRESULT WINAPI LightEnable(DWORD Index, BOOL Enable);
    static HRESULT WINAPI GetLightEnable(DWORD Index, BOOL *pEnable);
    static HRESULT WINAPI SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
    static HRESULT WINAPI GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue);
#if D3DCOMPILE_BEGINSTATEBLOCK
    static HRESULT WINAPI BeginStateBlock();
    static HRESULT WINAPI EndStateBlock(DWORD *pToken);
#endif
    static HRESULT WINAPI ApplyStateBlock(DWORD Token);
    static HRESULT WINAPI CaptureStateBlock(DWORD Token);
    static HRESULT WINAPI DeleteStateBlock(DWORD Token);
    static HRESULT WINAPI CreateStateBlock(D3DSTATEBLOCKTYPE Type,DWORD *pToken);
    static HRESULT WINAPI GetTexture(DWORD Stage, D3DBaseTexture **ppTexture);
    static HRESULT WINAPI SetTexture(DWORD Stage, D3DBaseTexture *pTexture);
    static HRESULT WINAPI GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue);
    static HRESULT WINAPI SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
    static HRESULT WINAPI DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
    static HRESULT WINAPI DrawIndexedPrimitive(D3DPRIMITIVETYPE, UINT UnusedMinIndex, UINT UnusedNumIndices, UINT StartIndex, UINT PrimitiveCount);
    static HRESULT WINAPI DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
    static HRESULT WINAPI DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT UnusedMinIndex, UINT UnusedNumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT UnusedIndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
    static HRESULT WINAPI CreateVertexShader(CONST DWORD *pDeclaration, CONST DWORD *pFunction, DWORD *pHandle, DWORD Usage);
    static HRESULT WINAPI SetVertexShader(DWORD Handle);
    static HRESULT WINAPI GetVertexShader(DWORD *pHandle);
    static HRESULT WINAPI DeleteVertexShader(DWORD Handle);
    static HRESULT WINAPI SetVertexShaderConstant(INT Register, CONST void *pConstantData, DWORD ConstantCount);
    static HRESULT WINAPI SetVertexShaderConstantFast(INT Register, CONST void *pConstantData, DWORD ConstantCount);
    static HRESULT WINAPI GetVertexShaderConstant(INT Register, void *pConstantData, DWORD ConstantCount);
    static HRESULT WINAPI GetVertexShaderDeclaration(DWORD Handle, void *pData, DWORD *pSizeOfData);
    static HRESULT WINAPI GetVertexShaderFunction(DWORD Handle,void *pData, DWORD *pSizeOfData);
    static HRESULT WINAPI SetStreamSource(UINT StreamNumber, D3DVertexBuffer *pStreamData, UINT Stride);
    static HRESULT WINAPI GetStreamSource(UINT StreamNumber, D3DVertexBuffer **ppStreamData, UINT *pStride);
    static HRESULT WINAPI SetIndices(D3DIndexBuffer *pIndexData, UINT BaseVertexIndex);
    static HRESULT WINAPI GetIndices(D3DIndexBuffer **ppIndexData, UINT *pBaseVertexIndex);
    static HRESULT WINAPI CreatePixelShader(CONST D3DPIXELSHADERDEF *pPSDef, DWORD *pHandle);
    static HRESULT WINAPI SetPixelShader(DWORD Handle);
    static HRESULT WINAPI SetPixelShaderProgram(CONST D3DPIXELSHADERDEF *pPSDef);
    static HRESULT WINAPI GetPixelShader(DWORD *pHandle);
    static HRESULT WINAPI DeletePixelShader(DWORD Handle);
    static HRESULT WINAPI SetPixelShaderConstant(DWORD Register, CONST void *pConstantData, DWORD ConstantCount);
    static HRESULT WINAPI GetPixelShaderConstant(DWORD Register, void *pConstantData, DWORD ConstantCount);
    static HRESULT WINAPI GetPixelShaderFunction(DWORD Handle, D3DPIXELSHADERDEF *pData);
    static HRESULT WINAPI DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo);
    static HRESULT WINAPI DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo);
    static HRESULT WINAPI DeletePatch(UINT Handle);

    // The following APIs are all Xbox extensions:

    static HRESULT WINAPI SetShaderConstantMode(D3DSHADERCONSTANTMODE Mode);
    static HRESULT WINAPI GetShaderConstantMode(D3DSHADERCONSTANTMODE *pMode);
    static HRESULT WINAPI LoadVertexShader(DWORD Handle, DWORD Address);
    static HRESULT WINAPI LoadVertexShaderProgram(CONST DWORD *pFunction, DWORD Address);
    static HRESULT WINAPI SelectVertexShader(DWORD Handle, DWORD Address);
    static HRESULT WINAPI SelectVertexShaderDirect(D3DVERTEXATTRIBUTEFORMAT *pVAF, DWORD Address);
    static HRESULT WINAPI RunVertexStateShader(DWORD Address, CONST float *pData);
    static HRESULT WINAPI GetVertexShaderSize(DWORD Handle, UINT *pSize);
    static HRESULT WINAPI GetVertexShaderType(DWORD Handle, DWORD *pType);
    static HRESULT WINAPI DrawVertices(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT VertexCount);
    static HRESULT WINAPI DrawIndexedVertices(D3DPRIMITIVETYPE, UINT VertexCount, CONST WORD *pIndexData);
    static HRESULT WINAPI DrawVerticesUP(D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
    static HRESULT WINAPI DrawIndexedVerticesUP(D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST void *pIndexData, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
    static HRESULT WINAPI PrimeVertexCache(UINT VertexCount, CONST WORD *pIndexData);
    static HRESULT WINAPI CreatePalette(D3DPALETTESIZE Size, D3DPalette **ppPalette);
    static HRESULT WINAPI GetPalette(DWORD Stage, D3DPalette **ppPalette);
    static HRESULT WINAPI SetPalette(DWORD Stage, D3DPalette *pPalette);
    static HRESULT WINAPI SetTextureStageStateNotInline(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
    static HRESULT WINAPI SetRenderStateNotInline(D3DRENDERSTATETYPE State, DWORD Value);
    static HRESULT WINAPI SetBackMaterial(CONST D3DMATERIAL8 *pMaterial);
    static HRESULT WINAPI GetBackMaterial(D3DMATERIAL8 *pMaterial);
    static HRESULT WINAPI UpdateOverlay(D3DSurface *pSurface, CONST RECT *SrcRect, CONST RECT *DstRect, BOOL EnableColorKey, D3DCOLOR ColorKey);
    static HRESULT WINAPI EnableOverlay(BOOL Enable);
    static HRESULT WINAPI BeginVisibilityTest();
    static HRESULT WINAPI EndVisibilityTest(DWORD Index);
    static HRESULT WINAPI GetVisibilityTestResult(DWORD Index, UINT* pResult, ULONGLONG* pTimeStamp);
    static BOOL    WINAPI GetOverlayUpdateStatus();
    static HRESULT WINAPI GetDisplayFieldStatus(D3DFIELD_STATUS *pFieldStatus);
    static HRESULT WINAPI SetVertexData2f(INT Register, float a, float b);
    static HRESULT WINAPI SetVertexData4f(INT Register, float a, float b, float c, float d);
    static HRESULT WINAPI SetVertexData2s(INT Register, SHORT a, SHORT b);
    static HRESULT WINAPI SetVertexData4s(INT Register, SHORT a, SHORT b, SHORT c, SHORT d);
    static HRESULT WINAPI SetVertexData4ub(INT Register, BYTE a, BYTE b, BYTE c, BYTE d);
    static HRESULT WINAPI SetVertexDataColor(INT Register, D3DCOLOR Color);
    static HRESULT WINAPI Begin(D3DPRIMITIVETYPE PrimitiveType);
    static HRESULT WINAPI End();
    static HRESULT WINAPI CreateFixup(UINT Size, D3DFixup **ppFixup);
    static HRESULT WINAPI CreatePushBuffer(UINT Size, BOOL RunUsingCpuCopy, D3DPushBuffer **ppPushBuffer);
    static HRESULT WINAPI BeginPushBuffer(D3DPushBuffer *pPushBuffer);
    static HRESULT WINAPI EndPushBuffer();
    static HRESULT WINAPI RunPushBuffer(D3DPushBuffer* pPushBuffer, D3DFixup *pFixup);
    static HRESULT WINAPI GetPushBufferOffset(DWORD* pOffset);
    static HRESULT WINAPI Nop();
    static HRESULT WINAPI GetProjectionViewportMatrix(D3DMATRIX* pProjectionViewport);
    static HRESULT WINAPI SetModelView(CONST D3DMATRIX* pModelView, CONST D3DMATRIX* pInverseModelView, CONST D3DMATRIX* pComposite);
    static HRESULT WINAPI GetModelView(D3DMATRIX* pModelView);
    static HRESULT WINAPI SetVertexBlendModelView(UINT Count, CONST D3DMATRIX* pModelViews, CONST D3DMATRIX* pInverseModelViews, CONST D3DMATRIX* pProjectionViewport);
    static HRESULT WINAPI GetVertexBlendModelView(UINT Count, D3DMATRIX* pModelViews, D3DMATRIX* pProjectionViewport);
    static HRESULT WINAPI SetVertexShaderInput(DWORD Handle, UINT StreamCount, CONST D3DSTREAM_INPUT* pStreamInputs);
    static HRESULT WINAPI SetVertexShaderInputDirect(D3DVERTEXATTRIBUTEFORMAT *pVAF, UINT StreamCount, CONST D3DSTREAM_INPUT* pStreamInputs);
    static HRESULT WINAPI GetVertexShaderInput(DWORD* pHandle, UINT* pStreamCount, D3DSTREAM_INPUT* pStreamInputs);
    static HRESULT WINAPI SwitchTexture(DWORD Stage, D3DBaseTexture *pTexture);
    static HRESULT WINAPI SetScissors(DWORD Count, BOOL Exclusive, CONST D3DRECT *pRects);
    static HRESULT WINAPI GetScissors(DWORD* pCount, BOOL *pExclusive, D3DRECT *pRects);
    static HRESULT WINAPI SetTile(DWORD Index, CONST D3DTILE* pTile);
    static HRESULT WINAPI GetTile(DWORD Index, D3DTILE* pTile);
    static DWORD   WINAPI GetTileCompressionTags(DWORD ZStartTag, DWORD ZEndTag);
    static void    WINAPI SetTileCompressionTagBits(DWORD Partition, DWORD Address, CONST DWORD *pData, DWORD Count);
    static void    WINAPI GetTileCompressionTagBits(DWORD Partition, DWORD Address, DWORD *pData, DWORD Count);
    static HRESULT WINAPI BeginPush(DWORD Count, DWORD **ppPush);
    static HRESULT WINAPI EndPush(DWORD *pPush);
    static HRESULT WINAPI BeginState(DWORD Count, DWORD **ppPush);
    static HRESULT WINAPI EndState(DWORD *pPush);
    static BOOL    WINAPI IsBusy();
    static void    WINAPI BlockUntilIdle();
    static void    WINAPI KickPushBuffer();
    static void    WINAPI SetVerticalBlankCallback(D3DVBLANKCALLBACK pCallback);
    static void    WINAPI SetSwapCallback(D3DSWAPCALLBACK pCallback);
    static void    WINAPI BlockUntilVerticalBlank();
    static DWORD   WINAPI InsertFence();
    static BOOL    WINAPI IsFencePending(DWORD Fence);
    static VOID    WINAPI BlockOnFence(DWORD Fence);
    static VOID    WINAPI InsertCallback(D3DCALLBACKTYPE Type, D3DCALLBACK pCallback, DWORD Context);
    static VOID    WINAPI FlushVertexCache();
    static void    WINAPI SetFlickerFilter(DWORD Filter);
    static void    WINAPI SetSoftDisplayFilter(BOOL Enable);
    static HRESULT WINAPI SetCopyRectsState(CONST D3DCOPYRECTSTATE *pCopyRectState, CONST D3DCOPYRECTROPSTATE *pCopyRectRopState);
    static HRESULT WINAPI GetCopyRectsState(D3DCOPYRECTSTATE *pCopyRectState, D3DCOPYRECTROPSTATE *pCopyRectRopState);
    static HRESULT WINAPI PersistDisplay();
    static HRESULT WINAPI GetPersistedSurface(IDirect3DSurface8 **ppSurface);
    static DWORD   WINAPI Swap(DWORD Flags);
    static HRESULT WINAPI SetBackBufferScale(float x, float y);
    static HRESULT WINAPI GetBackBufferScale(float *pX, float *pY);
    static HRESULT WINAPI SetScreenSpaceOffset(float x, float Y);
    static HRESULT WINAPI GetScreenSpaceOffset(float *pX, float *pY);
    static void    WINAPI SetOverscanColor(D3DCOLOR Color);
    static D3DCOLOR WINAPI GetOverscanColor();
    static void    WINAPI SetDepthClipPlanes(float Near, float Far, DWORD Flags);
    static void    WINAPI GetDepthClipPlanes(float *pNear, float *pFar, DWORD Flags);
    static void    WINAPI GetViewportOffsetAndScale(D3DVECTOR4 *pOffset, D3DVECTOR4 *pScale);
    static DWORD   WINAPI SetDebugMarker(DWORD Marker);
    static DWORD   WINAPI GetDebugMarker();
    static void    WINAPI SetStipple(CONST DWORD *pPattern);
    static void    WINAPI GetStipple(DWORD *pPattern);
    static void    WINAPI SetWaitCallback(D3DWAITCALLBACK pCallback);
    static DWORD   WINAPI GetPushDistance(DWORD Handle);
    static HRESULT WINAPI SetTimerCallback(ULONGLONG Time, D3DCALLBACK pCallback, DWORD Context);
    static void    WINAPI SetRenderTargetFast(D3DSurface *pRenderTarget, D3DSurface *pNewZStencil, DWORD Flags);
};

#endif __cplusplus

/*
 * D3DResource, IDirect3DResource8 interface
 *
 * The root structure of all D3D 'resources' such as textures and vertex buffers.
 */

#define D3DCOMMON_REFCOUNT_MASK      0x0000FFFF

#define D3DCOMMON_TYPE_MASK          0x00070000
#define D3DCOMMON_TYPE_SHIFT         16
#define D3DCOMMON_TYPE_VERTEXBUFFER  0x00000000
#define D3DCOMMON_TYPE_INDEXBUFFER   0x00010000
#define D3DCOMMON_TYPE_PUSHBUFFER    0x00020000
#define D3DCOMMON_TYPE_PALETTE       0x00030000
#define D3DCOMMON_TYPE_TEXTURE       0x00040000
#define D3DCOMMON_TYPE_SURFACE       0x00050000
#define D3DCOMMON_TYPE_FIXUP         0x00060000

#define D3DCOMMON_INTREFCOUNT_MASK   0x00780000
#define D3DCOMMON_INTREFCOUNT_SHIFT  19

// This flag was used for UMA emulation on pre-Beta development kit
// machines, and is deprecated on the final hardware.
//
#define D3DCOMMON_VIDEOMEMORY        0

// Internal flag to indicate that this resource was created by Direct3D
//
#define D3DCOMMON_D3DCREATED         0x01000000

// The rest of the bits may be used by derived classes.
#define D3DCOMMON_UNUSED_MASK        0xFE000000
#define D3DCOMMON_UNUSED_SHIFT       25

struct D3DResource
{

#ifdef __cplusplus

    ULONG WINAPI AddRef();

    // DOC:  If the GPU is currently using this object when the last call
    //   to release is made then this call will block until the GPU is done
    //   with this object.  The caller will have to manually check this if
    //   they do not want this call to block.
    //
    ULONG WINAPI Release();

    HRESULT WINAPI GetDevice(D3DDevice **ppDevice);
    D3DRESOURCETYPE WINAPI GetType();

    HRESULT WINAPI SetPrivateData(REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags);
    HRESULT WINAPI GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData);
    HRESULT WINAPI FreePrivateData(REFGUID refguid);

    // DOC: This additional method returns TRUE if the data for this
    //  : resource is currently being used by the GPU.  This can be used
    //  : to avoid blocking in Release and Lock.  This will always return
    //  : TRUE if the resource is currently set on the device.
    //
    BOOL WINAPI IsBusy();

    // DOC: A blocking form of 'IsBusy' that ensure that this resource is
    //  : no longer used by the GPU.  This will wait for the GPU to go
    //  : idle if this resource is set on the device.
    //
    void    WINAPI BlockUntilNotBusy();

    // DOC: On pre-beta development kits, this API set whether the we should
    //  ; use video or AGP memory for the data of this resource.
    //  ;
    //  ; This API has been deprecated on the final hardware.
    //
    void    WINAPI MoveResourceMemory(D3DMEMORY where);

    // DOC: Performs some debug checks and maps the resource's
    //  : data field from a contiguous memory address to the physical memory
    //  : address.  It should be called for any resource that is not created
    //  : through an D3DDevice "Create" routine.
    //  :
    //  : This API does not modify any state in the resource besides the
    //  : Data field nor does the library keep track of what resources
    //  : have been registered.
    //  :
    //  : This method takes a contiguous memory address, adds the current
    //  : contents of the Data field to it, converts it to a physical
    //  : address and sets that as the Data field of the resource.
    //
    void    WINAPI Register(void *pBase);

#endif __cplusplus

    // All resources need these fields.  Inherit them in C++.

    DWORD Common;           // Refcount and flags common to all resources
    DWORD Data;             // Offset to the data held by this resource
    DWORD Lock;             // Lock information, initialize to zero
};

/*
 * D3DPixelContainer interface
 *
 * A base structure that describes the shared layout between textures
 * and surfaces.
 */

// The layout of the Format field.

#define D3DFORMAT_RESERVED1_MASK        0x00000003      // Must be zero

#define D3DFORMAT_DMACHANNEL_MASK       0x00000003
#define D3DFORMAT_DMACHANNEL_A          0x00000001      // DMA channel A - the default for all system memory
#define D3DFORMAT_DMACHANNEL_B          0x00000002      // DMA channel B - unused
#define D3DFORMAT_CUBEMAP               0x00000004      // Set if the texture if a cube map
#define D3DFORMAT_BORDERSOURCE_COLOR    0x00000008
#define D3DFORMAT_DIMENSION_MASK        0x000000F0      // # of dimensions
#define D3DFORMAT_DIMENSION_SHIFT       4
#define D3DFORMAT_FORMAT_MASK           0x0000FF00
#define D3DFORMAT_FORMAT_SHIFT          8
#define D3DFORMAT_MIPMAP_MASK           0x000F0000
#define D3DFORMAT_MIPMAP_SHIFT          16
#define D3DFORMAT_USIZE_MASK            0x00F00000      // Log 2 of the U size of the base texture
#define D3DFORMAT_USIZE_SHIFT           20
#define D3DFORMAT_VSIZE_MASK            0x0F000000      // Log 2 of the V size of the base texture
#define D3DFORMAT_VSIZE_SHIFT           24
#define D3DFORMAT_PSIZE_MASK            0xF0000000      // Log 2 of the P size of the base texture
#define D3DFORMAT_PSIZE_SHIFT           28

// The layout of the size field, used for non swizzled or compressed textures.
//
// The Size field of a container will be zero if the texture is swizzled or compressed.
// It is guarenteed to be non-zero otherwise because either the height/width will be
// greater than one or the pitch adjust will be nonzero because the minimum texture
// pitch is 8 bytes.

#define D3DSIZE_WIDTH_MASK              0x00000FFF   // Width of the texture - 1, in texels
#define D3DSIZE_HEIGHT_MASK             0x00FFF000   // Height of the texture - 1, in texels
#define D3DSIZE_HEIGHT_SHIFT            12
#define D3DSIZE_PITCH_MASK              0xFF000000   // Pitch / 64 - 1
#define D3DSIZE_PITCH_SHIFT             24

#define D3DTEXTURE_ALIGNMENT            128
#define D3DTEXTURE_CUBEFACE_ALIGNMENT   128

#define D3DTEXTURE_PITCH_ALIGNMENT 64
#define D3DTEXTURE_PITCH_MIN       64

#ifdef __cplusplus

struct D3DPixelContainer : public D3DResource
{
    DWORD Format;   // Format information about the texture.
    DWORD Size;     // Size of a non power-of-2 texture, must be zero otherwise
};

#endif __cplusplus

/*
 * D3DBaseTexture interface
 *
 * The root structure of all D3D textures.  Inherits all of the methods
 * from D3DResource.
 *
 * The data memory pointed to by the Data field must be aligned on a
 * D3DTEXTURE_ALIGNMENT byte multiple.
 */

struct D3DBaseTexture
    #ifdef __cplusplus
        : public D3DPixelContainer
    #endif
{

#ifdef __cplusplus
    DWORD WINAPI GetLevelCount();
#endif

#ifndef __cplusplus

    // Manually inherit these from D3DResource
    DWORD Common;
    DWORD Data;
    DWORD Lock;

    DWORD Format;   // Format information about the texture.
    DWORD Size;     // Size of a non power-of-2 texture, must be zero otherwise
#endif

};


/*
 * D3DTexture, IDirect3DTexture8 interface
 *
 * A normal texture.  Inherits from D3DBaseTexture
 */

struct D3DTexture 
    #ifdef __cplusplus
        : public D3DBaseTexture
    #endif
{

#ifdef __cplusplus
    HRESULT WINAPI GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc);
    HRESULT WINAPI GetSurfaceLevel(UINT Level, D3DSurface **ppSurfaceLevel);
    HRESULT WINAPI LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags);
    HRESULT WINAPI UnlockRect(UINT Level);
#endif

#ifndef __cplusplus

    // Manually inherit these from D3DBaseTexture
    DWORD Common;
    DWORD Data;
    DWORD Lock;

    DWORD Format;   // Format information about the texture.
    DWORD Size;     // Size of a non power-of-2 texture, must be zero otherwise
#endif

};


/*
 * D3DVolumeTexture, IDirect3DVolumeTexture8 interface
 *
 * A volume texture.  Inherits from D3DBaseTexture
 */

struct D3DVolumeTexture 
    #ifdef __cplusplus
        : public D3DBaseTexture
    #endif
{

#ifdef __cplusplus
    HRESULT WINAPI GetLevelDesc(UINT Level, D3DVOLUME_DESC *pDesc);
    HRESULT WINAPI GetVolumeLevel(UINT Level, D3DVolume **ppVolumeLevel);
    HRESULT WINAPI LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags);
    HRESULT WINAPI UnlockBox(UINT Level);
#endif

#ifndef __cplusplus

    // Manually inherit these from D3DBaseTexture
    DWORD Common;
    DWORD Data;
    DWORD Lock;

    DWORD Format;   // Format information about the texture.
    DWORD Size;     // Size of a non power-of-2 texture, must be zero otherwise
#endif

};


/*
 * D3DCubeTexture, IDirect3DCubeTexture8 interface
 *
 * A cube texture.  Inherits from D3DBaseTexture
 */

struct D3DCubeTexture 
    #ifdef __cplusplus
        : public D3DBaseTexture
    #endif
{

#ifdef __cplusplus
    HRESULT WINAPI GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc);
    HRESULT WINAPI GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, D3DSurface **ppCubeMapSurface);
    HRESULT WINAPI LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags);
    HRESULT WINAPI UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level);
#endif

#ifndef __cplusplus

    // Manually inherit these from D3DBaseTexture
    DWORD Common;
    DWORD Data;
    DWORD Lock;

    DWORD Format;   // Format information about the texture.
    DWORD Size;     // Size of a non power-of-2 texture, must be zero otherwise
#endif

};


/*
 * D3DVertexBuffer, IDirect3DVertexBuffer8 interface
 *
 * A vertex buffer.
 *
 * The data for the vertex buffer must be aligned on a
 * D3DVERTEXBUFFER_ALIGNMENT byte multiple.
 */

#define D3DVERTEXBUFFER_ALIGNMENT     1

struct D3DVertexBuffer
    #ifdef __cplusplus
        : public D3DResource
    #endif
{
#ifdef __cplusplus

    HRESULT WINAPI Lock(UINT OffsetToLock, UINT UnusedSizeToLock, BYTE **ppbData, DWORD Flags);
    HRESULT WINAPI Unlock();
    HRESULT WINAPI GetDesc(D3DVERTEXBUFFER_DESC *pDesc);

#endif __cplusplus

#ifndef __cplusplus

    // Manually inherit these from D3DResource
    DWORD Common;
    DWORD Data;
    DWORD Lock;

#endif

};


/*
 * D3DIndexBuffer, IDirect3DIndexBuffer8 interface
 *
 * An index buffer.
 *
 * The data for the index buffer must be aligned on a D3DINDEXBUFFER_ALIGNMENT
 * byte multiple.
 */

#define D3DINDEXBUFFER_ALIGNMENT        4

struct D3DIndexBuffer
    #ifdef __cplusplus
        : public D3DResource
    #endif
{
#ifdef __cplusplus

    HRESULT WINAPI Lock(UINT OffsetToLock, UINT UnusedSizeToLock, BYTE **ppbData, DWORD Flags);
    HRESULT WINAPI Unlock();
    HRESULT WINAPI GetDesc(D3DINDEXBUFFER_DESC *pDesc);

#endif __cplusplus

#ifndef __cplusplus

    // Manually inherit these from D3DResource
    DWORD Common;
    DWORD Data;
    DWORD Lock;

#endif

};


/*
 * D3DPalette, IDirect3DPalette8 interface
 *
 * A palette.
 */

#define D3DPALETTE_ALIGNMENT 64

#define D3DPALETTE_COMMON_VIDEOMEMORY            0
#define D3DPALETTE_COMMON_UNUSED                 0x20000000
#define D3DPALETTE_COMMON_PALETTESIZE_MASK       0xC0000000
#define D3DPALETTE_COMMON_PALETTESIZE_SHIFT      30

#define D3DPALETTE_COMMON_PALETTESET_SHIFT       28
#define D3DPALETTE_COMMON_PALETTESET_MASK        0xF

struct D3DPalette
    #ifdef __cplusplus
        : public D3DResource
    #endif
{
#ifdef __cplusplus

    HRESULT WINAPI Lock(D3DCOLOR **ppColors, DWORD Flags);
    HRESULT WINAPI Unlock();
    D3DPALETTESIZE WINAPI GetSize();

#endif __cplusplus

#ifndef __cplusplus

    // Manually inherit these from D3DResource
    DWORD Common;
    DWORD Data;
    DWORD Lock;

#endif

};


/*
 * D3DSurface, IDirect3DSurface8 interface
 *
 * Abstracts a chunk of data that can be drawn to.  The Common and Format
 * fields use the D3DCOMMON and D3DFORMAT constants defined for
 * textures.
 */

#define D3DSURFACE_ALIGNMENT    D3D_RENDER_MEMORY_ALIGNMENT
#define D3DSURFACE_OWNSMEMORY   0x80000000

struct D3DSurface
    #if defined(__cplusplus)
        : public D3DPixelContainer
    #endif
{
#ifdef __cplusplus

    HRESULT WINAPI GetContainer(D3DBaseTexture **ppBaseTexture);
    HRESULT WINAPI GetDesc(D3DSURFACE_DESC *pDesc);
    HRESULT WINAPI LockRect(D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags);
    HRESULT WINAPI UnlockRect();

#endif __cplusplus

#ifndef __cplusplus

    // Manually inherit these from D3DPixelContainer
    DWORD Common;
    DWORD Data;
    DWORD Lock;
    DWORD Format;
    DWORD Size;

#endif

    D3DBaseTexture *Parent;
};


/*
 * D3DVolume, IDirect3DVolume8 interface
 */

#define D3DVOLUME_ALIGNMENT    D3D_RENDER_MEMORY_ALIGNMENT
#define D3DVOLUME_OWNSMEMORY   0x800000000

struct D3DVolume
    #if defined(__cplusplus)
        : public D3DPixelContainer
    #endif
{
#ifdef __cplusplus

    HRESULT WINAPI GetContainer(D3DBaseTexture **ppBaseTexture);
    HRESULT WINAPI GetDesc(D3DVOLUME_DESC *pDesc);
    HRESULT WINAPI LockBox(D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags);
    HRESULT WINAPI UnlockBox();

#endif

#ifndef __cplusplus

    // Manually inherit these from D3DResource
    DWORD Common;
    DWORD Data;
    DWORD Lock;
    DWORD Format;
    DWORD Size;

#endif

    D3DBaseTexture *Parent;
};


/*
 * D3DPushBuffer, IDirect3DPushBuffer8 interface
 *
 * A push-buffer resource.
 */

#define D3DPUSHBUFFER_ALIGNMENT 4

// The following flag, when set in the Common field, dictates that when
// RunPushBuffer is called, the push-buffer is copied using the CPU instead
// of executed in-place by the GPU.  This should be used for small push-
// buffers to avoid the high latency cost of the GPU JUMP command.  In this
// case, the memory should be cacheable (not write-combined), and need not
// be physically contiguous.
#define D3DPUSHBUFFER_RUN_USING_CPU_COPY  0x80000000

struct D3DPushBuffer
    #ifdef __cplusplus
        : public D3DResource
    #endif
{
#ifdef __cplusplus

    HRESULT WINAPI Verify(BOOL StampResources);
    HRESULT WINAPI BeginFixup(D3DFixup* pFixup, BOOL NoWait);
    HRESULT WINAPI EndFixup();
    HRESULT WINAPI RunPushBuffer(DWORD Offset, D3DPushBuffer* pDestinationPushBuffer, D3DFixup* pDestinationFixup);
    HRESULT WINAPI SetModelView(DWORD Offset, CONST D3DMATRIX* pModelView, CONST D3DMATRIX* pInverseModelView, CONST D3DMATRIX* pComposite);
    HRESULT WINAPI SetVertexBlendModelView(DWORD Offset, UINT Count, CONST D3DMATRIX* pModelViews, CONST D3DMATRIX* pInverseModelViews, CONST D3DMATRIX* pProjectionViewport);
    HRESULT WINAPI SetVertexShaderInput(DWORD Offset, DWORD Handle, UINT StreamCount, CONST D3DSTREAM_INPUT *pStreamInputs);
    HRESULT WINAPI SetVertexShaderInputDirect(DWORD Offset, D3DVERTEXATTRIBUTEFORMAT *pVAF, UINT StreamCount, CONST D3DSTREAM_INPUT *pStreamInputs);
    HRESULT WINAPI SetRenderTarget(DWORD Offset, D3DSurface* pRenderTarget, D3DSurface* pZBuffer);
    HRESULT WINAPI SetTexture(DWORD Offset, DWORD Stage, D3DBaseTexture *pTexture);
    HRESULT WINAPI SetPalette(DWORD Offset, DWORD Stage,D3DPalette *pPalette);
    HRESULT WINAPI EndVisibilityTest(DWORD Offset, DWORD Index);
    HRESULT WINAPI SetVertexShaderConstant(DWORD Offset, INT Register, CONST VOID* pConstantData, DWORD ConstantCount);
    HRESULT WINAPI SetRenderState(DWORD Offset, D3DRENDERSTATETYPE State, DWORD Value);
    HRESULT WINAPI CopyRects(DWORD Offset, D3DSurface *pSourceSurface, D3DSurface *pDestinationSurface);
    HRESULT WINAPI Jump(DWORD Offset, UINT DestinationOffset);
    HRESULT WINAPI GetSize(DWORD* pSize);

#endif __cplusplus

#ifndef __cplusplus

    // Manually inherit these from D3DResource.
    //
    // Note (in an exception to all other resources) that 'Data' is a virtual
    // address for push-buffers.
    DWORD Common;
    DWORD Data;
    DWORD Lock;

#endif

    // Size, in bytes, of the push-buffer program.
    DWORD Size;

    // Size, in bytes, of the allocation of the buffer pointed to by 'Data'.
    DWORD AllocationSize;

    // Internal D3D-only field that identifies the interrupt that the fix-up 
    // will be done on for the most recent RunPushBuffer call when using 
    // non-COPY push-buffers.
    DWORD InterruptId;
};


/*
 * D3DFixup, IFixup8 interface
 *
 * A fix-up resource for push-buffers.
 */

#define D3DFIXUP_ALIGNMENT 4

struct D3DFixup
    #ifdef __cplusplus
        : public D3DResource
    #endif
{
#ifdef __cplusplus

    HRESULT WINAPI Reset();
    HRESULT WINAPI GetSize(DWORD* pSize);
    HRESULT WINAPI GetSpace(DWORD* pSpace);

#endif __cplusplus

#ifndef __cplusplus

    // Manually inherit these from D3DResource.
    //
    // Note that 'Data' is always a virtual address.
    DWORD Common;
    DWORD Data;
    DWORD Lock;

#endif

    // Offset to the last completed fix-up.  RunPushBuffer uses this and 'Data'.
    DWORD Run;

    // Offset to where we'll append the next fix-up.
    DWORD Next;

    // Size of the whole allocation.
    DWORD Size;
};


/*
 * Helper methods.
 */

//----------------------------------------------------------------------------
// Allocate a block of contiguous memory and return a write-combined
// pointer to it.  This memory is suitable to be used as the data
// for any of the D3D structures.
//
// Returns NULL if the memory could not be allocated.
//
void* WINAPI D3D_AllocContiguousMemory(
    DWORD Size,         // The size of the allocation in bytes
    DWORD Alignment     // The alignment of the allocation
    );

//----------------------------------------------------------------------------
// Frees memory allocated by the above method.
//

void WINAPI D3D_FreeContiguousMemory(
    void *pMemory       // The block of memory to free
    );

//----------------------------------------------------------------------------
// D3D_CopyContiguousMemory has been deprecated on the final hardware.
// Use CopyRects instead.
//
// With the API as speced, this function was hideously expensive because
// it had to flush the GPU's entire push-buffer, and then spin the CPU until
// the copy was done.  CopyRects lets everything be nicely asynchronous,
// just as it is with all other GPU APIs.
//
// To get equivalent behavior with CopyRects, use XGSetSurfaceHeader to create
// wrapper surfaces around the memory (the pitches should equal the widths
// times the pixel size and not be more than 8128).  Then surface.IsBusy(),
// LockRect(), and the other standard synchronization APIs may be used to
// determine when the copy is done.
//
// void WINAPI D3D_CopyContiguousMemory(
//     void *pSource,
//     void *pDest,
//     DWORD Size
//     );

//----------------------------------------------------------------------------
// On pre-beta development kits, this function was used to copy data
// from AGP memory to video memory.
//
// This function is deprecated on the final hardware.
//
D3DINLINE void WINAPI D3D_CopyContiguousMemoryToVideo(
    void *pMemory          // Contiguous memory block to move.
    )
{
}


/*
 * C exported method definitions for the class methods defined above and the C++
 * thunks that defer to them.
 */

/* Direct3D */

D3DINLINE ULONG   WINAPI Direct3D_AddRef() { return 1; }
D3DINLINE ULONG   WINAPI Direct3D_Release() { return 1; }
D3DINLINE UINT    WINAPI Direct3D_GetAdapterCount() { return 1; }
HRESULT WINAPI Direct3D_GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier);
UINT    WINAPI Direct3D_GetAdapterModeCount(UINT Adapter);
HRESULT WINAPI Direct3D_EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE *pMode);
HRESULT WINAPI Direct3D_GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode);
HRESULT WINAPI Direct3D_CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed);
HRESULT WINAPI Direct3D_CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
HRESULT WINAPI Direct3D_CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType);
HRESULT WINAPI Direct3D_CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
HRESULT WINAPI Direct3D_GetDeviceCaps(UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS8 *pCaps);
HRESULT WINAPI Direct3D_CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, void *pUnused, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, D3DDevice **ppReturnedDeviceInterface);
void    WINAPI Direct3D_SetPushBufferSize(DWORD PushBufferSize, DWORD KickOffSize);

// Compatibility wrappers.

D3DINLINE ULONG   WINAPI IDirect3D8_AddRef(Direct3D *pThis) { return Direct3D_AddRef(); }
D3DINLINE ULONG   WINAPI IDirect3D8_Release(Direct3D *pThis) { return Direct3D_Release(); }
D3DINLINE UINT    WINAPI IDirect3D8_GetAdapterCount(Direct3D *pThis) { return Direct3D_GetAdapterCount(); }
D3DINLINE HRESULT WINAPI IDirect3D8_GetAdapterIdentifier(Direct3D *pThis, UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier) { return Direct3D_GetAdapterIdentifier(Adapter, Flags, pIdentifier); }
D3DINLINE UINT    WINAPI IDirect3D8_GetAdapterModeCount(Direct3D *pThis, UINT Adapter) { return Direct3D_GetAdapterModeCount(Adapter); }
D3DINLINE HRESULT WINAPI IDirect3D8_EnumAdapterModes(Direct3D *pThis, UINT Adapter, UINT iMode, D3DDISPLAYMODE *pMode) { return Direct3D_EnumAdapterModes(Adapter, iMode, pMode); }
D3DINLINE HRESULT WINAPI IDirect3D8_GetAdapterDisplayMode(Direct3D *pThis, UINT Adapter, D3DDISPLAYMODE *pMode) { return Direct3D_GetAdapterDisplayMode(Adapter, pMode); }
D3DINLINE HRESULT WINAPI IDirect3D8_CheckDeviceType(Direct3D *pThis, UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) { return Direct3D_CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed); }
D3DINLINE HRESULT WINAPI IDirect3D8_CheckDeviceFormat(Direct3D *pThis, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) { return Direct3D_CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat); }
D3DINLINE HRESULT WINAPI IDirect3D8_CheckDeviceMultiSampleType(Direct3D *pThis, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType) { return Direct3D_CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType); }
D3DINLINE HRESULT WINAPI IDirect3D8_CheckDepthStencilMatch(Direct3D *pThis, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) { return Direct3D_CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat); }
D3DINLINE HRESULT WINAPI IDirect3D8_GetDeviceCaps(Direct3D *pThis, UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS8 *pCaps) { return Direct3D_GetDeviceCaps(Adapter, DeviceType, pCaps); }
D3DINLINE HRESULT WINAPI IDirect3D8_CreateDevice(Direct3D *pThis, UINT Adapter, D3DDEVTYPE DeviceType, void *pUnused, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, D3DDevice **ppReturnedDeviceInterface) { return Direct3D_CreateDevice(Adapter, DeviceType, pUnused, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface); }
D3DINLINE HRESULT WINAPI IDirect3D8_SetPushBufferSize(Direct3D *pThis, DWORD PushBufferSize, DWORD KickOffSize) { Direct3D_SetPushBufferSize(PushBufferSize, KickOffSize); return S_OK; }

#ifdef __cplusplus

D3DMINLINE ULONG   WINAPI Direct3D::AddRef() { return Direct3D_AddRef(); }
D3DMINLINE ULONG   WINAPI Direct3D::Release() { return Direct3D_Release(); }
D3DMINLINE UINT    WINAPI Direct3D::GetAdapterCount() { return Direct3D_GetAdapterCount(); }
D3DMINLINE HRESULT WINAPI Direct3D::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier) { return Direct3D_GetAdapterIdentifier(Adapter, Flags, pIdentifier); }
D3DMINLINE UINT    WINAPI Direct3D::GetAdapterModeCount(UINT Adapter) { return Direct3D_GetAdapterModeCount(Adapter); }
D3DMINLINE HRESULT WINAPI Direct3D::EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE *pMode) { return Direct3D_EnumAdapterModes(Adapter, Mode, pMode); }
D3DMINLINE HRESULT WINAPI Direct3D::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode) { return Direct3D_GetAdapterDisplayMode(Adapter, pMode); }
D3DMINLINE HRESULT WINAPI Direct3D::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) { return Direct3D_CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed); }
D3DMINLINE HRESULT WINAPI Direct3D::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) { return Direct3D_CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat); }
D3DMINLINE HRESULT WINAPI Direct3D::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType) { return Direct3D_CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType); }
D3DMINLINE HRESULT WINAPI Direct3D::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) { return Direct3D_CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat); }
D3DMINLINE HRESULT WINAPI Direct3D::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8 *pCaps) { return Direct3D_GetDeviceCaps(Adapter, DeviceType, pCaps); }
D3DMINLINE HRESULT WINAPI Direct3D::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, void *pUnused, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, D3DDevice **ppReturnedDeviceInterface) { return Direct3D_CreateDevice(Adapter, DeviceType, pUnused, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface); }
D3DMINLINE HRESULT WINAPI Direct3D::SetPushBufferSize(DWORD PushBufferSize, DWORD KickOffSize) { Direct3D_SetPushBufferSize(PushBufferSize, KickOffSize); return S_OK; }

#endif __cplusplus

/****************************************************************************
 *
 * D3DDevice_*
 *
 * Private internal interfaces - Please don't access these directly, as they're
 *                               subject to change.
 *
 ****************************************************************************/

D3DSurface*         WINAPI D3DDevice_GetBackBuffer2(INT BackBuffer);
D3DTexture*         WINAPI D3DDevice_CreateTexture2(DWORD Width, DWORD Height, DWORD Depth, DWORD Levels, DWORD Usage, D3DFORMAT Format, D3DRESOURCETYPE D3DType);
D3DSurface*         WINAPI D3DDevice_CreateSurface2(DWORD Width, DWORD Height, DWORD Usage, D3DFORMAT Format);
D3DVertexBuffer*    WINAPI D3DDevice_CreateVertexBuffer2(UINT Length);
D3DIndexBuffer*     WINAPI D3DDevice_CreateIndexBuffer2(UINT Length);
D3DPalette*         WINAPI D3DDevice_CreatePalette2(D3DPALETTESIZE Size);
D3DSurface*         WINAPI D3DDevice_GetRenderTarget2();
D3DSurface*         WINAPI D3DDevice_GetDepthStencilSurface2();
D3DBaseTexture*     WINAPI D3DDevice_GetTexture2(DWORD Stage);
D3DPalette*         WINAPI D3DDevice_GetPalette2(DWORD Stage);
D3DVertexBuffer*    WINAPI D3DDevice_GetStreamSource2(UINT StreamNumber, UINT *pStride);
D3DIndexBuffer*     WINAPI D3DDevice_GetIndices2(UINT *pBaseVertexIndex);
D3DFixup*           WINAPI D3DDevice_CreateFixup2(UINT Size);
D3DPushBuffer*      WINAPI D3DDevice_CreatePushBuffer2(UINT Size, BOOL RunUsingCpuCopy);

ULONG   WINAPI D3DDevice_AddRef();
ULONG   WINAPI D3DDevice_Release();
void    WINAPI D3DDevice_GetDirect3D(Direct3D **ppD3D8);
void    WINAPI D3DDevice_GetDeviceCaps(D3DCAPS8 *pCaps);
void    WINAPI D3DDevice_GetDisplayMode(D3DDISPLAYMODE *pMode);
void    WINAPI D3DDevice_GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters);
HRESULT WINAPI D3DDevice_Reset(D3DPRESENT_PARAMETERS *pPresentationParameters);
void    WINAPI D3DDevice_GetRasterStatus(D3DRASTER_STATUS *pRasterStatus);
void    WINAPI D3DDevice_SetFlickerFilter(DWORD Filter);
void    WINAPI D3DDevice_SetSoftDisplayFilter(BOOL Enable);
void    WINAPI D3DDevice_SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP *pRamp);
void    WINAPI D3DDevice_GetGammaRamp(D3DGAMMARAMP *pRamp);
void    WINAPI D3DDevice_CopyRects(D3DSurface *pSourceSurface, CONST RECT *pSourceRectsArray, UINT cRects, D3DSurface *pDestinationSurface, CONST POINT *pDestPointsArray);
void    WINAPI D3DDevice_SetCopyRectsState(CONST D3DCOPYRECTSTATE *pCopyRectState, CONST D3DCOPYRECTROPSTATE *pCopyRectRopState);
void    WINAPI D3DDevice_GetCopyRectsState(D3DCOPYRECTSTATE *pCopyRectState, D3DCOPYRECTROPSTATE *pCopyRectRopState);
void    WINAPI D3DDevice_SetRenderTarget(D3DSurface *pRenderTarget, D3DSurface *pNewZStencil);
D3DINLINE void    WINAPI D3DDevice_BeginScene() { }
D3DINLINE void    WINAPI D3DDevice_EndScene() { }
void    WINAPI D3DDevice_Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
void    WINAPI D3DDevice_SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
void    WINAPI D3DDevice_GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix);
void    WINAPI D3DDevice_MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
void    WINAPI D3DDevice_SetViewport(CONST D3DVIEWPORT8 *pViewport);
void    WINAPI D3DDevice_GetViewport(D3DVIEWPORT8 *pViewport);
void    WINAPI D3DDevice_SetMaterial(CONST D3DMATERIAL8 *pMaterial);
void    WINAPI D3DDevice_GetMaterial(D3DMATERIAL8 *pMaterial);
void    WINAPI D3DDevice_SetBackMaterial(CONST D3DMATERIAL8 *pMaterial);
void    WINAPI D3DDevice_GetBackMaterial(D3DMATERIAL8 *pMaterial);
HRESULT WINAPI D3DDevice_SetLight(DWORD Index, CONST D3DLIGHT8 *pLight);
void    WINAPI D3DDevice_GetLight(DWORD Index, D3DLIGHT8 *pLight);
HRESULT WINAPI D3DDevice_LightEnable(DWORD Index, BOOL Enable);
void    WINAPI D3DDevice_GetLightEnable(DWORD Index, BOOL *pEnable);
void    WINAPI D3DDevice_SetRenderStateNotInline(D3DRENDERSTATETYPE State, DWORD Value);
HRESULT WINAPI D3DDevice_SetRenderState_ParameterCheck(D3DRENDERSTATETYPE State, DWORD Value);
void    D3DFASTCALL D3DDevice_SetRenderState_Simple(DWORD Method, DWORD Value);
void    WINAPI D3DDevice_SetRenderState_PSTextureModes(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_VertexBlend(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_FogColor(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_FillMode(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_BackFillMode(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_TwoSidedLighting(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_NormalizeNormals(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_ZEnable(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_StencilEnable(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_StencilFail(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_CullMode(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_FrontFace(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_TextureFactor(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_ZBias(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_LogicOp(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_EdgeAntiAlias(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_MultiSampleAntiAlias(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_MultiSampleMask(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_MultiSampleMode(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_MultiSampleRenderTargetMode(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_ShadowFunc(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_LineWidth(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_SampleAlpha(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_Dxt1NoiseEnable(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_YuvEnable(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_OcclusionCullEnable(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_StencilCullEnable(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_RopZCmpAlwaysRead(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_RopZRead(DWORD Value);
void    WINAPI D3DDevice_SetRenderState_DoNotCullUncompressed(DWORD Value);
void    WINAPI D3DDevice_SetTextureStageStateNotInline(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
HRESULT WINAPI D3DDevice_SetTextureState_ParameterCheck(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
void    WINAPI D3DDevice_SetTextureState_BumpEnv(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
void    WINAPI D3DDevice_SetTextureState_TexCoordIndex(DWORD Stage, DWORD Value);
void    WINAPI D3DDevice_SetTextureState_BorderColor(DWORD Stage, DWORD Value);
void    WINAPI D3DDevice_SetTextureState_ColorKeyColor(DWORD Stage, DWORD Value);
#if D3DCOMPILE_BEGINSTATEBLOCK
void    WINAPI D3DDevice_BeginStateBlock();
HRESULT WINAPI D3DDevice_EndStateBlock(DWORD *pToken);
#endif
void    WINAPI D3DDevice_ApplyStateBlock(DWORD Token);
void    WINAPI D3DDevice_CaptureStateBlock(DWORD Token);
void    WINAPI D3DDevice_DeleteStateBlock(DWORD Token);
HRESULT WINAPI D3DDevice_CreateStateBlock(D3DSTATEBLOCKTYPE Type,DWORD *pToken);
void    WINAPI D3DDevice_SetTexture(DWORD Stage, D3DBaseTexture *pTexture);
void    WINAPI D3DDevice_SetPalette(DWORD Stage, D3DPalette *pPalette);
void    WINAPI D3DDevice_DrawVertices(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT VertexCount);
void    WINAPI D3DDevice_DrawIndexedVertices(D3DPRIMITIVETYPE, UINT VertexCount, CONST WORD *pIndexData);
void    WINAPI D3DDevice_DrawVerticesUP(D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
void    WINAPI D3DDevice_DrawIndexedVerticesUP(D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST void *pIndexData, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
void    WINAPI D3DDevice_PrimeVertexCache(UINT VertexCount, CONST WORD *pIndexData);
HRESULT WINAPI D3DDevice_CreateVertexShader(CONST DWORD *pDeclaration, CONST DWORD *pFunction, DWORD *pHandle, DWORD Usage);
void    WINAPI D3DDevice_SetVertexShader(DWORD Handle);
void    WINAPI D3DDevice_GetVertexShader(DWORD *pHandle);
void    WINAPI D3DDevice_DeleteVertexShader(DWORD Handle);
void    D3DFASTCALL D3DDevice_SetVertexShaderConstant1(INT Register, CONST void *pConstantData);
void    D3DFASTCALL D3DDevice_SetVertexShaderConstant1Fast(INT Register, CONST void *pConstantData);
void    D3DFASTCALL D3DDevice_SetVertexShaderConstant4(INT Register, CONST void *pConstantData);
void    D3DFASTCALL D3DDevice_SetVertexShaderConstantNotInline(INT Register, CONST void *pConstantData, DWORD ConstantCount);
void    D3DFASTCALL D3DDevice_SetVertexShaderConstantNotInlineFast(INT Register, CONST void *pConstantData, DWORD ConstantCount);
void    WINAPI D3DDevice_GetVertexShaderConstant(INT Register, void *pConstantData, DWORD ConstantCount);
void    WINAPI D3DDevice_SetShaderConstantMode(D3DSHADERCONSTANTMODE Mode);
void    WINAPI D3DDevice_GetShaderConstantMode(D3DSHADERCONSTANTMODE *pMode);
void    WINAPI D3DDevice_LoadVertexShader(DWORD Handle, DWORD Address);
void    WINAPI D3DDevice_LoadVertexShaderProgram(CONST DWORD *pFunction, DWORD Address);
void    WINAPI D3DDevice_SelectVertexShader(DWORD Handle, DWORD Address);
void    WINAPI D3DDevice_SelectVertexShaderDirect(D3DVERTEXATTRIBUTEFORMAT *pVAF, DWORD Address);
void    WINAPI D3DDevice_RunVertexStateShader(DWORD Address, CONST float *pData);
void    WINAPI D3DDevice_GetVertexShaderSize(DWORD Handle, UINT* pSize);
void    WINAPI D3DDevice_GetVertexShaderType(DWORD Handle, DWORD* pType);
HRESULT WINAPI D3DDevice_GetVertexShaderDeclaration(DWORD Handle, void *pData, DWORD *pSizeOfData);
HRESULT WINAPI D3DDevice_GetVertexShaderFunction(DWORD Handle,void *pData, DWORD *pSizeOfData);
void    WINAPI D3DDevice_SetStreamSource(UINT StreamNumber, D3DVertexBuffer *pStreamData, UINT Stride);
void    WINAPI D3DDevice_SetIndices(D3DIndexBuffer* pIndexData, UINT BaseVertexIndex);
void    WINAPI D3DDevice_CreatePixelShader(CONST D3DPIXELSHADERDEF *pPSDef, DWORD *pHandle);
void    WINAPI D3DDevice_SetPixelShader(DWORD Handle);
void    WINAPI D3DDevice_SetPixelShaderProgram(CONST D3DPIXELSHADERDEF *pPSDef);
void    WINAPI D3DDevice_GetPixelShader(DWORD *pHandle);
void    WINAPI D3DDevice_DeletePixelShader(DWORD Handle);
void    WINAPI D3DDevice_SetPixelShaderConstant(DWORD Register, CONST void *pConstantData, DWORD ConstantCount);
void    WINAPI D3DDevice_GetPixelShaderConstant(DWORD Register, void *pConstantData, DWORD ConstantCount);
void    WINAPI D3DDevice_GetPixelShaderFunction(DWORD Handle, D3DPIXELSHADERDEF *pData);
HRESULT WINAPI D3DDevice_DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo);
HRESULT WINAPI D3DDevice_DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo);
void    WINAPI D3DDevice_DeletePatch(UINT Handle);
void    WINAPI D3DDevice_UpdateOverlay(D3DSurface *pSurface, CONST RECT *SrcRect, CONST RECT *DstRect, BOOL EnableColorKey, D3DCOLOR ColorKey);
void    WINAPI D3DDevice_EnableOverlay(BOOL Enable);
void    WINAPI D3DDevice_BeginVisibilityTest();
HRESULT WINAPI D3DDevice_EndVisibilityTest(DWORD Index);
HRESULT WINAPI D3DDevice_GetVisibilityTestResult(DWORD Index, UINT *pResult, ULONGLONG *pTimeStamp);
BOOL    WINAPI D3DDevice_IsBusy();
void    WINAPI D3DDevice_BlockUntilIdle();
void    WINAPI D3DDevice_KickPushBuffer();
void    WINAPI D3DDevice_SetVerticalBlankCallback(D3DVBLANKCALLBACK pCallback);
void    WINAPI D3DDevice_SetSwapCallback(D3DSWAPCALLBACK pCallback);
void    WINAPI D3DDevice_BlockUntilVerticalBlank();
DWORD   WINAPI D3DDevice_InsertFence();
BOOL    WINAPI D3DDevice_IsFencePending(DWORD Fence);
VOID    WINAPI D3DDevice_BlockOnFence(DWORD Fence);
void    WINAPI D3DDevice_InsertCallback(D3DCALLBACKTYPE Type, D3DCALLBACK pCallback, DWORD Context);
void    WINAPI D3DDevice_FlushVertexCache();
HRESULT WINAPI D3DDevice_PersistDisplay();
D3DSurface* WINAPI D3DDevice_GetPersistedSurface2();
BOOL    WINAPI D3DDevice_GetOverlayUpdateStatus();
void    WINAPI D3DDevice_GetDisplayFieldStatus(D3DFIELD_STATUS *pFieldStatus);
void    WINAPI D3DDevice_SetVertexData2f(INT Register, float a, float b);
void    WINAPI D3DDevice_SetVertexData4f(INT Register, float a, float b, float c, float d);
void    WINAPI D3DDevice_SetVertexData2s(INT Register, SHORT a, SHORT b);
void    WINAPI D3DDevice_SetVertexData4s(INT Register, SHORT a, SHORT b, SHORT c, SHORT d);
void    WINAPI D3DDevice_SetVertexData4ub(INT Register, BYTE a, BYTE b, BYTE c, BYTE d);
void    WINAPI D3DDevice_SetVertexDataColor(INT Register, D3DCOLOR Color);
void    WINAPI D3DDevice_Begin(D3DPRIMITIVETYPE PrimitiveType);
void    WINAPI D3DDevice_End();
void    WINAPI D3DDevice_BeginPushBuffer(D3DPushBuffer *pPushBuffer);
HRESULT WINAPI D3DDevice_EndPushBuffer();
void    WINAPI D3DDevice_RunPushBuffer(D3DPushBuffer *pPushBuffer, D3DFixup *pFixup);
void    WINAPI D3DDevice_GetPushBufferOffset(DWORD* pOffset);
void    WINAPI D3DDevice_Nop();
void    WINAPI D3DDevice_GetProjectionViewportMatrix(D3DMATRIX* pProjectionViewport);
void    WINAPI D3DDevice_SetModelView(CONST D3DMATRIX* pModelView, CONST D3DMATRIX* pInverseModelView, CONST D3DMATRIX* pComposite);
HRESULT WINAPI D3DDevice_GetModelView(D3DMATRIX* pModelView);
void    WINAPI D3DDevice_SetVertexBlendModelView(UINT Count, CONST D3DMATRIX* pModelViews, CONST D3DMATRIX* pInverseModelViews, CONST D3DMATRIX* pProjectionViewport);
HRESULT WINAPI D3DDevice_GetVertexBlendModelView(UINT Count, D3DMATRIX* pModelViews, D3DMATRIX* pProjectionViewport);
void    WINAPI D3DDevice_SetVertexShaderInput(DWORD Handle, UINT StreamCount, CONST D3DSTREAM_INPUT* pStreamInputs);
void    WINAPI D3DDevice_SetVertexShaderInputDirect(D3DVERTEXATTRIBUTEFORMAT *pVAF, UINT StreamCount, CONST D3DSTREAM_INPUT* pStreamInputs);
HRESULT WINAPI D3DDevice_GetVertexShaderInput(DWORD* pHandle, UINT* pStreamCount, D3DSTREAM_INPUT* pStreamInputs);
void    D3DFASTCALL D3DDevice_SwitchTexture(DWORD Method, DWORD Data, DWORD Format);
void    WINAPI D3DDevice_SetScissors(DWORD Count, BOOL Exclusive, CONST D3DRECT *pRects);
void    WINAPI D3DDevice_GetScissors(DWORD* pCount, BOOL *pExclusive, D3DRECT *pRects);
void    WINAPI D3DDevice_SetTile(DWORD Index, CONST D3DTILE* pTile);
void    WINAPI D3DDevice_GetTile(DWORD Index, D3DTILE* pTile);
DWORD   WINAPI D3DDevice_GetTileCompressionTags(DWORD ZStartTag, DWORD ZEndTag);
void    WINAPI D3DDevice_SetTileCompressionTagBits(DWORD Partition, DWORD Address, CONST DWORD *pData, DWORD Count);
void    WINAPI D3DDevice_GetTileCompressionTagBits(DWORD Partition, DWORD Address, DWORD *pData, DWORD Count);
DWORD*  WINAPI D3DDevice_BeginPush(DWORD Count);
void    WINAPI D3DDevice_EndPush(DWORD *pPush);
DWORD   WINAPI D3DDevice_Swap(DWORD Flags);
void    WINAPI D3DDevice_SetBackBufferScale(float x, float y);
void    WINAPI D3DDevice_GetBackBufferScale(float *pX, float *pY);
void    WINAPI D3DDevice_SetScreenSpaceOffset(float x, float y);
void    WINAPI D3DDevice_GetScreenSpaceOffset(float *pX, float *pY);
void    WINAPI D3DDevice_SetOverscanColor(D3DCOLOR Color);
D3DCOLOR WINAPI D3DDevice_GetOverscanColor();
void    WINAPI D3DDevice_SetDepthClipPlanes(float Near, float Far, DWORD Flags);
void    WINAPI D3DDevice_GetDepthClipPlanes(float *pNear, float *pFar, DWORD Flags);
void    WINAPI D3DDevice_GetViewportOffsetAndScale(D3DVECTOR4 *pOffset, D3DVECTOR4 *pScale);
void    WINAPI D3DDevice_SetStipple(CONST DWORD *pPattern);
void    WINAPI D3DDevice_GetStipple(DWORD *pPattern);
void    WINAPI D3DDevice_SetWaitCallback(D3DWAITCALLBACK pCallback);
DWORD   WINAPI D3DDevice_GetPushDistance(DWORD Handle);
HRESULT WINAPI D3DDevice_SetTimerCallback(ULONGLONG Time, D3DCALLBACK pCallback, DWORD Context);
DWORD*  WINAPI D3DDevice_MakeSpace();
void    WINAPI D3DDevice_BeginStateParameterCheck(DWORD Count);
void    WINAPI D3DDevice_EndStateParameterCheck(DWORD *pPush);
DWORD*  WINAPI D3DDevice_BeginStateBig(DWORD Count);
void    WINAPI D3DDevice_SetRenderTargetFast(D3DSurface *pRenderTarget, D3DSurface *pNewZStencil, DWORD Flags);

#ifdef _DEBUG
DWORD   WINAPI D3DDevice_SetDebugMarker(DWORD Marker);
DWORD   WINAPI D3DDevice_GetDebugMarker();
#else
D3DINLINE DWORD   WINAPI D3DDevice_SetDebugMarker(DWORD Marker) { return 0; }
D3DINLINE DWORD   WINAPI D3DDevice_GetDebugMarker() { return 0; }
#endif

D3DINLINE void D3DDevice_GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
    *pValue = D3D__RenderState[State];
}
D3DINLINE void D3DDevice_GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
    *pValue = D3D__TextureState[Stage][Type];
}

#if D3DCOMPILE_NOTINLINE

    D3DINLINE void D3DDevice_SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
    {
        D3DDevice_SetRenderStateNotInline(State, Value);
    }
    D3DINLINE void D3DDevice_SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
    {
        D3DDevice_SetTextureStageStateNotInline(Stage, Type, Value);
    }

#else

    // This D3DINLINE version of SetRenderState has the nice advantage that it
    // essentially moves to compile-time the big 'switch' statement for
    // handling all the render state types.  When given a constant value for
    // 'State', all of these 'if's get nicely compiled away when compiler
    // optimizations are enabled.  We can't use a static call-table because the
    // compiler cannot remove the indirect.
    //
    // If you're calling SetRenderState with a non-constant value for 'State',
    // it's better to call SetRenderStateNotInline (it will reduce code bloat
    // and be a bit faster because the non-D3DINLINE version uses a call-table).
    //
    // Please don't call any of the D3DDevice_ routines directly; they're
    // subject to change.
    //
    D3DINLINE void D3DDevice_SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
    {
        #ifdef _DEBUG
        if (D3DDevice_SetRenderState_ParameterCheck(State, Value) != S_OK)
            return;
        #endif

        if (State < D3DRS_SIMPLE_MAX)
        {
            // D3DCONST arrays like D3DSIMPLERENDERSTATEENCODE have the nice
            // property that the lookup of a constant index is done at compile-
            // time, not run-time.
            //
            D3DDevice_SetRenderState_Simple(D3DSIMPLERENDERSTATEENCODE[State], Value);
            #if !D3DCOMPILE_PUREDEVICE
            D3D__RenderState[State] = Value;
            #endif
        }
        else if (State < D3DRS_DEFERRED_MAX)
        {
            D3D__DirtyFlags |= D3DDIRTYFROMRENDERSTATE[State - D3DRS_SIMPLE_MAX];
            D3D__RenderState[State] = Value;
        }
        else if (State == D3DRS_PSTEXTUREMODES)
        {
            D3DDevice_SetRenderState_PSTextureModes(Value);
        }
        else if (State == D3DRS_VERTEXBLEND)
        {
            D3DDevice_SetRenderState_VertexBlend(Value);
        }
        else if (State == D3DRS_FOGCOLOR)
        {
            D3DDevice_SetRenderState_FogColor(Value);
        }
        else if (State == D3DRS_FILLMODE)
        {
            D3DDevice_SetRenderState_FillMode(Value);
        }
        else if (State == D3DRS_BACKFILLMODE)
        {
            D3DDevice_SetRenderState_BackFillMode(Value);
        }
        else if (State == D3DRS_TWOSIDEDLIGHTING)
        {
            D3DDevice_SetRenderState_TwoSidedLighting(Value);
        }
        else if (State == D3DRS_NORMALIZENORMALS)
        {
            D3DDevice_SetRenderState_NormalizeNormals(Value);
        }
        else if (State == D3DRS_ZENABLE)
        {
            D3DDevice_SetRenderState_ZEnable(Value);
        }
        else if (State == D3DRS_STENCILENABLE)
        {
            D3DDevice_SetRenderState_StencilEnable(Value);
        }
        else if (State == D3DRS_STENCILFAIL)
        {
            D3DDevice_SetRenderState_StencilFail(Value);
        }
        else if (State == D3DRS_CULLMODE)
        {
            D3DDevice_SetRenderState_CullMode(Value);
        }
        else if (State == D3DRS_FRONTFACE)
        {
            D3DDevice_SetRenderState_FrontFace(Value);
        }
        else if (State == D3DRS_TEXTUREFACTOR)
        {
            D3DDevice_SetRenderState_TextureFactor(Value);
        }
        else if (State == D3DRS_ZBIAS)
        {
            D3DDevice_SetRenderState_ZBias(Value);
        }
        else if (State == D3DRS_LOGICOP)
        {
            D3DDevice_SetRenderState_LogicOp(Value);
        }
        else if (State == D3DRS_EDGEANTIALIAS)
        {
            D3DDevice_SetRenderState_EdgeAntiAlias(Value);
        }
        else if (State == D3DRS_MULTISAMPLEANTIALIAS)
        {
            D3DDevice_SetRenderState_MultiSampleAntiAlias(Value);
        }
        else if (State == D3DRS_MULTISAMPLEMASK)
        {
            D3DDevice_SetRenderState_MultiSampleMask(Value);
        }
        else if (State == D3DRS_MULTISAMPLEMODE)
        {
            D3DDevice_SetRenderState_MultiSampleMode(Value);
        }
        else if (State == D3DRS_MULTISAMPLERENDERTARGETMODE)
        {
            D3DDevice_SetRenderState_MultiSampleRenderTargetMode(Value);
        }
        else if (State == D3DRS_SHADOWFUNC)
        {
            D3DDevice_SetRenderState_ShadowFunc(Value);
        }
        else if (State == D3DRS_LINEWIDTH)
        {
            D3DDevice_SetRenderState_LineWidth(Value);
        }
        else if (State == D3DRS_SAMPLEALPHA)
        {
            D3DDevice_SetRenderState_SampleAlpha(Value);
        }
        else if (State == D3DRS_DXT1NOISEENABLE)
        {
            D3DDevice_SetRenderState_Dxt1NoiseEnable(Value);
        }
        else if (State == D3DRS_YUVENABLE)
        {
            D3DDevice_SetRenderState_YuvEnable(Value);
        }
        else if (State == D3DRS_OCCLUSIONCULLENABLE)
        {
            D3DDevice_SetRenderState_OcclusionCullEnable(Value);
        }
        else if (State == D3DRS_STENCILCULLENABLE)
        {
            D3DDevice_SetRenderState_StencilCullEnable(Value);
        }
        else if (State == D3DRS_ROPZCMPALWAYSREAD)
        {
            D3DDevice_SetRenderState_RopZCmpAlwaysRead(Value);
        }
        else if (State == D3DRS_ROPZREAD)
        {
            D3DDevice_SetRenderState_RopZRead(Value);
        }
        else if (State == D3DRS_DONOTCULLUNCOMPRESSED)
        {
            D3DDevice_SetRenderState_DoNotCullUncompressed(Value);
        }
    }

    // As above, but for SetTextureStageState:

    D3DINLINE void D3DDevice_SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
    {
        #ifdef _DEBUG
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, Type, Value) != S_OK)
            return;
        #endif

        if (Type < D3DTSS_DEFERRED_TEXTURE_STATE_MAX)
        {
            D3D__DirtyFlags |= (D3DDIRTYFLAG_TEXTURE_STATE_0 << Stage);
            D3D__TextureState[Stage][Type] = Value;
        }
        else if (Type < D3DTSS_DEFERRED_MAX)
        {
            D3D__DirtyFlags |= D3DDIRTYFROMTEXTURESTATE[Type];
            D3D__TextureState[Stage][Type] = Value;
        }
        else if (Type == D3DTSS_TEXCOORDINDEX)
        {
            D3DDevice_SetTextureState_TexCoordIndex(Stage, Value);
        }
        else if (Type == D3DTSS_BORDERCOLOR)
        {
            D3DDevice_SetTextureState_BorderColor(Stage, Value);
        }
        else if (Type == D3DTSS_COLORKEYCOLOR)
        {
            D3DDevice_SetTextureState_ColorKeyColor(Stage, Value);
        }
        else if ((Type >= D3DTSS_BUMPENVMAT00) && (Type <= D3DTSS_BUMPENVLOFFSET))
        {
            D3DDevice_SetTextureState_BumpEnv(Stage, Type, Value);
        }
    }

#endif

D3DINLINE void D3DDevice_SetVertexShaderConstant(INT Register, CONST void *pConstantData, DWORD ConstantCount)
{
    if (ConstantCount == 1)
    {
        D3DDevice_SetVertexShaderConstant1(Register + 96, pConstantData);
    }
    else if (ConstantCount == 4)
    {
        D3DDevice_SetVertexShaderConstant4(Register + 96, pConstantData);
    }
    else
    {
        D3DDevice_SetVertexShaderConstantNotInline(Register + 96, pConstantData, 4 * ConstantCount);
    }
}

D3DINLINE void D3DDevice_SetVertexShaderConstantFast(INT Register, CONST void *pConstantData, DWORD ConstantCount)
{
    if (ConstantCount == 1)
    {
        D3DDevice_SetVertexShaderConstant1Fast(Register + 96, pConstantData);
    }
    else
    {
        D3DDevice_SetVertexShaderConstantNotInlineFast(Register + 96, pConstantData, 4 * ConstantCount);
    }
}

// For constant values of 'Count', D3DDevice_BeginState compiles into a 'mov', 
// 'cmp', and 'jb' for the common case:

D3DINLINE DWORD* D3DDevice_BeginState(DWORD Count)
{
    DWORD* pPush;
    
    if (Count < 128)
    {
        pPush = D3D__Device[D3DDEVICE_PUT / 4];
        if (pPush >= D3D__Device[D3DDEVICE_THRESHOLD / 4])
            pPush = D3DDevice_MakeSpace();
    }
    else
    {
        pPush = D3DDevice_BeginStateBig(Count);
    }

    #ifdef _DEBUG
    D3DDevice_BeginStateParameterCheck(Count);
    #endif

    return pPush;
}

D3DINLINE void D3DDevice_EndState(DWORD* pPush)
{
    #ifdef _DEBUG
    D3DDevice_EndStateParameterCheck(pPush);
    #endif
    
    D3D__Device[D3DDEVICE_PUT / 4] = pPush;
}

// Compatibility wrappers.

D3DINLINE ULONG   WINAPI IDirect3DDevice8_AddRef(D3DDevice *pThis) { return D3DDevice_AddRef(); }
D3DINLINE ULONG   WINAPI IDirect3DDevice8_Release(D3DDevice *pThis) { return D3DDevice_Release(); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetDirect3D(D3DDevice *pThis, Direct3D **ppD3D8) { D3DDevice_GetDirect3D(ppD3D8); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetDeviceCaps(D3DDevice *pThis, D3DCAPS8 *pCaps) { D3DDevice_GetDeviceCaps(pCaps); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetDisplayMode(D3DDevice *pThis, D3DDISPLAYMODE *pMode) { D3DDevice_GetDisplayMode(pMode); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetCreationParameters(D3DDevice *pThis, D3DDEVICE_CREATION_PARAMETERS *pParameters) { D3DDevice_GetCreationParameters(pParameters); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_Reset(D3DDevice *pThis, D3DPRESENT_PARAMETERS *pPresentationParameters) { return D3DDevice_Reset(pPresentationParameters); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_Present(D3DDevice *pThis, CONST RECT *pSourceRect, CONST RECT *pDestRect, void *pUnused, void *pUnused2) { D3DDevice_Swap(0); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetBackBuffer(D3DDevice *pThis, INT BackBuffer, D3DBACKBUFFER_TYPE UnusedType, D3DSurface **ppBackBuffer) { *ppBackBuffer = D3DDevice_GetBackBuffer2(BackBuffer); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetRasterStatus(D3DDevice *pThis, D3DRASTER_STATUS *pRasterStatus) { D3DDevice_GetRasterStatus(pRasterStatus); return S_OK; }
D3DINLINE void    WINAPI IDirect3DDevice8_SetFlickerFilter(D3DDevice *pThis, DWORD Filter) { D3DDevice_SetFlickerFilter(Filter); }
D3DINLINE void    WINAPI IDirect3DDevice8_SetSoftDisplayFilter(D3DDevice *pThis, BOOL Enable) { D3DDevice_SetSoftDisplayFilter(Enable); }
D3DINLINE void    WINAPI IDirect3DDevice8_SetGammaRamp(D3DDevice *pThis, DWORD Flags, CONST D3DGAMMARAMP *pRamp) { D3DDevice_SetGammaRamp(Flags, pRamp); }
D3DINLINE void    WINAPI IDirect3DDevice8_GetGammaRamp(D3DDevice *pThis, D3DGAMMARAMP *pRamp) { D3DDevice_GetGammaRamp(pRamp); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreateTexture(D3DDevice *pThis, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DTexture **ppTexture) { return (*ppTexture = D3DDevice_CreateTexture2(Width, Height, 1, Levels, Usage, Format, D3DRTYPE_TEXTURE)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreateVolumeTexture(D3DDevice *pThis, UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DVolumeTexture **ppVolumeTexture) { return (*ppVolumeTexture = (D3DVolumeTexture*) D3DDevice_CreateTexture2(Width, Height, Depth, Levels, Usage, Format, D3DRTYPE_VOLUMETEXTURE)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreateCubeTexture(D3DDevice *pThis, UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DCubeTexture **ppCubeTexture) { return (*ppCubeTexture = (D3DCubeTexture*) D3DDevice_CreateTexture2(EdgeLength, EdgeLength, 1, Levels, Usage, Format, D3DRTYPE_CUBETEXTURE)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreateVertexBuffer(D3DDevice *pThis, UINT Length, DWORD UnusedUsage, DWORD UnusedFVF, D3DPOOL UnusedPool, D3DVertexBuffer **ppVertexBuffer) { return (*ppVertexBuffer = D3DDevice_CreateVertexBuffer2(Length)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreateIndexBuffer(D3DDevice *pThis, UINT Length, DWORD UnusedUsage, D3DFORMAT UnusedFormat, D3DPOOL UnusedPool, D3DIndexBuffer **ppIndexBuffer) { return (*ppIndexBuffer = D3DDevice_CreateIndexBuffer2(Length)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreatePalette(D3DDevice *pThis, D3DPALETTESIZE Size, D3DPalette **ppPalette) { return (*ppPalette = D3DDevice_CreatePalette2(Size)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreateRenderTarget(D3DDevice *pThis, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE UnusedMultiSample, BOOL UnusedLockable, D3DSurface **ppSurface) { return (*ppSurface = D3DDevice_CreateSurface2(Width, Height, D3DUSAGE_RENDERTARGET, Format)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreateDepthStencilSurface(D3DDevice *pThis, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE UnusedMultiSample, D3DSurface **ppSurface) { return (*ppSurface = D3DDevice_CreateSurface2(Width, Height, D3DUSAGE_DEPTHSTENCIL, Format)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreateImageSurface(D3DDevice *pThis, UINT Width, UINT Height, D3DFORMAT Format, D3DSurface **ppSurface) { return (*ppSurface = D3DDevice_CreateSurface2(Width, Height, 0, Format)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CopyRects(D3DDevice *pThis, D3DSurface *pSourceSurface, CONST RECT *pSourceRectsArray, UINT cRects, D3DSurface *pDestinationSurface, CONST POINT *pDestPointsArray) { D3DDevice_CopyRects(pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface, pDestPointsArray); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetCopyRectsState(CONST D3DCOPYRECTSTATE *pCopyRectState, CONST D3DCOPYRECTROPSTATE *pCopyRectRopState) { D3DDevice_SetCopyRectsState(pCopyRectState, pCopyRectRopState); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetCopyRectsState(D3DCOPYRECTSTATE *pCopyRectState, D3DCOPYRECTROPSTATE *pCopyRectRopState) { D3DDevice_GetCopyRectsState(pCopyRectState, pCopyRectRopState); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetRenderTarget(D3DDevice *pThis, D3DSurface *pRenderTarget, D3DSurface *pNewZStencil) { D3DDevice_SetRenderTarget(pRenderTarget, pNewZStencil); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetRenderTarget(D3DDevice *pThis, D3DSurface **ppRenderTarget) { *ppRenderTarget = D3DDevice_GetRenderTarget2(); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetDepthStencilSurface(D3DDevice *pThis, D3DSurface **ppZStencilSurface) { return (*ppZStencilSurface = D3DDevice_GetDepthStencilSurface2()) != NULL ? S_OK : D3DERR_NOTFOUND; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_BeginScene(D3DDevice *pThis) { D3DDevice_BeginScene(); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_EndScene(D3DDevice *pThis) { D3DDevice_EndScene(); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_Clear(D3DDevice *pThis, DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) { D3DDevice_Clear(Count, pRects, Flags, Color, Z, Stencil); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetTransform(D3DDevice *pThis, D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix) { D3DDIRTY_TRANSFORM(State); D3DDevice_SetTransform(State, pMatrix); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetTransform(D3DDevice *pThis, D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix) { D3DDevice_GetTransform(State, pMatrix); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_MultiplyTransform(D3DDevice *pThis, D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix) { D3DDevice_MultiplyTransform(State, pMatrix); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetViewport(D3DDevice *pThis, CONST D3DVIEWPORT8 *pViewport) { D3DDIRTY_VIEWPORT(); D3DDevice_SetViewport(pViewport); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetViewport(D3DDevice *pThis, D3DVIEWPORT8 *pViewport) { D3DDevice_GetViewport(pViewport); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetMaterial(D3DDevice *pThis, CONST D3DMATERIAL8 *pMaterial) { D3DDIRTY_MATERIAL(); D3DDevice_SetMaterial(pMaterial); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetMaterial(D3DDevice *pThis, D3DMATERIAL8 *pMaterial) { D3DDevice_GetMaterial(pMaterial); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetBackMaterial(D3DDevice *pThis, CONST D3DMATERIAL8 *pMaterial) { D3DDIRTY_BACKMATERIAL(); D3DDevice_SetBackMaterial(pMaterial); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetBackMaterial(D3DDevice *pThis, D3DMATERIAL8 *pMaterial) { D3DDevice_GetBackMaterial(pMaterial); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetLight(D3DDevice *pThis, DWORD Index, CONST D3DLIGHT8 *pLight) { return D3DDevice_SetLight(Index, pLight); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetLight(D3DDevice *pThis, DWORD Index, D3DLIGHT8 *pLight) { D3DDevice_GetLight(Index, pLight); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_LightEnable(D3DDevice *pThis, DWORD Index, BOOL Enable) { return D3DDevice_LightEnable(Index, Enable); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetLightEnable(D3DDevice *pThis, DWORD Index, BOOL *pEnable) { D3DDevice_GetLightEnable(Index, pEnable); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetRenderState(D3DDevice *pThis, D3DRENDERSTATETYPE State, DWORD Value) { D3DDIRTY_RENDERSTATE(State); D3DDevice_SetRenderState(State, Value); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetRenderStateNotInline(D3DDevice *pThis, D3DRENDERSTATETYPE State, DWORD Value) { D3DDIRTY_RENDERSTATE(State); D3DDevice_SetRenderStateNotInline(State, Value); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetRenderState(D3DDevice *pThis, D3DRENDERSTATETYPE State, DWORD *pValue) { D3DDevice_GetRenderState(State, pValue); return S_OK; }
#if D3DCOMPILE_BEGINSTATEBLOCK
D3DINLINE HRESULT WINAPI IDirect3DDevice8_BeginStateBlock(D3DDevice *pThis) { D3DDevice_BeginStateBlock(); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_EndStateBlock(D3DDevice *pThis, DWORD *pToken) { return D3DDevice_EndStateBlock(pToken); }
#endif
D3DINLINE HRESULT WINAPI IDirect3DDevice8_ApplyStateBlock(D3DDevice *pThis, DWORD Token) { D3DDevice_ApplyStateBlock(Token); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CaptureStateBlock(D3DDevice *pThis, DWORD Token) { D3DDevice_CaptureStateBlock(Token); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DeleteStateBlock(D3DDevice *pThis, DWORD Token) { D3DDevice_DeleteStateBlock(Token); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreateStateBlock(D3DDevice *pThis, D3DSTATEBLOCKTYPE Type,DWORD *pToken) { return D3DDevice_CreateStateBlock(Type, pToken); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetTexture(D3DDevice *pThis, DWORD Stage, D3DBaseTexture **ppTexture) { *ppTexture = D3DDevice_GetTexture2(Stage); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetTexture(D3DDevice *pThis, DWORD Stage, D3DBaseTexture *pTexture) { D3DDIRTY_TEXTURE(Stage); D3DDevice_SetTexture(Stage, pTexture); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetPalette(D3DDevice *pThis, DWORD Stage, D3DPalette **ppPalette) { *ppPalette = D3DDevice_GetPalette2(Stage); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetPalette(D3DDevice *pThis, DWORD Stage, D3DPalette *pPalette) { D3DDevice_SetPalette(Stage, pPalette); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetTextureStageState(D3DDevice *pThis, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue) { D3DDevice_GetTextureStageState(Stage, Type, pValue); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetTextureStageState(D3DDevice *pThis, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) { D3DDIRTY_TEXTURESTATE(Stage, Type); D3DDevice_SetTextureStageState(Stage, Type, Value); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetTextureStageStateNotInline(D3DDevice *pThis, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) { D3DDIRTY_TEXTURESTATE(Stage, Type); D3DDevice_SetTextureStageStateNotInline(Stage, Type, Value); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DrawPrimitive(D3DDevice *pThis, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) { D3DDevice_DrawVertices(PrimitiveType, StartVertex, D3DVERTEXCOUNT(PrimitiveType, PrimitiveCount)); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DrawIndexedPrimitive(D3DDevice *pThis, D3DPRIMITIVETYPE PrimitiveType, UINT UnusedMinIndex, UINT UnusedNumIndices, UINT StartIndex, UINT PrimitiveCount) { D3DDevice_DrawIndexedVertices(PrimitiveType, D3DVERTEXCOUNT(PrimitiveType, PrimitiveCount), D3D__IndexData + StartIndex); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DrawPrimitiveUP(D3DDevice *pThis, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride) { D3DDevice_DrawVerticesUP(PrimitiveType, D3DVERTEXCOUNT(PrimitiveType, PrimitiveCount), pVertexStreamZeroData, VertexStreamZeroStride); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DrawIndexedPrimitiveUP(D3DDevice *pThis, D3DPRIMITIVETYPE PrimitiveType, UINT UnusedMinIndex, UINT UnusedNumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT UnusedIndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride) { D3DDevice_DrawIndexedVerticesUP(PrimitiveType, D3DVERTEXCOUNT(PrimitiveType, PrimitiveCount), pIndexData, pVertexStreamZeroData, VertexStreamZeroStride); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DrawVertices(D3DDevice *pThis, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT VertexCount) { D3DDevice_DrawVertices(PrimitiveType, StartVertex, VertexCount); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DrawIndexedVertices(D3DDevice *pThis, D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, WORD *pIndexData) { D3DDevice_DrawIndexedVertices(PrimitiveType, VertexCount, pIndexData); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DrawVerticesUP(D3DDevice *pThis, D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride) { D3DDevice_DrawVerticesUP(PrimitiveType, VertexCount, pVertexStreamZeroData, VertexStreamZeroStride); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DrawIndexedVerticesUP(D3DDevice *pThis, D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST void *pIndexData, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride) { D3DDevice_DrawIndexedVerticesUP(PrimitiveType, VertexCount, pIndexData, pVertexStreamZeroData, VertexStreamZeroStride); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_PrimeVertexCache(D3DDevice *pThis, UINT VertexCount, CONST WORD *pIndexData) { D3DDevice_PrimeVertexCache(VertexCount, pIndexData); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreateVertexShader(D3DDevice *pThis, CONST DWORD *pDeclaration, CONST DWORD *pFunction, DWORD *pHandle, DWORD Usage) { return D3DDevice_CreateVertexShader(pDeclaration, pFunction, pHandle, Usage); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexShader(D3DDevice *pThis, DWORD Handle) { D3DDIRTY_VERTEXSHADER(); D3DDevice_SetVertexShader(Handle); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetVertexShader(D3DDevice *pThis, DWORD *pHandle) { D3DDevice_GetVertexShader(pHandle); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DeleteVertexShader(D3DDevice *pThis, DWORD Handle) { D3DDevice_DeleteVertexShader(Handle); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexShaderConstant(D3DDevice *pThis, INT Register, CONST void *pConstantData, DWORD ConstantCount) { D3DDIRTY_VERTEXSHADERCONSTANT(Register, ConstantCount); D3DDevice_SetVertexShaderConstant(Register, pConstantData, ConstantCount); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexShaderConstantFast(D3DDevice *pThis, INT Register, CONST void *pConstantData, DWORD ConstantCount) { D3DDIRTY_VERTEXSHADERCONSTANT(Register, ConstantCount); D3DDevice_SetVertexShaderConstantFast(Register, pConstantData, ConstantCount); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetVertexShaderConstant(D3DDevice *pThis, INT Register, void *pConstantData, DWORD ConstantCount) { D3DDevice_GetVertexShaderConstant(Register, pConstantData, ConstantCount); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetShaderConstantMode(D3DDevice *pThis, D3DSHADERCONSTANTMODE Mode) { D3DDevice_SetShaderConstantMode(Mode); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetShaderConstantMode(D3DDevice *pThis, D3DSHADERCONSTANTMODE *pMode) { D3DDevice_GetShaderConstantMode(pMode); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_LoadVertexShader(D3DDevice *pThis, DWORD Handle, DWORD Address) { D3DDevice_LoadVertexShader(Handle, Address); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_LoadVertexShaderProgram(D3DDevice *pThis, CONST DWORD *pFunction, DWORD Address) { D3DDevice_LoadVertexShaderProgram(pFunction, Address); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SelectVertexShader(D3DDevice *pThis, DWORD Handle, DWORD Address) { D3DDevice_SelectVertexShader(Handle, Address); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SelectVertexShaderDirect(D3DDevice *pThis, D3DVERTEXATTRIBUTEFORMAT *pVAF, DWORD Address) { D3DDevice_SelectVertexShaderDirect(pVAF, Address); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_RunVertexStateShader(D3DDevice *pThis, DWORD Address, CONST float *pData) { D3DDevice_RunVertexStateShader(Address, pData); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetVertexShaderSize(D3DDevice *pThis, DWORD Handle, UINT* pSize) { D3DDevice_GetVertexShaderSize(Handle, pSize); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetVertexShaderType(D3DDevice *pThis, DWORD Handle, DWORD* pType) { D3DDevice_GetVertexShaderType(Handle, pType); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetVertexShaderDeclaration(D3DDevice *pThis, DWORD Handle, void *pData, DWORD *pSizeOfData) { return D3DDevice_GetVertexShaderDeclaration(Handle, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetVertexShaderFunction(D3DDevice *pThis, DWORD Handle,void *pData, DWORD *pSizeOfData) { return D3DDevice_GetVertexShaderFunction(Handle, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetStreamSource(D3DDevice *pThis, UINT StreamNumber, D3DVertexBuffer *pStreamData, UINT Stride) { D3DDIRTY_STREAM(StreamNumber); D3DDevice_SetStreamSource(StreamNumber, pStreamData, Stride); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetStreamSource(D3DDevice *pThis, UINT StreamNumber, D3DVertexBuffer **ppStreamData, UINT *pStride) { *ppStreamData = D3DDevice_GetStreamSource2(StreamNumber, pStride); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetIndices(D3DDevice *pThis, D3DIndexBuffer* pIndexData, UINT BaseVertexIndex) { D3DDIRTY_INDICES(); D3DDevice_SetIndices(pIndexData, BaseVertexIndex); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetIndices(D3DDevice *pThis, D3DIndexBuffer** ppIndexData, UINT *pBaseVertexIndex) { *ppIndexData = D3DDevice_GetIndices2(pBaseVertexIndex); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreatePixelShader(D3DDevice *pThis, CONST D3DPIXELSHADERDEF *pPSDef, DWORD *pHandle) { D3DDevice_CreatePixelShader(pPSDef, pHandle); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetPixelShader(D3DDevice *pThis, DWORD Handle) { D3DDIRTY_PIXELSHADER(); D3DDevice_SetPixelShader(Handle); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetPixelShaderProgram(D3DDevice *pThis, CONST D3DPIXELSHADERDEF *pPSDef) { D3DDIRTY_PIXELSHADER(); D3DDevice_SetPixelShaderProgram(pPSDef); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetPixelShader(D3DDevice *pThis, DWORD *pHandle) { D3DDevice_GetPixelShader(pHandle); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DeletePixelShader(D3DDevice *pThis, DWORD Handle) { D3DDevice_DeletePixelShader(Handle); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetPixelShaderConstant(D3DDevice *pThis, DWORD Register, CONST void *pConstantData, DWORD ConstantCount) { D3DDIRTY_PIXELSHADERCONSTANT(Register, ConstantCount); D3DDevice_SetPixelShaderConstant(Register, pConstantData, ConstantCount); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetPixelShaderConstant(D3DDevice *pThis, DWORD Register, void *pConstantData, DWORD ConstantCount) { D3DDevice_GetPixelShaderConstant(Register, pConstantData, ConstantCount); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetPixelShaderFunction(D3DDevice *pThis, DWORD Handle, D3DPIXELSHADERDEF *pData) { D3DDevice_GetPixelShaderFunction(Handle, pData); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DrawRectPatch(D3DDevice *pThis, UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo) { return D3DDevice_DrawRectPatch(Handle, pNumSegs, pRectPatchInfo); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DrawTriPatch(D3DDevice *pThis, UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo) { return D3DDevice_DrawTriPatch(Handle, pNumSegs, pTriPatchInfo); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_DeletePatch(D3DDevice *pThis, UINT Handle) { D3DDevice_DeletePatch(Handle); return S_OK; }
D3DINLINE BOOL    WINAPI IDirect3DDevice8_IsBusy(D3DDevice *pThis) { return D3DDevice_IsBusy(); }
D3DINLINE void    WINAPI IDirect3DDevice8_BlockUntilIdle(D3DDevice *pThis) { D3DDevice_BlockUntilIdle(); }
D3DINLINE void    WINAPI IDirect3DDevice8_KickPushBuffer(D3DDevice *pThis) { D3DDevice_KickPushBuffer(); }
D3DINLINE void    WINAPI IDirect3DDevice8_SetVerticalBlankCallback(D3DDevice *pThis, D3DVBLANKCALLBACK pCallback) { D3DDevice_SetVerticalBlankCallback(pCallback); }
D3DINLINE void    WINAPI IDirect3DDevice8_SetSwapCallback(D3DDevice *pThis, D3DSWAPCALLBACK pCallback) { D3DDevice_SetSwapCallback(pCallback); }
D3DINLINE void    WINAPI IDirect3DDevice8_BlockUntilVerticalBlank(D3DDevice *pThis) { D3DDevice_BlockUntilVerticalBlank(); }
D3DINLINE DWORD   WINAPI IDirect3DDevice8_InsertFence(D3DDevice *pThis) { return D3DDevice_InsertFence(); }
D3DINLINE BOOL    WINAPI IDirect3DDevice8_IsFencePending(D3DDevice *pThis, DWORD Fence) { return D3DDevice_IsFencePending(Fence); }
D3DINLINE VOID    WINAPI IDirect3DDevice8_BlockOnFence(D3DDevice *pThis, DWORD Fence) { D3DDevice_BlockOnFence(Fence); }
D3DINLINE void    WINAPI IDirect3DDevice8_InsertCallback(D3DDevice *pThis, D3DCALLBACKTYPE Type, D3DCALLBACK pCallback, DWORD Context) { D3DDevice_InsertCallback(Type, pCallback, Context); }
D3DINLINE void    WINAPI IDirect3DDevice8_FlushVertexCache(D3DDevice *pThis) { D3DDevice_FlushVertexCache(); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_PersistDisplay(D3DDevice *pThis) { return D3DDevice_PersistDisplay(); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetPersistedSurface(D3DDevice *pThis, IDirect3DSurface8 **ppSurface) { *ppSurface = D3DDevice_GetPersistedSurface2(); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_UpdateOverlay(D3DDevice *pThis, D3DSurface *pSurface, CONST RECT *SrcRect, CONST RECT *DstRect, BOOL EnableColorKey, D3DCOLOR ColorKey) { D3DDevice_UpdateOverlay(pSurface, SrcRect, DstRect, EnableColorKey, ColorKey); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_EnableOverlay(D3DDevice *pThis, BOOL Enable) { D3DDevice_EnableOverlay(Enable); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_BeginVisibilityTest(D3DDevice *pThis) { D3DDevice_BeginVisibilityTest(); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_EndVisibilityTest(D3DDevice *pThis, DWORD Index) { return D3DDevice_EndVisibilityTest(Index); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetVisibilityTestResult(D3DDevice *pThis, DWORD Index, UINT *pResult, ULONGLONG *pTimeStamp) { return D3DDevice_GetVisibilityTestResult(Index, pResult, pTimeStamp); }
D3DINLINE BOOL    WINAPI IDirect3DDevice8_GetOverlayUpdateStatus(D3DDevice *pThis) { return D3DDevice_GetOverlayUpdateStatus(); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetDisplayFieldStatus(D3DDevice *pThis, D3DFIELD_STATUS *pFieldStatus) { D3DDevice_GetDisplayFieldStatus(pFieldStatus); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexData2f(D3DDevice *pThis, INT Register, float a, float b) { D3DDevice_SetVertexData2f(Register, a, b); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexData4f(D3DDevice *pThis, INT Register, float a, float b, float c, float d) { D3DDevice_SetVertexData4f(Register, a, b, c, d); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexData2s(D3DDevice *pThis, INT Register, SHORT a, SHORT b) { D3DDevice_SetVertexData2s(Register, a, b); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexData4s(D3DDevice *pThis, INT Register, SHORT a, SHORT b, SHORT c, SHORT d) { D3DDevice_SetVertexData4s(Register, a, b, c, d); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexData4ub(D3DDevice *pThis, INT Register, BYTE a, BYTE b, BYTE c, BYTE d) { D3DDevice_SetVertexData4ub(Register, a, b, c, d); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexDataColor(D3DDevice *pThis, INT Register, D3DCOLOR Color) { D3DDevice_SetVertexDataColor(Register, Color); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_Begin(D3DDevice *pThis, D3DPRIMITIVETYPE PrimitiveType) { D3DDevice_Begin(PrimitiveType); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_End(D3DDevice *pThis) { D3DDevice_End(); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreateFixup(D3DDevice *pThis, UINT Size, D3DFixup **ppFixup) { return (*ppFixup = D3DDevice_CreateFixup2(Size)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_CreatePushBuffer(D3DDevice *pThis, UINT Size, BOOL RunUsingCpuCopy, D3DPushBuffer** ppPushBuffer) { return (*ppPushBuffer = D3DDevice_CreatePushBuffer2(Size, RunUsingCpuCopy)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_BeginPushBuffer(D3DDevice *pThis, D3DPushBuffer *pPushBuffer) { D3DDevice_BeginPushBuffer(pPushBuffer); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_EndPushBuffer(D3DDevice *pThis) { return D3DDevice_EndPushBuffer(); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_RunPushBuffer(D3DDevice *pThis, D3DPushBuffer* pPushBuffer, D3DFixup *pFixup) { D3DDevice_RunPushBuffer(pPushBuffer, pFixup); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetPushBufferOffset(D3DDevice *pThis, DWORD* pOffset) { D3DDevice_GetPushBufferOffset(pOffset); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_Nop(D3DDevice *pThis) { D3DDevice_Nop(); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetProjectionViewportMatrix(D3DDevice *pThis, D3DMATRIX* pProjectionViewport) { D3DDevice_GetProjectionViewportMatrix(pProjectionViewport); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetModelView(D3DDevice *pThis, CONST D3DMATRIX* pModelView, CONST D3DMATRIX* pInverseModelView, CONST D3DMATRIX* pComposite) { D3DDevice_SetModelView(pModelView, pInverseModelView, pComposite); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetModelView(D3DDevice *pThis, D3DMATRIX* pModelView) { return D3DDevice_GetModelView(pModelView); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexBlendModelView(D3DDevice *pThis, UINT Count, CONST D3DMATRIX* pModelViews, CONST D3DMATRIX* pInverseModelViews, CONST D3DMATRIX* pProjectionViewport) { D3DDevice_SetVertexBlendModelView(Count, pModelViews, pInverseModelViews, pProjectionViewport); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetVertexBlendModelView(D3DDevice *pThis, UINT Count, D3DMATRIX* pModelViews, D3DMATRIX* pProjectionViewport) { return D3DDevice_GetVertexBlendModelView(Count, pModelViews, pProjectionViewport); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexShaderInput(D3DDevice *pThis, DWORD Handle, UINT StreamCount, CONST D3DSTREAM_INPUT* pStreamInputs) { D3DDevice_SetVertexShaderInput(Handle, StreamCount, pStreamInputs); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetVertexShaderInputDirect(D3DDevice *pThis, D3DVERTEXATTRIBUTEFORMAT *pVAF, UINT StreamCount, CONST D3DSTREAM_INPUT* pStreamInputs) { D3DDevice_SetVertexShaderInputDirect(pVAF, StreamCount, pStreamInputs); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetVertexShaderInput(D3DDevice *pThis, DWORD* pHandle, UINT* pStreamCount, D3DSTREAM_INPUT* pStreamInputs) { return D3DDevice_GetVertexShaderInput(pHandle, pStreamCount, pStreamInputs); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SwitchTexture(D3DDevice *pThis, DWORD Stage, D3DBaseTexture *pTexture) { D3DDevice_SwitchTexture(D3DTEXTUREDIRECTENCODE[Stage], (pTexture)->Data, (pTexture)->Format); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetScissors(D3DDevice *pThis, DWORD Count, BOOL Exclusive, CONST D3DRECT *pRects) { D3DDevice_SetScissors(Count, Exclusive, pRects); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetScissors(D3DDevice *pThis, DWORD* pCount, BOOL *pExclusive, D3DRECT *pRects) { D3DDevice_GetScissors(pCount, pExclusive, pRects); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetTile(D3DDevice *pThis, DWORD Index, CONST D3DTILE* pTile) { D3DDevice_SetTile(Index, pTile); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetTile(D3DDevice *pThis, DWORD Index, D3DTILE* pTile) { D3DDevice_GetTile(Index, pTile); return S_OK; }
D3DINLINE DWORD   WINAPI IDirect3DDevice8_GetTileCompressionTags(D3DDevice *pThis, DWORD ZStartTag, DWORD ZEndTag) { return D3DDevice_GetTileCompressionTags(ZStartTag, ZEndTag); }
D3DINLINE void    WINAPI IDirect3DDevice8_SetTileCompressionTagBits(D3DDevice *pThis, DWORD Partition, DWORD Address, CONST DWORD *pData, DWORD Count) { D3DDevice_SetTileCompressionTagBits(Partition, Address, pData, Count); }
D3DINLINE void    WINAPI IDirect3DDevice8_GetTileCompressionTagBits(D3DDevice *pThis, DWORD Partition, DWORD Address, DWORD *pData, DWORD Count) { D3DDevice_GetTileCompressionTagBits(Partition, Address, pData, Count); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_BeginPush(D3DDevice *pThis, DWORD Count, DWORD **ppPush) { *ppPush = D3DDevice_BeginPush(Count); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_EndPush(D3DDevice *pThis, DWORD *pPush) { D3DDevice_EndPush(pPush); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_BeginState(D3DDevice *pThis, DWORD Count, DWORD **ppPush) { *ppPush = D3DDevice_BeginState(Count); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_EndState(D3DDevice *pThis, DWORD *pPush) { D3DDevice_EndState(pPush); return S_OK; }
D3DINLINE DWORD   WINAPI IDirect3DDevice8_Swap(D3DDevice *pThis, DWORD Flags) { return D3DDevice_Swap(Flags); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetBackBufferScale(D3DDevice *pThis, float x, float y) { D3DDevice_SetBackBufferScale(x, y); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetBackBufferScale(D3DDevice *pThis, float *pX, float *pY) { D3DDevice_GetBackBufferScale(pX, pY); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetScreenSpaceOffset(D3DDevice *pThis, float x, float y) { D3DDevice_SetScreenSpaceOffset(x, y); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_GetScreenSpaceOffset(D3DDevice *pThis, float *pX, float *pY) { D3DDevice_GetScreenSpaceOffset(pX, pY); return S_OK; }
D3DINLINE void    WINAPI IDirect3DDevice8_SetOverscanColor(D3DDevice *pThis, D3DCOLOR Color) { D3DDevice_SetOverscanColor(Color); }
D3DINLINE D3DCOLOR WINAPI IDirect3DDevice8_GetOverscanColor(D3DDevice *pThis) { return D3DDevice_GetOverscanColor(); }
D3DINLINE void    WINAPI IDirect3DDevice8_SetDepthClipPlanes(D3DDevice *pThis, float Near, float Far, DWORD Flags) { D3DDevice_SetDepthClipPlanes(Near, Far, Flags); }
D3DINLINE void    WINAPI IDirect3DDevice8_GetDepthClipPlanes(D3DDevice *pThis, float *pNear, float *pFar, DWORD Flags) { D3DDevice_GetDepthClipPlanes(pNear, pFar, Flags); }
D3DINLINE void    WINAPI IDirect3DDevice8_GetViewportOffsetAndScale(D3DDevice *pThis, D3DVECTOR4 *pOffset, D3DVECTOR4 *pScale) { D3DDevice_GetViewportOffsetAndScale(pOffset, pScale); }
D3DINLINE void    WINAPI IDirect3DDevice8_SetStipple(D3DDevice *pThis, CONST DWORD *pPattern) { D3DDevice_SetStipple(pPattern); }
D3DINLINE void    WINAPI IDirect3DDevice8_GetStipple(D3DDevice *pThis, DWORD *pPattern) { D3DDevice_GetStipple(pPattern); }
D3DINLINE void    WINAPI IDirect3DDevice8_SetWaitCallback(D3DDevice *pThis, D3DWAITCALLBACK pCallback) { D3DDevice_SetWaitCallback(pCallback); }
D3DINLINE DWORD   WINAPI IDirect3DDevice8_GetPushDistance(D3DDevice *pThis, DWORD Handle) { return D3DDevice_GetPushDistance(Handle); }
D3DINLINE HRESULT WINAPI IDirect3DDevice8_SetTimerCallback(D3DDevice *pThis, ULONGLONG Time, D3DCALLBACK pCallback, DWORD Context) { return D3DDevice_SetTimerCallback(Time, pCallback, Context); }
D3DINLINE void    WINAPI IDirect3DDevice8_SetRenderTargetFast(D3DSurface *pRenderTarget, D3DSurface *pNewZStencil, DWORD Flags) { D3DDevice_SetRenderTargetFast(pRenderTarget, pNewZStencil, Flags); }

#ifdef _DEBUG
D3DINLINE DWORD   WINAPI IDirect3DDevice8_SetDebugMarker(D3DDevice *pThis, DWORD Marker) { return D3DDevice_SetDebugMarker(Marker); }
D3DINLINE DWORD   WINAPI IDirect3DDevice8_GetDebugMarker(D3DDevice *pThis) { return D3DDevice_GetDebugMarker(); }
#else
D3DINLINE DWORD   WINAPI IDirect3DDevice8_SetDebugMarker(D3DDevice *pThis, DWORD Marker) { return 0; }
D3DINLINE DWORD   WINAPI IDirect3DDevice8_GetDebugMarker(D3DDevice *pThis) { return 0; }
#endif

#ifdef __cplusplus

D3DMINLINE ULONG   WINAPI D3DDevice::AddRef() { return D3DDevice_AddRef(); }
D3DMINLINE ULONG   WINAPI D3DDevice::Release() { return D3DDevice_Release(); }
D3DMINLINE HRESULT WINAPI D3DDevice::GetDirect3D(Direct3D **ppD3D8) { D3DDevice_GetDirect3D(ppD3D8); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetDeviceCaps(D3DCAPS8 *pCaps) { D3DDevice_GetDeviceCaps(pCaps); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetDisplayMode(D3DDISPLAYMODE *pMode) { D3DDevice_GetDisplayMode(pMode); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) { D3DDevice_GetCreationParameters(pParameters); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters) { return D3DDevice_Reset(pPresentationParameters); }
D3DMINLINE HRESULT WINAPI D3DDevice::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, void *pUnused, void *pUnused2) { D3DDevice_Swap(0); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetBackBuffer(INT BackBuffer, D3DBACKBUFFER_TYPE UnusedType, D3DSurface **ppBackBuffer) { *ppBackBuffer = D3DDevice_GetBackBuffer2(BackBuffer); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetRasterStatus(D3DRASTER_STATUS *pRasterStatus) { D3DDevice_GetRasterStatus(pRasterStatus); return S_OK; }
D3DMINLINE void    WINAPI D3DDevice::SetFlickerFilter(DWORD Filter) { D3DDevice_SetFlickerFilter(Filter); }
D3DMINLINE void    WINAPI D3DDevice::SetSoftDisplayFilter(BOOL Enable) { D3DDevice_SetSoftDisplayFilter(Enable); }
D3DMINLINE void    WINAPI D3DDevice::SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP *pRamp) { D3DDevice_SetGammaRamp(Flags, pRamp); }
D3DMINLINE void    WINAPI D3DDevice::GetGammaRamp(D3DGAMMARAMP *pRamp) { D3DDevice_GetGammaRamp(pRamp); }
D3DMINLINE HRESULT WINAPI D3DDevice::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DTexture **ppTexture) { return (*ppTexture = D3DDevice_CreateTexture2(Width, Height, 1, Levels, Usage, Format, D3DRTYPE_TEXTURE)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DVolumeTexture **ppVolumeTexture) { return (*ppVolumeTexture = (D3DVolumeTexture*) D3DDevice_CreateTexture2(Width, Height, Depth, Levels, Usage, Format, D3DRTYPE_VOLUMETEXTURE)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL UnusedPool, D3DCubeTexture **ppCubeTexture) { return (*ppCubeTexture = (D3DCubeTexture*) D3DDevice_CreateTexture2(EdgeLength, EdgeLength, 1, Levels, Usage, Format, D3DRTYPE_CUBETEXTURE)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreateVertexBuffer(UINT Length, DWORD UnusedUsage, DWORD UnusedFVF, D3DPOOL UnusedPool, D3DVertexBuffer **ppVertexBuffer) { return (*ppVertexBuffer = D3DDevice_CreateVertexBuffer2(Length)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreateIndexBuffer(UINT Length, DWORD UnusedUsage, D3DFORMAT UnusedFormat, D3DPOOL UnusedPool, D3DIndexBuffer **ppIndexBuffer) { return (*ppIndexBuffer = D3DDevice_CreateIndexBuffer2(Length)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreatePalette(D3DPALETTESIZE Size, D3DPalette **ppPalette) { return (*ppPalette = D3DDevice_CreatePalette2(Size)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE UnusedMultiSample, BOOL UnusedLockable, D3DSurface **ppSurface) { return (*ppSurface = D3DDevice_CreateSurface2(Width, Height, D3DUSAGE_RENDERTARGET, Format)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE UnusedMultiSample, D3DSurface **ppSurface) { return (*ppSurface = D3DDevice_CreateSurface2(Width, Height, D3DUSAGE_DEPTHSTENCIL, Format)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DSurface **ppSurface) { return (*ppSurface = D3DDevice_CreateSurface2(Width, Height, 0, Format)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DDevice::CopyRects(D3DSurface *pSourceSurface, CONST RECT *pSourceRectsArray, UINT cRects, D3DSurface *pDestinationSurface, CONST POINT *pDestPointsArray) { D3DDevice_CopyRects(pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface, pDestPointsArray); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetCopyRectsState(CONST D3DCOPYRECTSTATE *pCopyRectState, CONST D3DCOPYRECTROPSTATE *pCopyRectRopState) { D3DDevice_SetCopyRectsState(pCopyRectState, pCopyRectRopState); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetCopyRectsState(D3DCOPYRECTSTATE *pCopyRectState, D3DCOPYRECTROPSTATE *pCopyRectRopState) { D3DDevice_GetCopyRectsState(pCopyRectState, pCopyRectRopState); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetRenderTarget(D3DSurface *pRenderTarget, D3DSurface *pNewZStencil) { D3DDevice_SetRenderTarget(pRenderTarget, pNewZStencil); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetRenderTarget(D3DSurface **ppRenderTarget) { *ppRenderTarget = D3DDevice_GetRenderTarget2(); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetDepthStencilSurface(D3DSurface **ppZStencilSurface) { return (*ppZStencilSurface = D3DDevice_GetDepthStencilSurface2()) != NULL ? S_OK : D3DERR_NOTFOUND; }
D3DMINLINE HRESULT WINAPI D3DDevice::BeginScene() { D3DDevice_BeginScene(); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::EndScene() { D3DDevice_EndScene(); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) { D3DDevice_Clear(Count, pRects, Flags, Color, Z, Stencil); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix) { D3DDIRTY_TRANSFORM(State); D3DDevice_SetTransform(State, pMatrix); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix) { D3DDevice_GetTransform(State, pMatrix); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix) { D3DDevice_MultiplyTransform(State, pMatrix); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetViewport(CONST D3DVIEWPORT8 *pViewport) { D3DDIRTY_VIEWPORT(); D3DDevice_SetViewport(pViewport); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetViewport(D3DVIEWPORT8 *pViewport) { D3DDevice_GetViewport(pViewport); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetMaterial(CONST D3DMATERIAL8 *pMaterial) { D3DDIRTY_MATERIAL(); D3DDevice_SetMaterial(pMaterial); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetMaterial(D3DMATERIAL8 *pMaterial) { D3DDevice_GetMaterial(pMaterial); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetBackMaterial(CONST D3DMATERIAL8 *pBackMaterial) { D3DDIRTY_BACKMATERIAL(); D3DDevice_SetBackMaterial(pBackMaterial); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetBackMaterial(D3DMATERIAL8 *pBackMaterial) { D3DDevice_GetBackMaterial(pBackMaterial); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetLight(DWORD Index, CONST D3DLIGHT8 *pLight) { return D3DDevice_SetLight(Index, pLight); }
D3DMINLINE HRESULT WINAPI D3DDevice::GetLight(DWORD Index, D3DLIGHT8 *pLight) { D3DDevice_GetLight(Index, pLight); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::LightEnable(DWORD Index, BOOL Enable) { return D3DDevice_LightEnable(Index, Enable); }
D3DMINLINE HRESULT WINAPI D3DDevice::GetLightEnable(DWORD Index, BOOL *pEnable) { D3DDevice_GetLightEnable(Index, pEnable); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) { D3DDIRTY_RENDERSTATE(State); D3DDevice_SetRenderState(State, Value); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetRenderStateNotInline(D3DRENDERSTATETYPE State, DWORD Value) { D3DDIRTY_RENDERSTATE(State); D3DDevice_SetRenderStateNotInline(State, Value); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue) { D3DDevice_GetRenderState(State, pValue); return S_OK; }
#if D3DCOMPILE_BEGINSTATEBLOCK
D3DMINLINE HRESULT WINAPI D3DDevice::BeginStateBlock() { D3DDevice_BeginStateBlock(); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::EndStateBlock(DWORD *pToken) { return D3DDevice_EndStateBlock(pToken); }
#endif
D3DMINLINE HRESULT WINAPI D3DDevice::ApplyStateBlock(DWORD Token) { D3DDevice_ApplyStateBlock(Token); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::CaptureStateBlock(DWORD Token) { D3DDevice_CaptureStateBlock(Token); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::DeleteStateBlock(DWORD Token) { D3DDevice_DeleteStateBlock(Token); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreateStateBlock(D3DSTATEBLOCKTYPE Type,DWORD *pToken) { return D3DDevice_CreateStateBlock(Type, pToken); }
D3DMINLINE HRESULT WINAPI D3DDevice::GetTexture(DWORD Stage, D3DBaseTexture **ppTexture) { *ppTexture = D3DDevice_GetTexture2(Stage); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetTexture(DWORD Stage, D3DBaseTexture *pTexture) { D3DDIRTY_TEXTURE(Stage); D3DDevice_SetTexture(Stage, pTexture); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetPalette(DWORD Stage, D3DPalette **ppPalette) { *ppPalette = D3DDevice_GetPalette2(Stage); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetPalette(DWORD Stage, D3DPalette *pPalette) { D3DDevice_SetPalette(Stage, pPalette); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue) { D3DDevice_GetTextureStageState(Stage, Type, pValue); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) { D3DDIRTY_TEXTURESTATE(Stage, Type); D3DDevice_SetTextureStageState(Stage, Type, Value); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetTextureStageStateNotInline(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) { D3DDIRTY_TEXTURESTATE(Stage, Type); D3DDevice_SetTextureStageStateNotInline(Stage, Type, Value); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) { D3DDevice_DrawVertices(PrimitiveType, StartVertex, D3DVERTEXCOUNT(PrimitiveType, PrimitiveCount)); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT UnusedMinIndex, UINT UnusedNumIndices, UINT StartIndex, UINT PrimitiveCount) { D3DDevice_DrawIndexedVertices(PrimitiveType, D3DVERTEXCOUNT(PrimitiveType, PrimitiveCount), D3D__IndexData + StartIndex); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride) { D3DDevice_DrawVerticesUP(PrimitiveType, D3DVERTEXCOUNT(PrimitiveType, PrimitiveCount), pVertexStreamZeroData, VertexStreamZeroStride); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT UnusedMinIndex, UINT UnusedNumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT UnusedIndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride) { D3DDevice_DrawIndexedVerticesUP(PrimitiveType, D3DVERTEXCOUNT(PrimitiveType, PrimitiveCount), pIndexData, pVertexStreamZeroData, VertexStreamZeroStride); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::DrawVertices(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT VertexCount) { D3DDevice_DrawVertices(PrimitiveType, StartVertex, VertexCount); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::DrawIndexedVertices(D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST WORD* pIndexData) { D3DDevice_DrawIndexedVertices(PrimitiveType, VertexCount, pIndexData); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::DrawVerticesUP(D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride) { D3DDevice_DrawVerticesUP(PrimitiveType, VertexCount, pVertexStreamZeroData, VertexStreamZeroStride); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::DrawIndexedVerticesUP(D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST void *pIndexData, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride) { D3DDevice_DrawIndexedVerticesUP(PrimitiveType, VertexCount, pIndexData, pVertexStreamZeroData, VertexStreamZeroStride); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::PrimeVertexCache(UINT VertexCount, CONST WORD *pIndexData) { D3DDevice_PrimeVertexCache(VertexCount, pIndexData); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreateVertexShader(CONST DWORD *pDeclaration, CONST DWORD *pFunction, DWORD *pHandle, DWORD Usage) { return D3DDevice_CreateVertexShader(pDeclaration, pFunction, pHandle, Usage); }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexShader(DWORD Handle) { D3DDIRTY_VERTEXSHADER(); D3DDevice_SetVertexShader(Handle); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetVertexShader(DWORD *pHandle) { D3DDevice_GetVertexShader(pHandle); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetShaderConstantMode(D3DSHADERCONSTANTMODE Mode) { D3DDevice_SetShaderConstantMode(Mode); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetShaderConstantMode(D3DSHADERCONSTANTMODE *pMode) { D3DDevice_GetShaderConstantMode(pMode); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::LoadVertexShader(DWORD Handle, DWORD Address) { D3DDevice_LoadVertexShader(Handle, Address); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::LoadVertexShaderProgram(CONST DWORD *pFunction, DWORD Address) { D3DDevice_LoadVertexShaderProgram(pFunction, Address); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SelectVertexShader(DWORD Handle, DWORD Address) { D3DDevice_SelectVertexShader(Handle, Address); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SelectVertexShaderDirect(D3DVERTEXATTRIBUTEFORMAT *pVAF, DWORD Address) { D3DDevice_SelectVertexShaderDirect(pVAF, Address); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::RunVertexStateShader(DWORD Address, CONST float *pData) { D3DDevice_RunVertexStateShader(Address, pData); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetVertexShaderSize(DWORD Handle, UINT *pSize) { D3DDevice_GetVertexShaderSize(Handle, pSize); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetVertexShaderType(DWORD Handle, DWORD *pType) { D3DDevice_GetVertexShaderType(Handle, pType); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetVertexShaderDeclaration(DWORD Handle, void *pData, DWORD *pSizeOfData) { return D3DDevice_GetVertexShaderDeclaration(Handle, pData, pSizeOfData); }
D3DMINLINE HRESULT WINAPI D3DDevice::GetVertexShaderFunction(DWORD Handle, void *pData, DWORD *pSizeOfData) { return D3DDevice_GetVertexShaderFunction(Handle, pData, pSizeOfData); }
D3DMINLINE HRESULT WINAPI D3DDevice::DeleteVertexShader(DWORD Handle) { D3DDevice_DeleteVertexShader(Handle); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexShaderConstant(INT Register, CONST void *pConstantData, DWORD ConstantCount) { D3DDIRTY_VERTEXSHADERCONSTANT(Register, ConstantCount); D3DDevice_SetVertexShaderConstant(Register, pConstantData, ConstantCount); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexShaderConstantFast(INT Register, CONST void *pConstantData, DWORD ConstantCount) { D3DDIRTY_VERTEXSHADERCONSTANT(Register, ConstantCount); D3DDevice_SetVertexShaderConstantFast(Register, pConstantData, ConstantCount); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetVertexShaderConstant(INT Register, void *pConstantData, DWORD ConstantCount) { D3DDevice_GetVertexShaderConstant(Register, pConstantData, ConstantCount); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetStreamSource(UINT StreamNumber, D3DVertexBuffer *pStreamData, UINT Stride) { D3DDIRTY_STREAM(StreamNumber); D3DDevice_SetStreamSource(StreamNumber, pStreamData, Stride); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetStreamSource(UINT StreamNumber, D3DVertexBuffer **ppStreamData, UINT *pStride) { *ppStreamData = D3DDevice_GetStreamSource2(StreamNumber, pStride); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetIndices(D3DIndexBuffer* pIndexData, UINT BaseVertexIndex) { D3DDIRTY_INDICES(); D3DDevice_SetIndices(pIndexData, BaseVertexIndex); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetIndices(D3DIndexBuffer** ppIndexData, UINT *pBaseVertexIndex) { *ppIndexData = D3DDevice_GetIndices2(pBaseVertexIndex); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreatePixelShader(CONST D3DPIXELSHADERDEF *pPSDef, DWORD *pHandle) { D3DDevice_CreatePixelShader(pPSDef, pHandle); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetPixelShader(DWORD Handle) { D3DDIRTY_PIXELSHADER(); D3DDevice_SetPixelShader(Handle); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetPixelShaderProgram(CONST D3DPIXELSHADERDEF *pPSDef) { D3DDIRTY_PIXELSHADER(); D3DDevice_SetPixelShaderProgram(pPSDef); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetPixelShader(DWORD *pHandle) { D3DDevice_GetPixelShader(pHandle); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::DeletePixelShader(DWORD Handle) { D3DDevice_DeletePixelShader(Handle); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetPixelShaderConstant(DWORD Register, CONST void *pConstantData, DWORD ConstantCount) { D3DDIRTY_PIXELSHADERCONSTANT(Register, ConstantCount); D3DDevice_SetPixelShaderConstant(Register, pConstantData, ConstantCount); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetPixelShaderConstant(DWORD Register, void *pConstantData, DWORD ConstantCount) { D3DDevice_GetPixelShaderConstant(Register, pConstantData, ConstantCount); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetPixelShaderFunction(DWORD Handle, D3DPIXELSHADERDEF *pData) { D3DDevice_GetPixelShaderFunction(Handle, pData); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo) { return D3DDevice_DrawRectPatch(Handle, pNumSegs, pRectPatchInfo); }
D3DMINLINE HRESULT WINAPI D3DDevice::DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo) { return D3DDevice_DrawTriPatch(Handle, pNumSegs, pTriPatchInfo); }
D3DMINLINE HRESULT WINAPI D3DDevice::DeletePatch(UINT Handle) { D3DDevice_DeletePatch(Handle); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::UpdateOverlay(D3DSurface *pSurface, CONST RECT *SrcRect, CONST RECT *DstRect, BOOL EnableColorKey, D3DCOLOR ColorKey) { D3DDevice_UpdateOverlay(pSurface, SrcRect, DstRect, EnableColorKey, ColorKey); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::EnableOverlay(BOOL Enable) { D3DDevice_EnableOverlay(Enable); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::BeginVisibilityTest() { D3DDevice_BeginVisibilityTest(); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::EndVisibilityTest(DWORD Index) { return D3DDevice_EndVisibilityTest(Index); }
D3DMINLINE HRESULT WINAPI D3DDevice::GetVisibilityTestResult(DWORD Index, UINT *pResult, ULONGLONG *pTimeStamp) { return D3DDevice_GetVisibilityTestResult(Index, pResult, pTimeStamp); }
D3DMINLINE BOOL    WINAPI D3DDevice::IsBusy() { return D3DDevice_IsBusy(); }
D3DMINLINE void    WINAPI D3DDevice::BlockUntilIdle() { D3DDevice_BlockUntilIdle(); }
D3DMINLINE void    WINAPI D3DDevice::KickPushBuffer() { D3DDevice_KickPushBuffer(); }
D3DMINLINE void    WINAPI D3DDevice::SetVerticalBlankCallback(D3DVBLANKCALLBACK pCallback) { D3DDevice_SetVerticalBlankCallback(pCallback); }
D3DMINLINE void    WINAPI D3DDevice::SetSwapCallback(D3DSWAPCALLBACK pCallback) { D3DDevice_SetSwapCallback(pCallback); }
D3DMINLINE void    WINAPI D3DDevice::BlockUntilVerticalBlank() { D3DDevice_BlockUntilVerticalBlank(); }
D3DMINLINE DWORD   WINAPI D3DDevice::InsertFence() { return D3DDevice_InsertFence(); }
D3DMINLINE BOOL    WINAPI D3DDevice::IsFencePending(DWORD Fence) { return D3DDevice_IsFencePending(Fence); }
D3DMINLINE void    WINAPI D3DDevice::BlockOnFence(DWORD Fence) { D3DDevice_BlockOnFence(Fence); }
D3DMINLINE void    WINAPI D3DDevice::InsertCallback(D3DCALLBACKTYPE Type, D3DCALLBACK pCallback, DWORD Context) { D3DDevice_InsertCallback(Type, pCallback, Context); }
D3DMINLINE void    WINAPI D3DDevice::FlushVertexCache() { D3DDevice_FlushVertexCache(); }
D3DMINLINE HRESULT WINAPI D3DDevice::PersistDisplay() { return D3DDevice_PersistDisplay(); }
D3DMINLINE HRESULT WINAPI D3DDevice::GetPersistedSurface(D3DSurface **ppSurface) { *ppSurface = D3DDevice_GetPersistedSurface2(); return S_OK; }
D3DMINLINE BOOL    WINAPI D3DDevice::GetOverlayUpdateStatus() { return D3DDevice_GetOverlayUpdateStatus(); }
D3DMINLINE HRESULT WINAPI D3DDevice::GetDisplayFieldStatus(D3DFIELD_STATUS *pFieldStatus) { D3DDevice_GetDisplayFieldStatus(pFieldStatus); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexData2f(INT Register, float a, float b) { D3DDevice_SetVertexData2f(Register, a, b); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexData4f(INT Register, float a, float b, float c, float d) { D3DDevice_SetVertexData4f(Register, a, b, c, d); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexData2s(INT Register, SHORT a, SHORT b) { D3DDevice_SetVertexData2s(Register, a, b); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexData4s(INT Register, SHORT a, SHORT b, SHORT c, SHORT d) { D3DDevice_SetVertexData4s(Register, a, b, c, d); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexData4ub(INT Register, BYTE a, BYTE b, BYTE c, BYTE d) { D3DDevice_SetVertexData4ub(Register, a, b, c, d); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexDataColor(INT Register, D3DCOLOR Color) { D3DDevice_SetVertexDataColor(Register, Color); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::Begin(D3DPRIMITIVETYPE PrimitiveType) { D3DDevice_Begin(PrimitiveType); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::End() { D3DDevice_End(); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreateFixup(UINT Size, D3DFixup **ppFixup) { return (*ppFixup = D3DDevice_CreateFixup2(Size)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DDevice::CreatePushBuffer(UINT Size, BOOL RunUsingCpuCopy, D3DPushBuffer **ppPushBuffer) { return (*ppPushBuffer = D3DDevice_CreatePushBuffer2(Size, RunUsingCpuCopy)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DDevice::BeginPushBuffer(D3DPushBuffer* pPushBuffer) { D3DDevice_BeginPushBuffer(pPushBuffer); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::EndPushBuffer() { return D3DDevice_EndPushBuffer(); }
D3DMINLINE HRESULT WINAPI D3DDevice::RunPushBuffer(D3DPushBuffer* pPushBuffer, D3DFixup *pFixup) { D3DDevice_RunPushBuffer(pPushBuffer, pFixup); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetPushBufferOffset(DWORD* pOffset) { D3DDevice_GetPushBufferOffset(pOffset); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::Nop() { D3DDevice_Nop(); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetProjectionViewportMatrix(D3DMATRIX* pProjectionViewport) { D3DDevice_GetProjectionViewportMatrix(pProjectionViewport); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetModelView(CONST D3DMATRIX* pModelView, CONST D3DMATRIX* pInverseModelView, CONST D3DMATRIX* pComposite) { D3DDevice_SetModelView(pModelView, pInverseModelView, pComposite); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetModelView(D3DMATRIX* pModelView) { return D3DDevice_GetModelView(pModelView); }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexBlendModelView(UINT Count, CONST D3DMATRIX* pModelViews, CONST D3DMATRIX* pInverseModelViews, CONST D3DMATRIX* pProjectionViewport) { D3DDevice_SetVertexBlendModelView(Count, pModelViews, pInverseModelViews, pProjectionViewport); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetVertexBlendModelView(UINT Count, D3DMATRIX* pModelViews, D3DMATRIX* pProjectionViewport) { return D3DDevice_GetVertexBlendModelView(Count, pModelViews, pProjectionViewport); }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexShaderInput(DWORD Handle, UINT StreamCount, CONST D3DSTREAM_INPUT* pStreamInputs) { D3DDevice_SetVertexShaderInput(Handle, StreamCount, pStreamInputs); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetVertexShaderInputDirect(D3DVERTEXATTRIBUTEFORMAT *pVAF, UINT StreamCount, CONST D3DSTREAM_INPUT* pStreamInputs) { D3DDevice_SetVertexShaderInputDirect(pVAF, StreamCount, pStreamInputs); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetVertexShaderInput(DWORD* pHandle, UINT* pStreamCount, D3DSTREAM_INPUT* pStreamInputs) { return D3DDevice_GetVertexShaderInput(pHandle, pStreamCount, pStreamInputs); }
D3DMINLINE HRESULT WINAPI D3DDevice::SwitchTexture(DWORD Stage, D3DBaseTexture *pTexture) { D3DDevice_SwitchTexture(D3DTEXTUREDIRECTENCODE[Stage], (pTexture)->Data, (pTexture)->Format); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetScissors(DWORD Count, BOOL Exclusive, CONST D3DRECT *pRects) { D3DDevice_SetScissors(Count, Exclusive, pRects); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetScissors(DWORD* pCount, BOOL *pExclusive, D3DRECT *pRects) { D3DDevice_GetScissors(pCount, pExclusive, pRects); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetTile(DWORD Index, CONST D3DTILE* pTile) { D3DDevice_SetTile(Index, pTile); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetTile(DWORD Index, D3DTILE* pTile) { D3DDevice_GetTile(Index, pTile); return S_OK; }
D3DMINLINE DWORD   WINAPI D3DDevice::GetTileCompressionTags(DWORD ZStartTag, DWORD ZEndTag) { return D3DDevice_GetTileCompressionTags(ZStartTag, ZEndTag); }
D3DMINLINE void    WINAPI D3DDevice::SetTileCompressionTagBits(DWORD Partition, DWORD Address, CONST DWORD *pData, DWORD Count) { D3DDevice_SetTileCompressionTagBits(Partition, Address, pData, Count); }
D3DMINLINE void    WINAPI D3DDevice::GetTileCompressionTagBits(DWORD Partition, DWORD Address, DWORD *pData, DWORD Count) { D3DDevice_GetTileCompressionTagBits(Partition, Address, pData, Count); }
D3DMINLINE HRESULT WINAPI D3DDevice::BeginPush(DWORD Count, DWORD **ppPush) { *ppPush = D3DDevice_BeginPush(Count); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::EndPush(DWORD *pPush) { D3DDevice_EndPush(pPush); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::BeginState(DWORD Count, DWORD **ppPush) { *ppPush = D3DDevice_BeginState(Count); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::EndState(DWORD *pPush) { D3DDevice_EndState(pPush); return S_OK; }
D3DMINLINE DWORD   WINAPI D3DDevice::Swap(DWORD Flags) { return D3DDevice_Swap(Flags); }
D3DMINLINE HRESULT WINAPI D3DDevice::SetBackBufferScale(float x, float y) { D3DDevice_SetBackBufferScale(x, y); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetBackBufferScale(float *pX, float *pY) { D3DDevice_GetBackBufferScale(pX, pY); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::SetScreenSpaceOffset(float x, float y) { D3DDevice_SetScreenSpaceOffset(x, y); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DDevice::GetScreenSpaceOffset(float *pX, float *pY) { D3DDevice_GetScreenSpaceOffset(pX, pY); return S_OK; }
D3DMINLINE void    WINAPI D3DDevice::SetOverscanColor(D3DCOLOR Color) { D3DDevice_SetOverscanColor(Color); }
D3DMINLINE D3DCOLOR WINAPI D3DDevice::GetOverscanColor() { return D3DDevice_GetOverscanColor(); }
D3DMINLINE void    WINAPI D3DDevice::SetDepthClipPlanes(float Near, float Far, DWORD Flags) { D3DDevice_SetDepthClipPlanes(Near, Far, Flags); }
D3DMINLINE void    WINAPI D3DDevice::GetDepthClipPlanes(float *pNear, float *pFar, DWORD Flags) { D3DDevice_GetDepthClipPlanes(pNear, pFar, Flags); }
D3DMINLINE void    WINAPI D3DDevice::GetViewportOffsetAndScale(D3DVECTOR4 *pOffset, D3DVECTOR4 *pScale) { D3DDevice_GetViewportOffsetAndScale(pOffset, pScale); }
D3DMINLINE void    WINAPI D3DDevice::SetStipple(CONST DWORD *pPattern) { D3DDevice_SetStipple(pPattern); }
D3DMINLINE void    WINAPI D3DDevice::GetStipple(DWORD *pPattern) { D3DDevice_GetStipple(pPattern); }
D3DMINLINE void    WINAPI D3DDevice::SetWaitCallback(D3DWAITCALLBACK pCallback) { D3DDevice_SetWaitCallback(pCallback); }
D3DMINLINE DWORD   WINAPI D3DDevice::GetPushDistance(DWORD Handle) { return D3DDevice_GetPushDistance(Handle); }
D3DMINLINE HRESULT WINAPI D3DDevice::SetTimerCallback(ULONGLONG Time, D3DCALLBACK pCallback, DWORD Context) { return D3DDevice_SetTimerCallback(Time, pCallback, Context); }
D3DMINLINE void    WINAPI D3DDevice::SetRenderTargetFast(D3DSurface *pRenderTarget, D3DSurface *pNewZStencil, DWORD Flags) { D3DDevice_SetRenderTargetFast(pRenderTarget, pNewZStencil, Flags); }

#ifdef _DEBUG
D3DMINLINE DWORD   WINAPI D3DDevice::SetDebugMarker(DWORD Marker) { return D3DDevice_SetDebugMarker(Marker); }
D3DMINLINE DWORD   WINAPI D3DDevice::GetDebugMarker() { return D3DDevice_GetDebugMarker(); }
#else
D3DMINLINE DWORD   WINAPI D3DDevice::SetDebugMarker(DWORD Marker) { return 0; }
D3DMINLINE DWORD   WINAPI D3DDevice::GetDebugMarker() { return 0; }
#endif

#endif __cplusplus

/* D3DResource */

ULONG   WINAPI D3DResource_AddRef(D3DResource *pThis);
ULONG   WINAPI D3DResource_Release(D3DResource *pThis);
void    WINAPI D3DResource_GetDevice(D3DResource *pThis, D3DDevice **ppDevice);
HRESULT WINAPI D3DResource_SetPrivateData(D3DResource *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags);
HRESULT WINAPI D3DResource_GetPrivateData(D3DResource *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData);
void    WINAPI D3DResource_FreePrivateData(D3DResource *pThis, REFGUID refguid);
D3DRESOURCETYPE WINAPI D3DResource_GetType(D3DResource *pThis);
BOOL    WINAPI D3DResource_IsBusy(D3DResource *pThis);
void    WINAPI D3DResource_BlockUntilNotBusy(D3DResource *pThis);
void    WINAPI D3DResource_Register(D3DResource *pThis, void *pBase);
D3DINLINE void WINAPI D3DResource_MoveResourceMemory(D3DResource *pThis, D3DMEMORY where) { }


// Compatibility wrappers.

D3DINLINE ULONG   WINAPI IDirect3DResource8_AddRef(D3DResource *pThis) { return D3DResource_AddRef(pThis); }
D3DINLINE ULONG   WINAPI IDirect3DResource8_Release(D3DResource *pThis) { return D3DResource_Release(pThis); }
D3DINLINE HRESULT WINAPI IDirect3DResource8_GetDevice(D3DResource *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice(pThis, ppDevice); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DResource8_SetPrivateData(D3DResource *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData(pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DResource8_GetPrivateData(D3DResource *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData(pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DResource8_FreePrivateData(D3DResource *pThis, REFGUID refguid) { D3DResource_FreePrivateData(pThis, refguid); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DResource8_GetType(D3DResource *pThis) { return D3DResource_GetType(pThis); }
D3DINLINE BOOL    WINAPI IDirect3DResource8_IsBusy(D3DResource *pThis) { return D3DResource_IsBusy(pThis); }
D3DINLINE void    WINAPI IDirect3DResource8_BlockUntilNotBusy(D3DResource *pThis) { D3DResource_BlockUntilNotBusy(pThis); }
D3DINLINE void    WINAPI IDirect3DResource8_MoveResourceMemory(D3DResource *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory(pThis, where); }
D3DINLINE void    WINAPI IDirect3DResource8_Register(D3DResource *pThis, void *pBase) { D3DResource_Register(pThis, pBase); }

#ifdef __cplusplus

D3DMINLINE ULONG WINAPI D3DResource::AddRef() { return D3DResource_AddRef(this); }
D3DMINLINE ULONG WINAPI D3DResource::Release() { return D3DResource_Release(this); }
D3DMINLINE HRESULT WINAPI D3DResource::GetDevice(D3DDevice **ppDevice) { D3DResource_GetDevice(this, ppDevice); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DResource::SetPrivateData(REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData(this, refguid, pData, SizeOfData, Flags); }
D3DMINLINE HRESULT WINAPI D3DResource::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData(this, refguid, pData, pSizeOfData); }
D3DMINLINE HRESULT WINAPI D3DResource::FreePrivateData(REFGUID refguid) { D3DResource_FreePrivateData(this, refguid); return S_OK; }
D3DMINLINE D3DRESOURCETYPE WINAPI D3DResource::GetType() { return D3DResource_GetType(this); }
D3DMINLINE BOOL    WINAPI D3DResource::IsBusy() { return D3DResource_IsBusy(this); }
D3DMINLINE void    WINAPI D3DResource::BlockUntilNotBusy() { D3DResource_BlockUntilNotBusy(this); }
D3DMINLINE void    WINAPI D3DResource::MoveResourceMemory(D3DMEMORY where) { D3DResource_MoveResourceMemory(this, where); }
D3DMINLINE void    WINAPI D3DResource::Register(void *pBase) { D3DResource_Register(this, pBase); }

#endif __cplusplus

/* D3DBaseTexture */

D3DINLINE ULONG   WINAPI D3DBaseTexture_AddRef(D3DBaseTexture *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI D3DBaseTexture_Release(D3DBaseTexture *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DBaseTexture_GetDevice(D3DBaseTexture *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); }
D3DINLINE D3DRESOURCETYPE WINAPI D3DBaseTexture_GetType(D3DBaseTexture *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI D3DBaseTexture_IsBusy(D3DBaseTexture *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DBaseTexture_BlockUntilNotBusy(D3DBaseTexture *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DBaseTexture_MoveResourceMemory(D3DBaseTexture *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI D3DBaseTexture_Register(D3DBaseTexture *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI D3DBaseTexture_SetPrivateData(D3DBaseTexture *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI D3DBaseTexture_GetPrivateData(D3DBaseTexture *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE void    WINAPI D3DBaseTexture_FreePrivateData(D3DBaseTexture *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); }

DWORD WINAPI D3DBaseTexture_GetLevelCount(D3DBaseTexture *pThis);

// Compatibility wrappers.

D3DINLINE ULONG   WINAPI IDirect3DBaseTexture8_AddRef(D3DBaseTexture *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI IDirect3DBaseTexture8_Release(D3DBaseTexture *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DBaseTexture8_GetDevice(D3DBaseTexture *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DBaseTexture8_GetType(D3DBaseTexture *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI IDirect3DBaseTexture8_IsBusy(D3DBaseTexture *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DBaseTexture8_BlockUntilNotBusy(D3DBaseTexture *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DBaseTexture8_MoveResourceMemory(D3DBaseTexture *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI IDirect3DBaseTexture8_Register(D3DBaseTexture *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI IDirect3DBaseTexture8_SetPrivateData(D3DBaseTexture *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DBaseTexture8_GetPrivateData(D3DBaseTexture *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DBaseTexture8_FreePrivateData(D3DBaseTexture *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); return S_OK; }
D3DINLINE DWORD WINAPI IDirect3DBaseTexture8_GetLevelCount(D3DBaseTexture *pThis) { return D3DBaseTexture_GetLevelCount(pThis); }

#ifdef __cplusplus

D3DMINLINE DWORD WINAPI D3DBaseTexture::GetLevelCount() { return D3DBaseTexture_GetLevelCount(this); }

#endif __cplusplus

/* D3DTexture */

D3DINLINE ULONG   WINAPI D3DTexture_AddRef(D3DTexture *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI D3DTexture_Release(D3DTexture *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DTexture_GetDevice(D3DTexture *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); }
D3DINLINE D3DRESOURCETYPE WINAPI D3DTexture_GetType(D3DTexture *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI D3DTexture_IsBusy(D3DTexture *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DTexture_BlockUntilNotBusy(D3DTexture *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DTexture_MoveResourceMemory(D3DTexture *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI D3DTexture_Register(D3DTexture *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE DWORD   WINAPI D3DTexture_GetLevelCount(D3DTexture *pThis) { return D3DBaseTexture_GetLevelCount((D3DBaseTexture *)pThis); }
D3DINLINE HRESULT WINAPI D3DTexture_SetPrivateData(D3DTexture *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI D3DTexture_GetPrivateData(D3DTexture *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE void    WINAPI D3DTexture_FreePrivateData(D3DTexture *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); }

void        WINAPI D3DTexture_GetLevelDesc(D3DTexture *pThis, UINT Level, D3DSURFACE_DESC *pDesc);
D3DSurface* WINAPI D3DTexture_GetSurfaceLevel2(D3DTexture *pThis, UINT Level);
void        WINAPI D3DTexture_LockRect(D3DTexture *pThis, UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags);
D3DINLINE   void WINAPI D3DTexture_UnlockRect(D3DTexture *pThis, UINT Level) { }

// Compatibilty wrappers.

D3DINLINE ULONG   WINAPI IDirect3DTexture8_AddRef(D3DTexture *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI IDirect3DTexture8_Release(D3DTexture *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DTexture8_GetDevice(D3DTexture *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DTexture8_GetType(D3DTexture *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI IDirect3DTexture8_IsBusy(D3DTexture *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DTexture8_BlockUntilNotBusy(D3DTexture *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DTexture8_MoveResourceMemory(D3DTexture *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI IDirect3DTexture8_Register(D3DTexture *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE DWORD   WINAPI IDirect3DTexture8_GetLevelCount(D3DTexture *pThis) { return D3DBaseTexture_GetLevelCount((D3DBaseTexture *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DTexture8_SetPrivateData(D3DTexture *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DTexture8_GetPrivateData(D3DTexture *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DTexture8_FreePrivateData(D3DTexture *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DTexture8_GetLevelDesc(D3DTexture *pThis, UINT Level, D3DSURFACE_DESC *pDesc) { D3DTexture_GetLevelDesc(pThis, Level, pDesc); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DTexture8_GetSurfaceLevel(D3DTexture *pThis, UINT Level, D3DSurface **ppSurfaceLevel) { return (*ppSurfaceLevel = D3DTexture_GetSurfaceLevel2(pThis, Level)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DTexture8_LockRect(D3DTexture *pThis, UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags) { D3DTexture_LockRect(pThis, Level, pLockedRect, pRect, Flags); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DTexture8_UnlockRect(D3DTexture *pThis, UINT Level) { D3DTexture_UnlockRect(pThis, Level); return S_OK; }

#ifdef __cplusplus

D3DMINLINE HRESULT WINAPI D3DTexture::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) { D3DTexture_GetLevelDesc(this, Level, pDesc); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DTexture::GetSurfaceLevel(UINT Level, D3DSurface **ppSurfaceLevel) { return (*ppSurfaceLevel = D3DTexture_GetSurfaceLevel2(this, Level)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DTexture::LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags) { D3DTexture_LockRect(this, Level, pLockedRect, pRect, Flags); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DTexture::UnlockRect(UINT Level) { D3DTexture_UnlockRect(this, Level); return S_OK; }

#endif __cplusplus

/* D3DVolumeTexture */

D3DINLINE ULONG   WINAPI D3DVolumeTexture_AddRef(D3DVolumeTexture *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI D3DVolumeTexture_Release(D3DVolumeTexture *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DVolumeTexture_GetDevice(D3DVolumeTexture *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); }
D3DINLINE D3DRESOURCETYPE WINAPI D3DVolumeTexture_GetType(D3DVolumeTexture *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI D3DVolumeTexture_IsBusy(D3DVolumeTexture *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DVolumeTexture_BlockUntilNotBusy(D3DVolumeTexture *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DVolumeTexture_MoveResourceMemory(D3DVolumeTexture *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI D3DVolumeTexture_Register(D3DVolumeTexture *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE DWORD   WINAPI D3DVolumeTexture_GetLevelCount(D3DVolumeTexture *pThis) { return D3DBaseTexture_GetLevelCount((D3DBaseTexture *)pThis); }
D3DINLINE HRESULT WINAPI D3DVolumeTexture_SetPrivateData(D3DVolumeTexture *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI D3DVolumeTexture_GetPrivateData(D3DVolumeTexture *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE void    WINAPI D3DVolumeTexture_FreePrivateData(D3DVolumeTexture *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); }

void       WINAPI D3DVolumeTexture_GetLevelDesc(D3DVolumeTexture *pThis, UINT Level, D3DVOLUME_DESC *pDesc);
D3DVolume* WINAPI D3DVolumeTexture_GetVolumeLevel2(D3DVolumeTexture *pThis, UINT Level);
void       WINAPI D3DVolumeTexture_LockBox(D3DVolumeTexture *pThis, UINT Level, D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags);
D3DINLINE  void WINAPI D3DVolumeTexture_UnlockBox(D3DVolumeTexture *pThis, UINT Level) { }

// Compatibilty wrappers.

D3DINLINE ULONG   WINAPI IDirect3DVolumeTexture8_AddRef(D3DVolumeTexture *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI IDirect3DVolumeTexture8_Release(D3DVolumeTexture *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DVolumeTexture8_GetDevice(D3DVolumeTexture *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DVolumeTexture8_GetType(D3DVolumeTexture *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI IDirect3DVolumeTexture8_IsBusy(D3DVolumeTexture *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DVolumeTexture8_BlockUntilNotBusy(D3DVolumeTexture *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DVolumeTexture8_MoveResourceMemory(D3DVolumeTexture *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI IDirect3DVolumeTexture8_Register(D3DVolumeTexture *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE DWORD   WINAPI IDirect3DVolumeTexture8_GetLevelCount(D3DVolumeTexture *pThis) { return D3DBaseTexture_GetLevelCount((D3DBaseTexture *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DVolumeTexture8_SetPrivateData(D3DVolumeTexture *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DVolumeTexture8_GetPrivateData(D3DVolumeTexture *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DVolumeTexture8_FreePrivateData(D3DVolumeTexture *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DVolumeTexture8_GetLevelDesc(D3DVolumeTexture *pThis, UINT Level, D3DVOLUME_DESC *pDesc) { D3DVolumeTexture_GetLevelDesc(pThis, Level, pDesc); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DVolumeTexture8_GetVolumeLevel(D3DVolumeTexture *pThis, UINT Level, D3DVolume **ppVolumeLevel) { return (*ppVolumeLevel = D3DVolumeTexture_GetVolumeLevel2(pThis, Level)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DVolumeTexture8_LockBox(D3DVolumeTexture *pThis, UINT Level, D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags) { D3DVolumeTexture_LockBox(pThis, Level, pLockedVolume, pBox, Flags); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DVolumeTexture8_UnlockBox(D3DVolumeTexture *pThis, UINT Level) { D3DVolumeTexture_UnlockBox(pThis, Level); return S_OK; }


#ifdef __cplusplus

D3DMINLINE HRESULT WINAPI D3DVolumeTexture::GetLevelDesc(UINT Level, D3DVOLUME_DESC *pDesc) { D3DVolumeTexture_GetLevelDesc(this, Level, pDesc); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DVolumeTexture::GetVolumeLevel(UINT Level, D3DVolume **ppVolumeLevel) { return (*ppVolumeLevel = D3DVolumeTexture_GetVolumeLevel2(this, Level)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DVolumeTexture::LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags) { D3DVolumeTexture_LockBox(this, Level, pLockedVolume, pBox, Flags); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DVolumeTexture::UnlockBox(UINT Level) { D3DVolumeTexture_UnlockBox(this, Level); return S_OK; }

#endif __cplusplus

/* D3DCubeTexture */

D3DINLINE ULONG   WINAPI D3DCubeTexture_AddRef(D3DCubeTexture *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI D3DCubeTexture_Release(D3DCubeTexture *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DCubeTexture_GetDevice(D3DCubeTexture *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); }
D3DINLINE D3DRESOURCETYPE WINAPI D3DCubeTexture_GetType(D3DCubeTexture *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI D3DCubeTexture_IsBusy(D3DCubeTexture *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DCubeTexture_BlockUntilNotBusy(D3DCubeTexture *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DCubeTexture_MoveResourceMemory(D3DCubeTexture *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI D3DCubeTexture_Register(D3DCubeTexture *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE DWORD   WINAPI D3DCubeTexture_GetLevelCount(D3DCubeTexture *pThis) { return D3DBaseTexture_GetLevelCount((D3DBaseTexture *)pThis); }
D3DINLINE HRESULT WINAPI D3DCubeTexture_SetPrivateData(D3DCubeTexture *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI D3DCubeTexture_GetPrivateData(D3DCubeTexture *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE void    WINAPI D3DCubeTexture_FreePrivateData(D3DCubeTexture *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); }

void        WINAPI D3DCubeTexture_GetLevelDesc(D3DCubeTexture *pThis, UINT Level, D3DSURFACE_DESC *pDesc);
D3DSurface* WINAPI D3DCubeTexture_GetCubeMapSurface2(D3DCubeTexture *pThis, D3DCUBEMAP_FACES FaceType, UINT Level);
void        WINAPI D3DCubeTexture_LockRect(D3DCubeTexture *pThis, D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags);
D3DINLINE   void WINAPI D3DCubeTexture_UnlockRect(D3DCubeTexture *pThis, D3DCUBEMAP_FACES FaceType, UINT Level) { }

D3DINLINE ULONG   WINAPI IDirect3DCubeTexture8_AddRef(D3DCubeTexture *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI IDirect3DCubeTexture8_Release(D3DCubeTexture *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DCubeTexture8_GetDevice(D3DCubeTexture *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DCubeTexture8_GetType(D3DCubeTexture *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI IDirect3DCubeTexture8_IsBusy(D3DCubeTexture *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DCubeTexture8_BlockUntilNotBusy(D3DCubeTexture *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DCubeTexture8_MoveResourceMemory(D3DCubeTexture *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI IDirect3DCubeTexture8_Register(D3DCubeTexture *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE DWORD   WINAPI IDirect3DCubeTexture8_GetLevelCount(D3DCubeTexture *pThis) { return D3DBaseTexture_GetLevelCount((D3DBaseTexture *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DCubeTexture8_SetPrivateData(D3DCubeTexture *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DCubeTexture8_GetPrivateData(D3DCubeTexture *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DCubeTexture8_FreePrivateData(D3DCubeTexture *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DCubeTexture8_GetLevelDesc(D3DCubeTexture *pThis, UINT Level, D3DSURFACE_DESC *pDesc) { D3DCubeTexture_GetLevelDesc(pThis, Level, pDesc); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DCubeTexture8_GetCubeMapSurface(D3DCubeTexture *pThis, D3DCUBEMAP_FACES FaceType, UINT Level, D3DSurface **ppCubeMapSurface) { return (*ppCubeMapSurface = D3DCubeTexture_GetCubeMapSurface2(pThis, FaceType, Level)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DINLINE HRESULT WINAPI IDirect3DCubeTexture8_LockRect(D3DCubeTexture *pThis, D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags) { D3DCubeTexture_LockRect(pThis, FaceType, Level, pLockedRect, pRect, Flags); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DCubeTexture8_UnlockRect(D3DCubeTexture *pThis, D3DCUBEMAP_FACES FaceType, UINT Level) { D3DCubeTexture_UnlockRect(pThis, FaceType, Level); return S_OK; }

#ifdef __cplusplus

D3DMINLINE HRESULT WINAPI D3DCubeTexture::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) { D3DCubeTexture_GetLevelDesc(this, Level, pDesc); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DCubeTexture::GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, D3DSurface **ppCubeMapSurface) { return (*ppCubeMapSurface = D3DCubeTexture_GetCubeMapSurface2(this, FaceType, Level)) != NULL ? S_OK : E_OUTOFMEMORY; }
D3DMINLINE HRESULT WINAPI D3DCubeTexture::LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags) { D3DCubeTexture_LockRect(this, FaceType, Level, pLockedRect, pRect, Flags); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DCubeTexture::UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level) { D3DCubeTexture_UnlockRect(this, FaceType, Level); return S_OK; }

#endif __cplusplus

/* D3DVertexBuffer */

D3DINLINE ULONG   WINAPI D3DVertexBuffer_AddRef(D3DVertexBuffer *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI D3DVertexBuffer_Release(D3DVertexBuffer *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DVertexBuffer_GetDevice(D3DVertexBuffer *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); }
D3DINLINE D3DRESOURCETYPE WINAPI D3DVertexBuffer_GetType(D3DVertexBuffer *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI D3DVertexBuffer_IsBusy(D3DVertexBuffer *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DVertexBuffer_BlockUntilNotBusy(D3DVertexBuffer *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DVertexBuffer_MoveResourceMemory(D3DVertexBuffer *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI D3DVertexBuffer_Register(D3DVertexBuffer *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI D3DVertexBuffer_SetPrivateData(D3DVertexBuffer *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI D3DVertexBuffer_GetPrivateData(D3DVertexBuffer *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE void    WINAPI D3DVertexBuffer_FreePrivateData(D3DVertexBuffer *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); }

BYTE*   WINAPI D3DVertexBuffer_Lock2(D3DVertexBuffer *pThis, DWORD Flags);
void    WINAPI D3DVertexBuffer_GetDesc(D3DVertexBuffer *pThis, D3DVERTEXBUFFER_DESC *pDesc);
D3DINLINE void WINAPI D3DVertexBuffer_Unlock(D3DVertexBuffer *pThis) { }

D3DINLINE ULONG   WINAPI IDirect3DVertexBuffer8_AddRef(D3DVertexBuffer *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI IDirect3DVertexBuffer8_Release(D3DVertexBuffer *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DVertexBuffer8_GetDevice(D3DVertexBuffer *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DVertexBuffer8_GetType(D3DVertexBuffer *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI IDirect3DVertexBuffer8_IsBusy(D3DVertexBuffer *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DVertexBuffer8_BlockUntilNotBusy(D3DVertexBuffer *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DVertexBuffer8_MoveResourceMemory(D3DVertexBuffer *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI IDirect3DVertexBuffer8_Register(D3DVertexBuffer *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI IDirect3DVertexBuffer8_SetPrivateData(D3DVertexBuffer *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DVertexBuffer8_GetPrivateData(D3DVertexBuffer *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DVertexBuffer8_FreePrivateData(D3DVertexBuffer *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DVertexBuffer8_Lock(D3DVertexBuffer *pThis, UINT OffsetToLock, UINT UnusedSizeToLock, BYTE **ppbData, DWORD Flags) { *ppbData = D3DVertexBuffer_Lock2(pThis, Flags) + OffsetToLock; return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DVertexBuffer8_Unlock(D3DVertexBuffer *pThis) { D3DVertexBuffer_Unlock(pThis); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DVertexBuffer8_GetDesc(D3DVertexBuffer *pThis, D3DVERTEXBUFFER_DESC *pDesc) { D3DVertexBuffer_GetDesc(pThis, pDesc); return S_OK; }

#ifdef __cplusplus

D3DMINLINE HRESULT WINAPI D3DVertexBuffer::Lock(UINT OffsetToLock, UINT UnusedSizeToLock, BYTE **ppbData, DWORD Flags) { *ppbData = D3DVertexBuffer_Lock2(this, Flags) + OffsetToLock; return S_OK; }
D3DMINLINE HRESULT WINAPI D3DVertexBuffer::Unlock() { D3DVertexBuffer_Unlock(this); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DVertexBuffer::GetDesc(D3DVERTEXBUFFER_DESC *pDesc) { D3DVertexBuffer_GetDesc(this, pDesc); return S_OK; }

#endif __cplusplus

/* D3DIndexBuffer */

D3DINLINE ULONG   WINAPI D3DIndexBuffer_AddRef(D3DIndexBuffer *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI D3DIndexBuffer_Release(D3DIndexBuffer *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DIndexBuffer_GetDevice(D3DIndexBuffer *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); }
D3DINLINE D3DRESOURCETYPE WINAPI D3DIndexBuffer_GetType(D3DIndexBuffer *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI D3DIndexBuffer_IsBusy(D3DIndexBuffer *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DIndexBuffer_BlockUntilNotBusy(D3DIndexBuffer *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DIndexBuffer_MoveResourceMemory(D3DIndexBuffer *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI D3DIndexBuffer_Register(D3DIndexBuffer *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI D3DIndexBuffer_SetPrivateData(D3DIndexBuffer *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI D3DIndexBuffer_GetPrivateData(D3DIndexBuffer *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE void    WINAPI D3DIndexBuffer_FreePrivateData(D3DIndexBuffer *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); }

D3DINLINE void    WINAPI D3DIndexBuffer_Lock(D3DIndexBuffer *pThis, UINT OffsetToLock, UINT UnusedSizeToLock, BYTE **ppbData, DWORD UnusedFlags) { *(ppbData) = (BYTE*) (pThis)->Data + (OffsetToLock); }
D3DINLINE void    WINAPI D3DIndexBuffer_Unlock(D3DIndexBuffer *pThis) { }
void    WINAPI D3DIndexBuffer_GetDesc(D3DIndexBuffer *pThis, D3DINDEXBUFFER_DESC *pDesc);

// Compatibilty wrappers.

D3DINLINE ULONG   WINAPI IDirect3DIndexBuffer8_AddRef(D3DIndexBuffer *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI IDirect3DIndexBuffer8_Release(D3DIndexBuffer *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DIndexBuffer8_GetDevice(D3DIndexBuffer *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DIndexBuffer8_GetType(D3DIndexBuffer *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI IDirect3DIndexBuffer8_IsBusy(D3DIndexBuffer *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DIndexBuffer8_BlockUntilNotBusy(D3DIndexBuffer *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DIndexBuffer8_MoveResourceMemory(D3DIndexBuffer *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI IDirect3DIndexBuffer8_Register(D3DIndexBuffer *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI IDirect3DIndexBuffer8_SetPrivateData(D3DIndexBuffer *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DIndexBuffer8_GetPrivateData(D3DIndexBuffer *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DIndexBuffer8_FreePrivateData(D3DIndexBuffer *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DIndexBuffer8_Lock(D3DIndexBuffer *pThis, UINT OffsetToLock, UINT UnusedSizeToLock, BYTE **ppbData, DWORD UnusedFlags) { D3DIndexBuffer_Lock(pThis, OffsetToLock, UnusedSizeToLock, ppbData, UnusedFlags); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DIndexBuffer8_Unlock(D3DIndexBuffer *pThis) { D3DIndexBuffer_Unlock(pThis); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DIndexBuffer8_GetDesc(D3DIndexBuffer *pThis, D3DINDEXBUFFER_DESC *pDesc) { D3DIndexBuffer_GetDesc(pThis, pDesc); return S_OK; }

#ifdef __cplusplus

D3DMINLINE HRESULT WINAPI D3DIndexBuffer::Lock(UINT OffsetToLock, UINT UnusedSizeToLock, BYTE **ppbData, DWORD UnusedFlags) { D3DIndexBuffer_Lock(this, OffsetToLock, UnusedSizeToLock, ppbData, UnusedFlags); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DIndexBuffer::Unlock() { D3DIndexBuffer_Unlock(this); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DIndexBuffer::GetDesc(D3DINDEXBUFFER_DESC *pDesc) { D3DIndexBuffer_GetDesc(this, pDesc); return S_OK; }

#endif __cplusplus

/* D3DPalette */

D3DINLINE ULONG   WINAPI D3DPalette_AddRef(D3DPalette *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI D3DPalette_Release(D3DPalette *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DPalette_GetDevice(D3DPalette *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); }
D3DINLINE D3DRESOURCETYPE WINAPI D3DPalette_GetType(D3DPalette *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI D3DPalette_IsBusy(D3DPalette *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DPalette_BlockUntilNotBusy(D3DPalette *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DPalette_MoveResourceMemory(D3DPalette *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI D3DPalette_Register(D3DPalette *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI D3DPalette_SetPrivateData(D3DPalette *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI D3DPalette_GetPrivateData(D3DPalette *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE void    WINAPI D3DPalette_FreePrivateData(D3DPalette *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); }

D3DCOLOR*      WINAPI D3DPalette_Lock2(D3DPalette *pThis, DWORD Flags);
D3DPALETTESIZE WINAPI D3DPalette_GetSize(D3DPalette *pThis);
D3DINLINE void WINAPI D3DPalette_Unlock(D3DPalette *pThis) { }

D3DINLINE ULONG   WINAPI IDirect3DPalette8_AddRef(D3DPalette *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI IDirect3DPalette8_Release(D3DPalette *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DPalette8_GetDevice(D3DPalette *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DPalette8_GetType(D3DPalette *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI IDirect3DPalette8_IsBusy(D3DPalette *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DPalette8_BlockUntilNotBusy(D3DPalette *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DPalette8_MoveResourceMemory(D3DPalette *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI IDirect3DPalette8_Register(D3DPalette *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI IDirect3DPalette8_SetPrivateData(D3DPalette *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DPalette8_GetPrivateData(D3DPalette *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DPalette8_FreePrivateData(D3DPalette *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); return S_OK; }

D3DINLINE HRESULT WINAPI IDirect3DPalette8_Lock(D3DPalette *pThis, D3DCOLOR **ppColor, DWORD Flags) { *ppColor = D3DPalette_Lock2(pThis, Flags); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPalette8_Unlock(D3DPalette *pThis) { D3DPalette_Unlock(pThis); return S_OK; }
D3DINLINE D3DPALETTESIZE WINAPI IDirect3DPalette8_GetSize(D3DPalette *pThis) { return D3DPalette_GetSize(pThis); }

#ifdef __cplusplus

D3DMINLINE HRESULT WINAPI D3DPalette::Lock(D3DCOLOR **ppColor, DWORD Flags) { *ppColor = D3DPalette_Lock2(this, Flags); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPalette::Unlock() { D3DPalette_Unlock(this); return S_OK; }
D3DMINLINE D3DPALETTESIZE WINAPI D3DPalette::GetSize() { return D3DPalette_GetSize(this); }

#endif __cplusplus

/* D3DPushBuffer */

D3DINLINE ULONG   WINAPI D3DPushBuffer_AddRef(D3DPushBuffer *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI D3DPushBuffer_Release(D3DPushBuffer *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DPushBuffer_GetDevice(D3DPushBuffer *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); }
D3DINLINE D3DRESOURCETYPE WINAPI D3DPushBuffer_GetType(D3DPushBuffer *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI D3DPushBuffer_IsBusy(D3DPushBuffer *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DPushBuffer_BlockUntilNotBusy(D3DPushBuffer *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DPushBuffer_MoveResourceMemory(D3DPushBuffer *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI D3DPushBuffer_Register(D3DPushBuffer *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI D3DPushBuffer_SetPrivateData(D3DPushBuffer *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI D3DPushBuffer_GetPrivateData(D3DPushBuffer *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE void    WINAPI D3DPushBuffer_FreePrivateData(D3DPushBuffer *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); }

void    WINAPI D3DPushBuffer_Verify(D3DPushBuffer* pPushBuffer, BOOL StampResources);
void    WINAPI D3DPushBuffer_BeginFixup(D3DPushBuffer* pPushBuffer, D3DFixup* pFixup, BOOL NoWait);
HRESULT WINAPI D3DPushBuffer_EndFixup(D3DPushBuffer* pPushBuffer);
void    WINAPI D3DPushBuffer_RunPushBuffer(D3DPushBuffer* pPushBuffer, DWORD Offset, D3DPushBuffer* pDestinationPushBuffer, D3DFixup* pDestinationFixup);
void    WINAPI D3DPushBuffer_SetModelView(D3DPushBuffer* pPushBuffer, DWORD Offset, CONST D3DMATRIX* pModelView, CONST D3DMATRIX* pInverseModelView, CONST D3DMATRIX* pComposite);
void    WINAPI D3DPushBuffer_SetVertexBlendModelView(D3DPushBuffer* pPushBuffer, DWORD Offset, UINT Count, CONST D3DMATRIX* pModelViews, CONST D3DMATRIX* pInverseModelViews, CONST D3DMATRIX* pProjectionViewport);
void    WINAPI D3DPushBuffer_SetVertexShaderInput(D3DPushBuffer* pPushBuffer, DWORD Offset, DWORD Handle, UINT StreamCount, CONST D3DSTREAM_INPUT *pStreamInputs);
void    WINAPI D3DPushBuffer_SetVertexShaderInputDirect(D3DPushBuffer* pPushBuffer, DWORD Offset, D3DVERTEXATTRIBUTEFORMAT *pVAF, UINT StreamCount, CONST D3DSTREAM_INPUT *pStreamInputs);
void    WINAPI D3DPushBuffer_SetRenderTarget(D3DPushBuffer* pPushBuffer, DWORD Offset, D3DSurface* pRenderTarget, D3DSurface* pZBuffer);
void    WINAPI D3DPushBuffer_SetTexture(D3DPushBuffer* pPushBuffer, DWORD Offset, DWORD Stage, D3DBaseTexture *pTexture);
void    WINAPI D3DPushBuffer_SetPalette(D3DPushBuffer* pPushBuffer, DWORD Offset, DWORD Stage,D3DPalette *pPalette);
HRESULT WINAPI D3DPushBuffer_EndVisibilityTest(D3DPushBuffer* pPushBuffer, DWORD Offset, DWORD Index);
void    WINAPI D3DPushBuffer_SetVertexShaderConstant(D3DPushBuffer* pPushBuffer, DWORD Offset, INT Register, CONST void* pConstantData, DWORD ConstantCount);
void    WINAPI D3DPushBuffer_SetRenderState(D3DPushBuffer* pPushBuffer, DWORD Offset, D3DRENDERSTATETYPE State, DWORD Value);
void    WINAPI D3DPushBuffer_CopyRects(D3DPushBuffer* pPushBuffer, DWORD Offset, D3DSurface *pSourceSurface, D3DSurface *pDestinationSurface);
void    WINAPI D3DPushBuffer_Jump(D3DPushBuffer* pPushBuffer, DWORD Offset, UINT DestinationOffset);

D3DINLINE ULONG   WINAPI IDirect3DPushBuffer8_AddRef(D3DPushBuffer *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI IDirect3DPushBuffer8_Release(D3DPushBuffer *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_GetDevice(D3DPushBuffer *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DPushBuffer8_GetType(D3DPushBuffer *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI IDirect3DPushBuffer8_IsBusy(D3DPushBuffer *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DPushBuffer8_BlockUntilNotBusy(D3DPushBuffer *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DPushBuffer8_MoveResourceMemory(D3DPushBuffer *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI IDirect3DPushBuffer8_Register(D3DPushBuffer *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_SetPrivateData(D3DPushBuffer *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_GetPrivateData(D3DPushBuffer *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_FreePrivateData(D3DPushBuffer *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); return S_OK; }

D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_Verify(D3DPushBuffer* pPushBuffer, BOOL StampResources) { D3DPushBuffer_Verify(pPushBuffer, StampResources); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_BeginFixup(D3DPushBuffer* pPushBuffer, D3DFixup* pFixup, BOOL NoWait) { D3DPushBuffer_BeginFixup(pPushBuffer, pFixup, NoWait); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_EndFixup(D3DPushBuffer* pPushBuffer) { return D3DPushBuffer_EndFixup(pPushBuffer); }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_RunPushBuffer(D3DPushBuffer* pPushBuffer, DWORD Offset, D3DPushBuffer* pDestinationPushBuffer, D3DFixup* pDestinationFixup) { D3DPushBuffer_RunPushBuffer(pPushBuffer, Offset, pDestinationPushBuffer, pDestinationFixup); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_SetModelView(D3DPushBuffer* pPushBuffer, DWORD Offset, CONST D3DMATRIX* pModelView, CONST D3DMATRIX* pInverseModelView, CONST D3DMATRIX* pComposite) { D3DPushBuffer_SetModelView(pPushBuffer, Offset, pModelView, pInverseModelView, pComposite); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_SetVertexBlendModelView(D3DPushBuffer* pPushBuffer, DWORD Offset, UINT Count, CONST D3DMATRIX* pModelViews, CONST D3DMATRIX* pInverseModelViews, CONST D3DMATRIX* pProjectionViewport) { D3DPushBuffer_SetVertexBlendModelView(pPushBuffer, Offset, Count, pModelViews, pInverseModelViews, pProjectionViewport); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_SetVertexShaderInput(D3DPushBuffer* pPushBuffer, DWORD Offset, DWORD Handle, UINT StreamCount, CONST D3DSTREAM_INPUT *pStreamInputs) { D3DPushBuffer_SetVertexShaderInput(pPushBuffer, Offset, Handle, StreamCount, pStreamInputs); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_SetVertexShaderInputDirect(D3DPushBuffer* pPushBuffer, DWORD Offset, D3DVERTEXATTRIBUTEFORMAT *pVAF, UINT StreamCount, CONST D3DSTREAM_INPUT *pStreamInputs) { D3DPushBuffer_SetVertexShaderInputDirect(pPushBuffer, Offset, pVAF, StreamCount, pStreamInputs); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_SetRenderTarget(D3DPushBuffer* pPushBuffer, DWORD Offset, D3DSurface* pRenderTarget, D3DSurface* pZBuffer) { D3DPushBuffer_SetRenderTarget(pPushBuffer, Offset, pRenderTarget, pZBuffer); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_SetTexture(D3DPushBuffer* pPushBuffer, DWORD Offset, DWORD Stage, D3DBaseTexture *pTexture) { D3DPushBuffer_SetTexture(pPushBuffer, Offset, Stage, pTexture); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_SetPalette(D3DPushBuffer* pPushBuffer, DWORD Offset, DWORD Stage,D3DPalette *pPalette) { D3DPushBuffer_SetPalette(pPushBuffer, Offset, Stage, pPalette); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_EndVisibilityTest(D3DPushBuffer* pPushBuffer, DWORD Offset, DWORD Index) { return D3DPushBuffer_EndVisibilityTest(pPushBuffer, Offset, Index); }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_SetVertexShaderConstant(D3DPushBuffer* pPushBuffer, DWORD Offset, INT Register, CONST void* pConstantData, DWORD ConstantCount) { D3DPushBuffer_SetVertexShaderConstant(pPushBuffer, Offset, Register, pConstantData, ConstantCount); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_SetRenderState(D3DPushBuffer* pPushBuffer, DWORD Offset, D3DRENDERSTATETYPE State, DWORD Value) { D3DPushBuffer_SetRenderState(pPushBuffer, Offset, State, Value); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_CopyRects(D3DPushBuffer* pPushBuffer, DWORD Offset, D3DSurface *pSourceSurface, D3DSurface *pDestinationSurface) { D3DPushBuffer_CopyRects(pPushBuffer, Offset, pSourceSurface, pDestinationSurface); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_Jump(D3DPushBuffer* pPushBuffer, DWORD Offset, UINT DestinationOffset) { D3DPushBuffer_Jump(pPushBuffer, Offset, DestinationOffset); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DPushBuffer8_GetSize(D3DPushBuffer* pPushBuffer, DWORD* pSize) { *pSize = pPushBuffer->Size; return S_OK; }

#ifdef __cplusplus

D3DMINLINE HRESULT WINAPI D3DPushBuffer::Verify(BOOL StampResources) { D3DPushBuffer_Verify(this, StampResources); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::BeginFixup(D3DFixup* pFixup, BOOL NoWait) { D3DPushBuffer_BeginFixup(this, pFixup, NoWait); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::EndFixup() { return D3DPushBuffer_EndFixup(this); }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::RunPushBuffer(DWORD Offset, D3DPushBuffer* pDestinationPushBuffer, D3DFixup* pDestinationFixup) { D3DPushBuffer_RunPushBuffer(this, Offset, pDestinationPushBuffer, pDestinationFixup); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::SetModelView(DWORD Offset, CONST D3DMATRIX* pModelView, CONST D3DMATRIX* pInverseModelView, CONST D3DMATRIX* pComposite) { D3DPushBuffer_SetModelView(this, Offset, pModelView, pInverseModelView, pComposite); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::SetVertexBlendModelView(DWORD Offset, UINT Count, CONST D3DMATRIX* pModelViews, CONST D3DMATRIX* pInverseModelViews, CONST D3DMATRIX* pProjectionViewport) { D3DPushBuffer_SetVertexBlendModelView(this, Offset, Count, pModelViews, pInverseModelViews, pProjectionViewport); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::SetVertexShaderInput(DWORD Offset, DWORD Handle, UINT StreamCount, CONST D3DSTREAM_INPUT *pStreamInputs) { D3DPushBuffer_SetVertexShaderInput(this, Offset, Handle, StreamCount, pStreamInputs); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::SetVertexShaderInputDirect(DWORD Offset, D3DVERTEXATTRIBUTEFORMAT *pVAF, UINT StreamCount, CONST D3DSTREAM_INPUT *pStreamInputs) { D3DPushBuffer_SetVertexShaderInputDirect(this, Offset, pVAF, StreamCount, pStreamInputs); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::SetRenderTarget(DWORD Offset, D3DSurface* pRenderTarget, D3DSurface* pZBuffer) { D3DPushBuffer_SetRenderTarget(this, Offset, pRenderTarget, pZBuffer); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::SetTexture(DWORD Offset, DWORD Stage, D3DBaseTexture *pTexture) { D3DPushBuffer_SetTexture(this, Offset, Stage, pTexture); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::SetPalette(DWORD Offset, DWORD Stage,D3DPalette *pPalette) { D3DPushBuffer_SetPalette(this, Offset, Stage, pPalette); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::EndVisibilityTest(DWORD Offset, DWORD Index) { return D3DPushBuffer_EndVisibilityTest(this, Offset, Index); }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::SetVertexShaderConstant(DWORD Offset, INT Register, CONST void* pConstantData, DWORD ConstantCount) { D3DPushBuffer_SetVertexShaderConstant(this, Offset, Register, pConstantData, ConstantCount); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::SetRenderState(DWORD Offset, D3DRENDERSTATETYPE State, DWORD Value) { D3DPushBuffer_SetRenderState(this, Offset, State, Value); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::CopyRects(DWORD Offset, D3DSurface *pSourceSurface, D3DSurface *pDestinationSurface) { D3DPushBuffer_CopyRects(this, Offset, pSourceSurface, pDestinationSurface); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::Jump(DWORD Offset, UINT DestinationOffset) { D3DPushBuffer_Jump(this, Offset, DestinationOffset); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DPushBuffer::GetSize(DWORD* pSize) { *pSize = Size; return S_OK; }

#endif __cplusplus

/* D3DFixup */

D3DINLINE ULONG   WINAPI D3DFixup_AddRef(D3DFixup *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI D3DFixup_Release(D3DFixup *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DFixup_GetDevice(D3DFixup *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); }
D3DINLINE D3DRESOURCETYPE WINAPI D3DFixup_GetType(D3DFixup *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI D3DFixup_IsBusy(D3DFixup *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DFixup_BlockUntilNotBusy(D3DFixup *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DFixup_MoveResourceMemory(D3DFixup *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI D3DFixup_Register(D3DFixup *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI D3DFixup_SetPrivateData(D3DFixup *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI D3DFixup_GetPrivateData(D3DFixup *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE void    WINAPI D3DFixup_FreePrivateData(D3DFixup *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); }

void    WINAPI D3DFixup_Reset(D3DFixup* pFixup);
void    WINAPI D3DFixup_GetSize(D3DFixup* pFixup, DWORD* pSize);
void    WINAPI D3DFixup_GetSpace(D3DFixup* pFixup, DWORD* pSpace);

D3DINLINE ULONG   WINAPI IDirect3DFixup8_AddRef(D3DFixup *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI IDirect3DFixup8_Release(D3DFixup *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DFixup8_GetDevice(D3DFixup *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DFixup8_GetType(D3DFixup *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI IDirect3DFixup8_IsBusy(D3DFixup *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DFixup8_BlockUntilNotBusy(D3DFixup *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DFixup8_MoveResourceMemory(D3DFixup *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI IDirect3DFixup8_Register(D3DFixup *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI IDirect3DFixup8_SetPrivateData(D3DFixup *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DFixup8_GetPrivateData(D3DFixup *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DFixup8_FreePrivateData(D3DFixup *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); return S_OK; }

D3DINLINE HRESULT WINAPI IDirect3DFixup8_Reset(D3DFixup* pFixup) { D3DFixup_Reset(pFixup); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DFixup8_GetSize(D3DFixup* pFixup, DWORD* pSize) { D3DFixup_GetSize(pFixup, pSize); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DFixup8_GetSpace(D3DFixup* pFixup, DWORD* pSpace) { D3DFixup_GetSpace(pFixup, pSpace); return S_OK; }

#ifdef __cplusplus

D3DMINLINE HRESULT WINAPI D3DFixup::Reset() { D3DFixup_Reset(this); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DFixup::GetSize(DWORD* pSize) { D3DFixup_GetSize(this, pSize); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DFixup::GetSpace(DWORD* pSpace) { D3DFixup_GetSpace(this, pSpace); return S_OK; }

#endif __cplusplus

/* D3DSurface */

D3DINLINE ULONG   WINAPI D3DSurface_AddRef(D3DSurface *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI D3DSurface_Release(D3DSurface *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DSurface_GetDevice(D3DSurface *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); }
D3DINLINE D3DRESOURCETYPE WINAPI D3DSurface_GetType(D3DSurface *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI D3DSurface_IsBusy(D3DSurface *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DSurface_BlockUntilNotBusy(D3DSurface *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DSurface_MoveResourceMemory(D3DSurface *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI D3DSurface_Register(D3DSurface *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI D3DSurface_SetPrivateData(D3DSurface *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI D3DSurface_GetPrivateData(D3DSurface *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE void    WINAPI D3DSurface_FreePrivateData(D3DSurface *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); }

D3DBaseTexture* WINAPI D3DSurface_GetContainer2(D3DSurface *pThis);
void            WINAPI D3DSurface_GetDesc(D3DSurface *pThis, D3DSURFACE_DESC *pDesc);
void            WINAPI D3DSurface_LockRect(D3DSurface *pThis, D3DLOCKED_RECT *pLockedRect,CONST RECT *pRect, DWORD Flags);
D3DINLINE       void WINAPI D3DSurface_UnlockRect(D3DSurface *pThis) { }

D3DINLINE ULONG   WINAPI IDirect3DSurface8_AddRef(D3DSurface *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI IDirect3DSurface8_Release(D3DSurface *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DSurface8_GetDevice(D3DSurface *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DSurface8_GetType(D3DSurface *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI IDirect3DSurface8_IsBusy(D3DSurface *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DSurface8_BlockUntilNotBusy(D3DSurface *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DSurface8_MoveResourceMemory(D3DSurface *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI IDirect3DSurface8_Register(D3DSurface *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI IDirect3DSurface8_SetPrivateData(D3DSurface *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DSurface8_GetPrivateData(D3DSurface *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DSurface8_FreePrivateData(D3DSurface *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DSurface8_GetContainer(D3DSurface *pThis, D3DBaseTexture **ppBaseTexture) { return (*ppBaseTexture = D3DSurface_GetContainer2(pThis)) != NULL ? S_OK : E_FAIL; }
D3DINLINE HRESULT WINAPI IDirect3DSurface8_GetDesc(D3DSurface *pThis, D3DSURFACE_DESC *pDesc) { D3DSurface_GetDesc(pThis, pDesc); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DSurface8_LockRect(D3DSurface *pThis, D3DLOCKED_RECT *pLockedRect,CONST RECT *pRect, DWORD Flags) { D3DSurface_LockRect(pThis, pLockedRect,pRect, Flags); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DSurface8_UnlockRect(D3DSurface *pThis) { D3DSurface_UnlockRect(pThis); return S_OK; }

#ifdef __cplusplus

D3DMINLINE HRESULT WINAPI D3DSurface::GetContainer(D3DBaseTexture **ppBaseTexture) { return (*ppBaseTexture = D3DSurface_GetContainer2(this)) != NULL ? S_OK : E_FAIL; }
D3DMINLINE HRESULT WINAPI D3DSurface::GetDesc(D3DSURFACE_DESC *pDesc) { D3DSurface_GetDesc(this, pDesc); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DSurface::LockRect(D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect, DWORD Flags) { D3DSurface_LockRect(this, pLockedRect, pRect, Flags); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DSurface::UnlockRect() { D3DSurface_UnlockRect(this); return S_OK; }

#endif __cplusplus

/* D3DVolume */

D3DINLINE ULONG   WINAPI D3DVolume_AddRef(D3DVolume *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI D3DVolume_Release(D3DVolume *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DVolume_GetDevice(D3DVolume *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); }
D3DINLINE D3DRESOURCETYPE WINAPI D3DVolume_GetType(D3DVolume *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI D3DVolume_IsBusy(D3DVolume *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DVolume_BlockUntilNotBusy(D3DVolume *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI D3DVolume_MoveResourceMemory(D3DVolume *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI D3DVolume_Register(D3DVolume *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI D3DVolume_SetPrivateData(D3DVolume *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI D3DVolume_GetPrivateData(D3DVolume *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE void    WINAPI D3DVolume_FreePrivateData(D3DVolume *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); }

D3DBaseTexture* WINAPI D3DVolume_GetContainer2(D3DVolume *pThis);
void            WINAPI D3DVolume_GetDesc(D3DVolume *pThis, D3DVOLUME_DESC *pDesc);
void            WINAPI D3DVolume_LockBox(D3DVolume *pThis, D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags);
D3DINLINE void  WINAPI D3DVolume_UnlockBox(D3DVolume *pThis) { }

D3DINLINE ULONG   WINAPI IDirect3DVolume8_AddRef(D3DVolume *pThis) { return D3DResource_AddRef((D3DResource *)pThis); }
D3DINLINE ULONG   WINAPI IDirect3DVolume8_Release(D3DVolume *pThis) { return D3DResource_Release((D3DResource *)pThis); }
D3DINLINE HRESULT WINAPI IDirect3DVolume8_GetDevice(D3DVolume *pThis, D3DDevice **ppDevice) { D3DResource_GetDevice((D3DResource *)pThis, ppDevice); return S_OK; }
D3DINLINE D3DRESOURCETYPE WINAPI IDirect3DVolume8_GetType(D3DVolume *pThis) { return D3DResource_GetType((D3DResource *)pThis); }
D3DINLINE BOOL    WINAPI IDirect3DVolume8_IsBusy(D3DVolume *pThis) { return D3DResource_IsBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DVolume8_BlockUntilNotBusy(D3DVolume *pThis) { D3DResource_BlockUntilNotBusy((D3DResource *)pThis); }
D3DINLINE void    WINAPI IDirect3DVolume8_MoveResourceMemory(D3DVolume *pThis, D3DMEMORY where) { D3DResource_MoveResourceMemory((D3DResource *)pThis, where); }
D3DINLINE void    WINAPI IDirect3DVolume8_Register(D3DVolume *pThis, void *pBase) { D3DResource_Register((D3DResource *)pThis, pBase); }
D3DINLINE HRESULT WINAPI IDirect3DVolume8_SetPrivateData(D3DVolume *pThis, REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) { return D3DResource_SetPrivateData((D3DResource *)pThis, refguid, pData, SizeOfData, Flags); }
D3DINLINE HRESULT WINAPI IDirect3DVolume8_GetPrivateData(D3DVolume *pThis, REFGUID refguid, void *pData, DWORD *pSizeOfData) { return D3DResource_GetPrivateData((D3DResource *)pThis, refguid, pData, pSizeOfData); }
D3DINLINE HRESULT WINAPI IDirect3DVolume8_FreePrivateData(D3DVolume *pThis, REFGUID refguid) { D3DResource_FreePrivateData((D3DResource *)pThis, refguid); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DVolume8_GetContainer(D3DVolume *pThis, D3DBaseTexture **ppBaseTexture) { return (*ppBaseTexture = D3DVolume_GetContainer2(pThis)) != NULL ? S_OK : E_FAIL; }
D3DINLINE HRESULT WINAPI IDirect3DVolume8_GetDesc(D3DVolume *pThis, D3DVOLUME_DESC *pDesc) { D3DVolume_GetDesc(pThis, pDesc); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DVolume8_LockBox(D3DVolume *pThis, D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags) { D3DVolume_LockBox(pThis, pLockedVolume, pBox, Flags); return S_OK; }
D3DINLINE HRESULT WINAPI IDirect3DVolume8_UnlockBox(D3DVolume *pThis) { D3DVolume_UnlockBox(pThis); return S_OK; }

#ifdef __cplusplus

D3DMINLINE HRESULT WINAPI D3DVolume::GetContainer(D3DBaseTexture **ppBaseTexture) { return (*ppBaseTexture = D3DVolume_GetContainer2(this)) != NULL ? S_OK : E_FAIL; }
D3DMINLINE HRESULT WINAPI D3DVolume::GetDesc(D3DVOLUME_DESC *pDesc) { D3DVolume_GetDesc(this, pDesc); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DVolume::LockBox(D3DLOCKED_BOX *pLockedVolume, CONST D3DBOX *pBox, DWORD Flags) { D3DVolume_LockBox(this, pLockedVolume, pBox, Flags); return S_OK; }
D3DMINLINE HRESULT WINAPI D3DVolume::UnlockBox() { D3DVolume_UnlockBox(this); return S_OK; }

#endif __cplusplus

#if !D3DCOMPILE_NOTINLINE

    // Backwards compatibility wrappers for people who were directly using
    // internal interfaces that got changed.  They may not be so lucky the
    // next time we need to change our internals...

    D3DINLINE void WINAPI D3DDevice_GetBackBuffer(INT BackBuffer,D3DBACKBUFFER_TYPE UnusedType,D3DSurface** ppBackBuffer) {IDirect3DDevice8_GetBackBuffer(D3D__pDevice,BackBuffer,UnusedType,ppBackBuffer); }
    D3DINLINE HRESULT WINAPI D3DDevice_CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL UnusedPool, D3DTexture** ppTexture) {return IDirect3DDevice8_CreateTexture(D3D__pDevice,Width,Height,Levels,Usage,Format,UnusedPool,ppTexture );}
    D3DINLINE HRESULT WINAPI D3DDevice_CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL UnusedPool,D3DVolumeTexture** ppVolumeTexture) {return IDirect3DDevice8_CreateVolumeTexture(D3D__pDevice,Width,Height,Depth,Levels,Usage,Format,UnusedPool,ppVolumeTexture);}
    D3DINLINE HRESULT WINAPI D3DDevice_CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL UnusedPool,D3DCubeTexture** ppCubeTexture) { return IDirect3DDevice8_CreateCubeTexture(D3D__pDevice,EdgeLength,Levels,Usage,Format,UnusedPool,ppCubeTexture);}
    D3DINLINE HRESULT WINAPI D3DDevice_CreateVertexBuffer(UINT Length,DWORD UnusedUsage,DWORD UnusedFVF,D3DPOOL UnusedPool,D3DVertexBuffer **ppVertexBuffer){return IDirect3DDevice8_CreateVertexBuffer(D3D__pDevice,Length,UnusedUsage,UnusedFVF,UnusedPool,ppVertexBuffer);}
    D3DINLINE HRESULT WINAPI D3DDevice_CreateIndexBuffer(UINT Length,DWORD UnusedUsage,D3DFORMAT UnusedFormat,D3DPOOL UnusedPool,D3DIndexBuffer** ppIndexBuffer) {return IDirect3DDevice8_CreateIndexBuffer(D3D__pDevice,Length,UnusedUsage,UnusedFormat,UnusedPool,ppIndexBuffer);}
    D3DINLINE HRESULT WINAPI D3DDevice_CreatePalette(D3DPALETTESIZE Size, D3DPalette **ppPalette){return IDirect3DDevice8_CreatePalette(D3D__pDevice,Size,ppPalette);}
    D3DINLINE HRESULT WINAPI D3DDevice_CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE UnusedMultiSample, BOOL UnusedLockable, D3DSurface **ppSurface){return IDirect3DDevice8_CreateRenderTarget(D3D__pDevice,Width,Height,Format,UnusedMultiSample,UnusedLockable,ppSurface);}
    D3DINLINE HRESULT WINAPI D3DDevice_CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE UnusedMultiSample, D3DSurface **ppSurface){return IDirect3DDevice8_CreateDepthStencilSurface(D3D__pDevice,Width,Height,Format,UnusedMultiSample,ppSurface);}
    D3DINLINE HRESULT WINAPI D3DDevice_CreateImageSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DSurface** ppSurface) {return IDirect3DDevice8_CreateImageSurface(D3D__pDevice,Width,Height,Format,ppSurface);}
    D3DINLINE HRESULT WINAPI D3DDevice_GetRenderTarget(D3DSurface** ppRenderTarget) { return IDirect3DDevice8_GetRenderTarget(D3D__pDevice,ppRenderTarget);}
    D3DINLINE HRESULT WINAPI D3DDevice_GetDepthStencilSurface(D3DSurface** ppZStencilSurface) { return IDirect3DDevice8_GetDepthStencilSurface(D3D__pDevice,ppZStencilSurface);}
    D3DINLINE void WINAPI D3DDevice_GetTexture(DWORD Stage,D3DBaseTexture** ppTexture) {IDirect3DDevice8_GetTexture(D3D__pDevice,Stage,ppTexture);}
    D3DINLINE void WINAPI D3DDevice_GetPalette(DWORD Stage,D3DPalette** ppPalette) { IDirect3DDevice8_GetPalette(D3D__pDevice,Stage,ppPalette);}
    D3DINLINE void WINAPI D3DDevice_GetStreamSource(UINT StreamNumber,D3DVertexBuffer** ppVertexBuffer,UINT* pStride) {IDirect3DDevice8_GetStreamSource(D3D__pDevice,StreamNumber,ppVertexBuffer,pStride);}
    D3DINLINE void WINAPI D3DDevice_GetIndices(D3DIndexBuffer** ppIndexData,UINT* pBaseVertexIndex) {IDirect3DDevice8_GetIndices(D3D__pDevice,ppIndexData,pBaseVertexIndex);}
    D3DINLINE HRESULT WINAPI D3DDevice_CreateFixup(UINT Size,D3DFixup** ppFixup){return IDirect3DDevice8_CreateFixup(D3D__pDevice,Size,ppFixup);}
    D3DINLINE HRESULT WINAPI D3DDevice_CreatePushBuffer(UINT Size,BOOL RunUsingCpuCopy,D3DPushBuffer** ppPushBuffer){return IDirect3DDevice8_CreatePushBuffer(D3D__pDevice,Size,RunUsingCpuCopy,ppPushBuffer);}
    D3DINLINE void WINAPI D3DVertexBuffer_Lock(D3DVertexBuffer *pBuffer, UINT OffsetToLock, UINT UnusedSizeToLock, BYTE **ppbData, DWORD Flags){IDirect3DVertexBuffer8_Lock(pBuffer,OffsetToLock,UnusedSizeToLock,ppbData,Flags);}
    D3DINLINE void WINAPI D3DPalette_Lock(D3DPalette *pPalette, D3DCOLOR **ppColors, DWORD Flags){IDirect3DPalette8_Lock(pPalette,ppColors,Flags);}
    D3DINLINE HRESULT WINAPI D3DSurface_GetContainer(D3DSurface *pSurface,D3DBaseTexture **ppTexture) {return IDirect3DSurface8_GetContainer(pSurface,ppTexture);}
    D3DINLINE HRESULT WINAPI D3DVolume_GetContainer(D3DVolume *pVolume,D3DBaseTexture **ppTexture){return IDirect3DVolume8_GetContainer(pVolume,ppTexture);}
    D3DINLINE HRESULT WINAPI D3DTexture_GetSurfaceLevel(D3DTexture *pTexture, UINT Level, D3DSurface **ppSurfaceLevel){return IDirect3DTexture8_GetSurfaceLevel(pTexture,Level,ppSurfaceLevel);}
    D3DINLINE HRESULT WINAPI D3DVolumeTexture_GetVolumeLevel(D3DVolumeTexture *pTexture, UINT Level, D3DVolume **ppVolumeLevel){ return IDirect3DVolumeTexture8_GetVolumeLevel(pTexture,Level,ppVolumeLevel);}
    D3DINLINE HRESULT WINAPI D3DCubeTexture_GetCubeMapSurface(D3DCubeTexture *pTexture, D3DCUBEMAP_FACES FaceType, UINT Level, D3DSurface **ppCubeMapSurface){return IDirect3DCubeTexture8_GetCubeMapSurface(pTexture,FaceType,Level,ppCubeMapSurface);}
    D3DINLINE void WINAPI D3DDevice_GetPersistedSurface(IDirect3DSurface8 **ppSurface){IDirect3DDevice8_GetPersistedSurface(D3D__pDevice,ppSurface);}

#endif !D3DCOMPILE_NOTINLINE

#ifdef __cplusplus
};
#endif

#pragma warning( pop )

#endif /* _D3D_H_ */



