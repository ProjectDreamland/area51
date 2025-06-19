//=============================================================================
//
//  vu0.cpp
//
//  VU0 Library by JP
//
//=============================================================================

//
// TODO:
//
//  - Use scratchpad memory (for Output)
//  - Optimize the VU code
//  - Use [E] MSCNT method instead of jumptable
//  - Compute lighting inside VU0
//

#include "Entropy.hpp"
#include "vu0.hpp"

//==============================================================================

static xbool s_TransferStarted = FALSE;
static void* s_pMCode          = NULL;

//==============================================================================

void vu0_Init( void )
{
    s_TransferStarted = FALSE;
    s_pMCode          = NULL;
}

//==============================================================================

void vu0_Kill( void )
{
}

//==============================================================================

void vu0_UploadMicrocode( void* pMCode, s32 NumBytes )
{
    // Ensure that vu0_Sync() is called before starting another transfer
    ASSERT( s_TransferStarted == FALSE );

    // Check if we already have this microcode loaded to VU0 memory
    if( s_pMCode == pMCode )
        return;
    else
        s_pMCode = pMCode;

    // Maximum of only 2 uploads to fill the VU0 instruction memory and
    // 1 extra dmatag for END marker
    dmatag* pBase = (dmatag*)DLAlloc( sizeof( dmatag ) * 3 );
    dmatag* pDMA  = pBase;

    // Ensure microprogram is aligned for DMA
    u32 Size   = ALIGN_16( NumBytes ); 
    u32 Offset = 0;
    
    ASSERT( Size <= 4096 );
    CONFIRM_ALIGNMENT( pMCode );

    // Upload the microprogram in chunks    
    while( Size > 0 )
    {
        u32 NBytes = MIN( Size,  256 * 8 );
        u32 NSize  = ( NBytes == 256 * 8 ) ? 0 : NBytes / 8;
        
        pDMA->SetRef( NBytes, (u32)pMCode + Offset );
        pDMA->PAD[0] = SCE_VIF0_SET_FLUSHE( 0 );
        pDMA->PAD[1] = SCE_VIF0_SET_MPG( Offset / 8, NSize, 0 );
        pDMA++;

        Size   -= NBytes;
        Offset += NBytes;
    }

    // Set end marker for DMA
    pDMA->SetEnd( 0 );
    
    vu0_StartDMA( pBase );
}

//==============================================================================

void vu0_StartDMA( void* pData )
{
    // Ensure that vu0_Sync() is called before starting another transfer
    ASSERT( s_TransferStarted == FALSE );
    CONFIRM_ALIGNMENT( pData );

    s_TransferStarted = TRUE;

    // Set start address of DMA transfer
    DPUT_D0_QWC( 0 );
    DPUT_D0_TADR( (u32)pData | 0x00000000 );
    
    // Output DMA channel status to CPCOND[0]
    DPUT_D_PCR ( D_PCR_CPC0_M  );
    
    // Clear channel interrupt status flag
    DPUT_D_STAT( D_STAT_CIS0_M );
    
    // Setup the DMA channel control register
    u32 CHCR = 0;
    tD_CHCR *pCHCR = (tD_CHCR*)&CHCR;
    
    pCHCR->MOD = 0x1;   // Chain mode
    pCHCR->TTE = 0x1;   // Transfer Tag
    pCHCR->STR = 0x1;   // Start transfer
    
    // Make sure everything is written back to the main memory ready
    FlushCache( WRITEBACK_DCACHE );
    
    // Start the DMA transfer
    asm __volatile__ ( "sync.l" );
    DPUT_D0_CHCR( CHCR );
}

//==============================================================================

void vu0_Sync( void )
{
    // Stall the CPU until any transfers using VIF0 are completed
    if( s_TransferStarted == TRUE )
    {
        vu0_TransferSync();
        s_TransferStarted = FALSE;
    }
}

//==============================================================================

