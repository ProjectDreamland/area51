//=============================================================================
//
//  Polygon Renderer
//
//=============================================================================

#include "Entropy.hpp"
#include "Poly.hpp"
#include "mcode\memlayout.vu"

#ifdef TARGET_PS2

//=============================================================================
//  Types
//=============================================================================

struct boot_dma
{
    struct vu_plane
    {
        u32 Mask;
        u32 JumpAddr;
        u32 Unused[2];
    };

    struct vif_data
    {
        u32      Data[4];       // any data to pass to VU
        giftag   GIFTagTri;     // GIF tag for rendering triangles
        giftag   GIFTagStr;     // GIF tag for rendering strips
        matrix4  W2C;           // world-to-clip matrix
        matrix4  C2S;           // clip-to-screen matrix
        vu_plane VUPlane[7];    // clipping planes
    };

    dmatag   DMA;               // DMA packet info
    vif_data VIF_Data;          // data to transfer to VU memory
    u32      VIF_Boot[12];      // VIF commands
};

//=============================================================================
//  Variables
//=============================================================================

extern u32 VU_POLY_CODE_START __attribute__((section(".vudata")));
extern u32 VU_POLY_CODE_END   __attribute__((section(".vudata")));

static s32 s_MCodeHandle = -1;
static s32 s_StartVU     = 0;
static s32 s_InBegin     = FALSE;
static f32 s_ZBias       = 0;
static f32 s_ZBiasScalar = 64.0f;

s32 poly_db=0;      // turn VU double buffering on/off
s32 poly_stop=0;    // stop the VU in the debugger

//=============================================================================

void poly_Initialize( void )
{
    // register the microcode
    
    if( s_MCodeHandle == -1 )
    {
        u32 Start = (u32)( &VU_POLY_CODE_START );
        u32 End   = (u32)( &VU_POLY_CODE_END   );
        
        s_MCodeHandle = eng_RegisterMicrocode( "POLY", &VU_POLY_CODE_START, End - Start );
    }
}

//=============================================================================
    
void poly_Begin( u32 Flags )
{
    ASSERT( s_InBegin == FALSE );

    vram_Sync();
    
    s_InBegin = TRUE;
    s_StartVU = ( Flags & POLY_DRAW_SPRITES ) ? 16 : 2;
    
    s32 Alpha = ( Flags & POLY_USE_ALPHA ) ? GIF_FLAG_ALPHA : 0;
    s32 Texture = ( Flags & POLY_NO_TEXTURE ) ? 0 : GIF_FLAG_TEXTURE;

    //
    // activate the microcode
    //
    
    if( eng_GetActiveMicrocode() != s_MCodeHandle )
    {
        eng_ActivateMicrocode( s_MCodeHandle );
    }

    boot_dma* pPack = DLStruct( boot_dma );

    //
    // copy GIF Tag and matrices to VU
    //

    x_memset( &pPack->VIF_Data, 0, sizeof( pPack->VIF_Data ));
    
    pPack->DMA.SetCont( sizeof( boot_dma ) - sizeof( dmatag ));
    pPack->DMA.PAD[0] = VIF_SkipWrite( 1, 0 );
    pPack->DMA.PAD[1] = VIF_Unpack( VU_BASE_MEMORY, sizeof( boot_dma::vif_data ) / 16, VIF_V4_32, FALSE, FALSE, TRUE );

    pPack->VIF_Data.GIFTagTri.Build2( 3, 0, GIF_PRIM_TRIANGLE,      GIF_FLAG_SMOOTHSHADE | Texture | Alpha );
    pPack->VIF_Data.GIFTagStr.Build2( 3, 0, GIF_PRIM_TRIANGLESTRIP, GIF_FLAG_SMOOTHSHADE | Texture | Alpha );

    pPack->VIF_Data.GIFTagTri.Reg( GIF_REG_ST, GIF_REG_RGBAQ, GIF_REG_XYZ2 );
    pPack->VIF_Data.GIFTagStr.Reg( GIF_REG_ST, GIF_REG_RGBAQ, GIF_REG_XYZ2 );

    const view& View = *eng_GetActiveView( 0 );
    
    f32 NearZ, FarZ, ProjX0, ProjY0, ProjX1, ProjY1, Mult;
    
    View.GetZLimits( NearZ, FarZ );
    View.GetProjection( ProjX0, ProjX1, ProjY0, ProjY1 );

    Mult = ProjX1 * 0.5f;                           // multiplier for sprite width and height

    pPack->VIF_Data.W2C = View.GetW2C();
    pPack->VIF_Data.C2S = View.GetC2S();

    pPack->VIF_Data.Data[0] = *(u32*)&poly_stop;    // pass stop flag to VU program
    pPack->VIF_Data.Data[1] = *(u32*)&Mult;         // pass the screen depth
    pPack->VIF_Data.Data[2] = *(u32*)&s_ZBias;      // pass ZBias to VU (MUST be in Z component of vector)
    pPack->VIF_Data.Data[3] = *(u32*)&NearZ;        // pass the near Z value used for rejecting sprites

    //
    // setup clipping planes
    //
    
    boot_dma::vu_plane* pPlane = pPack->VIF_Data.VUPlane;

    pPlane[0].Mask     = CLIP_FLAG_NEG_Z;           // Front
    pPlane[0].JumpAddr = 14;                        
                                                    
    pPlane[1].Mask     = CLIP_FLAG_POS_Z;           // Back
    pPlane[1].JumpAddr = 8;                         
                                                    
    pPlane[2].Mask     = CLIP_FLAG_POS_X;           // Right
    pPlane[2].JumpAddr = 4;                         
                                                    
    pPlane[3].Mask     = CLIP_FLAG_NEG_X;           // Left
    pPlane[3].JumpAddr = 10;                        
                                                    
    pPlane[4].Mask     = CLIP_FLAG_POS_Y;           // Bottom
    pPlane[4].JumpAddr = 6;                         
                                                    
    pPlane[5].Mask     = CLIP_FLAG_NEG_Y;           // Top
    pPlane[5].JumpAddr = 12;                        
                                                    
    pPlane[6].Mask     = 0;                         // Terminate
    pPlane[6].JumpAddr = 0;
    
    //
    // setup the VU
    //

    u32 Mask[2];
    f32 Row = 1.0f;
    
    VIF_Mask( Mask, VIF_ASIS, VIF_ASIS, VIF_ROW,  VIF_ROW,
                    VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS,
                    VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS,
                    VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS );

    pPack->VIF_Boot[ 0] = SCE_VIF1_SET_FLUSH( 0 );                  // stall until previous transfer complete
    pPack->VIF_Boot[ 1] = VIF_SkipWrite( 1, 2 );                    // write 1 then skip 2 vectors (3 vectors per vertex)
    pPack->VIF_Boot[ 2] = SCE_VIF1_SET_BASE  ( 0, 0 );              // setup double buffering
    pPack->VIF_Boot[ 3] = SCE_VIF1_SET_OFFSET( VU_BUFFER_SIZE, 0 );
    pPack->VIF_Boot[ 4] = SCE_VIF1_SET_MSCAL( 0, 0 );               // execute initialize routine
    pPack->VIF_Boot[ 5] = Mask[0];                                  // set VIF mask              
    pPack->VIF_Boot[ 6] = Mask[1];                                                               
    pPack->VIF_Boot[ 7] = SCE_VIF1_SET_STROW( 0 );                  // set ROW register with [1.0  1.0  1.0  1.0]
    pPack->VIF_Boot[ 8] = *((u32*)&Row);
    pPack->VIF_Boot[ 9] = *((u32*)&Row);
    pPack->VIF_Boot[10] = *((u32*)&Row);
    pPack->VIF_Boot[11] = *((u32*)&Row);
    
    if( !poly_db ) pPack->VIF_Boot[3] = SCE_VIF1_SET_OFFSET( 0, 0 ); 
}

//=============================================================================

void poly_End( void )
{
    ASSERT( s_InBegin == TRUE );
    
    s_InBegin = FALSE;

    dmatag* pPack = DLStruct( dmatag );
    
    pPack->SetCont( 0 );
    pPack->PAD[0] = VIF_SkipWrite( 1, 0 );
}

//=============================================================================

volatile xbool POLY_WIREFRAME = FALSE;

void poly_Render( vector3* pVerts, vector2* pTex, ps2color* pColor, s32 nVerts )
{
    ASSERT(( nVerts % 3 ) == 0 );       // make sure we get complete triangles

    ASSERT( ALIGN_16( (s32)pVerts ) == (s32)pVerts );
    ASSERT( ALIGN_16( (s32)pTex   ) == (s32)pTex   );
    ASSERT( ALIGN_16( (s32)pColor ) == (s32)pColor );

    if( POLY_WIREFRAME == TRUE )
    {
        draw_Begin( DRAW_LINES );
        draw_Color( XCOLOR_RED );

        for( s32 i=0; i<nVerts; i += 3 )
        {
            draw_Vertex( pVerts[i+0] );
            draw_Vertex( pVerts[i+1] );
            draw_Vertex( pVerts[i+1] );
            draw_Vertex( pVerts[i+2] );
            draw_Vertex( pVerts[i+2] );
            draw_Vertex( pVerts[i+0] );
        }
        
        draw_End();
        return;
    }

    //
    // upload the data to VU memory in chunks
    //

    for( s32 n=0; n<nVerts; n += VU_MAX_VERTS )
    {
        // calculate the number of verts to upload in this chunk
    
        s32 NumVerts = ( nVerts - n ) < VU_MAX_VERTS ? nVerts - n : VU_MAX_VERTS;
        s32 VIFVerts = ALIGN_4( NumVerts );

        //
        // setup DMA packet to upload vertex count
        //
        
        dmatag* pPack = DLStruct( dmatag );

        pPack->SetCont( 0 );
        pPack->PAD[0] = VIF_Unpack( 0, 1, VIF_S_32, TRUE, FALSE, FALSE );
        pPack->PAD[1] = NumVerts;

        //
        // setup DMA packet to upload texture coordinates
        //
        
        pPack = DLStruct( dmatag );

        s32 NumBytes = VIFVerts * sizeof( vector2 );

        pPack->SetRef( NumBytes, (u32)pTex );
        pPack->PAD[1] = VIF_Unpack( 1, VIFVerts, VIF_V2_32, TRUE, TRUE, FALSE );

        //
        // setup DMA packet to upload colours
        //
        
        pPack = DLStruct( dmatag );

        NumBytes = VIFVerts * sizeof( ps2color );

        pPack->SetRef( NumBytes , (u32)pColor );
        pPack->PAD[1] = VIF_Unpack( 2, VIFVerts, VIF_V4_8, FALSE, FALSE, FALSE );

        //
        // setup DMA packet to upload world vertices
        //
        
        pPack = DLStruct( dmatag );

        NumBytes = VIFVerts * sizeof( vector3 );

        pPack->SetRef( NumBytes, (u32)pVerts );
        pPack->PAD[1] = VIF_Unpack( 3, VIFVerts, VIF_V3_32, TRUE, FALSE, FALSE );
        
        //
        // setup DMA packet to start VU
        //

        pPack = DLStruct( dmatag );
        
        pPack->SetCont( 0 );
        pPack->PAD[0] = SCE_VIF1_SET_FLUSH( 0 );
        pPack->PAD[1] = SCE_VIF1_SET_MSCAL( s_StartVU, 0 );

        //
        // turn double buffering on/off
        //

        if( !poly_db )
        {
            pPack = DLStruct( dmatag );
        
            pPack->SetCont( 0 );
            pPack->PAD[0] = SCE_VIF1_SET_FLUSH( 0 );
        }

        //
        // get next chunk
        //

        pVerts += VIFVerts;
        pTex   += VIFVerts;
        pColor += VIFVerts;
    }

    //
    // setup packet to wait until VU is finished
    //
    
    dmatag* pPack = DLStruct( dmatag );
    
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_FLUSH( 0 );
}

//=============================================================================

void poly_SetZBias( s32 Bias )
{
    s_ZBias = Bias * s_ZBiasScalar;
}

//=============================================================================

#else

void poly_Initialize( void )
{
}

void poly_Begin( u32 Flags )
{
    (void)Flags;
}

void poly_End( void )
{
}

void poly_Render( vector3* pVerts, vector2* pTex, ps2color* pColor, s32 nVerts )
{
    (void)pVerts;
    (void)pTex;
    (void)pColor;
    (void)nVerts;
}

void poly_SetZBias( s32 Bias )
{
    (void)Bias;
}

#endif
