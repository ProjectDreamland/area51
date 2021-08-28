/*==========================================================================;
 *
 *  Copyright (C) Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       d3d8types.h
 *  Content:    Xbox Direct3D types include file
 *
 ***************************************************************************/

#ifndef _D3D8TYPES_H_
#define _D3D8TYPES_H_

#ifndef DIRECT3D_VERSION
#define DIRECT3D_VERSION         0x0800
#endif  //DIRECT3D_VERSION

// include this file content only if compiling for DX8 interfaces
#if(DIRECT3D_VERSION >= 0x0800)

#include <float.h>

#pragma warning(disable:4201) // anonymous unions warning
#pragma pack(4)

// D3DCOLOR is equivalent to D3DFMT_A8R8G8B8
#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif

// maps unsigned 8 bits/channel to D3DCOLOR
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#define D3DCOLOR_XRGB(r,g,b)   D3DCOLOR_ARGB(0xff,r,g,b)

// maps floating point channels (0.f to 1.f range) to D3DCOLOR
#define D3DCOLOR_COLORVALUE(r,g,b,a) \
    D3DCOLOR_RGBA((DWORD)((r)*255.f),(DWORD)((g)*255.f),(DWORD)((b)*255.f),(DWORD)((a)*255.f))

#ifndef D3DVECTOR_DEFINED
typedef struct _D3DVECTOR {
    float x;
    float y;
    float z;
} D3DVECTOR;
#define D3DVECTOR_DEFINED
#endif
typedef struct _D3DVECTOR *LPD3DVECTOR;

#ifndef D3DVECTOR4_DEFINED
typedef struct _D3DVECTOR4 {
    float x;
    float y;
    float z;
    float w;
} D3DVECTOR4;
#define D3DVECTOR4_DEFINED
#endif
typedef struct _D3DVECTOR4 *LPD3DVECTOR4;

#ifndef D3DCOLORVALUE_DEFINED
typedef struct _D3DCOLORVALUE {
    float r;
    float g;
    float b;
    float a;
} D3DCOLORVALUE;
#define D3DCOLORVALUE_DEFINED
#endif

#ifndef D3DRECT_DEFINED
typedef struct _D3DRECT {
    LONG x1;
    LONG y1;
    LONG x2;
    LONG y2;
} D3DRECT;
#define D3DRECT_DEFINED
#endif

#ifndef D3DMATRIX_DEFINED
typedef struct _D3DMATRIX {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
    };
} D3DMATRIX;
#define D3DMATRIX_DEFINED
#endif

typedef struct _D3DVIEWPORT8 {
    DWORD       X;
    DWORD       Y;            /* Viewport Top left */
    DWORD       Width;
    DWORD       Height;       /* Viewport Dimensions */
    float       MinZ;         /* Min/max of clip Volume */
    float       MaxZ;
} D3DVIEWPORT8;

typedef struct _D3DMATERIAL8 {
    D3DCOLORVALUE   Diffuse;        /* Diffuse color RGBA */
    D3DCOLORVALUE   Ambient;        /* Ambient color RGB */
    D3DCOLORVALUE   Specular;       /* Specular 'shininess' */
    D3DCOLORVALUE   Emissive;       /* Emissive color RGB */
    float           Power;          /* Sharpness if specular highlight */
} D3DMATERIAL8;

typedef enum _D3DLIGHTTYPE {
    D3DLIGHT_POINT          = 1,
    D3DLIGHT_SPOT           = 2,
    D3DLIGHT_DIRECTIONAL    = 3,
    D3DLIGHT_FORCE_DWORD    = 0x7fffffff, /* force 32-bit size enum */
} D3DLIGHTTYPE;

typedef struct _D3DLIGHT8 {
    D3DLIGHTTYPE    Type;            /* Type of light source */
    D3DCOLORVALUE   Diffuse;         /* Diffuse color of light */
    D3DCOLORVALUE   Specular;        /* Specular color of light */
    D3DCOLORVALUE   Ambient;         /* Ambient color of light */
    D3DVECTOR       Position;         /* Position in world space */
    D3DVECTOR       Direction;        /* Direction in world space */
    float           Range;            /* Cutoff range */
    float           Falloff;          /* Falloff */
    float           Attenuation0;     /* Constant attenuation */
    float           Attenuation1;     /* Linear attenuation */
    float           Attenuation2;     /* Quadratic attenuation */
    float           Theta;            /* Inner angle of spotlight cone */
    float           Phi;              /* Outer angle of spotlight cone */
} D3DLIGHT8;

/*
 * Options for clearing
 */
#define D3DCLEAR_TARGET            0x000000f0l  /* Clear target surface */
#define D3DCLEAR_ZBUFFER           0x00000001l  /* Clear target z buffer */
#define D3DCLEAR_STENCIL           0x00000002l  /* Clear stencil planes */

// The following are Xbox extensions
#define D3DCLEAR_TARGET_R          0x00000010l  /* Clear target surface R component */
#define D3DCLEAR_TARGET_G          0x00000020l  /* Clear target surface G component */
#define D3DCLEAR_TARGET_B          0x00000040l  /* Clear target surface B component */
#define D3DCLEAR_TARGET_A          0x00000080l  /* Clear target surface A component */

// The driver uses these values as the maximum value for Z in the z-buffer.

#define D3DZ_MAX_D16         65535.0
#define D3DZ_MAX_D24S8       16777215.0
#define D3DZ_MAX_F16         511.9375

// We can't use the whole available range for the 24-bit floating point z
// because the maximum value is very, very close to the IEEE single
// precision maximum.  Any calculation using the value will cause an overflow.
//
// Back off a bit from the max (3.4028e38),
//
#define D3DZ_MAX_F24S8       1.0e30

typedef DWORD D3DSHADERCONSTANTMODE;            // Xbox extension

#define D3DSCM_96CONSTANTS                  0
#define D3DSCM_192CONSTANTS                 1
#define D3DSCM_192CONSTANTSANDFIXEDPIPELINE 2
#define D3DSCM_NORESERVEDCONSTANTS          0x10    // Flag

// Primitives supported by draw-primitive API
typedef enum _D3DPRIMITIVETYPE {
    D3DPT_POINTLIST             = 1,
    D3DPT_LINELIST              = 2,
    D3DPT_LINELOOP              = 3,  // Xbox extension
    D3DPT_LINESTRIP             = 4,
    D3DPT_TRIANGLELIST          = 5,
    D3DPT_TRIANGLESTRIP         = 6,
    D3DPT_TRIANGLEFAN           = 7,
    D3DPT_QUADLIST              = 8,  // Xbox extension
    D3DPT_QUADSTRIP             = 9,  // Xbox extension
    D3DPT_POLYGON               = 10, // Xbox extension

    D3DPT_MAX                   = 11,
    D3DPT_FORCE_DWORD           = 0x7fffffff, /* force 32-bit size enum */
} D3DPRIMITIVETYPE;

typedef enum _D3DTRANSFORMSTATETYPE {
    D3DTS_VIEW          = 0,
    D3DTS_PROJECTION    = 1,
    D3DTS_TEXTURE0      = 2,
    D3DTS_TEXTURE1      = 3,
    D3DTS_TEXTURE2      = 4,
    D3DTS_TEXTURE3      = 5,
    D3DTS_WORLD         = 6,
    D3DTS_WORLD1        = 7,
    D3DTS_WORLD2        = 8,
    D3DTS_WORLD3        = 9,

    D3DTS_MAX           = 10,
    D3DTS_FORCE_DWORD   = 0x7fffffff, /* force 32-bit size enum */
} D3DTRANSFORMSTATETYPE;

#define D3DTS_WORLDMATRIX(index) (D3DTRANSFORMSTATETYPE)(index + D3DTS_WORLD)

/*
 * The following defines the rendering states
 */

typedef enum _D3DSHADEMODE {
    D3DSHADE_FLAT               = 0x1d00,
    D3DSHADE_GOURAUD            = 0x1d01,
    D3DSHADE_FORCE_DWORD        = 0x7fffffff, /* force 32-bit size enum */
} D3DSHADEMODE;

typedef enum _D3DFILLMODE {
    D3DFILL_POINT               = 0x1b00,
    D3DFILL_WIREFRAME           = 0x1b01,
    D3DFILL_SOLID               = 0x1b02,
    D3DFILL_FORCE_DWORD         = 0x7fffffff, /* force 32-bit size enum */
} D3DFILLMODE;

typedef enum _D3DBLEND {
    D3DBLEND_ZERO               = 0,
    D3DBLEND_ONE                = 1,
    D3DBLEND_SRCCOLOR           = 0x300,
    D3DBLEND_INVSRCCOLOR        = 0x301,
    D3DBLEND_SRCALPHA           = 0x302,
    D3DBLEND_INVSRCALPHA        = 0x303,
    D3DBLEND_DESTALPHA          = 0x304,
    D3DBLEND_INVDESTALPHA       = 0x305,
    D3DBLEND_DESTCOLOR          = 0x306,
    D3DBLEND_INVDESTCOLOR       = 0x307,
    D3DBLEND_SRCALPHASAT        = 0x308,
    D3DBLEND_CONSTANTCOLOR      = 0x8001,
    D3DBLEND_INVCONSTANTCOLOR   = 0x8002,
    D3DBLEND_CONSTANTALPHA      = 0x8003,
    D3DBLEND_INVCONSTANTALPHA   = 0x8004,

    // D3DBLEND_BOTHSRCALPHA not supported on Xbox
    // D3DBLEND_BOTHINVSRCALPHA not supported on Xbox

    D3DBLEND_FORCE_DWORD        = 0x7fffffff, /* force 32-bit size enum */
} D3DBLEND;

typedef enum _D3DBLENDOP {
    D3DBLENDOP_ADD              = 0x8006,
    D3DBLENDOP_SUBTRACT         = 0x800a,
    D3DBLENDOP_REVSUBTRACT      = 0x800b,
    D3DBLENDOP_MIN              = 0x8007,
    D3DBLENDOP_MAX              = 0x8008,
    D3DBLENDOP_ADDSIGNED        = 0xf006,       // Xbox extension
    D3DBLENDOP_REVSUBTRACTSIGNED= 0xf005,       // Xbox extension
    D3DBLENDOP_FORCE_DWORD      = 0x7fffffff, /* force 32-bit size enum */
} D3DBLENDOP;

typedef enum _D3DCULL {
    D3DCULL_NONE                = 0,
    D3DCULL_CW                  = 0x900,
    D3DCULL_CCW                 = 0x901,
    D3DCULL_FORCE_DWORD         = 0x7fffffff, /* force 32-bit size enum */
} D3DCULL;

typedef enum _D3DFRONT {                        // Xbox extension
    D3DFRONT_CW                 = 0x900,
    D3DFRONT_CCW                = 0x901,
    D3DFRONT_FORCE_DWORD        = 0x7fffffff, /* force 32-bit size enum */
} D3DFRONT;

typedef enum _D3DCMPFUNC {
    D3DCMP_NEVER                = 0x200,
    D3DCMP_LESS                 = 0x201,
    D3DCMP_EQUAL                = 0x202,
    D3DCMP_LESSEQUAL            = 0x203,
    D3DCMP_GREATER              = 0x204,
    D3DCMP_NOTEQUAL             = 0x205,
    D3DCMP_GREATEREQUAL         = 0x206,
    D3DCMP_ALWAYS               = 0x207,
    D3DCMP_FORCE_DWORD          = 0x7fffffff, /* force 32-bit size enum */
} D3DCMPFUNC;

typedef enum _D3DSTENCILOP {
    D3DSTENCILOP_KEEP           = 0x1e00,
    D3DSTENCILOP_ZERO           = 0,
    D3DSTENCILOP_REPLACE        = 0x1e01,
    D3DSTENCILOP_INCRSAT        = 0x1e02,
    D3DSTENCILOP_DECRSAT        = 0x1e03,
    D3DSTENCILOP_INVERT         = 0x150a,
    D3DSTENCILOP_INCR           = 0x8507,
    D3DSTENCILOP_DECR           = 0x8508,
    D3DSTENCILOP_FORCE_DWORD    = 0x7fffffff, /* force 32-bit size enum */
} D3DSTENCILOP;

typedef enum _D3DFOGMODE {
    D3DFOG_NONE                 = 0,
    D3DFOG_EXP                  = 1,
    D3DFOG_EXP2                 = 2,
    D3DFOG_LINEAR               = 3,
    D3DFOG_FORCE_DWORD          = 0x7fffffff, /* force 32-bit size enum */
} D3DFOGMODE;

typedef enum _D3DSWATHWIDTH {                   // Xbox extension
    D3DSWATH_8                  = 0,
    D3DSWATH_16                 = 1,
    D3DSWATH_32                 = 2,
    D3DSWATH_64                 = 3,
    D3DSWATH_128                = 4,
    D3DSWATH_OFF                = 0xf,
    D3DSWATH_FORCE_DWORD        = 0x7fffffff, /* force 32-bit size enum */
} D3DSWATHWIDTH;

typedef enum _D3DLOGICOP {                      // Xbox extension
    D3DLOGICOP_NONE             = 0,
    D3DLOGICOP_CLEAR            = 0x1500,
    D3DLOGICOP_AND              = 0x1501,
    D3DLOGICOP_AND_REVERSE      = 0x1502,
    D3DLOGICOP_COPY             = 0x1503,
    D3DLOGICOP_AND_INVERTED     = 0x1504,
    D3DLOGICOP_NOOP             = 0x1505,
    D3DLOGICOP_XOR              = 0x1506,
    D3DLOGICOP_OR               = 0x1507,
    D3DLOGICOP_NOR              = 0x1508,
    D3DLOGICOP_EQUIV            = 0x1509,
    D3DLOGICOP_INVERT           = 0x150a,
    D3DLOGICOP_OR_REVERSE       = 0x150b,
    D3DLOGICOP_COPY_INVERTED    = 0x150c,
    D3DLOGICOP_OR_INVERTED      = 0x150d,
    D3DLOGICOP_NAND             = 0x150e,
    D3DLOGICOP_SET              = 0x150f,
    D3DLOGICOP_FORCE_DWORD      = 0x7fffffff, /* force 32-bit size enum */
} D3DLOGICOP;

typedef enum _D3DZBUFFERTYPE {
    D3DZB_FALSE                 = 0,
    D3DZB_TRUE                  = 1, // Z buffering
    D3DZB_USEW                  = 2, // W buffering
    D3DZB_FORCE_DWORD           = 0x7fffffff, /* force 32-bit size enum */
} D3DZBUFFERTYPE;

// The D3DVERTEXBLENDFLAGS type is used with D3DRS_VERTEXBLEND state.
//
typedef enum _D3DVERTEXBLENDFLAGS
{
    D3DVBF_DISABLE           = 0,     // Disable vertex blending
    D3DVBF_1WEIGHTS          = 1,     // 2 matrix blending
    D3DVBF_2WEIGHTS          = 3,     // 3 matrix blending
    D3DVBF_3WEIGHTS          = 5,     // 4 matrix blending
    D3DVBF_2WEIGHTS2MATRICES = 2,     // Xbox extension
    D3DVBF_3WEIGHTS3MATRICES = 4,     // Xbox extension
    D3DVBF_4WEIGHTS4MATRICES = 6,     // Xbox extension

    D3DVBF_MAX               = 7,
    D3DVBF_FORCE_DWORD       = 0x7fffffff, // force 32-bit size enum
} D3DVERTEXBLENDFLAGS;

// Values for material source
typedef enum _D3DMATERIALCOLORSOURCE
{
    D3DMCS_MATERIAL         = 0,            // Color from material is used
    D3DMCS_COLOR1           = 1,            // Diffuse vertex color is used
    D3DMCS_COLOR2           = 2,            // Specular vertex color is used
    D3DMCS_FORCE_DWORD      = 0x7fffffff,   // force 32-bit size enum
} D3DMATERIALCOLORSOURCE;

//
// Values for D3DRS_DEPTHCLIPCONTROL renderstate (Xbox extension)
//
#define D3DDCC_CULLPRIMITIVE 0x001
#define D3DDCC_CLAMP         0x010
#define D3DDCC_IGNORE_W_SIGN 0x100

/* Flags to construct D3DRS_COLORWRITEENABLE */
#define D3DCOLORWRITEENABLE_RED     (1L<<16)
#define D3DCOLORWRITEENABLE_GREEN   (1L<<8)
#define D3DCOLORWRITEENABLE_BLUE    (1L<<0)
#define D3DCOLORWRITEENABLE_ALPHA   (1L<<24)
#define D3DCOLORWRITEENABLE_ALL     0x01010101 // Xbox extension

/* Flags to construct D3DRS_SAMPLEALPHA */
#define D3DSAMPLEALPHA_TOCOVERAGE   0x0010
#define D3DSAMPLEALPHA_TOONE        0x0100

// Bias to apply to the texture coordinate set to apply a wrap to.
#define D3DRENDERSTATE_WRAPBIAS     D3DRS_WRAP0

/* Flags to construct the WRAP render states */
#define D3DWRAP_U   0x00000010L
#define D3DWRAP_V   0x00001000L
#define D3DWRAP_W   0x00100000L

/* Flags to construct the WRAP render states for 1D thru 4D texture coordinates */
#define D3DWRAPCOORD_0   0x00000010L    // same as D3DWRAP_U
#define D3DWRAPCOORD_1   0x00001000L    // same as D3DWRAP_V
#define D3DWRAPCOORD_2   0x00100000L    // same as D3DWRAP_W
#define D3DWRAPCOORD_3   0x01000000L

typedef enum _D3DRENDERSTATETYPE {

    // Simple render states that are processed by D3D immediately:

    D3DRS_PS_MIN                        = 0,

    // The following pixel-shader renderstates are all Xbox extensions:

    D3DRS_PSALPHAINPUTS0                = 0,   // Pixel shader, Stage 0 alpha inputs
    D3DRS_PSALPHAINPUTS1                = 1,   // Pixel shader, Stage 1 alpha inputs
    D3DRS_PSALPHAINPUTS2                = 2,   // Pixel shader, Stage 2 alpha inputs
    D3DRS_PSALPHAINPUTS3                = 3,   // Pixel shader, Stage 3 alpha inputs
    D3DRS_PSALPHAINPUTS4                = 4,   // Pixel shader, Stage 4 alpha inputs
    D3DRS_PSALPHAINPUTS5                = 5,   // Pixel shader, Stage 5 alpha inputs
    D3DRS_PSALPHAINPUTS6                = 6,   // Pixel shader, Stage 6 alpha inputs
    D3DRS_PSALPHAINPUTS7                = 7,   // Pixel shader, Stage 7 alpha inputs
    D3DRS_PSFINALCOMBINERINPUTSABCD     = 8,   // Pixel shader, Final combiner inputs ABCD
    D3DRS_PSFINALCOMBINERINPUTSEFG      = 9,   // Pixel shader, Final combiner inputs EFG
    D3DRS_PSCONSTANT0_0                 = 10,  // Pixel shader, C0 in stage 0
    D3DRS_PSCONSTANT0_1                 = 11,  // Pixel shader, C0 in stage 1
    D3DRS_PSCONSTANT0_2                 = 12,  // Pixel shader, C0 in stage 2
    D3DRS_PSCONSTANT0_3                 = 13,  // Pixel shader, C0 in stage 3
    D3DRS_PSCONSTANT0_4                 = 14,  // Pixel shader, C0 in stage 4
    D3DRS_PSCONSTANT0_5                 = 15,  // Pixel shader, C0 in stage 5
    D3DRS_PSCONSTANT0_6                 = 16,  // Pixel shader, C0 in stage 6
    D3DRS_PSCONSTANT0_7                 = 17,  // Pixel shader, C0 in stage 7
    D3DRS_PSCONSTANT1_0                 = 18,  // Pixel shader, C1 in stage 0
    D3DRS_PSCONSTANT1_1                 = 19,  // Pixel shader, C1 in stage 1
    D3DRS_PSCONSTANT1_2                 = 20,  // Pixel shader, C1 in stage 2
    D3DRS_PSCONSTANT1_3                 = 21,  // Pixel shader, C1 in stage 3
    D3DRS_PSCONSTANT1_4                 = 22,  // Pixel shader, C1 in stage 4
    D3DRS_PSCONSTANT1_5                 = 23,  // Pixel shader, C1 in stage 5
    D3DRS_PSCONSTANT1_6                 = 24,  // Pixel shader, C1 in stage 6
    D3DRS_PSCONSTANT1_7                 = 25,  // Pixel shader, C1 in stage 7
    D3DRS_PSALPHAOUTPUTS0               = 26,  // Pixel shader, Stage 0 alpha outputs
    D3DRS_PSALPHAOUTPUTS1               = 27,  // Pixel shader, Stage 1 alpha outputs
    D3DRS_PSALPHAOUTPUTS2               = 28,  // Pixel shader, Stage 2 alpha outputs
    D3DRS_PSALPHAOUTPUTS3               = 29,  // Pixel shader, Stage 3 alpha outputs
    D3DRS_PSALPHAOUTPUTS4               = 30,  // Pixel shader, Stage 4 alpha outputs
    D3DRS_PSALPHAOUTPUTS5               = 31,  // Pixel shader, Stage 5 alpha outputs
    D3DRS_PSALPHAOUTPUTS6               = 32,  // Pixel shader, Stage 6 alpha outputs
    D3DRS_PSALPHAOUTPUTS7               = 33,  // Pixel shader, Stage 7 alpha outputs
    D3DRS_PSRGBINPUTS0                  = 34,  // Pixel shader, Stage 0 RGB inputs
    D3DRS_PSRGBINPUTS1                  = 35,  // Pixel shader, Stage 1 RGB inputs
    D3DRS_PSRGBINPUTS2                  = 36,  // Pixel shader, Stage 2 RGB inputs
    D3DRS_PSRGBINPUTS3                  = 37,  // Pixel shader, Stage 3 RGB inputs
    D3DRS_PSRGBINPUTS4                  = 38,  // Pixel shader, Stage 4 RGB inputs
    D3DRS_PSRGBINPUTS5                  = 39,  // Pixel shader, Stage 5 RGB inputs
    D3DRS_PSRGBINPUTS6                  = 40,  // Pixel shader, Stage 6 RGB inputs
    D3DRS_PSRGBINPUTS7                  = 41,  // Pixel shader, Stage 7 RGB inputs
    D3DRS_PSCOMPAREMODE                 = 42,  // Pixel shader, Compare modes for clipplane texture mode
    D3DRS_PSFINALCOMBINERCONSTANT0      = 43,  // Pixel shader, C0 in final combiner
    D3DRS_PSFINALCOMBINERCONSTANT1      = 44,  // Pixel shader, C1 in final combiner
    D3DRS_PSRGBOUTPUTS0                 = 45,  // Pixel shader, Stage 0 RGB outputs
    D3DRS_PSRGBOUTPUTS1                 = 46,  // Pixel shader, Stage 1 RGB outputs
    D3DRS_PSRGBOUTPUTS2                 = 47,  // Pixel shader, Stage 2 RGB outputs
    D3DRS_PSRGBOUTPUTS3                 = 48,  // Pixel shader, Stage 3 RGB outputs
    D3DRS_PSRGBOUTPUTS4                 = 49,  // Pixel shader, Stage 4 RGB outputs
    D3DRS_PSRGBOUTPUTS5                 = 50,  // Pixel shader, Stage 5 RGB outputs
    D3DRS_PSRGBOUTPUTS6                 = 51,  // Pixel shader, Stage 6 RGB outputs
    D3DRS_PSRGBOUTPUTS7                 = 52,  // Pixel shader, Stage 7 RGB outputs
    D3DRS_PSCOMBINERCOUNT               = 53,  // Pixel shader, Active combiner count (Stages 0-7)
                                               // Pixel shader, Reserved
    D3DRS_PSDOTMAPPING                  = 55,  // Pixel shader, Input mapping for dot product modes
    D3DRS_PSINPUTTEXTURE                = 56,  // Pixel shader, Texture source for some texture modes

    D3DRS_PS_MAX                        = 57,

    D3DRS_ZFUNC                         = 57,  // D3DCMPFUNC
    D3DRS_ALPHAFUNC                     = 58,  // D3DCMPFUNC
    D3DRS_ALPHABLENDENABLE              = 59,  // TRUE to enable alpha blending
    D3DRS_ALPHATESTENABLE               = 60,  // TRUE to enable alpha tests
    D3DRS_ALPHAREF                      = 61,  // BYTE
    D3DRS_SRCBLEND                      = 62,  // D3DBLEND
    D3DRS_DESTBLEND                     = 63,  // D3DBLEND
    D3DRS_ZWRITEENABLE                  = 64,  // TRUE to enable Z writes
    D3DRS_DITHERENABLE                  = 65,  // TRUE to enable dithering
    D3DRS_SHADEMODE                     = 66,  // D3DSHADEMODE
    D3DRS_COLORWRITEENABLE              = 67,  // D3DCOLORWRITEENABLE_ALPHA, etc. per-channel write enable
    D3DRS_STENCILZFAIL                  = 68,  // D3DSTENCILOP to do if stencil test passes and Z test fails
    D3DRS_STENCILPASS                   = 69,  // D3DSTENCILOP to do if both stencil and Z tests pass
    D3DRS_STENCILFUNC                   = 70,  // D3DCMPFUNC
    D3DRS_STENCILREF                    = 71,  // BYTE reference value used in stencil test
    D3DRS_STENCILMASK                   = 72,  // BYTE mask value used in stencil test
    D3DRS_STENCILWRITEMASK              = 73,  // BYTE write mask applied to values written to stencil buffer
    D3DRS_BLENDOP                       = 74,  // D3DBLENDOP setting
    D3DRS_BLENDCOLOR                    = 75,  // D3DCOLOR for D3DBLEND_CONSTANTCOLOR, etc. (Xbox extension)
    D3DRS_SWATHWIDTH                    = 76,  // D3DSWATHWIDTH (Xbox extension)
    D3DRS_POLYGONOFFSETZSLOPESCALE      = 77,  // float Z factor for shadow maps (Xbox extension)
    D3DRS_POLYGONOFFSETZOFFSET          = 78,  // float bias for polygon offset (Xbox extension)
    D3DRS_POINTOFFSETENABLE             = 79,  // TRUE to enable polygon offset for points (Xbox extension)
    D3DRS_WIREFRAMEOFFSETENABLE         = 80,  // TRUE to enable polygon offset for lines (Xbox extension)
    D3DRS_SOLIDOFFSETENABLE             = 81,  // TRUE to enable polygon offset for fills (Xbox extension)
    D3DRS_DEPTHCLIPCONTROL              = 82,  // D3DDCC_CULLPRIMITIVE, etc. (Xbox extension)
    D3DRS_STIPPLEENABLE                 = 83,  // TRUE to enable stipple for polygons (Xbox extension)
    D3DRS_SIMPLE_UNUSED8                = 84,  // Reserved
    D3DRS_SIMPLE_UNUSED7                = 85,  // Reserved
    D3DRS_SIMPLE_UNUSED6                = 86,  // Reserved
    D3DRS_SIMPLE_UNUSED5                = 87,  // Reserved
    D3DRS_SIMPLE_UNUSED4                = 88,  // Reserved
    D3DRS_SIMPLE_UNUSED3                = 89,  // Reserved
    D3DRS_SIMPLE_UNUSED2                = 90,  // Reserved
    D3DRS_SIMPLE_UNUSED1                = 91,  // Reserved

    D3DRS_SIMPLE_MAX                    = 92,

    // State whose handling is deferred until the next Draw[Indexed]Vertices
    // call because of interdependencies on other states:

    D3DRS_FOGENABLE                     = 92,  // TRUE to enable fog blending
    D3DRS_FOGTABLEMODE                  = 93,  // D3DFOGMODE
    D3DRS_FOGSTART                      = 94,  // float fog start (for both vertex and pixel fog)
    D3DRS_FOGEND                        = 95,  // float fog end
    D3DRS_FOGDENSITY                    = 96,  // float fog density
    D3DRS_RANGEFOGENABLE                = 97,  // TRUE to enable range-based fog
    D3DRS_WRAP0                         = 98,  // D3DWRAPCOORD_0, etc. for 1st texture coord.
    D3DRS_WRAP1                         = 99,  // D3DWRAPCOORD_0, etc. for 2nd texture coord.
    D3DRS_WRAP2                         = 100, // D3DWRAPCOORD_0, etc. for 3rd texture coord.
    D3DRS_WRAP3                         = 101, // D3DWRAPCOORD_0, etc. for 4th texture coord.
    D3DRS_LIGHTING                      = 102, // TRUE to enable lighting
    D3DRS_SPECULARENABLE                = 103, // TRUE to enable specular
    D3DRS_LOCALVIEWER                   = 104, // TRUE to enable camera-relative specular highlights
    D3DRS_COLORVERTEX                   = 105, // TRUE to enable per-vertex color
    D3DRS_BACKSPECULARMATERIALSOURCE    = 106, // D3DMATERIALCOLORSOURCE (Xbox extension)
    D3DRS_BACKDIFFUSEMATERIALSOURCE     = 107, // D3DMATERIALCOLORSOURCE (Xbox extension)
    D3DRS_BACKAMBIENTMATERIALSOURCE     = 108, // D3DMATERIALCOLORSOURCE (Xbox extension)
    D3DRS_BACKEMISSIVEMATERIALSOURCE    = 109, // D3DMATERIALCOLORSOURCE (Xbox extension)
    D3DRS_SPECULARMATERIALSOURCE        = 110, // D3DMATERIALCOLORSOURCE
    D3DRS_DIFFUSEMATERIALSOURCE         = 111, // D3DMATERIALCOLORSOURCE
    D3DRS_AMBIENTMATERIALSOURCE         = 112, // D3DMATERIALCOLORSOURCE
    D3DRS_EMISSIVEMATERIALSOURCE        = 113, // D3DMATERIALCOLORSOURCE
    D3DRS_BACKAMBIENT                   = 114, // D3DCOLOR (Xbox extension)
    D3DRS_AMBIENT                       = 115, // D3DCOLOR
    D3DRS_POINTSIZE                     = 116, // float point size
    D3DRS_POINTSIZE_MIN                 = 117, // float point size min threshold
    D3DRS_POINTSPRITEENABLE             = 118, // TRUE to enable point sprites
    D3DRS_POINTSCALEENABLE              = 119, // TRUE to enable point size scaling
    D3DRS_POINTSCALE_A                  = 120, // float point attenuation A value
    D3DRS_POINTSCALE_B                  = 121, // float point attenuation B value
    D3DRS_POINTSCALE_C                  = 122, // float point attenuation C value
    D3DRS_POINTSIZE_MAX                 = 123, // float point size max threshold
    D3DRS_PATCHEDGESTYLE                = 124, // D3DPATCHEDGESTYLE
    D3DRS_PATCHSEGMENTS                 = 125, // DWORD number of segments per edge when drawing patches
    D3DRS_SWAPFILTER                    = 126, // D3DTEXF_LINEAR etc. filter to use for Swap (Xbox extension)
    D3DRS_PRESENTATIONINTERVAL          = 127, // D3DPRESENT_INTERVAL_ONE, etc. (Xbox extension)
    D3DRS_DEFERRED_UNUSED8              = 128, // Reserved
    D3DRS_DEFERRED_UNUSED7              = 129, // Reserved
    D3DRS_DEFERRED_UNUSED6              = 130, // Reserved
    D3DRS_DEFERRED_UNUSED5              = 131, // Reserved
    D3DRS_DEFERRED_UNUSED4              = 132, // Reserved
    D3DRS_DEFERRED_UNUSED3              = 133, // Reserved
    D3DRS_DEFERRED_UNUSED2              = 134, // Reserved
    D3DRS_DEFERRED_UNUSED1              = 135, // Reserved

    D3DRS_DEFERRED_MAX                  = 136,

    // Complex state that has immediate processing:

    D3DRS_PSTEXTUREMODES                = 136, // Pixel shader, Texture addressing modes (Xbox extension)
    D3DRS_VERTEXBLEND                   = 137, // D3DVERTEXBLENDFLAGS
    D3DRS_FOGCOLOR                      = 138, // D3DCOLOR
    D3DRS_FILLMODE                      = 139, // D3DFILLMODE
    D3DRS_BACKFILLMODE                  = 140, // D3DFILLMODE (Xbox extension)
    D3DRS_TWOSIDEDLIGHTING              = 141, // TRUE to enable two-sided lighting (Xbox extension)
    D3DRS_NORMALIZENORMALS              = 142, // TRUE to enable automatic normalization
    D3DRS_ZENABLE                       = 143, // D3DZBUFFERTYPE (or TRUE/FALSE for legacy)
    D3DRS_STENCILENABLE                 = 144, // TRUE to enable stenciling
    D3DRS_STENCILFAIL                   = 145, // D3DSTENCILOP to do if stencil test fails
    D3DRS_FRONTFACE                     = 146, // D3DFRONT (Xbox extension)
    D3DRS_CULLMODE                      = 147, // D3DCULL
    D3DRS_TEXTUREFACTOR                 = 148, // D3DCOLOR used for multi-texture blend
    D3DRS_ZBIAS                         = 149, // LONG Z bias
    D3DRS_LOGICOP                       = 150, // D3DLOGICOP (Xbox extension)
    D3DRS_EDGEANTIALIAS                 = 151, // TRUE to enable edge antialiasing (Xbox extension)
    D3DRS_MULTISAMPLEANTIALIAS          = 152, // TRUE to enable multisample antialiasing
    D3DRS_MULTISAMPLEMASK               = 153, // DWORD per-pixel and per-sample enable/disable
    D3DRS_MULTISAMPLEMODE               = 154, // D3DMULTISAMPLEMODE for the backbuffer (Xbox extension)
    D3DRS_MULTISAMPLERENDERTARGETMODE   = 155, // D3DMULTISAMPLEMODE for non-backbuffer render targets (Xbox extension)
    D3DRS_SHADOWFUNC                    = 156, // D3DCMPFUNC (Xbox extension)
    D3DRS_LINEWIDTH                     = 157, // float (Xbox extension)
    D3DRS_SAMPLEALPHA                   = 158, // D3DSAMPLEALPHA_TOCOVERAGE, etc. (Xbox extension)
    D3DRS_DXT1NOISEENABLE               = 159, // TRUE to enable DXT1 decompression noise (Xbox extension)
    D3DRS_YUVENABLE                     = 160, // TRUE to enable use of D3DFMT_YUY2 and D3DFMT_UYVY texture formats (Xbox extension)
    D3DRS_OCCLUSIONCULLENABLE           = 161, // TRUE to enable Z occlusion culling (Xbox extension)
    D3DRS_STENCILCULLENABLE             = 162, // TRUE to enable stencil culling (Xbox extension)
    D3DRS_ROPZCMPALWAYSREAD             = 163, // TRUE to always read target packet when Z enabled (Xbox extension)
    D3DRS_ROPZREAD                      = 164, // TRUE to always read Z (Xbox extension)
    D3DRS_DONOTCULLUNCOMPRESSED         = 165, // TRUE to never attempt occlusion culling (stencil or Z) on uncompressed packets (Xbox extension)

    D3DRS_MAX                           = 166, // Total number of renderstates

    // Render states that are not supported on Xbox:
    //
    // D3DRS_LINEPATTERN
    // D3DRS_LASTPIXEL
    // D3DRS_CLIPPING
    // D3DRS_FOGVERTEXMODE
    // D3DRS_CLIPPLANEENABLE
    // D3DRS_SOFTWAREVERTEXPROCESSING
    // D3DRS_DEBUGMONITORTOKEN
    // D3DRS_INDEXEDVERTEXBLENDENABLE
    // D3DRS_TWEENFACTOR

    D3DRS_FORCE_DWORD                   = 0x7fffffff, /* force 32-bit size enum */
} D3DRENDERSTATETYPE;

/*
 * The maximum number of texture stages supported on Xbox
 */
#define D3DTSS_MAXSTAGES 4

// Values, used with D3DTSS_TEXCOORDINDEX, to specify that the vertex data(position
// and normal in the camera space) should be taken as texture coordinates
// Low 16 bits are used to specify texture coordinate index, to take the WRAP mode from
//
#define D3DTSS_TCI_PASSTHRU                             0x00000000
#define D3DTSS_TCI_CAMERASPACENORMAL                    0x00010000
#define D3DTSS_TCI_CAMERASPACEPOSITION                  0x00020000
#define D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR          0x00030000
#define D3DTSS_TCI_OBJECT                               0x00040000

#define D3DTSS_TCI_TEXCOORDINDEX_MAX                    8
#define D3DTSS_TCI_TEXGEN_MAX                           0x00060000

/*
 * Enumerations for COLOROP and ALPHAOP texture blending operations set in
 * texture processing stage controls in D3DRENDERSTATE.
 */
typedef enum _D3DTEXTUREOP
{
    // Control
    D3DTOP_DISABLE              = 1,      // disables stage
    D3DTOP_SELECTARG1           = 2,      // the default
    D3DTOP_SELECTARG2           = 3,

    // Modulate
    D3DTOP_MODULATE             = 4,      // multiply args together
    D3DTOP_MODULATE2X           = 5,      // multiply and  1 bit
    D3DTOP_MODULATE4X           = 6,      // multiply and  2 bits

    // Add
    D3DTOP_ADD                  =  7,   // add arguments together
    D3DTOP_ADDSIGNED            =  8,   // add with -0.5 bias
    D3DTOP_ADDSIGNED2X          =  9,   // as above but left  1 bit
    D3DTOP_SUBTRACT             = 10,   // Arg1 - Arg2, with no saturation
    D3DTOP_ADDSMOOTH            = 11,   // add 2 args, subtract product
                                        // Arg1 + Arg2 - Arg1*Arg2
                                        // = Arg1 + (1-Arg1)*Arg2

    // Linear alpha blend: Arg1*(Alpha) + Arg2*(1-Alpha)
    D3DTOP_BLENDDIFFUSEALPHA    = 12, // iterated alpha
    D3DTOP_BLENDTEXTUREALPHA    = 14, // texture alpha
    D3DTOP_BLENDFACTORALPHA     = 15, // alpha from D3DRENDERSTATE_TEXTUREFACTOR

    // Linear alpha blend with pre-multiplied arg1 input: Arg1 + Arg2*(1-Alpha)
    D3DTOP_BLENDTEXTUREALPHAPM  = 16, // texture alpha
    D3DTOP_BLENDCURRENTALPHA    = 13, // by alpha of current color

    // Specular mapping
    D3DTOP_PREMODULATE            = 17,     // modulate with next texture before use
    D3DTOP_MODULATEALPHA_ADDCOLOR = 18,     // Arg1.RGB + Arg1.A*Arg2.RGB
                                            // COLOROP only
    D3DTOP_MODULATECOLOR_ADDALPHA = 19,     // Arg1.RGB*Arg2.RGB + Arg1.A
                                            // COLOROP only
    D3DTOP_MODULATEINVALPHA_ADDCOLOR = 20,  // (1-Arg1.A)*Arg2.RGB + Arg1.RGB
                                            // COLOROP only
    D3DTOP_MODULATEINVCOLOR_ADDALPHA = 21,  // (1-Arg1.RGB)*Arg2.RGB + Arg1.A
                                            // COLOROP only

    // This can do either diffuse or specular bump mapping with correct input.
    // Performs the function (Arg1.R*Arg2.R + Arg1.G*Arg2.G + Arg1.B*Arg2.B)
    // where each component has been scaled and offset to make it signed.
    // The result is replicated into all four (including alpha) channels.
    // This is a valid COLOROP only.
    D3DTOP_DOTPRODUCT3          = 22,

    // Triadic ops
    D3DTOP_MULTIPLYADD          = 23, // Arg0 + Arg1*Arg2
    D3DTOP_LERP                 = 24, // (Arg0)*Arg1 + (1-Arg0)*Arg2

    // Bump mapping
    D3DTOP_BUMPENVMAP           = 25, // per pixel env map perturbation
    D3DTOP_BUMPENVMAPLUMINANCE  = 26, // with luminance channel

    D3DTOP_MAX                  = 27,
    D3DTOP_FORCE_DWORD = 0x7fffffff,
} D3DTEXTUREOP;

typedef enum _D3DTEXTUREADDRESS {
    D3DTADDRESS_WRAP            = 1,
    D3DTADDRESS_MIRROR          = 2,
    D3DTADDRESS_CLAMP           = 3,
    D3DTADDRESS_BORDER          = 4,
    D3DTADDRESS_CLAMPTOEDGE     = 5, // Xbox extension

    // D3DTADDRESS_MIRRORONCE not supported on Xbox

    D3DTADDRESS_MAX             = 6,
    D3DTADDRESS_FORCE_DWORD     = 0x7fffffff, /* force 32-bit size enum */
} D3DTEXTUREADDRESS;

typedef enum _D3DTEXTURETRANSFORMFLAGS
{
    D3DTTFF_DISABLE         = 0,    // texture coordinates are passed directly
    D3DTTFF_COUNT1          = 1,    // rasterizer should expect 1-D texture coords
    D3DTTFF_COUNT2          = 2,    // rasterizer should expect 2-D texture coords
    D3DTTFF_COUNT3          = 3,    // rasterizer should expect 3-D texture coords
    D3DTTFF_COUNT4          = 4,    // rasterizer should expect 4-D texture coords
    D3DTTFF_PROJECTED       = 256,  // texcoords to be divided by COUNTth element

    D3DTTFF_FORCE_DWORD     = 0x7fffffff,
} D3DTEXTURETRANSFORMFLAGS;

typedef enum _D3DTEXTURECOLORKEYOP {            // Xbox extension
    D3DTCOLORKEYOP_DISABLE      = 0,
    D3DTCOLORKEYOP_ALPHA        = 1,
    D3DTCOLORKEYOP_RGBA         = 2,
    D3DTCOLORKEYOP_KILL         = 3,

    D3DTCOLORKEYOP_MAX          = 4,
    D3DTCOLORKEYOP_FORCE_DWORD  = 0x7fffffff, /* force 32-bit size enum */
} D3DTEXTURECOLORKEYOP;

typedef enum _D3DTEXTUREALPHAKILL {             // Xbox extension
    D3DTALPHAKILL_DISABLE       = 0,
    D3DTALPHAKILL_ENABLE        = 4,

    D3DTALPHAKILL_FORCE_DWORD   = 0x7fffffff, /* force 32-bit size enum */
} D3DTEXTUREALPHAKILL;

typedef enum _D3DPALETTESIZE {                  // Xbox extension
    D3DPALETTE_256              = 0,
    D3DPALETTE_128              = 1,
    D3DPALETTE_64               = 2,
    D3DPALETTE_32               = 3,

    D3DPALETTE_MAX              = 4,
    D3DPALETTE_FORCE_DWORD      = 0x7fffffff, /* force 32-bit size enum */
} D3DPALETTESIZE;

/*
 * Values for COLORARG0,1,2, ALPHAARG0,1,2, and RESULTARG texture blending
 * operations set in texture processing stage controls in D3DRENDERSTATE.
 */
#define D3DTA_SELECTMASK        0x0000000f  // mask for arg selector
#define D3DTA_DIFFUSE           0x00000000  // select diffuse color (read only)
#define D3DTA_CURRENT           0x00000001  // select stage destination register (read/write)
#define D3DTA_TEXTURE           0x00000002  // select texture color (read only)
#define D3DTA_TFACTOR           0x00000003  // select RENDERSTATE_TEXTUREFACTOR (read only)
#define D3DTA_SPECULAR          0x00000004  // select specular color (read only)
#define D3DTA_TEMP              0x00000005  // select temporary register color (read/write)
#define D3DTA_COMPLEMENT        0x00000010  // take 1.0 - x (read modifier)
#define D3DTA_ALPHAREPLICATE    0x00000020  // replicate alpha to color components (read modifier)

#define D3DTA_SELECTMAX         0x00000006
#define D3DTA_MODIFIERMAX       0x00000040

/*
 * Flags for D3DTSS_COLORSIGN (Xbox extension)
 */
#define D3DTSIGN_ASIGNED        0x10000000
#define D3DTSIGN_AUNSIGNED      0
#define D3DTSIGN_RSIGNED        0x20000000
#define D3DTSIGN_RUNSIGNED      0
#define D3DTSIGN_GSIGNED        0x40000000
#define D3DTSIGN_GUNSIGNED      0
#define D3DTSIGN_BSIGNED        0x80000000
#define D3DTSIGN_BUNSIGNED      0

//
// Values for D3DTSS_***FILTER texture stage states
//
typedef enum _D3DTEXTUREFILTERTYPE
{
    D3DTEXF_NONE            = 0,    // filtering disabled (valid for mip filter only)
    D3DTEXF_POINT           = 1,    // nearest
    D3DTEXF_LINEAR          = 2,    // linear interpolation
    D3DTEXF_ANISOTROPIC     = 3,    // anisotropic
    D3DTEXF_QUINCUNX        = 4,    // quincunx kernel (Xbox extension)
    D3DTEXF_GAUSSIANCUBIC   = 5,    // gaussian kernel (Xbox redefinition)

    D3DTEXF_MAX             = 6,
    D3DTEXF_FORCE_DWORD     = 0x7fffffff,   // force 32-bit size enum
} D3DTEXTUREFILTERTYPE;

/*
 * State enumerants for per-stage texture processing.
 */
typedef enum _D3DTEXTURESTAGESTATETYPE
{
    // State whose handling is deferred until the next Draw[Indexed]Vertices
    // call because of interdependencies on other states:

    D3DTSS_ADDRESSU              =  0,  // D3DTEXTUREADDRESS for U coordinate
    D3DTSS_ADDRESSV              =  1,  // D3DTEXTUREADDRESS for V coordinate
    D3DTSS_ADDRESSW              =  2,  // D3DTEXTUREADDRESS for W coordinate
    D3DTSS_MAGFILTER             =  3,  // D3DTEXF_LINEAR etc. filter to use for magnification
    D3DTSS_MINFILTER             =  4,  // D3DTEXF_LINEAR etc. filter to use for minification
    D3DTSS_MIPFILTER             =  5,  // D3DTEXF_LINEAR etc. filter to use between mipmaps during minification
    D3DTSS_MIPMAPLODBIAS         =  6,  // float mipmap LOD bias
    D3DTSS_MAXMIPLEVEL           =  7,  // DWORD 0..(n-1) LOD index of largest map to use (0 == largest)
    D3DTSS_MAXANISOTROPY         =  8,  // DWORD maximum anisotropy
    D3DTSS_COLORKEYOP            =  9,  // D3DTEXTURECOLORKEYOP (Xbox extension)
    D3DTSS_COLORSIGN             = 10,  // D3DTSIGN_ASIGNED etc. for signed color channels (xbox extension)
    D3DTSS_ALPHAKILL             = 11,  // D3DTEXTUREALPHAKILL (Xbox extension)

    D3DTSS_DEFERRED_TEXTURE_STATE_MAX = 12,

    D3DTSS_COLOROP               = 12,  // D3DTEXTUREOP - per-stage blending controls for color channels
    D3DTSS_COLORARG0             = 13,  // D3DTA_TEXTURE etc. - third arg for triadic ops
    D3DTSS_COLORARG1             = 14,  // D3DTA_TEXTURE etc. - texture arg
    D3DTSS_COLORARG2             = 15,  // D3DTA_TEXTURE etc. - texture arg
    D3DTSS_ALPHAOP               = 16,  // D3DTEXTUREOP - per-stage blending controls for alpha channel
    D3DTSS_ALPHAARG0             = 17,  // D3DTA_TEXTURE etc. - third arg for triadic ops
    D3DTSS_ALPHAARG1             = 18,  // D3DTA_TEXTURE etc. - texture arg
    D3DTSS_ALPHAARG2             = 19,  // D3DTA_TEXTURE etc. - texture arg)
    D3DTSS_RESULTARG             = 20,  // D3DTA_CURRENT or D3DTA_TEMP - arg for result
    D3DTSS_TEXTURETRANSFORMFLAGS = 21,  // D3DTEXTURETRANSFORMFLAGS controls texture transform

    D3DTSS_DEFERRED_MAX          = 22,

    // State that has immediate processing:

    D3DTSS_BUMPENVMAT00          = 22,  // float (bump mapping matrix)
    D3DTSS_BUMPENVMAT01          = 23,  // float (bump mapping matrix)
    D3DTSS_BUMPENVMAT11          = 24,  // float (bump mapping matrix)
    D3DTSS_BUMPENVMAT10          = 25,  // float (bump mapping matrix)
    D3DTSS_BUMPENVLSCALE         = 26,  // float scale for bump map luminance
    D3DTSS_BUMPENVLOFFSET        = 27,  // float offset for bump map luminance
    D3DTSS_TEXCOORDINDEX         = 28,  // DWORD identifies which set of texture coordinates index this texture
    D3DTSS_BORDERCOLOR           = 29,  // D3DCOLOR
    D3DTSS_COLORKEYCOLOR         = 30,  // D3DCOLOR value for color key (Xbox extension)

    D3DTSS_MAX                   = 32,  // Total number of texture stage states (bumped to a power of 2)

    D3DTSS_FORCE_DWORD           = 0x7fffffff, // force 32-bit size enum
} D3DTEXTURESTAGESTATETYPE;

/*
 * The maximum number of vertices user can pass to any d3d
 * drawing function or to create vertex buffer with
 */
#define D3DMAXNUMVERTICES       ((1<<16) - 1)

/*
 * The maximum number of primitives user can pass to any d3d
 * drawing function.
 */
#define D3DMAXNUMPRIMITIVES     ((1<<16) - 1)

/*
 * The maximum index value possible for EndVisibilityTest
 */
#define D3DVISIBILITY_TEST_MAX  (16*1024)

//-------------------------------------------------------------------
// Flexible vertex format bits
//
#define D3DFVF_RESERVED0        0x001
#define D3DFVF_POSITION_MASK    0x00E
#define D3DFVF_XYZ              0x002
#define D3DFVF_XYZRHW           0x004
#define D3DFVF_XYZB1            0x006
#define D3DFVF_XYZB2            0x008
#define D3DFVF_XYZB3            0x00a
#define D3DFVF_XYZB4            0x00c

#define D3DFVF_NORMAL           0x010
#define D3DFVF_DIFFUSE          0x040
#define D3DFVF_SPECULAR         0x080

#define D3DFVF_TEXCOUNT_MASK    0xf00
#define D3DFVF_TEXCOUNT_SHIFT   8
#define D3DFVF_TEX0             0x000
#define D3DFVF_TEX1             0x100
#define D3DFVF_TEX2             0x200
#define D3DFVF_TEX3             0x300
#define D3DFVF_TEX4             0x400

#define D3DFVF_RESERVED2        0xE000  // 4 reserved bits

// D3DFVF_PSIZE is not supported on Xbox with the fixed function pipeline
// (a programmable vertex shader can be written to support per-vertex point
// sizes)

// Macros to set texture coordinate format bits in the FVF id

#define D3DFVF_TEXTUREFORMAT2 0         // Two floating point values
#define D3DFVF_TEXTUREFORMAT1 3         // One floating point value
#define D3DFVF_TEXTUREFORMAT3 1         // Three floating point values
#define D3DFVF_TEXTUREFORMAT4 2         // Four floating point values

#define D3DFVF_TEXCOORDSIZE3(CoordIndex) (D3DFVF_TEXTUREFORMAT3 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE2(CoordIndex) (D3DFVF_TEXTUREFORMAT2)
#define D3DFVF_TEXCOORDSIZE4(CoordIndex) (D3DFVF_TEXTUREFORMAT4 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE1(CoordIndex) (D3DFVF_TEXTUREFORMAT1 << (CoordIndex*2 + 16))

//---------------------------------------------------------------------
// Vertex Shaders
//

/*

Vertex Shader Declaration

The declaration portion of a vertex shader defines the static external
interface of the shader.  The information in the declaration includes:

- Assignments of vertex shader input registers to data streams.  These
assignments bind a specific vertex register to a single component within a
vertex stream.  A vertex stream element is identified by a byte offset
within the stream and a type.  The type specifies the arithmetic data type
plus the dimensionality (1, 2, 3, or 4 values).  Stream data which is
less than 4 values are always expanded out to 4 values with zero or more
0.F values and one 1.F value.

- Assignment of vertex shader input registers to implicit data from the
primitive tessellator.  This controls the loading of vertex data which is
not loaded from a stream, but rather is generated during primitive
tessellation prior to the vertex shader.

- Loading data into the constant memory at the time a shader is set as the
current shader.  Each token specifies values for one or more contiguous 4
DWORD constant registers.  This allows the shader to update an arbitrary
subset of the constant memory, overwriting the device state (which
contains the current values of the constant memory).  Note that these
values can be subsequently overwritten (between DrawPrimitive calls)
during the time a shader is bound to a device via the
SetVertexShaderConstant method.


Declaration arrays are single-dimensional arrays of DWORDs composed of
multiple tokens each of which is one or more DWORDs.  The single-DWORD
token value 0xFFFFFFFF is a special token used to indicate the end of the
declaration array.  The single DWORD token value 0x00000000 is a NOP token
with is ignored during the declaration parsing.  Note that 0x00000000 is a
valid value for DWORDs following the first DWORD for multiple word tokens.

[31:29] TokenType
    0x0 - NOP (requires all DWORD bits to be zero)
    0x1 - stream selector
    0x2 - stream data definition (map to vertex input memory)
    0x3 - vertex input memory from tessellator
    0x4 - constant memory from shader
    0x5 - extension
    0x6 - reserved
    0x7 - end-of-array (requires all DWORD bits to be 1)

NOP Token (single DWORD token)
    [31:29] 0x0
    [28:00] 0x0

Stream Selector (single DWORD token)
    [31:29] 0x1
    [28]    indicates whether this is a tessellator stream
    [27:04] 0x0
    [03:00] stream selector (0..15)

Stream Data Definition (single DWORD token)
    Vertex Input Register Load
      [31:29] 0x2
      [28]    0x0
      [27:24] 0x0
      [23:16] type (dimensionality and data type)
      [15:04] 0x0
      [03:00] vertex register address (0..15)
    DWORD Data Skip (no register load)
      [31:29] 0x2
      [28]    0x1
      [27]    0x0
      [26:20] 0x0
      [19:16] count of DWORDS to skip over (0..15)
      [15:00] 0x0
    BYTE Data Skip (no register load)
      [31:29] 0x2
      [28]    0x1
      [27]    0x1
      [26:20] 0x0
      [19:16] count of BYTES to skip over (0..15)
      [15:00] 0x0

Vertex Input Memory from Tessellator Data (single DWORD token)
    [31:29] 0x3
    [28]    indicates whether data is normals or u/v
    [27:24] 0x0
    [23:20] vertex register address (0..15)
    [19:04] 0x0
    [03:00] vertex register address (0..15)

Constant Memory from Shader (multiple DWORD token)
    [31:29] 0x4
    [28:25] count of 4*DWORD constants to load (0..15)
    [24:08] 0x0
    [07:00] constant memory address (0..191 - biased by 96)

Extension Token (single or multiple DWORD token)
    [31:29] 0x5
    [28:24] count of additional DWORDs in token (0..31)
    [23:00] extension-specific information

End-of-array token (single DWORD token)
    [31:29] 0x7
    [28:00] 0x1fffffff

The stream selector token must be immediately followed by a contiguous set
of stream data definition tokens.  This token sequence fully defines that
stream, including the set of elements within the stream, the order in
which the elements appear, the type of each element, and the vertex
register into which to load an element.

Streams are allowed to include data which is not loaded into a vertex
register, thus allowing data which is not used for this shader to exist
in the vertex stream.  This skipped data is defined only by a count of
DWORDs to skip over, since the type i nformation is irrelevant.

The token sequence:
Stream Select: stream=0
Stream Data Definition (Load): type=FLOAT3; register=3
Stream Data Definition (Load): type=FLOAT3; register=4
Stream Data Definition (Skip): count=2
Stream Data Definition (Load): type=FLOAT2; register=7

defines stream zero to consist of 4 elements, 3 of which are loaded into
registers and the fourth skipped over.  Register 3 is loaded with the
first three DWORDs in each vertex interpreted as FLOAT data.  Register 4
is loaded with the 4th, 5th, and 6th DWORDs interpreted as FLOAT data.
The next two DWORDs (7th and 8th) are skipped over and not loaded into
any vertex input register.   Register 7 is loaded with the 9th and 10th
DWORDS interpreted as FLOAT data.  Placing of tokens other than NOPs
between the Stream Selector and Stream Data Definition tokens is disallowed.

*/

// Vertex Shader 1.0 register limits. D3D device must provide at least
// specified number of registers
//

#define D3DVS_STREAMS_MAX_V1_0          16
#define D3DVS_INPUTREG_MAX_V1_0         16
#define D3DVS_TEMPREG_MAX_V1_0          12
// This max required number. Device could have more registers. Check caps.
#define D3DVS_CONSTREG_COUNT_XBOX       192
#define D3DVS_TCRDOUTREG_MAX_V1_0       8
#define D3DVS_ADDRREG_MAX_V1_0          1
#define D3DVS_ATTROUTREG_MAX_V1_0       2
#define D3DVS_MAXINSTRUCTIONCOUNT_V1_0  128

// The following two vertex shader constant addresses are reserved on Xbox
// when the SetShaderConstantMode does not have the D3DSCM_NORESERVEDCONSTANTS
// bit set (that is, these are reserved unless all of your vertex shaders use
// #pragma screenspace):
//
#define D3DVS_XBOX_RESERVEDCONSTANT1    -38
#define D3DVS_XBOX_RESERVEDCONSTANT2    -37

// Calling SetVertexShader with FVF_XYZRHW invalidates the first 11 or 12
// vertex shader program slots as loaded via LoadVertexShader. Max is 12...
//
#define D3DVS_XBOX_RESERVEDXYZRHWSLOTS  12

// Pixel Shader DX8 register limits. D3D device will have at most these
// specified number of registers
//
#define D3DPS_INPUTREG_MAX_DX8         8
#define D3DPS_TEMPREG_MAX_DX8          8
#define D3DPS_CONSTREG_MAX_DX8         16
#define D3DPS_TEXTUREREG_MAX_DX8       8


typedef enum _D3DVSD_TOKENTYPE
{
    D3DVSD_TOKEN_NOP        = 0,    // NOP or extension
    D3DVSD_TOKEN_STREAM,            // stream selector
    D3DVSD_TOKEN_STREAMDATA,        // stream data definition (map to vertex input memory)
    D3DVSD_TOKEN_TESSELLATOR,       // vertex input memory from tessellator
    D3DVSD_TOKEN_CONSTMEM,          // constant memory from shader
    D3DVSD_TOKEN_EXT,               // extension
    D3DVSD_TOKEN_END = 7,           // end-of-array (requires all DWORD bits to be 1)
    D3DVSD_FORCE_DWORD = 0x7fffffff,// force 32-bit size enum
} D3DVSD_TOKENTYPE;

#define D3DVSD_TOKENTYPESHIFT   29
#define D3DVSD_TOKENTYPEMASK    (7 << D3DVSD_TOKENTYPESHIFT)

#define D3DVSD_STREAMNUMBERSHIFT 0
#define D3DVSD_STREAMNUMBERMASK (0xF << D3DVSD_STREAMNUMBERSHIFT)

#define D3DVSD_DATALOADTYPESHIFT 28
#define D3DVSD_DATALOADTYPEMASK (0x1 << D3DVSD_DATALOADTYPESHIFT)

#define D3DVSD_DATATYPESHIFT 16
#define D3DVSD_DATATYPEMASK (0xFF << D3DVSD_DATATYPESHIFT)

#define D3DVSD_SKIPCOUNTSHIFT 16
#define D3DVSD_SKIPCOUNTMASK (0xF << D3DVSD_SKIPCOUNTSHIFT)

#define D3DVSD_VERTEXREGSHIFT 0
#define D3DVSD_VERTEXREGMASK (0x1F << D3DVSD_VERTEXREGSHIFT)

#define D3DVSD_VERTEXREGINSHIFT 20
#define D3DVSD_VERTEXREGINMASK (0xF << D3DVSD_VERTEXREGINSHIFT)

#define D3DVSD_CONSTCOUNTSHIFT 25
#define D3DVSD_CONSTCOUNTMASK (0xF << D3DVSD_CONSTCOUNTSHIFT)

#define D3DVSD_CONSTADDRESSSHIFT 0
#define D3DVSD_CONSTADDRESSMASK (0xFF << D3DVSD_CONSTADDRESSSHIFT)

#define D3DVSD_CONSTRSSHIFT 16
#define D3DVSD_CONSTRSMASK (0x1FFF << D3DVSD_CONSTRSSHIFT)

#define D3DVSD_EXTCOUNTSHIFT 24
#define D3DVSD_EXTCOUNTMASK (0x1F << D3DVSD_EXTCOUNTSHIFT)

#define D3DVSD_EXTINFOSHIFT 0
#define D3DVSD_EXTINFOMASK (0xFFFFFF << D3DVSD_EXTINFOSHIFT)

#define D3DVSD_MAKETOKENTYPE(tokenType) ((tokenType << D3DVSD_TOKENTYPESHIFT) & D3DVSD_TOKENTYPEMASK)

// macros for generation of CreateVertexShader Declaration token array

// Set current stream
// _StreamNumber [0..(MaxStreams-1)] stream to get data from
//
#define D3DVSD_STREAM( _StreamNumber ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAM) | (_StreamNumber))

// Set tessellator stream
//
#define D3DVSD_STREAMTESSSHIFT 28
#define D3DVSD_STREAMTESSMASK (1 << D3DVSD_STREAMTESSSHIFT)
#define D3DVSD_STREAM_TESS( ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAM) | (D3DVSD_STREAMTESSMASK))

// bind single vertex register to vertex element from vertex stream
//
// _VertexRegister [0..26] address of the vertex register
// _Type [D3DVSDT_*] dimensionality and arithmetic data type

#define D3DVSD_REG( _VertexRegister, _Type ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA) |            \
     ((_Type) << D3DVSD_DATATYPESHIFT) | (_VertexRegister))

// Skip _DWORDCount DWORDs in vertex
//
#define D3DVSD_SKIP( _DWORDCount ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA) | 0x10000000 | \
     ((_DWORDCount) << D3DVSD_SKIPCOUNTSHIFT))

// Skip _BYTECount BYTEs in vertex (Xbox extension)
//
#define D3DVSD_SKIPBYTES( _BYTECount ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA) | 0x18000000 | \
     ((_BYTECount) << D3DVSD_SKIPCOUNTSHIFT))

// load data into vertex shader constant memory
//
// _ConstantAddress [-96..95] - address of constant array to begin filling data
// _Count [0..15] - number of constant vectors to load (4 DWORDs each)
// followed by 4*_Count DWORDS of data
//
#define D3DVSD_CONST( _ConstantAddress, _Count ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_CONSTMEM) | \
     ((_Count) << D3DVSD_CONSTCOUNTSHIFT) | ((_ConstantAddress) + 96))

// enable tessellator generated normals
//
// _VertexRegisterIn  [0..15] address of vertex register whose input stream
//                            will be used in normal computation
// _VertexRegisterOut [0..15] address of vertex register to output the normal to
//
#define D3DVSD_TESSNORMAL( _VertexRegisterIn, _VertexRegisterOut ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_TESSELLATOR) | \
     ((_VertexRegisterIn) << D3DVSD_VERTEXREGINSHIFT) | \
     (_VertexRegisterOut))

// enable tessellator generated surface parameters
//
// _VertexRegister [0..15] address of vertex register to output parameters
//
#define D3DVSD_TESSUV( _VertexRegister ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_TESSELLATOR) | 0x10000000 | \
     (_VertexRegister))

// Generates END token
//
#define D3DVSD_END() 0xFFFFFFFF

// Generates NOP token
#define D3DVSD_NOP() 0x00000000

// bit declarations for _Type fields
#define D3DVSDT_FLOAT1      0x12    // 1D float expanded to (value, 0., 0., 1.)
#define D3DVSDT_FLOAT2      0x22    // 2D float expanded to (value, value, 0., 1.)
#define D3DVSDT_FLOAT3      0x32    // 3D float expanded to (value, value, value, 1.)
#define D3DVSDT_FLOAT4      0x42    // 4D float
#define D3DVSDT_D3DCOLOR    0x40    // 4D packed unsigned bytes mapped to 0. to 1. range
                                    // Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
#define D3DVSDT_SHORT2      0x25    // 2D signed short expanded to (value, value, 0., 1.)
#define D3DVSDT_SHORT4      0x45    // 4D signed short

// The following are Xbox extensions
#define D3DVSDT_NORMSHORT1  0x11    // 1D signed, normalized short expanded to (value, 0, 0., 1.)
                                    // (signed, normalized short maps from -1.0 to 1.0)
#define D3DVSDT_NORMSHORT2  0x21    // 2D signed, normalized short expanded to (value, value, 0., 1.)
#define D3DVSDT_NORMSHORT3  0x31    // 3D signed, normalized short expanded to (value, value, value, 1.)
#define D3DVSDT_NORMSHORT4  0x41    // 4D signed, normalized short expanded to (value, value, value, value)
#define D3DVSDT_NORMPACKED3 0x16    // 3 signed, normalized components packed in 32-bits.  (11,11,10).
                                    // Each component ranges from -1.0 to 1.0.
                                    // Expanded to (value, value, value, 1.)
#define D3DVSDT_SHORT1      0x15    // 1D signed short expanded to (value, 0., 0., 1.)
                                    // Signed shorts map to the range [-32768, 32767]
#define D3DVSDT_SHORT3      0x35    // 3D signed short expanded to (value, value, value, 1.)
#define D3DVSDT_PBYTE1      0x14    // 1D packed byte expanded to (value, 0., 0., 1.)
                                    // Packed bytes map to the range [0, 1]
#define D3DVSDT_PBYTE2      0x24    // 2D packed byte expanded to (value, value, 0., 1.)
#define D3DVSDT_PBYTE3      0x34    // 3D packed byte expanded to (value, value, value, 1.)
#define D3DVSDT_PBYTE4      0x44    // 4D packed byte expanded to (value, value, value, value)
#define D3DVSDT_FLOAT2H     0x72    // 2D homogeneous float expanded to (value, value,0., value.)
                                    // Useful for projective texture coordinates.
#define D3DVSDT_NONE        0x02    // No stream data

// D3DVSDT_UBYTE4 not supported on Xbox (hardware can't do it)

// assignments of vertex input registers for fixed function vertex shader
//
#define D3DVSDE_VERTEX          -1  // Xbox extension, used only in Begin/End bracket
#define D3DVSDE_POSITION        0
#define D3DVSDE_BLENDWEIGHT     1
#define D3DVSDE_NORMAL          2
#define D3DVSDE_DIFFUSE         3
#define D3DVSDE_SPECULAR        4
#define D3DVSDE_FOG             5   // Xbox extension
#define D3DVSDE_PSIZE           6   // Xbox extension
#define D3DVSDE_BACKDIFFUSE     7   // Xbox extension
#define D3DVSDE_BACKSPECULAR    8   // Xbox extension
#define D3DVSDE_TEXCOORD0       9
#define D3DVSDE_TEXCOORD1       10
#define D3DVSDE_TEXCOORD2       11
#define D3DVSDE_TEXCOORD3       12

// D3DVSDE_PSIZE is not supported on Xbox with the fixed function pipeline
// (a programmable vertex shader can be written to support per-vertex point
// sizes)

// Maximum supported number of texture coordinate sets
#define D3DDP_MAXTEXCOORD   4

// Some Vertex Shader APIs take allow the vertex format to be specified with
// a structure rather than a handle to a vertex shader object.  The following
// structure defines how vertex attributes are read from one or more streams.

typedef struct _D3DVERTEXSHADERINPUT
{
    DWORD StreamIndex;      // which stream
    DWORD Offset;           // Byte offset from the start of the stream.
    DWORD Format;           // format of this attribute (D3DVSDT_*)
    BYTE  TessType;         // 0=none, 1=normal, 2=autotex
    BYTE  TessSource;       // source register for normal/uv autocalc
} D3DVERTEXSHADERINPUT;

// Up to 16 different inputs are supported by the vertex shader hardware.
// Each of the 16 entries in the following structure defines which
// stream that value comes from, what byte offset corresponds to an
// attribute, and the format of the attribute.

typedef struct _D3DVERTEXATTRIBUTEFORMAT
{
    D3DVERTEXSHADERINPUT Input[16];
} D3DVERTEXATTRIBUTEFORMAT;


//
// Instruction Token Bit Definitions
//
#define D3DSI_OPCODE_MASK       0x0000FFFF

typedef enum _D3DSHADER_INSTRUCTION_OPCODE_TYPE
{
    D3DSIO_NOP          = 0,    // PS/VS
    D3DSIO_MOV          ,       // PS/VS
    D3DSIO_ADD          ,       // PS/VS
    D3DSIO_SUB          ,       // PS
    D3DSIO_MAD          ,       // PS/VS
    D3DSIO_MUL          ,       // PS/VS
    D3DSIO_RCP          ,       // VS
    D3DSIO_RSQ          ,       // VS
    D3DSIO_DP3          ,       // PS/VS
    D3DSIO_DP4          ,       // VS
    D3DSIO_MIN          ,       // VS
    D3DSIO_MAX          ,       // VS
    D3DSIO_SLT          ,       // VS
    D3DSIO_SGE          ,       // VS
    D3DSIO_EXP          ,       // VS
    D3DSIO_LOG          ,       // VS
    D3DSIO_LIT          ,       // VS
    D3DSIO_DST          ,       // VS
    D3DSIO_LRP          ,       // PS
    D3DSIO_FRC          ,       // VS
    D3DSIO_M4x4         ,       // VS
    D3DSIO_M4x3         ,       // VS
    D3DSIO_M3x4         ,       // VS
    D3DSIO_M3x3         ,       // VS
    D3DSIO_M3x2         ,       // VS

    D3DSIO_TEXCOORD     = 64,   // PS
    D3DSIO_TEXKILL      ,       // PS
    D3DSIO_TEX          ,       // PS
    D3DSIO_TEXBEM       ,       // PS
    D3DSIO_TEXBEML      ,       // PS
    D3DSIO_TEXREG2AR    ,       // PS
    D3DSIO_TEXREG2GB    ,       // PS
    D3DSIO_TEXM3x2PAD   ,       // PS
    D3DSIO_TEXM3x2TEX   ,       // PS
    D3DSIO_TEXM3x3PAD   ,       // PS
    D3DSIO_TEXM3x3TEX   ,       // PS
    D3DSIO_TEXM3x3DIFF  ,       // PS
    D3DSIO_TEXM3x3SPEC  ,       // PS
    D3DSIO_TEXM3x3VSPEC ,       // PS
    D3DSIO_EXPP         ,       // VS
    D3DSIO_LOGP         ,       // VS
    D3DSIO_CND          ,       // PS
    D3DSIO_DEF          ,       // PS

    // Xbox Extensions
    D3DSIO_DPH          =256,   // VS
    D3DSIO_RCC          ,       // VS
    D3DSIO_XMMA         ,       // PS
    D3DSIO_XMMC         ,       // PS
    D3DSIO_XDM          ,       // PS
    D3DSIO_XDD          ,       // PS
    D3DSIO_XFC          ,       // PS
    D3DSIO_TEXM3x2DEPTH ,       // PS
    D3DSIO_TEXBRDF      ,       // PS

    D3DSIO_COMMENT      = 0xFFFE,
    D3DSIO_END          = 0xFFFF,

    D3DSIO_FORCE_DWORD  = 0x7fffffff,   // force 32-bit size enum
} D3DSHADER_INSTRUCTION_OPCODE_TYPE;

//
// Co-Issue Instruction Modifier - if set then this instruction is to be
// issued in parallel with the previous instruction(s) for which this bit
// is not set.
//
#define D3DSI_COISSUE           0x40000000

//
// Parameter Token Bit Definitions
//
#define D3DSP_REGNUM_MASK       0x00000FFF

// destination parameter write mask
#define D3DSP_WRITEMASK_0       0x00010000  // Component 0 (X;Red)
#define D3DSP_WRITEMASK_1       0x00020000  // Component 1 (Y;Green)
#define D3DSP_WRITEMASK_2       0x00040000  // Component 2 (Z;Blue)
#define D3DSP_WRITEMASK_3       0x00080000  // Component 3 (W;Alpha)
#define D3DSP_WRITEMASK_ALL     0x000F0000  // All Components

// destination parameter modifiers
#define D3DSP_DSTMOD_SHIFT      20
#define D3DSP_DSTMOD_MASK       0x00F00000

#define D3DSP_DSTSRC_SHIFT      15          // indicates src or dst
#define D3DSP_DSTSRC_MASK       0x00008000

typedef enum _D3DSHADER_PARAM_DSTMOD_TYPE
{
    D3DSPDM_NONE    = 0<<D3DSP_DSTMOD_SHIFT, // nop
    D3DSPDM_BIAS    = 1<<D3DSP_DSTMOD_SHIFT, // subtract 0.5
    D3DSPDM_FORCE_DWORD  = 0x7fffffff,      // force 32-bit size enum
} D3DSHADER_PARAM_DSTMOD_TYPE;

// destination parameter
#define D3DSP_DSTSHIFT_SHIFT    24
#define D3DSP_DSTSHIFT_MASK     0x0F000000

// destination/source parameter register type
#define D3DSP_REGTYPE_SHIFT     28
#define D3DSP_REGTYPE_MASK      0x70000000

typedef enum _D3DSHADER_PARAM_REGISTER_TYPE
{
    D3DSPR_TEMP     = 0<<D3DSP_REGTYPE_SHIFT, // Temporary Register File
    D3DSPR_INPUT    = 1<<D3DSP_REGTYPE_SHIFT, // Input Register File
    D3DSPR_CONST    = 2<<D3DSP_REGTYPE_SHIFT, // Constant Register File
    D3DSPR_ADDR     = 3<<D3DSP_REGTYPE_SHIFT, // Address Register (VS)
    D3DSPR_TEXTURE  = 3<<D3DSP_REGTYPE_SHIFT, // Texture Register File (PS)
    D3DSPR_RASTOUT  = 4<<D3DSP_REGTYPE_SHIFT, // Rasterizer Register File
    D3DSPR_ATTROUT  = 5<<D3DSP_REGTYPE_SHIFT, // Attribute Output Register File
    D3DSPR_TEXCRDOUT= 6<<D3DSP_REGTYPE_SHIFT, // Texture Coordinate Output Register File
    D3DSPR_FORCE_DWORD  = 0x7fffffff,         // force 32-bit size enum
} D3DSHADER_PARAM_REGISTER_TYPE;

// Register offsets in the Rasterizer Register File
//
typedef enum _D3DVS_RASTOUT_OFFSETS
{
    D3DSRO_POSITION = 0,
    D3DSRO_FOG,
    D3DSRO_POINT_SIZE,
    D3DSRO_FORCE_DWORD  = 0x7fffffff,         // force 32-bit size enum
} D3DVS_RASTOUT_OFFSETS;

// Source operand addressing modes

#define D3DVS_ADDRESSMODE_SHIFT 13
#define D3DVS_ADDRESSMODE_MASK  (3 << D3DVS_ADDRESSMODE_SHIFT)

enum D3DVS_ADRRESSMODE
{
    D3DVS_ADDRMODE_ABSOLUTE  = (0 << D3DVS_ADDRESSMODE_SHIFT),
    D3DVS_ADDRMODE_RELATIVE  = (1 << D3DVS_ADDRESSMODE_SHIFT),   // Relative to register A0
    D3DVS_ADDRMODE_FORCE_DWORD = 0x7fffffff, // force 32-bit size enum
};

// Source operand swizzle definitions
//
#define D3DVS_SWIZZLE_SHIFT     16
#define D3DVS_SWIZZLE_MASK      0x00FF0000

// The following bits define where to take component X:

#define D3DVS_X_X       (0 << D3DVS_SWIZZLE_SHIFT)
#define D3DVS_X_Y       (1 << D3DVS_SWIZZLE_SHIFT)
#define D3DVS_X_Z       (2 << D3DVS_SWIZZLE_SHIFT)
#define D3DVS_X_W       (3 << D3DVS_SWIZZLE_SHIFT)

// The following bits define where to take component Y:

#define D3DVS_Y_X       (0 << (D3DVS_SWIZZLE_SHIFT + 2))
#define D3DVS_Y_Y       (1 << (D3DVS_SWIZZLE_SHIFT + 2))
#define D3DVS_Y_Z       (2 << (D3DVS_SWIZZLE_SHIFT + 2))
#define D3DVS_Y_W       (3 << (D3DVS_SWIZZLE_SHIFT + 2))

// The following bits define where to take component Z:

#define D3DVS_Z_X       (0 << (D3DVS_SWIZZLE_SHIFT + 4))
#define D3DVS_Z_Y       (1 << (D3DVS_SWIZZLE_SHIFT + 4))
#define D3DVS_Z_Z       (2 << (D3DVS_SWIZZLE_SHIFT + 4))
#define D3DVS_Z_W       (3 << (D3DVS_SWIZZLE_SHIFT + 4))

// The following bits define where to take component W:

#define D3DVS_W_X       (0 << (D3DVS_SWIZZLE_SHIFT + 6))
#define D3DVS_W_Y       (1 << (D3DVS_SWIZZLE_SHIFT + 6))
#define D3DVS_W_Z       (2 << (D3DVS_SWIZZLE_SHIFT + 6))
#define D3DVS_W_W       (3 << (D3DVS_SWIZZLE_SHIFT + 6))

// Value when there is no swizzle (X is taken from X, Y is taken from Y,
// Z is taken from Z, W is taken from W
//
#define D3DVS_NOSWIZZLE (D3DVS_X_X | D3DVS_Y_Y | D3DVS_Z_Z | D3DVS_W_W)

// source parameter swizzle
#define D3DSP_SWIZZLE_SHIFT     16
#define D3DSP_SWIZZLE_MASK      0x00FF0000

#define D3DSP_NOSWIZZLE \
    ( (0 << (D3DSP_SWIZZLE_SHIFT + 0)) | \
      (1 << (D3DSP_SWIZZLE_SHIFT + 2)) | \
      (2 << (D3DSP_SWIZZLE_SHIFT + 4)) | \
      (3 << (D3DSP_SWIZZLE_SHIFT + 6)) )

// pixel-shader swizzle ops
#define D3DSP_REPLICATEALPHA \
    ( (3 << (D3DSP_SWIZZLE_SHIFT + 0)) | \
      (3 << (D3DSP_SWIZZLE_SHIFT + 2)) | \
      (3 << (D3DSP_SWIZZLE_SHIFT + 4)) | \
      (3 << (D3DSP_SWIZZLE_SHIFT + 6)) )

// source parameter modifiers
#define D3DSP_SRCMOD_SHIFT      24
#define D3DSP_SRCMOD_MASK       0x0F000000

typedef enum _D3DSHADER_PARAM_SRCMOD_TYPE
{
    D3DSPSM_NONE    = 0<<D3DSP_SRCMOD_SHIFT, // nop
    D3DSPSM_NEG     = 1<<D3DSP_SRCMOD_SHIFT, // negate
    D3DSPSM_BIAS    = 2<<D3DSP_SRCMOD_SHIFT, // bias
    D3DSPSM_BIASNEG = 3<<D3DSP_SRCMOD_SHIFT, // bias and negate
    D3DSPSM_SIGN    = 4<<D3DSP_SRCMOD_SHIFT, // sign
    D3DSPSM_SIGNNEG = 5<<D3DSP_SRCMOD_SHIFT, // sign and negate
    D3DSPSM_COMP    = 6<<D3DSP_SRCMOD_SHIFT, // complement
    D3DSPSM_SAT     = 7<<D3DSP_SRCMOD_SHIFT, // saturate

    D3DSPSM_FORCE_DWORD = 0x7fffffff,        // force 32-bit size enum
} D3DSHADER_PARAM_SRCMOD_TYPE;

// pixel shader version token
#define D3DPS_VERSION(_Major,_Minor) (0xFFFF0000|((_Major)<<8)|(_Minor))

// vertex shader version token
#define D3DVS_VERSION(_Major,_Minor) (0xFFFE0000|((_Major)<<8)|(_Minor))

// extract major/minor from version cap
#define D3DSHADER_VERSION_MAJOR(_Version) (((_Version)>>8)&0xFF)
#define D3DSHADER_VERSION_MINOR(_Version) (((_Version)>>0)&0xFF)

// destination/source parameter register type
#define D3DSI_COMMENTSIZE_SHIFT     16
#define D3DSI_COMMENTSIZE_MASK      0x7FFF0000
#define D3DSHADER_COMMENT(_DWordSize) \
    ((((_DWordSize)<<D3DSI_COMMENTSIZE_SHIFT)&D3DSI_COMMENTSIZE_MASK)|D3DSIO_COMMENT)

// pixel/vertex shader end token
#define D3DPS_END()  0x0000FFFF
#define D3DVS_END()  0x0000FFFF

//-------------------------------------------------------------------------
// D3D Vertex Shader Microcode Type values:
// --------------
//
// D3DSMT_VERTEXSHADER
//   An ordinary vertex shader.
//
// D3DSMT_READWRITE_VERTEXSHADER
//   A vertex shader that can write to the constant registers.
//
// D3DSMT_VERTEXSTATESHADER
//   A vertex state shader.
//
//-------------------------------------------------------------------------

#define D3DSMT_VERTEXSHADER              1
#define D3DSMT_READWRITE_VERTEXSHADER    2
#define D3DSMT_VERTEXSTATESHADER         3

//---------------------------------------------------------------------

// High order surfaces
//
typedef enum _D3DBASISTYPE
{
   D3DBASIS_BEZIER      = 0,
   D3DBASIS_BSPLINE     = 1,
   D3DBASIS_INTERPOLATE = 2,
   D3DBASIS_FORCE_DWORD = 0x7fffffff,
} D3DBASISTYPE;

typedef enum _D3DORDERTYPE
{
   D3DORDER_LINEAR      = 1,
   D3DORDER_CUBIC       = 3,
   D3DORDER_QUINTIC     = 5,
   D3DORDER_FORCE_DWORD = 0x7fffffff,
} D3DORDERTYPE;

typedef enum _D3DPATCHEDGESTYLE
{
   D3DPATCHEDGE_DISCRETE    = 0,
   D3DPATCHEDGE_CONTINUOUS  = 1,
   D3DPATCHEDGE_FORCE_DWORD = 0x7fffffff,
} D3DPATCHEDGESTYLE;

typedef enum _D3DSTATEBLOCKTYPE
{
    D3DSBT_ALL           = 1, // capture all state
    D3DSBT_PIXELSTATE    = 2, // capture pixel state
    D3DSBT_VERTEXSTATE   = 3, // capture vertex state
    D3DSBT_FORCE_DWORD   = 0x7fffffff,
} D3DSTATEBLOCKTYPE;


//---------------------------------------------------------------------

/* Direct3D8 Device types */
typedef enum _D3DDEVTYPE
{
    D3DDEVTYPE_HAL         = 1,
    D3DDEVTYPE_REF         = 2,
    D3DDEVTYPE_SW          = 3,

    D3DDEVTYPE_FORCE_DWORD  = 0x7fffffff
} D3DDEVTYPE;

/* AntiAliasing buffer types */

typedef DWORD D3DMULTISAMPLE_TYPE;

#define D3DMULTISAMPLE_NONE                                      0x0011

// Number of samples, sample type, and filter (Xbox extensions):
//
#define D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_LINEAR              0x1021
#define D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX            0x1121
#define D3DMULTISAMPLE_2_SAMPLES_SUPERSAMPLE_HORIZONTAL_LINEAR   0x2021
#define D3DMULTISAMPLE_2_SAMPLES_SUPERSAMPLE_VERTICAL_LINEAR     0x2012

#define D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_LINEAR              0x1022
#define D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_GAUSSIAN            0x1222
#define D3DMULTISAMPLE_4_SAMPLES_SUPERSAMPLE_LINEAR              0x2022
#define D3DMULTISAMPLE_4_SAMPLES_SUPERSAMPLE_GAUSSIAN            0x2222

#define D3DMULTISAMPLE_9_SAMPLES_MULTISAMPLE_GAUSSIAN            0x1233
#define D3DMULTISAMPLE_9_SAMPLES_SUPERSAMPLE_GAUSSIAN            0x2233

// Format of the pre-filter (big) color buffer (Xbox extensions):
//
#define D3DMULTISAMPLE_PREFILTER_FORMAT_DEFAULT                  0x00000
#define D3DMULTISAMPLE_PREFILTER_FORMAT_X1R5G5B5                 0x10000
#define D3DMULTISAMPLE_PREFILTER_FORMAT_R5G6B5                   0x20000
#define D3DMULTISAMPLE_PREFILTER_FORMAT_X8R8G8B8                 0x30000
#define D3DMULTISAMPLE_PREFILTER_FORMAT_A8R8G8B8                 0x40000

// Defaults:
//
#define D3DMULTISAMPLE_2_SAMPLES D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX
#define D3DMULTISAMPLE_4_SAMPLES D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_GAUSSIAN
#define D3DMULTISAMPLE_9_SAMPLES D3DMULTISAMPLE_9_SAMPLES_SUPERSAMPLE_GAUSSIAN

/* Types for D3DRS_MULTISAMPLEMODE and D3DRS_MULTISAMPLERENDERTARGETMODE */
typedef enum _D3DMULTISAMPLEMODE
{
    D3DMULTISAMPLEMODE_1X                                   = 0,
    D3DMULTISAMPLEMODE_2X                                   = 1,
    D3DMULTISAMPLEMODE_4X                                   = 2,

    D3DMULTISAMPLEMODE_FORCE_DWORD                          = 0x7fffffff
} D3DMULTISAMPLEMODE;


/* Formats
 * Most of these names have the following convention:
 *      A = Alpha
 *      R = Red
 *      G = Green
 *      B = Blue
 *      X = Unused Bits
 *      P = Palette
 *      L = Luminance
 *      U = dU coordinate for BumpMap
 *      V = dV coordinate for BumpMap
 *      S = Stencil
 *      D = Depth (e.g. Z or W buffer)
 *
 *      Further, the order of the pieces are from MSB first; hence
 *      D3DFMT_A8L8 indicates that the high byte of this two byte
 *      format is alpha.
 *
 *      D16 indicates:
 *           - An integer 16-bit value.
 *           - An app-lockable surface.
 *
 *      All Depth/Stencil formats except D3DFMT_D16_LOCKABLE indicate:
 *          - no particular bit ordering per pixel, and
 *          - are not app lockable, and
 *          - the driver is allowed to consume more than the indicated
 *            number of bits per Depth channel (but not Stencil channel).
 */

typedef enum _D3DFORMAT
{
    D3DFMT_UNKNOWN              = 0xFFFFFFFF,

    /* Swizzled formats */

    D3DFMT_A8R8G8B8             = 0x00000006,
    D3DFMT_X8R8G8B8             = 0x00000007,
    D3DFMT_R5G6B5               = 0x00000005,
    D3DFMT_R6G5B5               = 0x00000027,
    D3DFMT_X1R5G5B5             = 0x00000003,
    D3DFMT_A1R5G5B5             = 0x00000002,
    D3DFMT_A4R4G4B4             = 0x00000004,
    D3DFMT_A8                   = 0x00000019,
    D3DFMT_A8B8G8R8             = 0x0000003A,
    D3DFMT_B8G8R8A8             = 0x0000003B,
    D3DFMT_R4G4B4A4             = 0x00000039,
    D3DFMT_R5G5B5A1             = 0x00000038,
    D3DFMT_R8G8B8A8             = 0x0000003C,
    D3DFMT_R8B8                 = 0x00000029,
    D3DFMT_G8B8                 = 0x00000028,

    D3DFMT_P8                   = 0x0000000B,

    D3DFMT_L8                   = 0x00000000,
    D3DFMT_A8L8                 = 0x0000001A,
    D3DFMT_AL8                  = 0x00000001,
    D3DFMT_L16                  = 0x00000032,

    D3DFMT_V8U8                 = 0x00000028,
    D3DFMT_L6V5U5               = 0x00000027,
    D3DFMT_X8L8V8U8             = 0x00000007,
    D3DFMT_Q8W8V8U8             = 0x0000003A,
    D3DFMT_V16U16               = 0x00000033,

    D3DFMT_D16_LOCKABLE         = 0x0000002C,
    D3DFMT_D16                  = 0x0000002C,
    D3DFMT_D24S8                = 0x0000002A,
    D3DFMT_F16                  = 0x0000002D,
    D3DFMT_F24S8                = 0x0000002B,

    /* YUV formats */

    D3DFMT_YUY2                 = 0x00000024,
    D3DFMT_UYVY                 = 0x00000025,

    /* Compressed formats */

    D3DFMT_DXT1                 = 0x0000000C,
    D3DFMT_DXT2                 = 0x0000000E,
    D3DFMT_DXT3                 = 0x0000000E,
    D3DFMT_DXT4                 = 0x0000000F,
    D3DFMT_DXT5                 = 0x0000000F,

    /* Linear formats */

    D3DFMT_LIN_A1R5G5B5         = 0x00000010,
    D3DFMT_LIN_A4R4G4B4         = 0x0000001D,
    D3DFMT_LIN_A8               = 0x0000001F,
    D3DFMT_LIN_A8B8G8R8         = 0x0000003F,
    D3DFMT_LIN_A8R8G8B8         = 0x00000012,
    D3DFMT_LIN_B8G8R8A8         = 0x00000040,
    D3DFMT_LIN_G8B8             = 0x00000017,
    D3DFMT_LIN_R4G4B4A4         = 0x0000003E,
    D3DFMT_LIN_R5G5B5A1         = 0x0000003D,
    D3DFMT_LIN_R5G6B5           = 0x00000011,
    D3DFMT_LIN_R6G5B5           = 0x00000037,
    D3DFMT_LIN_R8B8             = 0x00000016,
    D3DFMT_LIN_R8G8B8A8         = 0x00000041,
    D3DFMT_LIN_X1R5G5B5         = 0x0000001C,
    D3DFMT_LIN_X8R8G8B8         = 0x0000001E,

    D3DFMT_LIN_A8L8             = 0x00000020,
    D3DFMT_LIN_AL8              = 0x0000001B,
    D3DFMT_LIN_L16              = 0x00000035,
    D3DFMT_LIN_L8               = 0x00000013,

    D3DFMT_LIN_V16U16           = 0x00000036,
    D3DFMT_LIN_V8U8             = 0x00000017,
    D3DFMT_LIN_L6V5U5           = 0x00000037,
    D3DFMT_LIN_X8L8V8U8         = 0x0000001E,
    D3DFMT_LIN_Q8W8V8U8         = 0x00000012,

    D3DFMT_LIN_D24S8            = 0x0000002E,
    D3DFMT_LIN_F24S8            = 0x0000002F,
    D3DFMT_LIN_D16              = 0x00000030,
    D3DFMT_LIN_F16              = 0x00000031,

    D3DFMT_VERTEXDATA           = 100,
    D3DFMT_INDEX16              = 101,

    D3DFMT_FORCE_DWORD          =0x7fffffff
} D3DFORMAT;

/* Display mode flags */

#define D3DPRESENTFLAG_LOCKABLE_BACKBUFFER      0x00000001
#define D3DPRESENTFLAG_WIDESCREEN               0x00000010
#define D3DPRESENTFLAG_INTERLACED               0x00000020
#define D3DPRESENTFLAG_PROGRESSIVE              0x00000040
#define D3DPRESENTFLAG_FIELD                    0x00000080
#define D3DPRESENTFLAG_10X11PIXELASPECTRATIO    0x00000100
#define D3DPRESENTFLAG_EMULATE_REFRESH_RATE     0x00000200              

/* Display Modes */
typedef struct _D3DDISPLAYMODE
{
    UINT            Width;
    UINT            Height;
    UINT            RefreshRate;
    DWORD           Flags;
    D3DFORMAT       Format;
} D3DDISPLAYMODE;

/* Creation Parameters */
typedef struct _D3DDEVICE_CREATION_PARAMETERS
{
    UINT            AdapterOrdinal;
    D3DDEVTYPE      DeviceType;
    HWND            hFocusWindow;
    DWORD           BehaviorFlags;
} D3DDEVICE_CREATION_PARAMETERS;


/* SwapEffects */
typedef enum _D3DSWAPEFFECT
{
    D3DSWAPEFFECT_DISCARD           = 1,
    D3DSWAPEFFECT_FLIP              = 2,
    D3DSWAPEFFECT_COPY              = 3,
    D3DSWAPEFFECT_COPY_VSYNC        = 4,

    D3DSWAPEFFECT_FORCE_DWORD       = 0x7fffffff
} D3DSWAPEFFECT;

/* Swap flags (Xbox extension) */
#define D3DSWAP_COPY                0x00000001L
#define D3DSWAP_BYPASSCOPY          0x00000002L
#define D3DSWAP_FINISH              0x00000004L

#define D3DSWAP_DEFAULT             0x00000000L

/* Insert Callback (Xbox extension) */
typedef enum _D3DCALLBACKTYPE
{
    D3DCALLBACK_READ                = 0,
    D3DCALLBACK_WRITE               = 1,

    D3DCALLBACKTYPE_FORCE_DWORD     = 0x7fffffff
} D3DCALLBACKTYPE;

/* VBlank data (Xbox extension) */
#define D3DVBLANK_SWAPDONE          0x00000001L
#define D3DVBLANK_SWAPMISSED        0x00000002L

typedef struct _D3DVBLANKDATA
{
    DWORD           VBlank;
    DWORD           Swap;
    DWORD           Flags;
} D3DVBLANKDATA;

/* Swap data (Xbox extension) */
typedef struct _D3DSWAPDATA
{
    DWORD           Swap;
    DWORD           SwapVBlank;
    DWORD           MissedVBlanks;
    DWORD           TimeUntilSwapVBlank;
    DWORD           TimeBetweenSwapVBlanks;
} D3DSWAPDATA;

/* SetDepthClipPlanes */
typedef enum _D3DSET_DEPTH_CLIP_PLANES_FLAGS
{
    D3DSDCP_SET_VERTEXPROGRAM_PLANES         = 1,
    D3DSDCP_SET_FIXEDFUNCTION_PLANES         = 2,
    D3DSDCP_USE_DEFAULT_VERTEXPROGRAM_PLANES = 3,
    D3DSDCP_USE_DEFAULT_FIXEDFUNCTION_PLANES = 4,
} D3DSET_DEPTH_CLIP_PLANES_FLAGS;

/* GetDepthClipPlanes */
typedef enum _D3DGET_DEPTH_CLIP_PLANES_FLAGS
{
    D3DGDCP_GET_VERTEXPROGRAM_PLANES         = 10,
    D3DGDCP_GET_FIXEDFUNCTION_PLANES         = 11,
} D3DGET_DEPTH_CLIP_PLANES_FLAGS;


/* Pool types */

typedef DWORD D3DPOOL;

#define D3DPOOL_DEFAULT             0
#define D3DPOOL_MANAGED             1
#define D3DPOOL_SYSTEMMEM           2

/* RefreshRate pre-defines */
#define D3DPRESENT_RATE_DEFAULT         0x00000000
#define D3DPRESENT_RATE_UNLIMITED       0x00000000


/* Reset and CreateDevice Parameters */
typedef struct _D3DPRESENT_PARAMETERS_
{
    UINT                BackBufferWidth;
    UINT                BackBufferHeight;
    D3DFORMAT           BackBufferFormat;
    UINT                BackBufferCount;
    D3DMULTISAMPLE_TYPE MultiSampleType;
    D3DSWAPEFFECT       SwapEffect;
    HWND                hDeviceWindow;
    BOOL                Windowed;
    BOOL                EnableAutoDepthStencil;
    D3DFORMAT           AutoDepthStencilFormat;
    DWORD               Flags;
    UINT                FullScreen_RefreshRateInHz;
    UINT                FullScreen_PresentationInterval;
    D3DSurface         *BufferSurfaces[3];
    D3DSurface         *DepthStencilSurface;

} D3DPRESENT_PARAMETERS;

/* Gamma Ramp: Xbox changes WORD values to BYTE */
typedef struct _D3DGAMMARAMP
{
    BYTE                red  [256];
    BYTE                green[256];
    BYTE                blue [256];
} D3DGAMMARAMP;

/* Back buffer types */
typedef DWORD D3DBACKBUFFER_TYPE;

#define D3DBACKBUFFER_TYPE_MONO   0

/* Resource types */
typedef enum _D3DRESOURCETYPE
{
    D3DRTYPE_NONE                   =  0,
    D3DRTYPE_SURFACE                =  1,
    D3DRTYPE_VOLUME                 =  2,
    D3DRTYPE_TEXTURE                =  3,
    D3DRTYPE_VOLUMETEXTURE          =  4,
    D3DRTYPE_CUBETEXTURE            =  5,
    D3DRTYPE_VERTEXBUFFER           =  6,
    D3DRTYPE_INDEXBUFFER            =  7,
    D3DRTYPE_PUSHBUFFER             =  8,
    D3DRTYPE_PALETTE                =  9,
    D3DRTYPE_FIXUP                  =  10,

    D3DRTYPE_FORCE_DWORD            = 0x7fffffff
} D3DRESOURCETYPE;

/* Locations */

// Enumeration used for memory movement (deprecated Xbox extension)
//
typedef enum _D3DMEMORY
{
    D3DMEM_AGP                      = 0,
    D3DMEM_VIDEO                    = 1
} D3DMEMORY;

/* Usages */
#define D3DUSAGE_RENDERTARGET           (0x00000001L)
#define D3DUSAGE_DEPTHSTENCIL           (0x00000002L)

/* Usages for Vertex/Index buffers */
#define D3DUSAGE_WRITEONLY              (0x00000008L)
#define D3DUSAGE_POINTS                 (0x00000040L)
#define D3DUSAGE_RTPATCHES              (0x00000080L)
#define D3DUSAGE_DYNAMIC                (0x00000200L)

/* Usages for CreateVertexShader */
#define D3DUSAGE_PERSISTENTDIFFUSE      (0x00000400L)   // Xbox extension
#define D3DUSAGE_PERSISTENTSPECULAR     (0x00000800L)   // Xbox extension
#define D3DUSAGE_PERSISTENTBACKDIFFUSE  (0x00001000L)   // Xbox extension
#define D3DUSAGE_PERSISTENTBACKSPECULAR (0x00002000L)   // Xbox extension

/* Usages for CreateTexture/CreateImageSurface */
#define D3DUSAGE_BORDERSOURCE_COLOR     (0x00000000L)   // Xbox extension
#define D3DUSAGE_BORDERSOURCE_TEXTURE   (0x00010000L)   // Xbox extension


/* CubeMap Face identifiers */
typedef enum _D3DCUBEMAP_FACES
{
    D3DCUBEMAP_FACE_POSITIVE_X     = 0,
    D3DCUBEMAP_FACE_NEGATIVE_X     = 1,
    D3DCUBEMAP_FACE_POSITIVE_Y     = 2,
    D3DCUBEMAP_FACE_NEGATIVE_Y     = 3,
    D3DCUBEMAP_FACE_POSITIVE_Z     = 4,
    D3DCUBEMAP_FACE_NEGATIVE_Z     = 5,

    D3DCUBEMAP_FACE_FORCE_DWORD    = 0x7fffffff
} D3DCUBEMAP_FACES;

/* Lock flags */
#define D3DLOCK_NOFLUSH             0x00000010L // Xbox extension
#define D3DLOCK_NOOVERWRITE         0x00000020L
#define D3DLOCK_TILED               0x00000040L // Xbox extension
#define D3DLOCK_READONLY            0x00000080L

// D3DLOCK_NOSYSLOCK not supported on Xbox
// D3DLOCK_NO_DIRTY_UPDATE not supported on Xbox
// D3DLOCK_DISCARD not supported on Xbox
//
//     (NOTE: The lack of D3DLOCK_DISCARD support can cause significant
//      performance degradation for the unwary.  See the documentation
//      for notes on using IsBusy or fences in order to manage your own
//      pool of temporary vertex buffers when generating vertex data
//      dynamically.)

/* Vertex Buffer Description */
typedef struct _D3DVERTEXBUFFER_DESC
{
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;

} D3DVERTEXBUFFER_DESC;

/* Index Buffer Description */
typedef struct _D3DINDEXBUFFER_DESC
{
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
} D3DINDEXBUFFER_DESC;


/* Surface Description */
typedef struct _D3DSURFACE_DESC
{
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
    DWORD               Usage;
    UINT                Size;

    D3DMULTISAMPLE_TYPE MultiSampleType;
    UINT                Width;
    UINT                Height;
} D3DSURFACE_DESC;

typedef struct _D3DVOLUME_DESC
{
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
    DWORD               Usage;
    UINT                Size;

    UINT                Width;
    UINT                Height;
    UINT                Depth;
} D3DVOLUME_DESC;

/* Structure for LockRect */
typedef struct _D3DLOCKED_RECT
{
    INT                 Pitch;
    void*               pBits;
} D3DLOCKED_RECT;

/* Structures for LockBox */
typedef struct _D3DBOX
{
    UINT                Left;
    UINT                Top;
    UINT                Right;
    UINT                Bottom;
    UINT                Front;
    UINT                Back;
} D3DBOX;

typedef struct _D3DLOCKED_BOX
{
    INT                 RowPitch;
    INT                 SlicePitch;
    void*               pBits;
} D3DLOCKED_BOX;

/* Structures for high order primitives */
typedef struct _D3DRECTPATCH_INFO
{
    UINT                StartVertexOffsetWidth;
    UINT                StartVertexOffsetHeight;
    UINT                Width;
    UINT                Height;
    UINT                Stride;
    D3DBASISTYPE        Basis;
    D3DORDERTYPE        Order;
} D3DRECTPATCH_INFO;

typedef struct _D3DTRIPATCH_INFO
{
    UINT                StartVertexOffset;
    UINT                NumVertices;
    D3DBASISTYPE        Basis;
    D3DORDERTYPE        Order;
} D3DTRIPATCH_INFO;

/* Adapter Identifier */
#define MAX_DEVICE_IDENTIFIER_STRING        512
typedef struct _D3DADAPTER_IDENTIFIER8
{
    char            Driver[MAX_DEVICE_IDENTIFIER_STRING];
    char            Description[MAX_DEVICE_IDENTIFIER_STRING];

#ifdef _WIN32
    LARGE_INTEGER   DriverVersion;            /* Defined for 32 bit components */
#else
    DWORD           DriverVersionLowPart;     /* Defined for 16 bit driver components */
    DWORD           DriverVersionHighPart;
#endif

    DWORD           VendorId;
    DWORD           DeviceId;
    DWORD           SubSysId;
    DWORD           Revision;

    GUID            DeviceIdentifier;

    DWORD           WHQLLevel;

} D3DADAPTER_IDENTIFIER8;

/* Raster Status structure returned by GetRasterStatus */
typedef struct _D3DRASTER_STATUS
{
    BOOL            InVBlank;
    UINT            ScanLine;
} D3DRASTER_STATUS;

/* Field Type enum defines possible values for a display field */
typedef enum _D3DFIELDTYPE                      // Xbox extension
{
    D3DFIELD_ODD            = 1,
    D3DFIELD_EVEN           = 2,
    D3DFIELD_PROGRESSIVE    = 3,
    D3DFIELD_FORCE_DWORD    = 0x7fffffff, /* force 32-bit size enum */
} D3DFIELDTYPE;

/* Enumerants that can be passed to GetPushDistance (in addition to fence values) */
typedef enum _D3DDISTANCETYPE                   // Xbox extension
{
    D3DDISTANCE_FENCES_TOIDLE   = 2,
    D3DDISTANCE_FENCES_TOWAIT   = 4,

    D3DDISTANCE_FORCE_DWORD     = 0x7fffffff, /* force 32-bit size enum */
} D3DDISTANCETYPE;

/* Wait status passed to SetWaitCallback callback function */
#define D3DWAIT_PRESENT            0x01
#define D3DWAIT_BLOCKUNTILIDLE     0x02
#define D3DWAIT_BLOCKONFENCE       0x04
#define D3DWAIT_PUSHBUFFER         0x08
#define D3DWAIT_OBJECTLOCK         0x10
#define D3DWAIT_BUSYWAIT_FLAG      0x80000000       // Flag

/* Field Status structure is returned by GetDisplayFieldStatus */
typedef struct _D3DFIELD_STATUS                 // Xbox extension
{
    D3DFIELDTYPE        Field;
    UINT                VBlankCount;
} D3DFIELD_STATUS;

/* SetVertexInput struct */
typedef struct _D3DSTREAM_INPUT                 // Xbox extension
{
    D3DVertexBuffer    *VertexBuffer;
    UINT                Stride;
    UINT                Offset;
} D3DSTREAM_INPUT;

/* Maximum number of scissors rectangles */
#define D3DSCISSORS_MAX             8

/* D3DTILE constants */
#define D3DTILE_MAXTILES            8
#define D3DTILE_MAXTAGS             76800
#define D3DTILE_TAGSIZE             64
#define D3DTILE_ALIGNMENT           0x4000

/* D3DTILE macro for calculating the end tag for a Z-compressed tile */
#define D3DTILE_ZENDTAG(pTile) ((((pTile)->ZStartTag + ((pTile)->Size / D3DTILE_TAGSIZE)) + 255) & ~255)

/* D3DTILE Flags */
#define D3DTILE_FLAGS_ZBUFFER       0x00000001
#define D3DTILE_FLAGS_ZCOMPRESS     0x80000000
#define D3DTILE_FLAGS_Z32BITS       0x04000000
#define D3DTILE_FLAGS_Z16BITS       0x00000000

/* D3DTILE Pitch values */
#define D3DTILE_PITCH_0200          0x0200
#define D3DTILE_PITCH_0300          0x0300
#define D3DTILE_PITCH_0400          0x0400
#define D3DTILE_PITCH_0500          0x0500
#define D3DTILE_PITCH_0600          0x0600
#define D3DTILE_PITCH_0700          0x0700
#define D3DTILE_PITCH_0800          0x0800
#define D3DTILE_PITCH_0A00          0x0A00
#define D3DTILE_PITCH_0C00          0x0C00
#define D3DTILE_PITCH_0E00          0x0E00
#define D3DTILE_PITCH_1000          0x1000
#define D3DTILE_PITCH_1400          0x1400
#define D3DTILE_PITCH_1800          0x1800
#define D3DTILE_PITCH_1C00          0x1C00
#define D3DTILE_PITCH_2000          0x2000
#define D3DTILE_PITCH_2800          0x2800
#define D3DTILE_PITCH_3000          0x3000
#define D3DTILE_PITCH_3800          0x3800
#define D3DTILE_PITCH_4000          0x4000
#define D3DTILE_PITCH_5000          0x5000
#define D3DTILE_PITCH_6000          0x6000
#define D3DTILE_PITCH_7000          0x7000
#define D3DTILE_PITCH_8000          0x8000
#define D3DTILE_PITCH_A000          0xA000
#define D3DTILE_PITCH_C000          0xC000
#define D3DTILE_PITCH_E000          0xE000

/* SetTile struct */
typedef struct _D3DTILE                         // Xbox extension
{
    DWORD   Flags;
    void*   pMemory;
    DWORD   Size;
    DWORD   Pitch;
    DWORD   ZStartTag;
    DWORD   ZOffset;
} D3DTILE;

/* CopyRectsState Operation values */
typedef enum _D3DCOPYRECTOPERATION
{
    D3DCOPYRECT_SRCCOPY_AND         = 0,
    D3DCOPYRECT_ROP_AND             = 1,
    D3DCOPYRECT_BLEND_AND           = 2,
    D3DCOPYRECT_SRCCOPY             = 3,
    D3DCOPYRECT_SRCCOPY_PREMULT     = 4,
    D3DCOPYRECT_BLEND_PREMULT       = 5,
    D3DCOPYRECT_FORCE_DWORD         = 0x7fffffff, /* force 32-bit size enum */
} D3DCOPYRECTOPERATION;

/* CopyRectsState Color Format values */
typedef enum _D3DCOPYRECTCOLORFORMAT
{
    D3DCOPYRECT_COLOR_FORMAT_DEFAULT                 = 0,
    D3DCOPYRECT_COLOR_FORMAT_Y8                      = 1,
    D3DCOPYRECT_COLOR_FORMAT_X1R5G5B5_Z1R5G5B5       = 2,
    D3DCOPYRECT_COLOR_FORMAT_X1R5G5B5_O1R5G5B5       = 3,
    D3DCOPYRECT_COLOR_FORMAT_R5G6B5                  = 4,
    D3DCOPYRECT_COLOR_FORMAT_Y16                     = 5,
    D3DCOPYRECT_COLOR_FORMAT_X8R8G8B8_Z8R8G8B8       = 6,
    D3DCOPYRECT_COLOR_FORMAT_X8R8G8B8_O8R8G8B8       = 7,
    D3DCOPYRECT_COLOR_FORMAT_X1A7R8G8B8_Z1A7R8G8B8   = 8,
    D3DCOPYRECT_COLOR_FORMAT_X1A7R8G8B8_O1A7R8G8B8   = 9,
    D3DCOPYRECT_COLOR_FORMAT_A8R8G8B8                = 10,
    D3DCOPYRECT_COLOR_FORMAT_Y32                     = 11,
    D3DCOPYRECT_COLOR_FORMAT_FORCE_DWORD             = 0x7fffffff, /* force 32-bit size enum */
} D3DCOPYRECTCOLORFORMAT;

/* CopyRectsState struct */
typedef struct _D3DCOPYRECTSTATE                // Xbox extension
{
    D3DCOPYRECTCOLORFORMAT ColorFormat;
    D3DCOPYRECTOPERATION Operation;

    BOOL ColorKeyEnable;
    DWORD ColorKeyValue;

    // D3DCOPYRECT_BLEND_AND alpha value
    // The VALUE_FRACTION bits (30:21) contain the 10 bit unsigned fraction of the alpha value.
    // The VALUE bits (31:31) contain the 1 bit signed integer of the alpha value.
    DWORD BlendAlpha;

    // D3DCOPYRECT_*_PREMULT alpha value
    // Contains an alpha value for all four channels.
    DWORD PremultAlpha;

    // Clipping Rect
    DWORD ClippingPoint;    // y_x S16_S16
    DWORD ClippingSize;     // height_width U16_U16

} D3DCOPYRECTSTATE;

/* CopyRectRopState struct */
typedef struct _D3DCOPYRECTROPSTATE             // Xbox extension
{
    DWORD Rop;              // Ternary raster operation.
                            //   DSTINVERT:0x55, SRCCOPY:0xCC,
                            //   SRCPAINT:0xEE, SRCINVERT:0x66,
                            //   ...

    DWORD Shape;            // 0:8X_8Y, 1:64X_1Y, 2:1X_64Y
    DWORD PatternSelect;    // 1:monochrome, 2:color

    DWORD MonoColor0;       // Color to use when bit is "0"
    DWORD MonoColor1;       // Color to use when bit is "1"

    DWORD MonoPattern0;     // 8x8 = 64 bit pattern
    DWORD MonoPattern1;     //

    CONST DWORD *ColorPattern;  // Color Pattern used if PatternSelect == color
                                // 32-bit: Array of 64 DWORDS
                                // 16-bit: Array of 32 DWORDS

} D3DCOPYRECTROPSTATE;

/*----------------------------------------------
/* Pixel Shader Binary Format
/*----------------------------------------------

/*---------------------------------------------------*/
/*  This structure holds all of the state necessary  */
/*  to define an Xbox Pixel Shader.  It is the       */
/*  structure generated when a pixel shader is       */
/*  assembled.  Each member of this structure        */
/*  corresponds to a D3D Renderstate that can be     */
/*  set at any time using the SetRenderState()       */
/*  method.                                          */
/*  The members of this structure have been ordered  */
/*  to enable the most efficient hardware load       */
/*  possible.                                        */
/*---------------------------------------------------*/

typedef struct _D3DPixelShaderDef
{
   DWORD    PSAlphaInputs[8];          // Alpha inputs for each stage
   DWORD    PSFinalCombinerInputsABCD; // Final combiner inputs
   DWORD    PSFinalCombinerInputsEFG;  // Final combiner inputs (continued)
   DWORD    PSConstant0[8];            // C0 for each stage
   DWORD    PSConstant1[8];            // C1 for each stage
   DWORD    PSAlphaOutputs[8];         // Alpha output for each stage
   DWORD    PSRGBInputs[8];            // RGB inputs for each stage
   DWORD    PSCompareMode;             // Compare modes for clipplane texture mode
   DWORD    PSFinalCombinerConstant0;  // C0 in final combiner
   DWORD    PSFinalCombinerConstant1;  // C1 in final combiner
   DWORD    PSRGBOutputs[8];           // Stage 0 RGB outputs
   DWORD    PSCombinerCount;           // Active combiner count (Stages 0-7)
   DWORD    PSTextureModes;            // Texture addressing modes
   DWORD    PSDotMapping;              // Input mapping for dot product modes
   DWORD    PSInputTexture;            // Texture source for some texture modes
   DWORD    PSC0Mapping;               // Mapping of c0 regs to D3D constants
   DWORD    PSC1Mapping;               // Mapping of c1 regs to D3D constants
   DWORD    PSFinalCombinerConstants;  // Final combiner constant mapping
} D3DPIXELSHADERDEF;

/*---------------------------------------------------------------------------*/
/*  Texture configuration - The following members of the D3DPixelShaderDef   */
/*  structure define the addressing modes of each of the four texture stages:*/
/*      PSTextureModes                                                       */
/*      PSDotMapping                                                         */
/*      PSInputTexture                                                       */
/*      PSCompareMode                                                        */
/*---------------------------------------------------------------------------*/

// =========================================================================================================
// PSTextureModes
// --------.--------.--------.---xxxxx stage0
// --------.--------.------xx.xxx----- stage1
// --------.--------.-xxxxx--.-------- stage2
// --------.----xxxx.x-------.-------- stage3

#define PS_TEXTUREMODES(t0,t1,t2,t3) ((DWORD)(((t3)<<15)|((t2)<<10)|((t1)<<5)|(t0)))

/*
Texture modes:
NONE           :stage inactive
PROJECT2D      :argb = texture(s/q, t/q)
PROJECT3D      :argb = texture(s/q, t/q, r/q)
CUBEMAP        :argb = cubemap(s,t,r)
PASSTHRU       :argb = s,t,r,q
CLIPPLANE      :pixel not drawn if s,t,r, or q < 0.  PSCompareMode affects comparison
BUMPENVMAP     :argb=texture(s+mat00*src.r+mat01*src.g,
                             t+mat10*src.r+mat11*src.g)
                mat00 set via D3DTSS_BUMPENVMAT00, etc.
BUMPENVMAP_LUM :argb=texture(s+mat00*src.r+mat01*src.g,
                             t+mat10*src.r+mat11*src.g);
                rgb *= (lum_scale*src.b + lum_bias); (a is not affected)
                lum_scale set by D3DTSS_BUMPENVLSCALE
                lum_bias set by D3DTSS_BUMPENVLOFFSET
                mat00 set via D3DTSS_BUMPENVMAT00, etc.
BRDF           :argb = texture(eyeSigma, lightSigma, dPhi)
                       eyeSigma = Sigma of eye vector in spherical coordinates
                       lightSigma = Sigma of light vector in spherical coordinates
                       dPhi = Phi of eye - Phi of light
DOT_ST         :argb = texture(<DotResult of stage-1>, (s,t,r).(src.r,src.g,src.b))
DOT_ZW         :frag depth = (<DotResult of stage-1>/((s,t,r).(src.r,src.g,src.b))
DOT_RFLCT_DIFF :n = (<DotResult of stage-1>,(s,t,r).(src.r,src.g,src.b),<DotResult of stage+1>)
                argb = cubemap(n)
DOT_RFLCT_SPEC :n = (<DotResult of stage-2>,<DotResult of stage-1>,(s,t,r).(src.r,src.g,src.b))
                r = 2*n*(n.e)/(n.n) - e where e is eye vector built from q coord of each stage
                argb = cubemap(r)
DOT_STR_3D     :argb=texture((<DotResult of stage-2>,<DotResult of stage-1>,(s,t,r).(src.r,src.g,src.b)))
DOT_STR_CUBE   :argb=cubemap((<DotResult of stage-2>,<DotResult of stage-1>,(s,t,r).(src.r,src.g,src.b)))
DEPENDENT_AR   :argb = texture(src.a, src.r)
DEPENDENT_GB   :argb = texture(src.g, src.b)
DOTPRODUCT     :argb = (s,t,r).(src.r,src.g,src.b)
DOT_RFLCT_SPEC_CONST :n = (<DotResult of stage-2>,<DotResult of stage-1>,(s,t,r).(src.r,src.g,src.b))
                r = 2*n*(n.e)/(n.n) - e where e is eye vector set via SetEyeVector()
                argb = cubemap(r)
*/

enum PS_TEXTUREMODES
{                                 // valid in stage 0 1 2 3
    PS_TEXTUREMODES_NONE=                 0x00L, // * * * *
    PS_TEXTUREMODES_PROJECT2D=            0x01L, // * * * *
    PS_TEXTUREMODES_PROJECT3D=            0x02L, // * * * *
    PS_TEXTUREMODES_CUBEMAP=              0x03L, // * * * *
    PS_TEXTUREMODES_PASSTHRU=             0x04L, // * * * *
    PS_TEXTUREMODES_CLIPPLANE=            0x05L, // * * * *
    PS_TEXTUREMODES_BUMPENVMAP=           0x06L, // - * * *
    PS_TEXTUREMODES_BUMPENVMAP_LUM=       0x07L, // - * * *
    PS_TEXTUREMODES_BRDF=                 0x08L, // - - * *
    PS_TEXTUREMODES_DOT_ST=               0x09L, // - - * *
    PS_TEXTUREMODES_DOT_ZW=               0x0aL, // - - * *
    PS_TEXTUREMODES_DOT_RFLCT_DIFF=       0x0bL, // - - * -
    PS_TEXTUREMODES_DOT_RFLCT_SPEC=       0x0cL, // - - - *
    PS_TEXTUREMODES_DOT_STR_3D=           0x0dL, // - - - *
    PS_TEXTUREMODES_DOT_STR_CUBE=         0x0eL, // - - - *
    PS_TEXTUREMODES_DPNDNT_AR=            0x0fL, // - * * *
    PS_TEXTUREMODES_DPNDNT_GB=            0x10L, // - * * *
    PS_TEXTUREMODES_DOTPRODUCT=           0x11L, // - * * -
    PS_TEXTUREMODES_DOT_RFLCT_SPEC_CONST= 0x12L, // - - - *
    // 0x13-0x1f reserved
};


// =========================================================================================================
// PSDotMapping
// --------.--------.--------.-----xxx // stage1
// --------.--------.--------.-xxx---- // stage2
// --------.--------.-----xxx.-------- // stage3

#define PS_DOTMAPPING(t0,t1,t2,t3) ((DWORD)(((t3)<<8)|((t2)<<4)|(t1)))

// Mappings:
// ZERO_TO_ONE         :rgb->(r,g,b): 0x0=>0.0, 0xff=>1.0
// MINUS1_TO_1_D3D     :rgb->(r,g,b): 0x0=>-128/127, 0x01=>-1.0, 0x80=>0.0, 0xff=>1.0
// MINUS1_TO_1_GL      :rgb->(r,g,b): 0x80=>-1.0, 0x7f=>1.0
// MINUS1_TO_1         :rgb->(r,g,b): 0x80=>-128/127, 0x81=>-1.0, 0x0=>0.0, 0x7f=>1.0
// HILO_1              :HL->(H,L,1.0): 0x0000=>0.0, 0xffff=>1.0
// HILO_HEMISPHERE     :HL->(H,L,sqrt(1-H*H-L*L)): 0x8001=>-1.0, 0x0=>0.0, 0x7fff=>1.0, 0x8000=>-32768/32767

enum PS_DOTMAPPING
{                              // valid in stage 0 1 2 3
    PS_DOTMAPPING_ZERO_TO_ONE=         0x00L, // - * * *
    PS_DOTMAPPING_MINUS1_TO_1_D3D=     0x01L, // - * * *
    PS_DOTMAPPING_MINUS1_TO_1_GL=      0x02L, // - * * *
    PS_DOTMAPPING_MINUS1_TO_1=         0x03L, // - * * *
    PS_DOTMAPPING_HILO_1=              0x04L, // - * * *
    PS_DOTMAPPING_HILO_HEMISPHERE=     0x07L, // - * * *
};

// =========================================================================================================
// PSCompareMode
// --------.--------.--------.----xxxx // stage0
// --------.--------.--------.xxxx---- // stage1
// --------.--------.----xxxx.-------- // stage2
// --------.--------.xxxx----.-------- // stage3

#define PS_COMPAREMODE(t0,t1,t2,t3) ((DWORD)(((t3)<<12)|((t2)<<8)|((t1)<<4)|(t0)))

enum PS_COMPAREMODE
{
    PS_COMPAREMODE_S_LT= 0x00L,
    PS_COMPAREMODE_S_GE= 0x01L,

    PS_COMPAREMODE_T_LT= 0x00L,
    PS_COMPAREMODE_T_GE= 0x02L,

    PS_COMPAREMODE_R_LT= 0x00L,
    PS_COMPAREMODE_R_GE= 0x04L,

    PS_COMPAREMODE_Q_LT= 0x00L,
    PS_COMPAREMODE_Q_GE= 0x08L,
};

// =========================================================================================================
// PSInputTexture
// --------.-------x.--------.-------- // stage2
// --------.--xx----.--------.-------- // stage3
//
// Selects the other texture to use as an input in the following texture modes:
// DOT_ST, DOT_STR_3D, DOT_STR_CUBE, DOT_ZW, DOT_RFLCT_SPEC,
// DOT_RFLCT_DIFF, DPNDNT_AR, DPNDNT_GB, BUMPENVMAP,
// BUMPENVMAP_LUM, DOT_PRODUCT

#define PS_INPUTTEXTURE(t0,t1,t2,t3) ((DWORD)(((t3)<<20)|((t2)<<16)))


/*---------------------------------------------------------------------------------*/
/*  Color combiners - The following members of the D3DPixelShaderDef structure     */
/*  define the state for the eight stages of color combiners:                      */
/*      PSCombinerCount - Number of stages                                         */
/*      PSAlphaInputs[8] - Inputs for alpha portion of each stage                  */
/*      PSRGBInputs[8] - Inputs for RGB portion of each stage                      */
/*      PSConstant0[8] - Constant 0 for each stage                                 */
/*      PSConstant1[8] - Constant 1 for each stage                                 */
/*      PSFinalCombinerConstant0 - Constant 0 for final combiner                   */
/*      PSFinalCombinerConstant1 - Constant 1 for final combiner                   */
/*      PSAlphaOutputs[8] - Outputs for alpha portion of each stage                */
/*      PSRGBOutputs[8] - Outputs for RGB portion of each stage                    */
/*---------------------------------------------------------------------------------*/


// =========================================================================================================
// PSCombinerCount
// --------.--------.--------.----xxxx // number of combiners (1-8)
// --------.--------.-------x.-------- // mux bit (0= LSB, 1= MSB)
// --------.--------.---x----.-------- // separate C0
// --------.-------x.--------.-------- // separate C1

#define PS_COMBINERCOUNT(count, flags) ((DWORD)(((flags)<<8)|(count)))
// count is 1-8, flags contains one or more values from PS_COMBINERCOUNTFLAGS

enum PS_COMBINERCOUNTFLAGS
{
    PS_COMBINERCOUNT_MUX_LSB=     0x0000L, // mux on r0.a lsb
    PS_COMBINERCOUNT_MUX_MSB=     0x0001L, // mux on r0.a msb

    PS_COMBINERCOUNT_SAME_C0=     0x0000L, // c0 same in each stage
    PS_COMBINERCOUNT_UNIQUE_C0=   0x0010L, // c0 unique in each stage

    PS_COMBINERCOUNT_SAME_C1=     0x0000L, // c1 same in each stage
    PS_COMBINERCOUNT_UNIQUE_C1=   0x0100L  // c1 unique in each stage
};


// =========================================================================================================
// PSRGBInputs[0-7]
// PSAlphaInputs[0-7]
// PSFinalCombinerInputsABCD
// PSFinalCombinerInputsEFG
// --------.--------.--------.----xxxx // D register
// --------.--------.--------.---x---- // D channel (0= RGB/BLUE, 1= ALPHA)
// --------.--------.--------.xxx----- // D input mapping
// --------.--------.----xxxx.-------- // C register
// --------.--------.---x----.-------- // C channel (0= RGB/BLUE, 1= ALPHA)
// --------.--------.xxx-----.-------- // C input mapping
// --------.----xxxx.--------.-------- // B register
// --------.---x----.--------.-------- // B channel (0= RGB/BLUE, 1= ALPHA)
// --------.xxx-----.--------.-------- // B input mapping
// ----xxxx.--------.--------.-------- // A register
// ---x----.--------.--------.-------- // A channel (0= RGB/BLUE, 1= ALPHA)
// xxx-----.--------.--------.-------- // A input mapping

// examples:
//
// shader.PSRGBInputs[3]= PS_COMBINERINPUTS(
//     PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL     | PS_CHANNEL_RGB,
//     PS_REGISTER_C0 | PS_INPUTMAPPING_UNSIGNED_IDENTITY | PS_CHANNEL_ALPHA,
//     PS_REGISTER_ZERO,
//     PS_REGISTER_ZERO);
//
// shader.PSFinalCombinerInputsABCD= PS_COMBINERINPUTS(
//     PS_REGISTER_T0     | PS_INPUTMAPPING_UNSIGNED_IDENTITY | PS_CHANNEL_ALPHA,
//     PS_REGISTER_ZERO   | PS_INPUTMAPPING_EXPAND_NORMAL     | PS_CHANNEL_RGB,
//     PS_REGISTER_EFPROD | PS_INPUTMAPPING_UNSIGNED_INVERT   | PS_CHANNEL_RGB,
//     PS_REGISTER_ZERO);
//
// PS_FINALCOMBINERSETTING is set in 4th field of PSFinalCombinerInputsEFG with PS_COMBINERINPUTS
// example:
//
// shader.PSFinalCombinerInputsEFG= PS_COMBINERINPUTS(
//     PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_IDENTITY | PS_CHANNEL_RGB,
//     PS_REGISTER_R1 | PS_INPUTMAPPING_UNSIGNED_IDENTITY | PS_CHANNEL_RGB,
//     PS_REGISTER_R1 | PS_INPUTMAPPING_UNSIGNED_IDENTITY | PS_CHANNEL_BLUE,
//    PS_FINALCOMBINERSETTING_CLAMP_SUM | PS_FINALCOMBINERSETTING_COMPLEMENT_R0);

#define PS_COMBINERINPUTS(a,b,c,d) ((DWORD)(((a)<<24)|((b)<<16)|((c)<<8)|(d)))
// For PSFinalCombinerInputsEFG,
//     a,b,c contain a value from PS_REGISTER, PS_CHANNEL, and PS_INPUTMAPPING for input E,F, and G
//     d contains values from PS_FINALCOMBINERSETTING
// For all other inputs,
//     a,b,c,d each contain a value from PS_REGISTER, PS_CHANNEL, and PS_INPUTMAPPING

enum PS_INPUTMAPPING
{
    PS_INPUTMAPPING_UNSIGNED_IDENTITY= 0x00L, // max(0,x)         OK for final combiner
    PS_INPUTMAPPING_UNSIGNED_INVERT=   0x20L, // 1 - max(0,x)     OK for final combiner
    PS_INPUTMAPPING_EXPAND_NORMAL=     0x40L, // 2*max(0,x) - 1   invalid for final combiner
    PS_INPUTMAPPING_EXPAND_NEGATE=     0x60L, // 1 - 2*max(0,x)   invalid for final combiner
    PS_INPUTMAPPING_HALFBIAS_NORMAL=   0x80L, // max(0,x) - 1/2   invalid for final combiner
    PS_INPUTMAPPING_HALFBIAS_NEGATE=   0xa0L, // 1/2 - max(0,x)   invalid for final combiner
    PS_INPUTMAPPING_SIGNED_IDENTITY=   0xc0L, // x                invalid for final combiner
    PS_INPUTMAPPING_SIGNED_NEGATE=     0xe0L, // -x               invalid for final combiner
};

enum PS_REGISTER
{
    PS_REGISTER_ZERO=              0x00L, // r
    PS_REGISTER_DISCARD=           0x00L, // w
    PS_REGISTER_C0=                0x01L, // r
    PS_REGISTER_C1=                0x02L, // r
    PS_REGISTER_FOG=               0x03L, // r
    PS_REGISTER_V0=                0x04L, // r/w
    PS_REGISTER_V1=                0x05L, // r/w
    PS_REGISTER_T0=                0x08L, // r/w
    PS_REGISTER_T1=                0x09L, // r/w
    PS_REGISTER_T2=                0x0aL, // r/w
    PS_REGISTER_T3=                0x0bL, // r/w
    PS_REGISTER_R0=                0x0cL, // r/w
    PS_REGISTER_R1=                0x0dL, // r/w
    PS_REGISTER_V1R0_SUM=          0x0eL, // r
    PS_REGISTER_EF_PROD=           0x0fL, // r

    PS_REGISTER_ONE=               PS_REGISTER_ZERO | PS_INPUTMAPPING_UNSIGNED_INVERT, // OK for final combiner
    PS_REGISTER_NEGATIVE_ONE=      PS_REGISTER_ZERO | PS_INPUTMAPPING_EXPAND_NORMAL,   // invalid for final combiner
    PS_REGISTER_ONE_HALF=          PS_REGISTER_ZERO | PS_INPUTMAPPING_HALFBIAS_NEGATE, // invalid for final combiner
    PS_REGISTER_NEGATIVE_ONE_HALF= PS_REGISTER_ZERO | PS_INPUTMAPPING_HALFBIAS_NORMAL, // invalid for final combiner
};

// FOG ALPHA is only available in final combiner
// V1R0_SUM and EF_PROD are only available in final combiner (A,B,C,D inputs only)
// V1R0_SUM_ALPHA and EF_PROD_ALPHA are not available
// R0_ALPHA is initialized to T0_ALPHA in stage0

enum PS_CHANNEL
{
    PS_CHANNEL_RGB=   0x00, // used as RGB source
    PS_CHANNEL_BLUE=  0x00, // used as ALPHA source
    PS_CHANNEL_ALPHA= 0x10, // used as RGB or ALPHA source
};


enum PS_FINALCOMBINERSETTING
{
    PS_FINALCOMBINERSETTING_CLAMP_SUM=     0x80, // V1+R0 sum clamped to [0,1]

    PS_FINALCOMBINERSETTING_COMPLEMENT_V1= 0x40, // unsigned invert mapping

    PS_FINALCOMBINERSETTING_COMPLEMENT_R0= 0x20, // unsigned invert mapping
};

// =========================================================================================================
// PSRGBOutputs[0-7]
// PSAlphaOutputs[0-7]
// --------.--------.--------.----xxxx // CD register
// --------.--------.--------.xxxx---- // AB register
// --------.--------.----xxxx.-------- // SUM register
// --------.--------.---x----.-------- // CD output (0= multiply, 1= dot product)
// --------.--------.--x-----.-------- // AB output (0= multiply, 1= dot product)
// --------.--------.-x------.-------- // AB_CD mux/sum select (0= sum, 1= mux)
// --------.------xx.x-------.-------- // Output mapping
// --------.-----x--.--------.-------- // CD blue to alpha
// --------.----x---.--------.-------- // AB blue to alpha

#define PS_COMBINEROUTPUTS(ab,cd,mux_sum,flags) ((DWORD)(((flags)<<12)|((mux_sum)<<8)|((ab)<<4)|(cd)))
// ab,cd,mux_sum contain a value from PS_REGISTER
// flags contains values from PS_COMBINEROUTPUT

enum PS_COMBINEROUTPUT
{
    PS_COMBINEROUTPUT_IDENTITY=            0x00L, // y = x
    PS_COMBINEROUTPUT_BIAS=                0x08L, // y = x - 0.5
    PS_COMBINEROUTPUT_SHIFTLEFT_1=         0x10L, // y = x*2
    PS_COMBINEROUTPUT_SHIFTLEFT_1_BIAS=    0x18L, // y = (x - 0.5)*2
    PS_COMBINEROUTPUT_SHIFTLEFT_2=         0x20L, // y = x*4
    PS_COMBINEROUTPUT_SHIFTRIGHT_1=        0x30L, // y = x/2

    PS_COMBINEROUTPUT_AB_BLUE_TO_ALPHA=    0x80L, // RGB only

    PS_COMBINEROUTPUT_CD_BLUE_TO_ALPHA=    0x40L, // RGB only

    PS_COMBINEROUTPUT_AB_MULTIPLY=         0x00L,
    PS_COMBINEROUTPUT_AB_DOT_PRODUCT=      0x02L, // RGB only

    PS_COMBINEROUTPUT_CD_MULTIPLY=         0x00L,
    PS_COMBINEROUTPUT_CD_DOT_PRODUCT=      0x01L, // RGB only

    PS_COMBINEROUTPUT_AB_CD_SUM=           0x00L, // 3rd output is AB+CD
    PS_COMBINEROUTPUT_AB_CD_MUX=           0x04L, // 3rd output is MUX(AB,CD) based on R0.a
};

// AB_CD register output must be DISCARD if either AB_DOT_PRODUCT or CD_DOT_PRODUCT are set

// =========================================================================================================
// PSC0Mapping
// PSC1Mapping
// --------.--------.--------.----xxxx // offset of D3D constant for stage 0
// --------.--------.--------.xxxx---- // offset of D3D constant for stage 1
// --------.--------.----xxxx.-------- // offset of D3D constant for stage 2
// --------.--------.xxxx----.-------- // offset of D3D constant for stage 3
// --------.----xxxx.--------.-------- // offset of D3D constant for stage 4
// --------.xxxx----.--------.-------- // offset of D3D constant for stage 5
// ----xxxx.--------.--------.-------- // offset of D3D constant for stage 6
// xxxx----.--------.--------.-------- // offset of D3D constant for stage 7

#define PS_CONSTANTMAPPING(s0,s1,s2,s3,s4,s5,s6,s7) \
     ((DWORD)(((DWORD)(s0)&0xf)<< 0) | (((DWORD)(s1)&0xf)<< 4)  | \
              (((DWORD)(s2)&0xf)<< 8) | (((DWORD)(s3)&0xf)<<12) | \
              (((DWORD)(s4)&0xf)<<16) | (((DWORD)(s5)&0xf)<<20) | \
              (((DWORD)(s6)&0xf)<<24) | (((DWORD)(s7)&0xf)<<28))
// s0-s7 contain the offset of the D3D constant that corresponds to the
// c0 or c1 constant in stages 0 through 7.  These mappings are only used in
// SetPixelShaderConstant().

// =========================================================================================================
// PSFinalCombinerConstants
// --------.--------.--------.----xxxx // offset of D3D constant for C0
// --------.--------.--------.xxxx---- // offset of D3D constant for C1
// --------.--------.-------x.-------- // Adjust texture flag

#define PS_FINALCOMBINERCONSTANTS(c0,c1,flags) ((DWORD)(((DWORD)(flags) << 8) | ((DWORD)(c0)&0xf)<< 0) | (((DWORD)(c1)&0xf)<< 4))
// c0 and c1 contain the offset of the D3D constant that corresponds to the
// constants in the final combiner.  These mappings are only used in
// SetPixelShaderConstant().  Flags contains values from PS_GLOBALFLAGS

enum PS_GLOBALFLAGS
{
    // if this flag is set, the texture mode for each texture stage is adjusted as follows:
    //     if set texture is a cubemap,
    //         change PS_TEXTUREMODES_PROJECT2D to PS_TEXTUREMODES_CUBEMAP
    //         change PS_TEXTUREMODES_PROJECT3D to PS_TEXTUREMODES_CUBEMAP
    //         change PS_TEXTUREMODES_DOT_STR_3D to PS_TEXTUREMODES_DOT_STR_CUBE
    //     if set texture is a volume texture,
    //         change PS_TEXTUREMODES_PROJECT2D to PS_TEXTUREMODES_PROJECT3D
    //         change PS_TEXTUREMODES_CUBEMAP to PS_TEXTUREMODES_PROJECT3D
    //         change PS_TEXTUREMODES_DOT_STR_CUBE to PS_TEXTUREMODES_DOT_STR_3D
    //     if set texture is neither cubemap or volume texture,
    //         change PS_TEXTUREMODES_PROJECT3D to PS_TEXTUREMODES_PROJECT2D
    //         change PS_TEXTUREMODES_CUBEMAP to PS_TEXTUREMODES_PROJECT2D

    PS_GLOBALFLAGS_NO_TEXMODE_ADJUST=     0x0000L, // don't adjust texture modes
    PS_GLOBALFLAGS_TEXMODE_ADJUST=        0x0001L, // adjust texture modes according to set texture
};


typedef struct _D3DPixelShaderDefFile
{
    DWORD               FileID;         // Uniquely identifies the file as pixel shader binary
    D3DPIXELSHADERDEF   Psd;            // The pixel shader def structure
} D3DPIXELSHADERDEF_FILE;

#define D3DPIXELSHADERDEF_FILE_ID   0x30425350  // "PSB0"

/*----------------------- End of Pixel Shader Defines -----------------------*/

/*----------------------------------------------
/* Push-Buffer Binary Format
/*----------------------------------------------

/*---------------------------------------------------*/
/*  These defines describe the binary format of the  */
/*  push-buffer.  They may be used in conjunction    */
/*  with BeginState/EndState, BeginPush/EndPush and  */
/*  RunPushBuffer.                                   */
/*---------------------------------------------------*/

/* Encoding support */

#define D3DPUSH_ENCODE(Method, Count)   (((Count) << 18) + (Method))
#define D3DPUSH_NOINCREMENT_FLAG        0x40000000
#define D3DPUSH_MAX_COUNT               2047

/* Methods */

#define D3DPUSH_NO_OPERATION                0x00000100 // Parameter must be zero
#define D3DPUSH_SET_BEGIN_END               0x000017fc // Parameter is D3DPRIMITIVETYPE or 0 to end
#define D3DPUSH_INLINE_ARRAY                0x00001818 // Use NOINCREMENT_FLAG
#define D3DPUSH_SET_TRANSFORM_CONSTANT_LOAD 0x00001ea4 // Add 96 to constant index parameter
#define D3DPUSH_SET_TRANSFORM_CONSTANT      0x00000b80 // Can't use NOINCREMENT_FLAG, maximum of 32 writes

/*----------------------- End of Push-Buffer Defines -----------------------*/


#pragma pack()
#pragma warning(default:4201)

#endif /* (DIRECT3D_VERSION >= 0x0800) */
#endif /* _D3D8TYPES(P)_H_ */


