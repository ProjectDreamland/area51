//==============================================================================
//
//  skin.cpp
//
//  VU0 Skinning by JP
//
//==============================================================================

#include "Entropy.hpp"
#include "Skin.hpp"
#include "vu0.hpp"

//==============================================================================

struct bone_data
{
    dmatag    Init;
    dmatag    Quat;
    dmatag    Pos;
    dmatag    END;
    
} PS2_ALIGNMENT;

//==============================================================================

extern u32 VU0_SKINNING_CODE_START __attribute__((section(".vudata")));
extern u32 VU0_SKINNING_CODE_END   __attribute__((section(".vudata")));

//==============================================================================

void skin_Init( void )
{
    // Upload microcode
    u32 Start = (u32)( &VU0_SKINNING_CODE_START );
    u32 End   = (u32)( &VU0_SKINNING_CODE_END   );
        
    vu0_Sync();
    vu0_UploadMicrocode( &VU0_SKINNING_CODE_START, End - Start );
}

//==============================================================================

void skin_UploadBones( const quaternion* pQuat, const vector3* pVert, s32 NumVectors )
{
    // Calculate address to upload the bone data to
    // Note: vector4 is used to keep everything 16 byte aligned
    s32 Addr = (4096 - ((sizeof( quaternion ) + sizeof( vector4 )) * NumVectors)) / 16;

    bone_data* pPack = DLStruct( bone_data );

    pPack->Init.SetCont( 0 );
    pPack->Init.PAD[0] = VIF_SkipWrite( 1, 1 );

    pPack->Quat.SetRef( NumVectors * sizeof( quaternion ), (u32)pQuat );
    pPack->Quat.PAD[1] = VIF_Unpack( Addr, NumVectors, VIF_V4_32, TRUE, TRUE, TRUE );

    pPack->Pos.SetRef( NumVectors * sizeof( vector3 ), (u32)pVert );
    pPack->Pos.PAD[1] = VIF_Unpack( Addr + 1, NumVectors, VIF_V3_32, TRUE, TRUE, TRUE );

    pPack->END.SetEnd( 0 );
    
    vu0_Sync();
    vu0_StartDMA( pPack );
}

//==============================================================================

void skin_UploadMatrices( const matrix4* pMatrix, s32 NumMatrices )
{
    dmatag DMA[2];
    
    DMA[0].SetRef( NumMatrices * sizeof( matrix4 ), (u32)pMatrix );
    DMA[0].PAD[0] = VIF_SkipWrite( 1, 0 );
    DMA[0].PAD[1] = VIF_Unpack( 0, NumMatrices * 4, VIF_V4_32, TRUE, TRUE, TRUE );
    DMA[1].SetEnd( 0 );
    
    vu0_Sync();
    vu0_StartDMA( &DMA );
}

//==============================================================================

