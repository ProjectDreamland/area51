//=========================================================================
//
// ps2_gsreg.hpp
//
//=========================================================================

#ifndef PS2_GSREG_HPP
#define PS2_GSREG_HPP

//=========================================================================

#include "x_types.hpp"

#include "entropy.hpp"

//=========================================================================
//  GS REGISTER SYSTEM
//=========================================================================
//
//  By wrapping REG calls with REG_Begin and REG_End, all the updates will
//  be shipped in one dma
//
//=========================================================================

void    gsreg_Begin ( s32 NRegs );
void    gsreg_Set   ( s32 Addr, u64 Data );
void    gsreg_End   ( void );

//=========================================================================
//  ALPHA BLENDING EQUATION Color = (A-B)*C>>7 + D
//=========================================================================

// These defines are used in the blend_mode macro
#define C_SRC       0
#define C_DST       1
#define C_ZERO      2
#define A_SRC       0
#define A_DST       1
#define A_FIX       2

#define ALPHA_BLEND_MODE(c1,c2,a,c3) (((c1)&0x3)|(((c2)&0x03)<<2)|(((a)&0x03)<<4)|(((c3)&0x03)<<6))

// Here are some predefined modes
#define ALPHA_BLEND_OFF     -1
#define ALPHA_BLEND_INTERP      ALPHA_BLEND_MODE(C_SRC,C_DST,A_SRC,C_DST)   // (Src-Dst)*SrcA + Dst
#define ALPHA_BLEND_ADD         ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_FIX,C_DST)  // (Src-  0)*127  + Dst

// Here is the call to set the alpha mode
void    gsreg_SetAlphaBlend  ( s32 Mode, s32 FixedAlpha=0 );

//=========================================================================

// Used to set the frame buffer write mask
void    gsreg_SetFBMASK ( u32 Mask );

//=========================================================================

// Set mip equation
#define MIP_MAG_POINT                   0
#define MIP_MAG_BILINEAR                1
#define MIP_MIN_POINT                   0
#define MIP_MIN_BILINEAR                1
#define MIP_MIN_CLAMP_MIPMAP_POINT      2
#define MIP_MIN_CLAMP_MIPMAP_BILINEAR   3
#define MIP_MIN_INTERP_MIPMAP_POINT     4
#define MIP_MIN_INTERP_MIPMAP_BILINEAR  5

void    gsreg_SetMipEquation( xbool Manual, f32 MipK, s32 NMips, s32 MagBlend, s32 MinBlend );

//=========================================================================

#define ALPHA_TEST_NEVER            0   // Off
#define ALPHA_TEST_ALWAYS           1   // All pass
#define ALPHA_TEST_LESS             2   // A <  AlphaRef pass
#define ALPHA_TEST_LEQUAL           3   // A <= AlphaRef pass
#define ALPHA_TEST_EQUAL            4   // A == AlphaRef pass
#define ALPHA_TEST_GEQUAL           5   // A >= AlphaRef pass
#define ALPHA_TEST_GREATER          6   // A >  AlphaRef pass
#define ALPHA_TEST_NOTEQUAL         7   // A != AlphaRef pass

#define ALPHA_TEST_FAIL_KEEP        0   // Neither frame buffer nor Z buffer updated
#define ALPHA_TEST_FAIL_FB_ONLY     1   // Only frame buffer is updated
#define ALPHA_TEST_FAIL_ZB_ONLY     2   // Only z buffer is updated
#define ALPHA_TEST_FAIL_RGB_ONLY    3   // Only frame RGB buffer is updated

#define DEST_ALPHA_TEST_0           0   // Pixels, whose dest alpha==0 pass
#define DEST_ALPHA_TEST_1           1   // Pixels, whose dest alpha==1 pass
                                    
#define ZBUFFER_TEST_NEVER          0   // All pixels fail
#define ZBUFFER_TEST_ALWAYS         1   // All pixels pass
#define ZBUFFER_TEST_GEQUAL         2   // Z >= Pass
#define ZBUFFER_TEST_GREATER        3   // Z >  Pass
                                    
// Turns Z-buffering on and off
void    gsreg_SetZBufferUpdate( xbool On );
void    gsreg_SetZBufferTest( s32 TestMethod );

// Turns alpha and z buffer testing on/off
// Can be used to setup special stencil, punch thru methods etc
void gsreg_SetAlphaAndZBufferTests ( xbool  AlphaTest,
                                     s32    AlphaTestMethod,
                                     u32    AlphaRef,
                                     s32    AlphaTestFail,
                                     xbool  DestAlphaTest,
                                     s32    DestAlphaTestMethod,
                                     xbool  ZBufferTest,
                                     s32    ZBufferTestMethod ) ;

//=========================================================================

// Sets alpha corretion (FBA) value. 
// Can be used for stencil methods together with dest alpha testing
// RGBA32 Mode: A = As | (FBA << 8) before writing to frame buffer
// RGBA16 Mode: A = As | (FBA & 0x01) before writing to frame buffer
void gsreg_SetAlphaCorrection ( xbool On ) ;

//=========================================================================

// Turns clamping on/off for both S and T (FALSE = REPEAT, TRUE = CLAMP)
void    gsreg_SetClamping( xbool On );

// Turns clamping on/off individually for S and T (FALSE = REPEAT, TRUE = CLAMP)
void    gsreg_SetClamping( xbool S, xbool T ) ;

//=========================================================================

// Sets scissor region
void    gsreg_SetScissor( s32 X0, s32 Y0, s32 X1, s32 Y1 );

//=========================================================================


#endif
//=========================================================================
//=========================================================================
//=========================================================================
// PRIVATE PRIVATE PRIVATE PRIVATE
//=========================================================================
//=========================================================================
//=========================================================================

struct gsreg
{
    u64 Data;
    u64 Addr;
};

struct gsreg_header
{
    dmatag  DMA;
    u64     GIF[2];
};

extern byte*            s_GsregData;
extern dlist            DLIST;

#ifdef X_ASSERT
extern gsreg_header*    s_pGsregHeader;
#endif

//=========================================================================

inline
void gsreg_Begin( s32 NRegs )
{
    ASSERT( !s_pGsregHeader );
    ASSERT( DLIST.InsideTask() );

    // allocate space for the register data and header
    s_GsregData = (byte*)DLIST.Alloc( sizeof(gsreg_header) + (NRegs*16) );

    // fill in the header
    gsreg_header* pHeader = (gsreg_header*)s_GsregData;
    s_GsregData += sizeof(gsreg_header);
    pHeader->DMA.SetCont( sizeof(gsreg_header)- sizeof(dmatag) + (NRegs*16) );
    pHeader->DMA.MakeDirect();
    pHeader->GIF[0] = SCE_GIF_SET_TAG( (u64)NRegs, (u64)1, (u64)0, (u64)0, (u64)0, (u64)1 );
    pHeader->GIF[1] = 0x000000000000000EL;

#ifdef X_ASSERT
    s_pGsregHeader = pHeader;
#endif
}

//=========================================================================

inline
void gsreg_End( void )
{
    ASSERT( s_pGsregHeader );
    ASSERT( (u32)(s_GsregData - (u32)(s_pGsregHeader->GIF[0] & 0x7fffL)*16) ==
            (u32)(s_pGsregHeader+1) );

#ifdef X_ASSERT
    s_pGsregHeader = NULL;
#endif
}

//=========================================================================

inline
void gsreg_Set( s32 Addr, u64 Data )
{
    ASSERT( s_pGsregHeader );

    gsreg* pReg = (gsreg*)s_GsregData;
    s_GsregData += sizeof(gsreg);
    pReg->Data = Data;
    pReg->Addr = (u64)Addr;
}

//=========================================================================

// Set mip equation
inline
void gsreg_SetMipEquation( xbool Manual, f32 MipK, s32 NMips, s32 MagBlend, s32 MinBlend  )
{
    ASSERT( (MagBlend>=0) && (MagBlend<=1) );
    ASSERT( (MinBlend>=0) && (MinBlend<=5) );

    u64 RegData = SCE_GS_SET_TEX1_1( Manual?1:0,                // LCM
                                     NMips,                     // MXL
                                     MagBlend,                  // MMAG
                                     MinBlend,                  // MMIN
                                     0,                         // MTBA
                                     0,                         // L
                                     (s32)(MipK*16.0f) );       // K
    gsreg_Set( (SCE_GS_TEX1_1 + eng_GetGSContext()), RegData );
}

//=========================================================================

inline
void gsreg_SetAlphaBlend  ( s32 Mode, s32 FixedAlpha )
{
    // Check if we are turning blending off
    if( Mode==-1 )
    {
        Mode = ALPHA_BLEND_MODE(C_ZERO,C_ZERO,A_SRC,C_SRC);
    }

    u64 RegData = ((u64)Mode) | (((u64)FixedAlpha)<<32) ;

    gsreg_Set( (SCE_GS_ALPHA_1+eng_GetGSContext()), RegData ) ;
}

//=========================================================================

inline
void gsreg_SetFBMASK ( u32 Mask )
{
    u64 RegData = eng_GetFRAMEReg();
    RegData = (RegData&0xffffffff) | (((u64)Mask)<<32);

    gsreg_Set( (SCE_GS_FRAME_1+eng_GetGSContext()), RegData );
}

//=========================================================================
void FB_GetFrameBufferPositions( s32& FB0, s32& FB1, s32& ZB );

//=========================================================================

inline
void gsreg_SetZBufferTest( s32 TestMethod )
{
    u64 RegData = SCE_GS_SET_TEST( 0,               // ATE
                                   1,               // ATST
                                   0,               // AREF
                                   0,               // AFAIL
                                   0,               // DATE
                                   0,               // DATM
                                   1,               // ZTE (must always be on due to hardware bug--see TRC's)
                                   TestMethod );    // ZTST
    gsreg_Set( (SCE_GS_TEST_1+eng_GetGSContext()), RegData );
}

//=========================================================================

inline
void gsreg_SetZBufferUpdate( xbool On )
{
    s32 FB0,FB1,ZB;
    FB_GetFrameBufferPositions(FB0,FB1,ZB);
    
    u64 RegData = SCE_GS_SET_ZBUF( ZB, SCE_GS_PSMZ24, (On)?(0):(1) );
    gsreg_Set( (SCE_GS_ZBUF_1+eng_GetGSContext()), RegData );
}

//=========================================================================

// General purpose alpha, dest alpha, and z buffer test setting
inline
void gsreg_SetAlphaAndZBufferTests ( xbool  AlphaTest,
                                     s32    AlphaTestMethod,
                                     u32    AlphaRef,
                                     s32    AlphaTestFail,
                                     xbool  DestAlphaTest,
                                     s32    DestAlphaTestMethod,
                                     xbool  ZBufferTest,
                                     s32    ZBufferTestMethod )
{
    ASSERT( ZBufferTest == 1 ); // hardware bug forces this--check out the TRC's
    u64 RegData = SCE_GS_SET_TEST( AlphaTest,               // ATE
                                   AlphaTestMethod,         // ATST
                                   AlphaRef,                // AREF
                                   AlphaTestFail,           // AFAIL
                                   DestAlphaTest,           // DATE
                                   DestAlphaTestMethod,     // DATM
                                   ZBufferTest,             // ZTE
                                   ZBufferTestMethod );     // ZTST

    gsreg_Set( (SCE_GS_TEST_1+eng_GetGSContext()), RegData );
}

//=========================================================================

inline
void gsreg_SetAlphaCorrection ( xbool On )
{
    u64 RegData = SCE_GS_SET_FBA( On?1:0 );    
    gsreg_Set( (SCE_GS_FBA_1+eng_GetGSContext()), RegData );
}

//=========================================================================

inline
void gsreg_SetClamping( xbool On )
{
    u64 RegData = SCE_GS_SET_CLAMP( (On)?(1):(0), (On)?(1):(0), 0, 0, 0, 0 );
    gsreg_Set( (SCE_GS_CLAMP_1+eng_GetGSContext()), RegData );
}

//=========================================================================

inline
void gsreg_SetClamping( xbool S, xbool T )
{
    u64 RegData = SCE_GS_SET_CLAMP( (S)?(1):(0), (T)?(1):(0), 0, 0, 0, 0 );
    gsreg_Set( (SCE_GS_CLAMP_1+eng_GetGSContext()), RegData );
}

//=========================================================================

inline
void gsreg_SetScissor( s32 X0, s32 Y0, s32 X1, s32 Y1 )
{
    u64 RegData = SCE_GS_SET_SCISSOR( X0, X1-1, Y0, Y1-1 );
    gsreg_Set( (SCE_GS_SCISSOR_1+eng_GetGSContext()), RegData );
}

//=========================================================================




