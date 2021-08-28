//=========================================================================
//
// ps2_dma.hpp
//
//=========================================================================

#ifndef PS2_DMA_HPP
#define PS2_DMA_HPP

#include "x_types.hpp"
#include "x_debug.hpp"

//=========================================================================

struct dmatag
{
    // SONY FORMAT
    u16     QWC;
    u8      MARK;
    u8      ID;
    dmatag* NEXT;
    u32     PAD[2];

    dmatag  ( void );
    ~dmatag ( void );

    // These build the different dma types
    void SetCont ( s32 NBytes );
    void SetEnd  ( s32 NBytes );
    void SetRef  ( s32 NBytes, u32 Address );
    void SetCall ( s32 NBytes, u32 Address );
    void SetRet  ( s32 NBytes );

    // Adds VIF1 DIRECT cmd into dma's PAD[1]
    void MakeDirect( s32 NBytes=-1 );

} PS2_ALIGNMENT(16);

//=========================================================================
//=========================================================================
//=========================================================================
// INLINE FUNCTIONS
//=========================================================================
//=========================================================================
//=========================================================================

#if 1
#define CLEAR_DMATAG         ((u64*)this)[0] = 0; ((u64*)this)[1] = 0;
#define CONFIRM_ALIGNMENT(x) ASSERT((((u32)(x))&0x0F)==0);
#else
#define CLEAR_DMATAG         
#define CONFIRM_ALIGNMENT(x) 
#endif

//=========================================================================
inline 
dmatag::dmatag  ( void )
{
    CONFIRM_ALIGNMENT(this);
    CLEAR_DMATAG;
}

//=========================================================================
inline 
dmatag::~dmatag ( void )
{
    CONFIRM_ALIGNMENT(this);
    CLEAR_DMATAG;
}

//=========================================================================

inline 
void dmatag::SetCont ( s32 NBytes )
{
    CONFIRM_ALIGNMENT(NBytes);
    QWC     = (u16)(NBytes>>4);
    MARK    = 0;
    ID      = 0x10;
    NEXT    = NULL;
    PAD[0]  = 0;
    PAD[1]  = 0;
}

//=========================================================================
inline 
void dmatag::SetEnd  ( s32 NBytes )
{
    CONFIRM_ALIGNMENT(NBytes);
    QWC     = (u16)(NBytes>>4);
    ID      = 0x70;
    MARK    = 0;
    NEXT    = NULL;
    PAD[0]  = 0;
    PAD[1]  = 0;
}

//=========================================================================
inline 
void dmatag::SetRef  ( s32 NBytes, u32 Address )
{
    CONFIRM_ALIGNMENT(NBytes);
    QWC     = (u16)(NBytes>>4);
    ID      = 0x30;
    NEXT    = (dmatag*)(Address & 0x0fffffff);
    MARK    = 0;
    PAD[0]  = 0;
    PAD[1]  = 0;
}

//=========================================================================
inline 
void dmatag::SetCall ( s32 NBytes, u32 Address )
{
    CONFIRM_ALIGNMENT(NBytes);
    QWC     = (u16)(NBytes>>4);
    ID      = 0x50;
    NEXT    = (dmatag*)Address;
    MARK    = 0;
    PAD[0]  = 0;
    PAD[1]  = 0;
}

//=========================================================================
inline 
void dmatag::SetRet  ( s32 NBytes )
{
    CONFIRM_ALIGNMENT(NBytes);
    QWC     = (u16)(NBytes>>4);
    ID      = 0x60;
    NEXT    = NULL;
    MARK    = 0;
    PAD[0]  = 0;
    PAD[1]  = 0;
}

//=========================================================================
inline 
void dmatag::MakeDirect( s32 NBytes )
{
    if( NBytes==-1 )
        PAD[1] = ((u32)QWC) | (0x50<<24);
    else
    {
        CONFIRM_ALIGNMENT(NBytes);
        PAD[1] = ((u32)(NBytes>>4)) | (0x50<<24);
    }
}

//=========================================================================

#endif
