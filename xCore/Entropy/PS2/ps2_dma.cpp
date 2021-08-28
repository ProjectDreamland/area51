//=========================================================================
//
// ps2_dma.cpp
//
//=========================================================================
#include "ps2_dma.hpp"
#include "x_debug.hpp"
/*
#if 1
#define CLEAR_DMATAG         ((u64*)this)[0] = 0; ((u64*)this)[1] = 0;
#define CONFIRM_ALIGNMENT(x) ASSERT((((u32)(x))&0x0F)==0);
#else
#define CLEAR_DMATAG         
#define CONFIRM_ALIGNMENT(x) 
#endif
*/

/*
//=========================================================================

dmatag::dmatag  ( void )
{
    CONFIRM_ALIGNMENT(this);
    CLEAR_DMATAG;
}

//=========================================================================

dmatag::~dmatag ( void )
{
    CONFIRM_ALIGNMENT(this);
    CLEAR_DMATAG;
}

//=========================================================================

void dmatag::SetCont ( s32 NBytes )
{
    CONFIRM_ALIGNMENT(NBytes);
    CLEAR_DMATAG;
    QWC     = (u16)(NBytes>>4);
    ID      = 0x10;
}

//=========================================================================

void dmatag::SetEnd  ( s32 NBytes )
{
    CONFIRM_ALIGNMENT(NBytes);
    CLEAR_DMATAG;
    QWC     = (u16)(NBytes>>4);
    ID      = 0x70;
}

//=========================================================================

void dmatag::SetRef  ( s32 NBytes, u32 Address )
{
    CONFIRM_ALIGNMENT(NBytes);
    CLEAR_DMATAG;
    QWC     = (u16)(NBytes>>4);
    ID      = 0x30;
    NEXT    = (dmatag*)Address;
}

//=========================================================================

void dmatag::SetCall ( s32 NBytes, u32 Address )
{
    CONFIRM_ALIGNMENT(NBytes);
    CLEAR_DMATAG;
    QWC     = (u16)(NBytes>>4);
    ID      = 0x50;
    NEXT    = (dmatag*)Address;
}

//=========================================================================

void dmatag::SetRet  ( s32 NBytes )
{
    CONFIRM_ALIGNMENT(NBytes);
    CLEAR_DMATAG;
    QWC     = (u16)(NBytes>>4);
    ID      = 0x60;
    NEXT    = NULL;
}

//=========================================================================

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
*/




