//=============================================================================
//
//  vu1.cpp
//
//  VU1 Renderer by JP
//
//=============================================================================

#include "Entropy.hpp"
#include "vu1.hpp"
#include "mcode\include1.vu"

//=============================================================================

// Jumptable addresses for MSCAL VIF packets

#define VU1_INIT             0
#define VU1_SETL2S           2
#define VU1_SETL2C           4
#define VU1_SETC2S           6
#define VU1_FAST             8
#define VU1_SLOW            10
#define VU1_FAST_LIT        12
#define VU1_SLOW_LIT        14
#define VU1_BREAK           16
#define VU1_CLIP            18

#define VU1_PLANES_NVECTORS  7

//=============================================================================

struct vu1_plane
{
    u32 Mask;
    u32 JumpAddr;
    u32 Pad0;
    u32 Pad1;
};

struct vu1_init
{
    dmatag    DMA;
    u32       VIF1[ 8 ];
    u32       Data[ 4 ];
    vu1_plane Planes[ VU1_PLANES_NVECTORS ];
    u32       Kick[ 4 ];
};

struct vu1_matrix
{
    dmatag  DMA;
    matrix4 M;
    u32     Kick[ 4 ];
};

struct vu1_begin
{
    dmatag DMA;
    giftag GIF;
};

struct vu1_break
{
    dmatag DMA;
    u32    Kick[ 4 ];
};

//=============================================================================

volatile xbool VU1_STOP = FALSE;

static s32   s_MCodeHandle = -1;
static xbool s_InBegin     = FALSE;

extern u32 VU1_RENDER_CODE_START __attribute__((section(".vudata")));
extern u32 VU1_RENDER_CODE_END   __attribute__((section(".vudata")));

//=============================================================================

void vu1_Init( const matrix4& C2S )
{
    u32 Start = (u32)( &VU1_RENDER_CODE_START );
    u32 End   = (u32)( &VU1_RENDER_CODE_END   );

    if( s_MCodeHandle == -1 )
        s_MCodeHandle = eng_RegisterMicrocode( "VU1Render", &VU1_RENDER_CODE_START, End - Start );

    eng_ActivateMicrocode( s_MCodeHandle );

    vu1_init* pPack = (vu1_init*)DLStruct( vu1_init );
    
    // Setup Double Buffer registers
    pPack->DMA.SetCont( sizeof( vu1_init ) - sizeof( dmatag ) );

    // Setup VIF1 Mask and Row registers
    u32 Mask[2];
    f32 Row = 1.0f;
    
    VIF_Mask( Mask, VIF_ASIS, VIF_ASIS, VIF_ROW,  VIF_ROW,
                    VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS,
                    VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS,
                    VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS );

    pPack->VIF1[0] = Mask[0];
    pPack->VIF1[1] = Mask[1];
    pPack->VIF1[2] = SCE_VIF1_SET_STROW( 0 );
    pPack->VIF1[3] = *((u32*)&Row);
    pPack->VIF1[4] = *((u32*)&Row);
    pPack->VIF1[5] = *((u32*)&Row);
    pPack->VIF1[6] = *((u32*)&Row);
    pPack->VIF1[7] = 0;

    // Setup double buffer registers and no skip write
    pPack->Data[0] = SCE_VIF1_SET_BASE  (   0, 0 );
//    pPack->Data[1] = SCE_VIF1_SET_OFFSET( 512, 0 );
    pPack->Data[1] = SCE_VIF1_SET_OFFSET( 400, 0 );
    pPack->Data[2] = VIF_SkipWrite( 1, 0 );
    pPack->Data[3] = VIF_Unpack( CLIPPER_PLANE_TABLE, VU1_PLANES_NVECTORS, VIF_V4_32, TRUE, FALSE, TRUE );

    // Setup clipping planes
    vu1_plane* pPlane  = pPack->Planes;

    pPlane[0].Mask     = CLIP_FLAG_NEG_Z;           // Front
    pPlane[0].JumpAddr = VU1_CLIP + 10;                        
                                                    
    pPlane[1].Mask     = CLIP_FLAG_POS_Z;           // Back
    pPlane[1].JumpAddr = VU1_CLIP +  4;                         
                                                    
    pPlane[2].Mask     = CLIP_FLAG_POS_X;           // Right
    pPlane[2].JumpAddr = VU1_CLIP +  0;                         
                                                    
    pPlane[3].Mask     = CLIP_FLAG_NEG_X;           // Left
    pPlane[3].JumpAddr = VU1_CLIP +  6;                        
                                                    
    pPlane[4].Mask     = CLIP_FLAG_POS_Y;           // Bottom
    pPlane[4].JumpAddr = VU1_CLIP +  2;
                                                    
    pPlane[5].Mask     = CLIP_FLAG_NEG_Y;           // Top
    pPlane[5].JumpAddr = VU1_CLIP +  8;                        
                                                    
    pPlane[6].Mask     = 0;                         // Terminate
    pPlane[6].JumpAddr = 0;

    // Sync and kick VU1
    pPack->Kick[0] = SCE_VIF1_SET_FLUSH( 0 );
    pPack->Kick[1] = SCE_VIF1_SET_MSCAL( VU1_INIT, 0 );
    pPack->Kick[2] = SCE_VIF1_SET_FLUSH( 0 );
    pPack->Kick[3] = 0;

    // Upload C2S matrix which is fixed each frame
    vu1_SetC2S( C2S );
    
    s_InBegin = FALSE;
}

//=============================================================================

void vu1_SetL2S( const matrix4& L2S )
{
    ASSERT( s_InBegin == TRUE );
    
    vu1_matrix* pPack = (vu1_matrix*)DLStruct( vu1_matrix );
    
    pPack->DMA.SetCont( sizeof( vu1_matrix ) - sizeof( dmatag ) );
    pPack->DMA.PAD[1] = VIF_Unpack( 0, 4, VIF_V4_32, TRUE, FALSE, TRUE );
    pPack->M = L2S;
    
    pPack->Kick[0] = SCE_VIF1_SET_FLUSH( 0 );
    pPack->Kick[1] = SCE_VIF1_SET_MSCAL( VU1_SETL2S, 0 );
    pPack->Kick[2] = SCE_VIF1_SET_FLUSH( 0 ); 
    pPack->Kick[3] = 0;
}

//=============================================================================

void vu1_SetL2C( const matrix4& L2C )
{
    ASSERT( s_InBegin == TRUE );
    
    vu1_matrix* pPack = (vu1_matrix*)DLStruct( vu1_matrix );
    
    pPack->DMA.SetCont( sizeof( vu1_matrix ) - sizeof( dmatag ) );
    pPack->DMA.PAD[1] = VIF_Unpack( 0, 4, VIF_V4_32, TRUE, FALSE, TRUE );
    pPack->M = L2C;
    
    pPack->Kick[0] = SCE_VIF1_SET_FLUSH( 0 );
    pPack->Kick[1] = SCE_VIF1_SET_MSCAL( VU1_SETL2C, 0 );
    pPack->Kick[2] = SCE_VIF1_SET_FLUSH( 0 ); 
    pPack->Kick[3] = 0;
}

//=============================================================================

void vu1_SetC2S( const matrix4& C2S )
{
    vu1_matrix* pPack = (vu1_matrix*)DLStruct( vu1_matrix );
    
    pPack->DMA.SetCont( sizeof( vu1_matrix ) - sizeof( dmatag ) );
    pPack->DMA.PAD[1] = VIF_Unpack( 0, 4, VIF_V4_32, TRUE, FALSE, TRUE );
    pPack->M = C2S;
    
    pPack->Kick[0] = SCE_VIF1_SET_FLUSH( 0 );
    pPack->Kick[1] = SCE_VIF1_SET_MSCAL( VU1_SETC2S, 0 );
    pPack->Kick[2] = SCE_VIF1_SET_FLUSH( 0 ); 
    pPack->Kick[3] = 0;
}

//=============================================================================

void vu1_Begin( u32 Flags )
{
    ASSERT( s_InBegin == FALSE );
    s_InBegin = TRUE;

    u32 Alpha   = (Flags & VU1_USE_ALPHA ) ? GIF_FLAG_ALPHA : 0;
    u32 Texture = (Flags & VU1_NO_TEXTURE) ? 0 : GIF_FLAG_TEXTURE;
    u32 Shading = (Flags & VU1_NO_GOURAUD) ? 0 : GIF_FLAG_SMOOTHSHADE;
    
    eng_ActivateMicrocode( s_MCodeHandle );

    vu1_begin* pPack = (vu1_begin*)DLStruct( vu1_begin );
    
    pPack->DMA.SetCont( sizeof( vu1_begin ) - sizeof( dmatag ) );
    pPack->DMA.PAD[0] = SCE_VIF1_SET_FLUSH( 0 );
    pPack->DMA.PAD[1] = VIF_Unpack( VU1_GIFTAG, 1, VIF_V4_32, FALSE, FALSE, TRUE );

    // Setup GIFTags according to rendering flags
    pPack->GIF.Build2( 3, 0, GIF_PRIM_TRIANGLESTRIP, Shading | Texture | Alpha );
    pPack->GIF.Reg( GIF_REG_ST, GIF_REG_RGBAQ, GIF_REG_XYZ2 );
}

//=============================================================================

void vu1_End( void )
{
    ASSERT( s_InBegin == TRUE );
    s_InBegin = FALSE;

    // Stall until the VU1 is idle
    dmatag* pDMA = (dmatag*)DLStruct( dmatag );
    pDMA->SetCont( 0 );
    pDMA->PAD[0] = SCE_VIF1_SET_FLUSH( 0 );
}

//=============================================================================

void vu1_Break( void )
{
    vu1_break* pPack = (vu1_break*)DLStruct( vu1_break );
    pPack->DMA.SetCont( sizeof( vu1_break ) - sizeof( dmatag ) );
    pPack->Kick[0] = SCE_VIF1_SET_FLUSH( 0 );
    pPack->Kick[1] = SCE_VIF1_SET_MSCAL( VU1_BREAK, 0 );
    pPack->Kick[2] = SCE_VIF1_SET_FLUSH( 0 );
    pPack->Kick[3] = 0;
}

//=============================================================================

void vu1_Render( const vector4* pVert,
                 const s16*     pUV,
                 const xcolor*  pCol,
                 s32            NumVerts,
                 xbool          DoClipping )
{
    ASSERT( s_InBegin ==  TRUE );
    ASSERT( NumVerts  >=     3 );
    ASSERT( NumVerts  <  32767 );
    
    s32 Size   = NumVerts;
    s32 Offset = 0;
    s32 Jump   = (DoClipping == TRUE) ? VU1_SLOW : VU1_FAST;

    // Upload data to VU1 memory in chunks
    while( Size > 0 )
    {
        dmatag* pPack;
        s32     NBytes;
        s32     NVectors;
        s32     Num  = MIN( Size, VU1_MAX_VERTS );
        s32     Addr = 0;

        pPack = (dmatag*)DLStruct( dmatag );
        pPack->SetCont( 0 );
        pPack->PAD[0] = VIF_Unpack( Addr, 1, VIF_S_32, TRUE, FALSE, FALSE );
        pPack->PAD[1] = Num | (VU1_STOP ? 0x8000 : 0);
        Addr += 1;

        // Upload vertices
        pPack    = (dmatag*)DLStruct( dmatag );
        NBytes   = ALIGN_16( sizeof( vector4 ) * Num );
        NVectors = NBytes / 16;
        pPack->SetRef( NBytes, (u32)(pVert + Offset) );
        pPack->PAD[1] = VIF_Unpack( Addr, NVectors, VIF_V4_32,  TRUE, FALSE, FALSE );
        Addr += Num;
        
        // Upload texture Coordinates
        pPack    = (dmatag*)DLStruct( dmatag );
        NBytes   = ALIGN_16( (sizeof( s16 ) * 2) * Num );
        NVectors = NBytes  / (sizeof( s16 ) * 2);
        pPack->SetRef( NBytes, (u32)(pUV + (Offset * 2)) );
        pPack->PAD[1] = VIF_Unpack( Addr, NVectors, VIF_V2_16,  TRUE,  TRUE, FALSE );
        Addr += Num;
        
        // Upload colors
        pPack    = (dmatag*)DLStruct( dmatag );
        NBytes   = ALIGN_16( sizeof( xcolor ) * Num );
        NVectors = NBytes  / sizeof( xcolor );
        pPack->SetRef( NBytes, (u32)(pCol + Offset) );
        pPack->PAD[1] = VIF_Unpack( Addr, NVectors, VIF_V4_8,  FALSE, FALSE, FALSE );
        Addr += Num;
        
        // Kick VU1
        pPack = (dmatag*)DLStruct( dmatag );
        pPack->SetCont( 0 );
        //pPack->PAD[0] = SCE_VIF1_SET_FLUSH( 0 );
        pPack->PAD[1] = SCE_VIF1_SET_MSCAL( Jump, 0 );

        Size   -= Num;
        Offset += Num;
    }
}

//=============================================================================

void vu1_Render( const vector4* pVert,
                 const s16*     pUV,
                 const u16*     pCol,
                 s32            NumVerts,
                 xbool          DoClipping )
{
    ASSERT( s_InBegin ==  TRUE );
    ASSERT( NumVerts  >=     3 );
    ASSERT( NumVerts  <  32767 );
    
    s32 Size   = NumVerts;
    s32 Offset = 0;
    s32 Jump   = (DoClipping == TRUE) ? VU1_SLOW : VU1_FAST;

    // Upload data to VU1 memory in chunks
    while( Size > 0 )
    {
        dmatag* pPack;
        s32     NBytes;
        s32     NVectors;
        s32     Num  = MIN( Size, VU1_MAX_VERTS );
        s32     Addr = 0;

        pPack = (dmatag*)DLStruct( dmatag );
        pPack->SetCont( 0 );
        pPack->PAD[0] = VIF_Unpack( Addr, 1, VIF_S_32, TRUE, FALSE, FALSE );
        pPack->PAD[1] = Num | (VU1_STOP ? 0x8000 : 0);
        Addr += 1;

        // Upload vertices
        pPack    = (dmatag*)DLStruct( dmatag );
        NBytes   = ALIGN_16( sizeof( vector4 ) * Num );
        NVectors = NBytes / 16;
        pPack->SetRef( NBytes, (u32)(pVert + Offset) );
        pPack->PAD[1] = VIF_Unpack( Addr, NVectors, VIF_V4_32,  TRUE, FALSE, FALSE );
        Addr += Num;
        
        // Upload texture Coordinates
        pPack    = (dmatag*)DLStruct( dmatag );
        NBytes   = ALIGN_16( (sizeof( s16 ) * 2) * Num );
        NVectors = NBytes  / (sizeof( s16 ) * 2);
        pPack->SetRef( NBytes, (u32)(pUV + (Offset * 2)) );
        pPack->PAD[1] = VIF_Unpack( Addr, NVectors, VIF_V2_16,  TRUE,  TRUE, FALSE );
        Addr += Num;
        
        // Upload colors
        pPack    = (dmatag*)DLStruct( dmatag );
        NBytes   = ALIGN_16( (sizeof( u16 ) * 2) * Num );
        NVectors = NBytes  / (sizeof( u16 ) * 2);
        pPack->SetRef( NBytes, (u32)(pCol + (Offset * 2)) );
        pPack->PAD[1] = VIF_Unpack( Addr, NVectors, VIF_V4_5,  FALSE, FALSE, FALSE );
        Addr += Num;
        
        // Kick VU1
        pPack = (dmatag*)DLStruct( dmatag );
        pPack->SetCont( 0 );
        //pPack->PAD[0] = SCE_VIF1_SET_FLUSH( 0 );
        pPack->PAD[1] = SCE_VIF1_SET_MSCAL( Jump, 0 );

        Size   -= Num;
        Offset += Num;
    }
}

//=============================================================================

// TODO: use compressed normals?
// Note: for 8-8-8-bit normals - we need an extra 5 vectors after the normal table.
// This is due to the ALIGN_16 in the DMA packet

void vu1_Render( const vector4* pVert,
                 const s16*     pUV,
                 const vector3* pNorm,
                 s32            NumVerts,
                 xbool          DoClipping )
{
    (void)pVert;
    (void)pUV;
    (void)pNorm;
    (void)NumVerts;
    (void)DoClipping;

/*    
    ASSERT( s_InBegin == TRUE );
    
    s32 Size   = NumVerts;
    s32 Offset = 0;
    s32 Jump   = (DoClipping == TRUE) ? VU1_SLOW : VU1_FAST;

    // Upload data to VU1 memory in chunks
    while( Size > 0 )
    {
        dmatag* pPack;
        s32     NBytes;
        s32     NVectors;
        s32     Num  = MIN( Size, VU1_MAX_VERTS );
        s32     Addr = 0;

        pPack = (dmatag*)DLStruct( dmatag );
        pPack->SetCont( 0 );
        pPack->PAD[0] = VIF_Unpack( Addr, 1, VIF_S_32, TRUE, FALSE, FALSE );
        pPack->PAD[1] = Num;
        Addr += 1;

        // Upload vertices
        pPack    = (dmatag*)DLStruct( dmatag );
        NBytes   = ALIGN_16( sizeof( vector4 ) * Num );
        NVectors = NBytes / 16;
        pPack->SetRef( NBytes, (u32)(pVert + Offset) );
        pPack->PAD[1] = VIF_Unpack( Addr, NVectors, VIF_V4_32,  TRUE, FALSE, FALSE );
        Addr += Num;
        
        // Upload texture Coordinates
        pPack    = (dmatag*)DLStruct( dmatag );
        NBytes   = ALIGN_16( (sizeof( s16 ) * 2) * Num );
        NVectors = NBytes  / (sizeof( s16 ) * 2);
        pPack->SetRef( NBytes, (u32)(pUV + (Offset * 2)) );
        pPack->PAD[1] = VIF_Unpack( Addr, NVectors, VIF_V2_16,  TRUE,  TRUE, FALSE );
        Addr += Num;
        
        // Upload normals
        pPack    = (dmatag*)DLStruct( dmatag );
        NBytes   = ALIGN_16( sizeof( vector3 ) * Num );
        NVectors = NBytes  / sizeof( vector3 );
        pPack->SetRef( NBytes, (u32)(pNorm + Offset) );
        pPack->PAD[1] = VIF_Unpack( Addr, NVectors, VIF_V3_32,  TRUE, FALSE, FALSE );
        Addr += Num;
        
        // Kick VU1
        pPack = (dmatag*)DLStruct( dmatag );
        pPack->SetCont( 0 );
        //pPack->PAD[0] = SCE_VIF1_SET_FLUSH( 0 );
        pPack->PAD[1] = SCE_VIF1_SET_MSCAL( Jump, 0 );

        Size   -= Num;
        Offset += Num;
    }
*/
}

//=============================================================================

