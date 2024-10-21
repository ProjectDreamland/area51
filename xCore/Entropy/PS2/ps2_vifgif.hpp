//=========================================================================
//
// E_VIFGIF.H
//
//=========================================================================

#ifndef E_VIFGIF_H
#define E_VIFGIF_H

#include "x_types.hpp"
#include "ps2_dlist.hpp"

//=========================================================================
//=========================================================================
//=========================================================================
// VIF
//=========================================================================
//=========================================================================
//=========================================================================

#define VIF_S_32     0x60
#define VIF_S_16     0x61
#define VIF_S_8      0x62

#define VIF_V2_32    0x64
#define VIF_V2_16    0x65
#define VIF_V2_8     0x66

#define VIF_V3_32    0x68
#define VIF_V3_16    0x69
#define VIF_V3_8     0x6A

#define VIF_V4_32    0x6C
#define VIF_V4_16    0x6D
#define VIF_V4_8     0x6E
#define VIF_V4_5     0x6F

u32    VIF_Unpack  (  s32    DestVUAddr,
                      s32    NVectors,
                      s32    Format,
                      xbool  Signed,
                      xbool  Masked,
                      xbool  AbsoluteAddress );

//=========================================================================

u32    VIF_SkipWrite( s32   NVectorsToWrite,
                      s32   NVectorsToSkip );

//=========================================================================

#define VIF_ASIS       0
#define VIF_ROW        1
#define VIF_COL        2
#define VIF_BLOCKED    3

// This command uses 2 u32s... one for the command and one for the bits

void    VIF_Mask    ( u32* pVifData,
                      s32 X0, s32 Y1, s32 Z0, s32 W0, 
                      s32 X1, s32 Y2, s32 Z1, s32 W1, 
                      s32 X2, s32 Y3, s32 Z2, s32 W2, 
                      s32 X3, s32 Y4, s32 Z3, s32 W3 );
                           
//=========================================================================
//=========================================================================
//=========================================================================
// GIF
//=========================================================================
//=========================================================================
//=========================================================================

#define     GIF_REG_PRIM         0x00
#define     GIF_REG_RGBAQ        0x01
#define     GIF_REG_ST           0x02
#define     GIF_REG_UV           0x03
#define     GIF_REG_XYZF2        0x04
#define     GIF_REG_XYZ2         0x05
#define     GIF_REG_TEX0_1       0x06
#define     GIF_REG_TEX0_2       0x07
#define     GIF_REG_CLAMP_1      0x08
#define     GIF_REG_CLAMP_2      0x09
#define     GIF_REG_FOG          0x0A
#define     GIF_REG_RESERVED     0x0B
#define     GIF_REG_XYZF3        0x0C
#define     GIF_REG_XYZ3         0x0D
#define     GIF_REG_AD           0x0E
#define     GIF_REG_NOP          0x0F
    
#define     GIF_MODE_PACKED      0
#define     GIF_MODE_REGLIST     1
#define     GIF_MODE_IMAGE       2
#define     GIF_MODE_IMAGE2      3

#define     GIF_PRIM_POINT               0
#define     GIF_PRIM_LINE                1
#define     GIF_PRIM_LINESTRIP           2
#define     GIF_PRIM_TRIANGLE            3
#define     GIF_PRIM_TRIANGLESTRIP       4
#define     GIF_PRIM_TRIANGLEFAN         5
#define     GIF_PRIM_SPRITE              6

#define     GIF_FLAG_SMOOTHSHADE        0x01
#define     GIF_FLAG_TEXTURE            0x02
#define     GIF_FLAG_FOG                0x04
#define     GIF_FLAG_ALPHA              0x08
#define     GIF_FLAG_ANTIALIAS          0x10
#define     GIF_FLAG_UV                 0x20
#define     GIF_FLAG_CONTEXT            0x40
#define     GIF_FLAG_INTERPFIX          0x80

struct giftag
{
    // SONY FORMAT
	u32 NLOOP:15;
	u32 EOP:1;
	u32 PAD16:16;
	u32 ID:14;
	u32 PRE:1;
	u32 PRIM:11;
	u32 MODE:2;
	u32 NREG:4;
	u32 R00:4;
	u32 R01:4;
	u32 R02:4;
	u32 R03:4;
	u32 R04:4;
	u32 R05:4;
	u32 R06:4;
	u32 R07:4;
	u32 R08:4;
	u32 R09:4;
	u32 R10:4;
	u32 R11:4;
	u32 R12:4;
	u32 R13:4;
	u32 R14:4;
	u32 R15:4;

    giftag      ( void );
    ~giftag     ( void );

    void Dump   ( void );
    
    void Build  ( s32   Mode,  
                  s32   NRegs, 
                  s32   NLoops, 
                  xbool UsePrim,
                  s32   PrimType,
                  u32   PrimFlags, 
                  xbool EOP );

    // Assumes PACKED, EOP=1, UsePrim=TRUE
    void Build2( s32 NRegs, 
                 s32 NLoops,
                 s32 PrimType,
                 u32 PrimFlags );

    void BuildRegLoad( s32 NRegs, xbool EOP );

    void BuildImageLoad( s32 NBytes, xbool EOP );

    void Reg    ( u8 R00 );
    void Reg    ( u8 R00, u8 R01 );
    void Reg    ( u8 R00, u8 R01, u8 R02 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04, u8 R05 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04, u8 R05, u8 R06 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04, u8 R05, u8 R06, u8 R07 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04, u8 R05, u8 R06, u8 R07, u8 R08 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04, u8 R05, u8 R06, u8 R07, u8 R08, u8 R09 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04, u8 R05, u8 R06, u8 R07, u8 R08, u8 R09, u8 R10 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04, u8 R05, u8 R06, u8 R07, u8 R08, u8 R09, u8 R10, u8 R11 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04, u8 R05, u8 R06, u8 R07, u8 R08, u8 R09, u8 R10, u8 R11, u8 R12 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04, u8 R05, u8 R06, u8 R07, u8 R08, u8 R09, u8 R10, u8 R11, u8 R12, u8 R13 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04, u8 R05, u8 R06, u8 R07, u8 R08, u8 R09, u8 R10, u8 R11, u8 R12, u8 R13, u8 R14 );
    void Reg    ( u8 R00, u8 R01, u8 R02, u8 R03, u8 R04, u8 R05, u8 R06, u8 R07, u8 R08, u8 R09, u8 R10, u8 R11, u8 R12, u8 R13, u8 R14, u8 R15 );

} PS2_ALIGNMENT(16);

//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
// INLINED ROUTINES
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================

#include "ps2_vifgif_inline.hpp"

//=========================================================================
#endif
//=========================================================================

