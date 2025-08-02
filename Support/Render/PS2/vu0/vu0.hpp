//=============================================================================

#ifndef _VU0_HPP_
#define _VU0_HPP_

//=============================================================================

#include "Entropy.hpp"

//=============================================================================

void    vu0_Init                ( void );
void    vu0_Kill                ( void );
void    vu0_UploadMicrocode     ( void* pMCode, s32 NumBytes );
void    vu0_ExecuteMicrocode    ( u32 Addr );
void    vu0_StartDMA            ( void* pData );
void    vu0_Sync                ( void );

//=============================================================================

inline void vu0_TransferSync( void )
{
    do
    {
        asm __volatile__ ( "WAIT:" );
        asm __volatile__ ( "nop" );
        asm __volatile__ ( "nop" );
        asm __volatile__ ( "nop" );
        asm __volatile__ ( "nop" );
        asm __volatile__ ( "bc0f WAIT" );
        asm __volatile__ ( "nop" );
    
    } while( (DGET_D_STAT() & D_STAT_CIS0_M) == 0 );
}

//=============================================================================

inline void vu0_ExecuteSync( void )
{
    asm __volatile__ ("
        cfc2.i $8, $vi29
        "
        : : : "$8" );
}

//=============================================================================

inline void vu0_ExecuteMicrocode( u32 Addr )
{
    asm __volatile__ ("
        ctc2 %0, $vi27
        vnop
        vnop
        vcallmsr $vi27
        " : : "r" (Addr) );
}

//=============================================================================

#endif

