//=========================================================================
//
// PS2_VIFGIF_INLINE.HPP
//
//=========================================================================

#ifndef PS2_VIFGIF_INLINE_H
#define PS2_VIFGIF_INLINE_H

//=========================================================================
//=========================================================================
//=========================================================================
// VIF
//=========================================================================
//=========================================================================
//=========================================================================

inline
u32    VIF_Unpack  (  s32    DestVUAddr,
                      s32    NVectors,
                      s32    Format,
                      xbool  Signed,
                      xbool  Masked,
                      xbool  AbsoluteAddress )
{
    u32 VP;
 
    ASSERT(NVectors <= 255) ;   // NVectors is only 8 bits

    VP  = (u32)(DestVUAddr&0x0000FFFF);
    VP |= (u32)((NVectors<<16)&0x00FF0000);
    if( !AbsoluteAddress ) VP |= (1<<15);
    if( Signed == FALSE ) VP |= (1<<14);
    VP |= (((u32)(0x60|Format))<<24);  
    if( Masked == TRUE ) VP |= 0x10000000;      
    return VP;
}

//=========================================================================

inline
u32    VIF_SkipWrite( s32   NVectorsToWrite,
                      s32   NVectorsToSkip )
{
    ASSERT( (NVectorsToWrite>=0) && (NVectorsToSkip>=0) );
    s32 WL = NVectorsToWrite;
    s32 CL = NVectorsToWrite + NVectorsToSkip;
    return (u32)(CL|(WL<<8)) | (1<<24);
}

//=========================================================================

inline
void    VIF_Mask    ( u32* pVifData,
                      s32 X0, s32 Y1, s32 Z0, s32 W0, 
                      s32 X1, s32 Y2, s32 Z1, s32 W1, 
                      s32 X2, s32 Y3, s32 Z2, s32 W2, 
                      s32 X3, s32 Y4, s32 Z3, s32 W3 )
{
    u32 VP0;
    u32 VP1;
    
    VP0 =((u32)0x20 << 24);
    VP1 =(u32)((X0<< 0)|(Y1<< 2)|(Z0<< 4)|(W0<< 6)|
               (X1<< 8)|(Y2<<10)|(Z1<<12)|(W1<<14)|
               (X2<<16)|(Y3<<18)|(Z2<<20)|(W2<<22)|
               (X3<<24)|(Y4<<26)|(Z3<<28)|(W3<<30));

    pVifData[0] = VP0;
    pVifData[1] = VP1;
}
                           
//=========================================================================
//=========================================================================
//=========================================================================
// GIF
//=========================================================================
//=========================================================================
//=========================================================================

inline giftag::giftag      ( void )
{
    ASSERT( (((u32)this)&0x0F) == 0 );
    ((u64*)this)[0] = 0;
    ((u64*)this)[1] = 0;
}

//=========================================================================

inline giftag::~giftag     ( void )
{
}

//=========================================================================

inline void giftag::Build  ( s32   Mode,  
                      s32   NRegs, 
                      s32   NLoops, 
                      xbool UsePrim,
                      s32   PrimType,
                      u32   PrimFlags, 
                      xbool aEOP )
{
    ((u64*)this)[0] = 0;
    ((u64*)this)[1] = 0;
    MODE  = Mode;
    NREG  = NRegs;
    NLOOP = NLoops;
    PRE   = (UsePrim)?(1):(0);
    PRIM  = ((u32)PrimType) | (PrimFlags<<3);    
    EOP   = (aEOP)?(1):(0);
}

//=========================================================================

inline void giftag::BuildImageLoad( s32 NBytes, xbool aEOP )
{
    ASSERT( (NBytes>>4) <= 32767 );
    ((u64*)this)[0] = 0;
    ((u64*)this)[1] = 0;
    PRE   = 0;
    PRIM  = 0;    
    MODE  = GIF_MODE_IMAGE;
    NREG  = 0;
    NLOOP = (NBytes>>4);
    EOP   = (aEOP)?(1):(0);
}

//=========================================================================

inline void giftag::Build2( s32 NRegs, 
                     s32 NLoops,
                     s32 PrimType,
                     u32 PrimFlags )
{
    ((u64*)this)[0] = 0;
    ((u64*)this)[1] = 0;
    EOP   = 1;
    PRE   = 1;
    MODE  = GIF_MODE_PACKED;
    NREG  = NRegs;
    NLOOP = NLoops;
    PRIM  = ((u32)PrimType) | (PrimFlags<<3);    
}

//=========================================================================

inline void giftag::BuildRegLoad( s32 NRegs, xbool aEOP )
{
    ((u64*)this)[0] = 0;
    ((u64*)this)[1] = 0;
    MODE  = 0;
    PRE   = 0;
    PRIM  = 0;
    NREG  = 1;
    NLOOP = NRegs;
    EOP   = (aEOP)?(1):(0);
    R00   = GIF_REG_AD;
}

//=========================================================================

#define SR00 R00=aR00;
#define SR01 R01=aR01;
#define SR02 R02=aR02;
#define SR03 R03=aR03;
#define SR04 R04=aR04;
#define SR05 R05=aR05;
#define SR06 R06=aR06;
#define SR07 R07=aR07;
#define SR08 R08=aR08;
#define SR09 R09=aR09;
#define SR10 R10=aR10;
#define SR11 R11=aR11;
#define SR12 R12=aR12;
#define SR13 R13=aR13;
#define SR14 R14=aR14;
#define SR15 R15=aR15;
#define SETREGS { SR00 SR01 SR02 SR03 SR04 SR05 SR06 SR07 SR08 SR09 SR10 SR11 SR12 SR13 SR14 SR15 }

inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04, u8 aR05, u8 aR06, u8 aR07, u8 aR08, u8 aR09, u8 aR10, u8 aR11, u8 aR12, u8 aR13, u8 aR14, u8 aR15 ) SETREGS
#undef  SR15 
#define SR15
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04, u8 aR05, u8 aR06, u8 aR07, u8 aR08, u8 aR09, u8 aR10, u8 aR11, u8 aR12, u8 aR13, u8 aR14 ) SETREGS
#undef  SR14 
#define SR14
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04, u8 aR05, u8 aR06, u8 aR07, u8 aR08, u8 aR09, u8 aR10, u8 aR11, u8 aR12, u8 aR13 ) SETREGS
#undef  SR13 
#define SR13
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04, u8 aR05, u8 aR06, u8 aR07, u8 aR08, u8 aR09, u8 aR10, u8 aR11, u8 aR12 )SETREGS
#undef  SR12 
#define SR12
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04, u8 aR05, u8 aR06, u8 aR07, u8 aR08, u8 aR09, u8 aR10, u8 aR11 )SETREGS
#undef  SR11 
#define SR11
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04, u8 aR05, u8 aR06, u8 aR07, u8 aR08, u8 aR09, u8 aR10 )SETREGS
#undef  SR10 
#define SR10
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04, u8 aR05, u8 aR06, u8 aR07, u8 aR08, u8 aR09 )SETREGS
#undef  SR09 
#define SR09
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04, u8 aR05, u8 aR06, u8 aR07, u8 aR08 )SETREGS
#undef  SR08 
#define SR08
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04, u8 aR05, u8 aR06, u8 aR07 )SETREGS
#undef  SR07 
#define SR07
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04, u8 aR05, u8 aR06 )SETREGS
#undef  SR06 
#define SR06
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04, u8 aR05 )SETREGS
#undef  SR05 
#define SR05
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03, u8 aR04 )SETREGS
#undef  SR04 
#define SR04
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02, u8 aR03 )SETREGS
#undef  SR03 
#define SR03
inline void giftag::Reg    ( u8 aR00, u8 aR01, u8 aR02 )SETREGS
#undef  SR02 
#define SR02
inline void giftag::Reg    ( u8 aR00, u8 aR01 )SETREGS
#undef  SR01 
#define SR01
inline void giftag::Reg    ( u8 aR00 )SETREGS
#undef  SR00 
#define SR00


//=========================================================================
#endif
//=========================================================================

