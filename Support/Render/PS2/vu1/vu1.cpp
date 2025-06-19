//=============================================================================
//
//  VU1 Renderer
//
//=============================================================================

#include "Entropy.hpp"
#include "vu1.hpp"
#include "../../render.hpp"
#include "Entropy/PS2/ps2_spad.hpp"

//=============================================================================
//  DEFINES
//=============================================================================

// fit into scratchpad (16k)
#define MAX_NONCLIPPED_INSTANCES    (s32)((6*1024)/sizeof(vu1_instance))
#define MAX_CLIPPED_INSTANCES       (s32)((2*1024)/sizeof(vu1_instance))
#define NONCLIPPED_SPAD_START       ((u32)SPAD.GetUsableStartAddr())
#define CLIPPED_SPAD_START          ((u32)SPAD.GetUsableStartAddr() + 0x1800)

//=============================================================================
//  STATICS
//=============================================================================

static  u16 s_UnlitColor[ VU1_VERTS ] PS2_ALIGNMENT( 16 ) =
{
    0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108,
    0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108,
    0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108,
    0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108,
    0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108,
    0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108,
    0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108,
    0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108,
    0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108,
    0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108, 0xA108
};

// NOTE ABOUT THESE CONSTANTS:
// They are simply the coefficients for doing a Taylor expansion, laid out in a
// nice form for the vector units.

// THIS IS TODO: Since we're only doing the taylor expansion
// to the 7th degree for sin, and 6th degree for cosine, we need to make sure
// it is accurate as possible, so we'll take advantage of the minimax polynomial
// and also represent the coefficient as numbers that can be exactly
// representable in IEEE format. (Remember there is not an exact representation
// for 1/6) For a good explanation, check out Robin Green's "Faster Math Functions"
// presentations.

#define SINCOS_COEFF_A  1.0f
#define SINCOS_COEFF_B  (1.0f/2.0f)
#define SINCOS_COEFF_C  (1.0f/24.0f)
#define SINCOS_COEFF_D  (1.0f/720.0f)
#define SINCOS_COEFF_E  1.0f
#define SINCOS_COEFF_F  (1.0f/6.0f)
#define SINCOS_COEFF_G  (1.0f/120.0f)
#define SINCOS_COEFF_H  (1.0f/5040.0f)

static f32 s_SinCosCoeff[16] =
{
     SINCOS_COEFF_A, -SINCOS_COEFF_A,  SINCOS_COEFF_E, -SINCOS_COEFF_E,
    -SINCOS_COEFF_B,  SINCOS_COEFF_B, -SINCOS_COEFF_F,  SINCOS_COEFF_F,
     SINCOS_COEFF_C, -SINCOS_COEFF_C,  SINCOS_COEFF_G, -SINCOS_COEFF_G,
    -SINCOS_COEFF_D,  SINCOS_COEFF_D, -SINCOS_COEFF_H,  SINCOS_COEFF_H
};

vu1_interface g_VU1;

//=============================================================================
//  EXTERNALS
//=============================================================================

// microcode ranges
extern u32 VU1_SYNC_CODE_START                  __attribute__((section(".vudata")));
extern u32 VU1_SYNC_CODE_END                    __attribute__((section(".vudata")));
extern u32 VU1_INIT_CODE_START                  __attribute__((section(".vudata")));
extern u32 VU1_INIT_CODE_END                    __attribute__((section(".vudata")));
extern u32 VU1_CLIPPER_CODE_START               __attribute__((section(".vudata")));
extern u32 VU1_CLIPPER_CODE_END                 __attribute__((section(".vudata")));
extern u32 VU1_MATERIAL_CODE_START              __attribute__((section(".vudata")));
extern u32 VU1_MATERIAL_CODE_END                __attribute__((section(".vudata")));
extern u32 VU1_SHADRECEIVE_PASSES_CODE_START    __attribute__((section(".vudata")));
extern u32 VU1_SHADRECEIVE_PASSES_CODE_END      __attribute__((section(".vudata")));
extern u32 VU1_SKIN_XFORM_CODE_START            __attribute__((section(".vudata")));
extern u32 VU1_SKIN_XFORM_CODE_END              __attribute__((section(".vudata")));
extern u32 VU1_RIGID_XFORM_CODE_START           __attribute__((section(".vudata")));
extern u32 VU1_RIGID_XFORM_CODE_END             __attribute__((section(".vudata")));
extern u32 VU1_SHADCAST_XFORM_CODE_START        __attribute__((section(".vudata")));
extern u32 VU1_SHADCAST_XFORM_CODE_END          __attribute__((section(".vudata")));
extern u32 VU1_SHADRECEIVE_XFORM_CODE_START     __attribute__((section(".vudata")));
extern u32 VU1_SHADRECEIVE_XFORM_CODE_END       __attribute__((section(".vudata")));
extern u32 VU1_SPRITE_XFORM_CODE_START          __attribute__((section(".vudata")));
extern u32 VU1_SPRITE_XFORM_CODE_END            __attribute__((section(".vudata")));

//=============================================================================
//  FUNCTIONS
//=============================================================================

vu1_interface::vu1_interface( void ) :
    m_ActiveSyncMCode       ( -1 ),
    m_ActiveInitMCode       ( -1 ),
    m_ActiveClipperMCode    ( -1 ),
    m_ActiveMaterialMCode   ( -1 ),
    m_ActiveXFormMCode      ( -1 ),
    m_VUBuffer              ( 0 ),
    m_VUSkinBuffer          ( 0 ),
    m_NextShadowEntry       ( 0 ),
    m_FilterColor           ( 128, 128, 128, 128 )
#ifdef X_ASSERT
   ,m_InBegin       ( 0 ),
    m_InShadowBegin ( 0 ),
    m_InBeginInst   ( 0 ),
    m_InBeginSkin   ( 0 )
#endif
{
    m_IdentityL2W.Identity();
    m_VUBuffers[0] = VU1_BUFFER_0;
    m_VUBuffers[1] = VU1_BUFFER_1;
    m_VUBuffers[2] = VU1_BUFFER_2;
    m_VUSkinBuffers[0] = VU1_SKIN_BUFFER_0;
    m_VUSkinBuffers[1] = VU1_SKIN_BUFFER_1;
    m_VUSkinBuffers[2] = VU1_SKIN_BACKUP_BUFFER;
}

//=============================================================================

void vu1_interface::LoadMCode( u32 MainMemAddr, u32 VUAddr, u32 Size )
{
    ASSERT( Size < 16*1024 );

    // we can only dma 2048 bytes at a time, so do this in chunks
    u32 Offset = 0;
    while ( Size > 0 )
    {
        u32     NBytes = MIN( Size, 2048-sizeof(dmatag) );

        DLPtrAlloc( pDMA, dmatag );
        pDMA->SetRef( (s32)NBytes, MainMemAddr+Offset );
        pDMA->PAD[0] = SCE_VIF1_SET_FLUSHE( 0 );
        pDMA->PAD[1] = SCE_VIF1_SET_MPG( (VUAddr+Offset)/8, NBytes/8, 0 );

        Size   -= NBytes;
        Offset += NBytes;
    }
}

//=============================================================================

void vu1_interface::LoadSyncMCode( void )
{
    if ( m_ActiveSyncMCode != MCODE_SYNC )
    {
        u32 MainMemMCodeStart = (u32)&VU1_SYNC_CODE_START;
        u32 MainMemMCodeEnd   = (u32)&VU1_SYNC_CODE_END;
        u32 MCodeSize         = MainMemMCodeEnd-MainMemMCodeStart;
        MCodeSize             = ALIGN_16(MCodeSize);
        ASSERT( MCodeSize <= (MCODE_START_INIT-MCODE_START_SYNC) );
        LoadMCode( MainMemMCodeStart,   // start address in main mem
                   MCODE_START_SYNC,    // start address in vu
                   MCodeSize );         // size of code

        m_ActiveSyncMCode = MCODE_SYNC;
    }
}

//=============================================================================

void vu1_interface::LoadInitMCode( void )
{
    if ( m_ActiveInitMCode != MCODE_INIT )
    {
        u32 MainMemMCodeStart = (u32)&VU1_INIT_CODE_START;
        u32 MainMemMCodeEnd   = (u32)&VU1_INIT_CODE_END;
        u32 MCodeSize         = MainMemMCodeEnd-MainMemMCodeStart;
        MCodeSize             = ALIGN_16(MCodeSize);
        ASSERT( MCodeSize <= (MCODE_START_CLIPPER-MCODE_START_INIT) );
        LoadMCode( MainMemMCodeStart,   // start address in main mem
                   MCODE_START_INIT,    // start address in vu
                   MCodeSize );         // size of code

        m_ActiveInitMCode = MCODE_INIT;
    }
}

//=============================================================================

void vu1_interface::LoadClipperMCode( void )
{
    if ( m_ActiveClipperMCode != MCODE_CLIPPER )
    {
        u32 MainMemMCodeStart = (u32)&VU1_CLIPPER_CODE_START;
        u32 MainMemMCodeEnd   = (u32)&VU1_CLIPPER_CODE_END;
        u32 MCodeSize         = MainMemMCodeEnd-MainMemMCodeStart;
        MCodeSize             = ALIGN_16(MCodeSize);
        ASSERT( MCodeSize <= (MCODE_START_MATERIAL - MCODE_START_CLIPPER) );
        LoadMCode( MainMemMCodeStart,   // start address in main mem
                   MCODE_START_CLIPPER, // start address in vu
                   MCodeSize );         // size of code

        m_ActiveClipperMCode = MCODE_CLIPPER;
    }
}

//=============================================================================

void vu1_interface::LoadMaterialMCode( s32 MCodeType )
{
    if ( m_ActiveMaterialMCode != MCodeType )
    {
        u32 MainMemMCodeStart;
        u32 MainMemMCodeEnd;

        switch( MCodeType )
        {
        default:
            ASSERT( FALSE );
        case MCODE_MATERIALS:
            MainMemMCodeStart = (u32)&VU1_MATERIAL_CODE_START;
            MainMemMCodeEnd   = (u32)&VU1_MATERIAL_CODE_END;
            break;
        case MCODE_SHADRECEIVE_PASSES:
            MainMemMCodeStart = (u32)&VU1_SHADRECEIVE_PASSES_CODE_START;
            MainMemMCodeEnd   = (u32)&VU1_SHADRECEIVE_PASSES_CODE_END;
            break;
        }

        u32 MCodeSize = MainMemMCodeEnd-MainMemMCodeStart;
        MCodeSize     = ALIGN_16(MCodeSize);
        ASSERT( MCodeSize <= (MCODE_START_TRANSFORM - MCODE_START_MATERIAL) );
        LoadMCode( MainMemMCodeStart,       // start address in main mem
                   MCODE_START_MATERIAL,    // start address in vu
                   MCodeSize );             // size of code

        m_ActiveMaterialMCode = MCodeType;
    }
}

//=============================================================================

void vu1_interface::LoadXFormMCode( s32 MCodeType )
{
    if ( m_ActiveXFormMCode != MCodeType )
    {
        u32 MainMemMCodeStart;
        u32 MainMemMCodeEnd;

        switch( MCodeType )
        {
        default:
            ASSERT( FALSE );
        case MCODE_SKIN_XFORM:
            MainMemMCodeStart = (u32)&VU1_SKIN_XFORM_CODE_START;
            MainMemMCodeEnd   = (u32)&VU1_SKIN_XFORM_CODE_END;
            break;
        case MCODE_RIGID_XFORM:
            MainMemMCodeStart = (u32)&VU1_RIGID_XFORM_CODE_START;
            MainMemMCodeEnd   = (u32)&VU1_RIGID_XFORM_CODE_END;
            break;
        case MCODE_SHADCAST_XFORM:
            MainMemMCodeStart = (u32)&VU1_SHADCAST_XFORM_CODE_START;
            MainMemMCodeEnd   = (u32)&VU1_SHADCAST_XFORM_CODE_END;
            break;
        case MCODE_SHADRECEIVE_XFORM:
            MainMemMCodeStart = (u32)&VU1_SHADRECEIVE_XFORM_CODE_START;
            MainMemMCodeEnd   = (u32)&VU1_SHADRECEIVE_XFORM_CODE_END;
            break;
        case MCODE_SPRITE_XFORM:
            MainMemMCodeStart = (u32)&VU1_SPRITE_XFORM_CODE_START;
            MainMemMCodeEnd   = (u32)&VU1_SPRITE_XFORM_CODE_END;
            break;
        }

        u32 MCodeSize = MainMemMCodeEnd-MainMemMCodeStart;
        MCodeSize     = ALIGN_16(MCodeSize);
        ASSERT( MCodeSize <= (0x4000 - MCODE_START_TRANSFORM) );
        LoadMCode( MainMemMCodeStart,       // start address in main mem
                   MCODE_START_TRANSFORM,   // start address in vu
                   MCodeSize );             // size of code

        m_ActiveXFormMCode = MCodeType;
    }
}

//=============================================================================

void vu1_interface::SetTextureRegisters( const xbitmap& Bitmap, vu1_tex0& Texture, xbool IsDecal ) const
{
    // Setup TEX0 register
    Texture.TEX0Addr = SCE_GS_TEX0_1 + eng_GetGSContext();
    Texture.TEXData  = vram_GetActiveTex0( Bitmap );
    
    if( IsDecal == TRUE )
    {
        // Set Decal mode
        Texture.TEX0.TCC = 1;
        Texture.TEX0.TFX = 1;
    }
}

//=============================================================================

void vu1_interface::InitMaterial( DLPtr( pMaterial, vu1_material ) ) const
{
    // fill in the dma tag so that it goes to an absolute address in vu1 memory
    pMaterial->DMA.SetCont( sizeof( vu1_material ) - sizeof( dmatag ) );
    pMaterial->DMA.PAD[0] = VIF_SkipWrite( 1, 0 );
    pMaterial->DMA.PAD[1] = VIF_Unpack( VU1_MATERIAL, VU1_MATERIAL_SIZE, VIF_V4_32, FALSE, FALSE, TRUE );

    // clear out the pass flags
    pMaterial->Flags        = 0;
    pMaterial->Unused1      = 0.0f;
    pMaterial->DetailScale  = 1.0f;
    pMaterial->PerPolyAlpha = 0;

    // default to no detail mips
    pMaterial->Mips.Data = SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 );
    pMaterial->Mips.Addr = SCE_GS_TEX1_2;

    // and the rest will be filled in as needed by the different material types
}

//=============================================================================

void vu1_interface::DiffuseTexture( DLPtr(pMaterial, vu1_material), const xbitmap& Bitmap ) const
{
    SetTextureRegisters( Bitmap, pMaterial->DiffuseTex, FALSE );
    SetTextureRegisters( Bitmap, pMaterial->DecalTex,   TRUE  );
    
    pMaterial->Flags |= DIFFUSE_PASS;
}

//=============================================================================

void vu1_interface::EnvironmentTexture( DLPtr(pMaterial, vu1_material), const xbitmap* pBitmap ) const
{
    // Activate Context 2
    eng_PushGSContext( 1 );

    if ( pBitmap )
    {
        vram_Activate( *pBitmap, 2 );
        SetTextureRegisters( *pBitmap, pMaterial->EnvTex );
    }
    else
    {
        // the fake "cube" map trick gets the dynamically created texture
        pMaterial->EnvTex.TEXData  = SCE_GS_SET_TEX0_1( vram_GetPermanentArea(), 1, SCE_GS_PSMCT32, 6, 6, 0, 0/*1*/, 0, 0, 0, 0, 0 );
        pMaterial->EnvTex.TEX0Addr = SCE_GS_TEX0_1 + eng_GetGSContext();
    }
    
    eng_PopGSContext();
}

//=============================================================================

void vu1_interface::SetupRawMat( DLPtr(pMaterial, vu1_material), const xbitmap& Bitmap, s32 BlendMode ) const
{
    // Setup registers that are constant for all material types
    InitMaterial( pMaterial );
    DiffuseTexture( pMaterial, Bitmap );

    // set up the blend
    switch ( BlendMode )
    {
    case render::BLEND_MODE_ADDITIVE:
        pMaterial->DiffuseAlpha.Data = SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_SRC, C_DST, 0 );
        pMaterial->DiffuseAlpha.Addr = SCE_GS_ALPHA_1;
        break;
    case render::BLEND_MODE_SUBTRACTIVE:
        pMaterial->DiffuseAlpha.Data = SCE_GS_SET_ALPHA( C_ZERO, C_SRC, A_SRC, C_DST, 0 );
        pMaterial->DiffuseAlpha.Addr = SCE_GS_ALPHA_1;
        break;
    case render::BLEND_MODE_NORMAL:
        pMaterial->DiffuseAlpha.Data = SCE_GS_SET_ALPHA( C_SRC, C_DST, A_SRC, C_DST, 0 );
        pMaterial->DiffuseAlpha.Addr = SCE_GS_ALPHA_1;
        break;
    case render::BLEND_MODE_INTENSITY:
        pMaterial->DiffuseAlpha.Data = SCE_GS_SET_ALPHA( C_DST, C_ZERO, A_SRC, C_ZERO, 0 );
        pMaterial->DiffuseAlpha.Addr = SCE_GS_ALPHA_1;
        break;
    }
}

//=============================================================================

void vu1_interface::SetDiffuseMaterial( const xbitmap& Bitmap, s32 BlendMode, xbool ZTestOn ) const
{
    // Activate the diffuse texture
    vram_Activate( Bitmap, 0 );

    // set up the material
    DLPtrAlloc( pMaterial, vu1_material );
    SetupRawMat( pMaterial, Bitmap, BlendMode );

    gsreg_Begin( 3 );
    gsreg_SetClamping(TRUE);
    gsreg_SetZBufferUpdate( FALSE );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0,
                                   TRUE, ZTestOn ? ZBUFFER_TEST_GEQUAL : ZBUFFER_TEST_ALWAYS );
    gsreg_End();
}

//=============================================================================

void vu1_interface::SetGlowMaterial( const xbitmap& Bitmap, s32 BlendMode, xbool ZTestOn ) const
{
    // Activate the diffuse texture
    vram_Activate( Bitmap, 0 );

    // set up the material
    DLPtrAlloc( pMaterial, vu1_material );
    SetupRawMat( pMaterial, Bitmap, BlendMode );
    pMaterial->Flags |= SELFILLUM_PASS | SELFILLUM_DIFFUSELIT;
    pMaterial->EnvIllumAlpha.Data = pMaterial->DiffuseAlpha.Data;
    pMaterial->EnvIllumAlpha.Addr = SCE_GS_ALPHA_1;

    gsreg_Begin( 4 );
    gsreg_SetClamping(TRUE);
    gsreg_SetZBufferUpdate( FALSE );
    gsreg_SetAlphaAndZBufferTests( TRUE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0,
                                   TRUE, ZTestOn ? ZBUFFER_TEST_GEQUAL : ZBUFFER_TEST_ALWAYS );
    eng_PushGSContext(1);
    gsreg_SetAlphaAndZBufferTests( TRUE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0,
                                   TRUE, ZTestOn ? ZBUFFER_TEST_GEQUAL : ZBUFFER_TEST_ALWAYS );
    eng_PopGSContext();
    gsreg_End();
}

//=============================================================================

void vu1_interface::SetEnvMapMaterial( const xbitmap& Bitmap, s32 BlendMode, xbool ZTestOn ) const
{
    // Activate the diffuse texture
    vram_Activate( Bitmap, 0 );

    // set up the material
    DLPtrAlloc( pMaterial, vu1_material );
    SetupRawMat( pMaterial, Bitmap, BlendMode );

    pMaterial->Flags |= ENVIRONMENT_PASS | USE_POSITIONS;
    pMaterial->EnvIllumAlpha.Data = SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_DST, C_DST, 0 );
    pMaterial->EnvIllumAlpha.Addr = SCE_GS_ALPHA_2;

    EnvironmentTexture( pMaterial, NULL );   // NULL tells this function to use the default cube map

    gsreg_Begin( 4 );
    gsreg_SetClamping(TRUE);
    gsreg_SetZBufferUpdate( FALSE );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0,
                                   TRUE, ZTestOn ? ZBUFFER_TEST_GEQUAL : ZBUFFER_TEST_ALWAYS );
    eng_PushGSContext(1);
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0,
                                   TRUE, ZTestOn ? ZBUFFER_TEST_GEQUAL : ZBUFFER_TEST_ALWAYS );
    eng_PopGSContext();
    gsreg_End();
}

//=============================================================================

void vu1_interface::SetDistortionMaterial( s32   BlendMode,
                                           xbool ZTestOn ) const
{
    // TODO: Finish this function
    (void)BlendMode;
    (void)ZTestOn;
}


//=============================================================================

void vu1_interface::SetMaterial( const material& Mat ) const
{
    // upload the diffuse texture and save off the registers for later use
    u64 DiffuseTex0;
    u64 DiffuseTex1;
    u64 DiffuseMip1;
    u64 DiffuseMip2;
    if( (Mat.m_Type == Material_Distortion) || (Mat.m_Type == Material_Distortion_PerPolyEnv) )
    {
        DiffuseTex0 = m_DistortionTex0;
        DiffuseTex1 = SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 );
        DiffuseMip1 = SCE_GS_SET_MIPTBP1( 0, 0, 0, 0, 0, 0 );
        DiffuseMip2 = SCE_GS_SET_MIPTBP2( 0, 0, 0, 0, 0, 0 );
    }
    else
    {
        ASSERT( Mat.m_pDiffuseTex );
        vram_Upload( Mat.m_pDiffuseTex->m_Bitmap, 0 );
        DiffuseTex0 = vram_GetActiveTex0( Mat.m_pDiffuseTex->m_Bitmap );
        DiffuseTex1 = vram_GetActiveTex1( Mat.m_pDiffuseTex->m_Bitmap );
        DiffuseMip1 = vram_GetActiveMip1( Mat.m_pDiffuseTex->m_Bitmap );
        DiffuseMip2 = vram_GetActiveMip2( Mat.m_pDiffuseTex->m_Bitmap );
    }

    // upload the environment texture and save out the register for later use
    xbool bUseEnvMap  = FALSE;
    xbool bNotCubeMap = FALSE;
    if( (Mat.m_Type == Material_Diff_PerPixelEnv) ||
        (Mat.m_Type == Material_Alpha_PerPolyEnv) )
    {
        bUseEnvMap  = TRUE;
        bNotCubeMap = !(Mat.m_Flags & geom::material::FLAG_ENV_CUBE_MAP);
    }
    else if( Mat.m_Type == Material_Distortion_PerPolyEnv )
    {
        bUseEnvMap  = TRUE;
        bNotCubeMap = TRUE;
    }

    u64 EnvTex0;
    if( bUseEnvMap )
    {
        if( bNotCubeMap )
        {
            // if its not a "cube" map, then we upload a texture
            ASSERT( Mat.m_pEnvTex );
            vram_Upload( Mat.m_pEnvTex->m_Bitmap, 2 );
            EnvTex0 = vram_GetActiveTex0( Mat.m_pEnvTex->m_Bitmap );
        }
        else
        {
            // the fake "cube" map trick gets the dynamically created texture
            EnvTex0 = SCE_GS_SET_TEX0( vram_GetPermanentArea(), 1, SCE_GS_PSMCT32, 6, 6, 0, 0, 0, 0, 0, 0, 0 );
        }
    }
    else
    {
        EnvTex0 = DiffuseTex0;
    }

    // upload the detail texture and copy out the appropriate registers for later use
    u64 DetailTex0;
    u64 DetailMip1;
    if( Mat.m_Flags & geom::material::FLAG_HAS_DETAIL_MAP )
    {
        ASSERT( Mat.m_pDetailTex );
        vram_Upload( Mat.m_pDetailTex->m_Bitmap, 1 );
        DetailTex0 = vram_GetActiveTex0( Mat.m_pDetailTex->m_Bitmap );
        DetailMip1 = vram_GetActiveMip1( Mat.m_pDetailTex->m_Bitmap );
    }
    else
    {
        DetailTex0 = DiffuseTex0;
        DetailMip1 = DiffuseMip1;
    }

    // set up the dma and unpack for the microcode material data
    DLPtrAlloc( pSet, vu1_material_set );
    pSet->Mat.DMA.SetCont( sizeof(vu1_material) - sizeof(dmatag) );
    pSet->Mat.DMA.PAD[0] = VIF_SkipWrite( 1, 0 );
    pSet->Mat.DMA.PAD[1] = VIF_Unpack( VU1_MATERIAL, VU1_MATERIAL_SIZE, VIF_V4_32, FALSE, FALSE, TRUE );
    
    pSet->Init.DMA.SetCont( sizeof(vu1_material_init) - sizeof(dmatag) );
    pSet->Init.DMA.PAD[0] = 0;
    pSet->Init.DMA.PAD[1] = SCE_VIF1_SET_DIRECT( 10, 0 );

    // fill in the flags, detail scale, and per poly alpha
    pSet->Mat.Flags        = Mat.m_MCodeFlags;
    pSet->Mat.Unused1      = 0;
    pSet->Mat.DetailScale  = Mat.m_DetailScale;
    pSet->Mat.PerPolyAlpha = Mat.m_PerPolyAlpha;

    // clear the mips register to start with
    // each piece of geometry may choose to fill it in if a detail map is present
    pSet->Mat.Mips.Data = SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 );
    pSet->Mat.Mips.Addr = SCE_GS_TEX1_2;

    // fill in the diffuse alpha register
    pSet->Mat.DiffuseAlpha.Data = Mat.m_DiffuseAlphaReg;
    pSet->Mat.DiffuseAlpha.Addr = SCE_GS_ALPHA_1;

    // fill in the env/illum alpha register
    pSet->Mat.EnvIllumAlpha.Data = Mat.m_EnvIllumAlphaReg;
    pSet->Mat.EnvIllumAlpha.Addr = Mat.m_EnvIllumAlphaAddr;

    // fill in the env. map matrix
    if( (Mat.m_Type == Material_Distortion) ||
        (Mat.m_Type == Material_Distortion_PerPolyEnv) ||
        (Mat.m_Flags & geom::material::FLAG_ENV_VIEW_SPACE) )
    {
        pSet->Mat.EnvMatrix0 = m_ViewEnvMapMatrix0;
        pSet->Mat.EnvMatrix1 = m_ViewEnvMapMatrix1;
    }
    else
    {
        pSet->Mat.EnvMatrix0 = Mat.m_EnvMapVectors[0];
        pSet->Mat.EnvMatrix1 = Mat.m_EnvMapVectors[1];
    }

    // fill in the clamping register
    pSet->Init.Clamp1.Addr = SCE_GS_CLAMP_1;
    if( (Mat.m_Type == Material_Distortion) || (Mat.m_Type == Material_Distortion_PerPolyEnv) )
    {
        pSet->Init.Clamp1.Data = m_DistortionClamp;
    }
    else
    {
        pSet->Init.Clamp1.Data = SCE_GS_SET_CLAMP( 0, 0, 0, 0, 0, 0 );
    }

    // set the z buffer register update
    switch( Mat.m_Type )
    {
    default:
        ASSERT( FALSE );

    case Material_Diff:
    case Material_Diff_PerPixelEnv:
    case Material_Diff_PerPixelIllum:
    case Material_Distortion:
    case Material_Distortion_PerPolyEnv:
        pSet->Init.ZBuf1.Data = SCE_GS_SET_ZBUF( VRAM_ZBUFFER_START/8192, SCE_GS_PSMZ24, 0 );
        break;

    case Material_Alpha:
    case Material_Alpha_PerPolyEnv:
    case Material_Alpha_PerPixelIllum:
    case Material_Alpha_PerPolyIllum:
        if( Mat.m_Flags & geom::material::FLAG_FORCE_ZFILL )
            pSet->Init.ZBuf1.Data = SCE_GS_SET_ZBUF( VRAM_ZBUFFER_START/8192, SCE_GS_PSMZ24, 0 );
        else
            pSet->Init.ZBuf1.Data = SCE_GS_SET_ZBUF( VRAM_ZBUFFER_START/8192, SCE_GS_PSMZ24, 1 );
        break;
    }
    pSet->Init.ZBuf1.Addr = SCE_GS_ZBUF_1;
    pSet->Init.ZBuf2.Data = SCE_GS_SET_ZBUF( VRAM_ZBUFFER_START/8192, SCE_GS_PSMZ24, 1 );
    pSet->Init.ZBuf2.Addr = SCE_GS_ZBUF_2;

    // set up the punch-through test register
    pSet->Init.Test.Addr = SCE_GS_TEST_1;
    if( Mat.m_Flags & geom::material::FLAG_IS_PUNCH_THRU )
    {
        pSet->Init.Test.Data = SCE_GS_SET_TEST( TRUE, ALPHA_TEST_GEQUAL, 64, ALPHA_TEST_FAIL_KEEP,
                                                FALSE, DEST_ALPHA_TEST_0,
                                                TRUE, ZBUFFER_TEST_GEQUAL );
    }
    else
    {
        pSet->Init.Test.Data = SCE_GS_SET_TEST( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                                FALSE, DEST_ALPHA_TEST_0,
                                                TRUE, ZBUFFER_TEST_GEQUAL );
    }

    // set up the giftag for the register settings
    pSet->Init.RegLoadGif.BuildRegLoad( 9, TRUE );
    pSet->Init.Texflush.Data = 0;
    pSet->Init.Texflush.Addr = SCE_GS_TEXFLUSH;

    // fill in the diffuse texture registers
    pSet->Mat.DiffuseTex.TEX0Addr = SCE_GS_TEX0_1;
    pSet->Mat.DecalTex.TEX0Addr   = SCE_GS_TEX0_1;
    pSet->Init.DiffuseTex1.Addr   = SCE_GS_TEX1_1;
    pSet->Init.DiffuseMip1.Addr   = SCE_GS_MIPTBP1_1;
    pSet->Init.DiffuseMip2.Addr   = SCE_GS_MIPTBP2_1;
    pSet->Mat.DiffuseTex.TEXData  = DiffuseTex0;
    pSet->Init.DiffuseTex1.Data   = DiffuseTex1;
    pSet->Init.DiffuseMip1.Data   = DiffuseMip1;
    pSet->Init.DiffuseMip2.Data   = DiffuseMip2;
    pSet->Mat.DecalTex.TEXData    = DiffuseTex0;
    pSet->Mat.DecalTex.TEX0.TCC   = 1;
    pSet->Mat.DecalTex.TEX0.TFX   = 1;

    // fill in the environment texture registers
    pSet->Mat.EnvTex.TEX0Addr = SCE_GS_TEX0_2;
    pSet->Mat.EnvTex.TEXData  = EnvTex0;

    // fill in the detail texture registers
    pSet->Mat.DetailTex.TEX0Addr = SCE_GS_TEX0_2;
    pSet->Mat.DetailTex.TEXData  = DetailTex0;
    pSet->Init.DetailMip1.Addr   = SCE_GS_MIPTBP1_2;
    pSet->Init.DetailMip1.Data   = DetailMip1;
}

//=============================================================================

void vu1_interface::SetZPrimeMaterial( void ) const
{
    DLPtrAlloc( pMaterial, vu1_material );
    InitMaterial( pMaterial );

    pMaterial->DiffuseAlpha.Addr = SCE_GS_ALPHA_1;
    pMaterial->DiffuseAlpha.Data = SCE_GS_SET_ALPHA( C_ZERO, C_ZERO, A_SRC, C_SRC, 0x80 );
    pMaterial->Flags = ZPRIME_PASS;

    // fill the z-buffer
    gsreg_Begin( 1 );
    eng_PushGSContext(1);
    gsreg_SetZBufferUpdate( TRUE );
    eng_PopGSContext();
    gsreg_End();
}

//=============================================================================

void vu1_interface::SetDistortionMaterial( const material* pMaterial, const radian3& NormalRot ) const
{
    // save off diffuse texture info
    u64 DiffuseTex0 = m_DistortionTex0;
    u64 DiffuseTex1 = SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 );
    u64 DiffuseMip1 = SCE_GS_SET_MIPTBP1( 0, 0, 0, 0, 0, 0 );
    u64 DiffuseMip2 = SCE_GS_SET_MIPTBP2( 0, 0, 0, 0, 0, 0 );

    // upload the environment texture and save out the register for later use (if necessary)
    u64 EnvTex0;
    if( pMaterial && (pMaterial->m_Type == Material_Distortion_PerPolyEnv) )
    {
        ASSERT( pMaterial->m_pEnvTex );
        vram_Upload( pMaterial->m_pEnvTex->m_Bitmap, 2 );
        EnvTex0 = vram_GetActiveTex0( pMaterial->m_pEnvTex->m_Bitmap );
    }
    else
    {
        EnvTex0 = DiffuseTex0;
    }

    u64 DetailTex0 = DiffuseTex0;
    u64 DetailMip1 = DiffuseMip1;

    // set up the dma and unpack for the microcode material data
    DLPtrAlloc( pSet, vu1_material_set );
    pSet->Mat.DMA.SetCont( sizeof(vu1_material) - sizeof(dmatag) );
    pSet->Mat.DMA.PAD[0] = VIF_SkipWrite( 1, 0 );
    pSet->Mat.DMA.PAD[1] = VIF_Unpack( VU1_MATERIAL, VU1_MATERIAL_SIZE, VIF_V4_32, FALSE, FALSE, TRUE );

    pSet->Init.DMA.SetCont( sizeof(vu1_material_init) - sizeof(dmatag) );
    pSet->Init.DMA.PAD[0] = 0;
    pSet->Init.DMA.PAD[1] = SCE_VIF1_SET_DIRECT( 10, 0 );

    // fill in the flags, detail scale, and per poly alpha
    pSet->Mat.Flags = DIFFUSE_PASS | DISTORTION_PASS;
    if( pMaterial && (pMaterial->m_Type == Material_Distortion_PerPolyEnv) )
        pSet->Mat.Flags |= ENVIRONMENT_PASS;
    pSet->Mat.Unused1      = 0;
    pSet->Mat.DetailScale  = 1.0f;
    pSet->Mat.PerPolyAlpha = 128;

    // clear the mips register
    pSet->Mat.Mips.Data = SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 );
    pSet->Mat.Mips.Addr = SCE_GS_TEX1_2;

    // fill in the diffuse alpha register
    pSet->Mat.DiffuseAlpha.Data = SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_FIX, C_ZERO, 0x70 );
    pSet->Mat.DiffuseAlpha.Addr = SCE_GS_ALPHA_1;

    // fill in the env/illum alpha register
    u32 FixedAlpha = pMaterial ? (u32)(pMaterial->m_FixedAlpha*128.0f) : 0x80;
    if( pMaterial && (pMaterial->m_Flags & geom::material::FLAG_IS_ADDITIVE) )
        pSet->Mat.EnvIllumAlpha.Data = SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_FIX, C_DST, FixedAlpha );
    else if( pMaterial && (pMaterial->m_Flags & geom::material::FLAG_IS_SUBTRACTIVE) )
        pSet->Mat.EnvIllumAlpha.Data = SCE_GS_SET_ALPHA( C_ZERO, C_SRC, A_FIX, C_DST, FixedAlpha );
    else
        pSet->Mat.EnvIllumAlpha.Data = SCE_GS_SET_ALPHA( C_SRC, C_DST, A_FIX, C_DST, FixedAlpha );
    pSet->Mat.EnvIllumAlpha.Addr = SCE_GS_ALPHA_2;

    // normals need to be transformed to screen space for this to work...we'll
    // make the env. map matrix dual-purpose to save vu1 space
    matrix4 W2V = eng_GetView()->GetW2V();
    W2V = W2V * matrix4( NormalRot );
    pSet->Mat.EnvMatrix0.Set( W2V(0,0), W2V(0,1), W2V(2,0), W2V(2,1) );
    pSet->Mat.EnvMatrix1.Set( W2V(1,0), W2V(1,1), 0.0f,     0.0f     );

    // fill in the clamping register
    pSet->Init.Clamp1.Addr = SCE_GS_CLAMP_1;
    pSet->Init.Clamp1.Data = m_DistortionClamp;

    // set the z-buffer register update
    pSet->Init.ZBuf1.Data = SCE_GS_SET_ZBUF( VRAM_ZBUFFER_START/8192, SCE_GS_PSMZ24, 0 );
    pSet->Init.ZBuf1.Addr = SCE_GS_ZBUF_1;
    pSet->Init.ZBuf2.Data = SCE_GS_SET_ZBUF( VRAM_ZBUFFER_START/8192, SCE_GS_PSMZ24, 1 );
    pSet->Init.ZBuf2.Addr = SCE_GS_ZBUF_2;

    // set up the test register
    pSet->Init.Test.Addr = SCE_GS_TEST_1;
    pSet->Init.Test.Data = SCE_GS_SET_TEST( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                            FALSE, DEST_ALPHA_TEST_0,
                                            TRUE, ZBUFFER_TEST_GEQUAL );

    // set up the giftag for the register settings
    pSet->Init.RegLoadGif.BuildRegLoad( 9, TRUE );
    pSet->Init.Texflush.Data = 0;
    pSet->Init.Texflush.Addr = SCE_GS_TEXFLUSH;

    // fill in the diffuse texture registers
    pSet->Mat.DiffuseTex.TEX0Addr = SCE_GS_TEX0_1;
    pSet->Mat.DecalTex.TEX0Addr   = SCE_GS_TEX0_1;
    pSet->Init.DiffuseTex1.Addr   = SCE_GS_TEX1_1;
    pSet->Init.DiffuseMip1.Addr   = SCE_GS_MIPTBP1_1;
    pSet->Init.DiffuseMip2.Addr   = SCE_GS_MIPTBP2_1;
    pSet->Mat.DiffuseTex.TEXData  = DiffuseTex0;
    pSet->Init.DiffuseTex1.Data   = DiffuseTex1;
    pSet->Init.DiffuseMip1.Data   = DiffuseMip1;
    pSet->Init.DiffuseMip2.Data   = DiffuseMip2;
    pSet->Mat.DecalTex.TEXData    = DiffuseTex0;
    pSet->Mat.DecalTex.TEX0.TCC   = 1;
    pSet->Mat.DecalTex.TEX0.TFX   = 1;

    // fill in the environment texture registers
    pSet->Mat.EnvTex.TEX0Addr = SCE_GS_TEX0_2;
    pSet->Mat.EnvTex.TEXData  = EnvTex0;

    // fill in the detail texture registers
    pSet->Mat.DetailTex.TEX0Addr = SCE_GS_TEX0_2;
    pSet->Mat.DetailTex.TEXData  = DetailTex0;
    pSet->Init.DetailMip1.Addr   = SCE_GS_MIPTBP1_2;
    pSet->Init.DetailMip1.Data   = DetailMip1;
}

//=============================================================================

void vu1_interface::SetL2W( const matrix4& L2W ) const
{
    ASSERT( m_InBegin == TRUE );
    DLPtrAlloc( pDMA, dmatag );
    pDMA->SetRef( sizeof(matrix4), (u32)&L2W );
    pDMA->PAD[0] = VIF_SkipWrite( 1, 0 );
    pDMA->PAD[1] = VIF_Unpack( VU1_L2W, 4, VIF_V4_32, TRUE, FALSE, FALSE );
}

//=============================================================================

void vu1_interface::InitClipper( void ) const
{
    // ref to the plane and matrix data
    DLPtrAlloc( pRef, dmatag );
    pRef->SetRef( sizeof(vu1_interface::vu1_clip_data), (u32)m_pClipData );
    pRef->PAD[0] = VIF_SkipWrite( 1, 0 );
    pRef->PAD[1] = VIF_Unpack( VU1_CLIP_PLANES, VU1_CLIP_NUM_PLANES+3*4, VIF_V4_32, FALSE, FALSE, TRUE );
}

//=============================================================================

void vu1_interface::InitSkinClipper( void ) const
{
    // ref to the plane and matrix data
    DLPtrAlloc( pRef, dmatag );
    pRef->SetRef( sizeof(vu1_interface::vu1_clip_data), (u32)m_pClipData );
    pRef->PAD[0] = VIF_SkipWrite( 1, 0 );
    pRef->PAD[1] = VIF_Unpack( VU1_CLIP_PLANES+VU1_SKIN_BUFFER_0, VU1_CLIP_NUM_PLANES+3*4, VIF_V4_32, FALSE, FALSE, TRUE );
}

//=============================================================================

void vu1_interface::SetDetailTex1( u64 Tex1 ) const
{
    DLPtrAlloc( pPack, vu1_tex1 );
    pPack->DMA.SetCont( sizeof( vu1_tex1 ) - sizeof( dmatag ) );

    pPack->DMA.PAD[0]   = VIF_SkipWrite( 1, 0 );
    pPack->DMA.PAD[1]   = VIF_Unpack( VU1_Mips, 1, VIF_V4_32, FALSE, FALSE, TRUE );
    pPack->TEX1.Data    = Tex1;
    pPack->TEX1.Addr    = SCE_GS_TEX1_2;
}

//=============================================================================

void vu1_interface::UploadVertChunk( const vector4* pVert, const s16* pUV, const s8* pNormal, s32 nVerts ) const
{
    CONFIRM_ALIGNMENT( pVert   );
    CONFIRM_ALIGNMENT( pUV     );
    CONFIRM_ALIGNMENT( pNormal );

    // ADC bits MUST be present on first 2 vertices of each chunk!
    ASSERT( *((u32*)&pVert[ m_Offset + 0 ] + 3) & 0x8000 );
    ASSERT( *((u32*)&pVert[ m_Offset + 1 ] + 3) & 0x8000 );

    s32     nBytes;
    s32     nVectors;

    // Upload diffuse texture coordinates
    DLArrayAlloc( pPack, dmatag, 3 );
    nBytes   = ALIGN_16( (sizeof( s16 ) * 2) * nVerts );
    nVectors = nBytes  / (sizeof( s16 ) * 2);
    pPack[0].SetRef( nBytes, (u32)(pUV + (m_Offset * 2)) );
    pPack[0].PAD[0] = VIF_SkipWrite( 1, 3 );
    pPack[0].PAD[1] = VIF_Unpack( VU1_UV0,    nVectors, VIF_V2_16, TRUE,  TRUE, FALSE );

    // Upload normals
    nBytes   = ALIGN_16( (sizeof( s8 ) * 3) * nVerts );
    nVectors = nBytes  / (sizeof( s8 ) * 3);
    pPack[1].SetRef( nBytes, (u32)(pNormal + (m_Offset * 3)) );
    pPack[1].PAD[1] = VIF_Unpack( VU1_NORMAL, nVectors, VIF_V3_8,  TRUE, FALSE, FALSE );
    
    // Upload vertices
    nBytes   = ALIGN_16( sizeof( vector4 ) * nVerts );
    nVectors = nBytes / 16;
    pPack[2].SetRef( nBytes, (u32)(pVert + m_Offset) );
    pPack[2].PAD[1] = VIF_Unpack( VU1_XYZ,    nVectors, VIF_V4_32, TRUE, FALSE, FALSE );
}

//=============================================================================

void vu1_interface::UploadRawVertChunk( const vector4* pPos,
                                        const s16*     pUV,
                                        const u32*     pColor,
                                        s32            nVerts ) const
{
    CONFIRM_ALIGNMENT( pPos   );
    CONFIRM_ALIGNMENT( pUV    );
    CONFIRM_ALIGNMENT( pColor );

    // ADC bits MUST be present on first 2 vertices of each chunk!
    ASSERT( *((u32*)&pPos[ 0 ] + 3) & 0x8000 );
    ASSERT( *((u32*)&pPos[ 1 ] + 3) & 0x8000 );

    s32     nBytes;
    s32     nVectors;

    DLArrayAlloc( pPack, dmatag, 3 );

    // Upload vertices
    nBytes   = ALIGN_16( sizeof( vector4 ) * nVerts );
    nVectors = nBytes / sizeof( vector4 );
    pPack[0].SetRef( nBytes, (u32)pPos );
    pPack[0].PAD[0] = VIF_SkipWrite( 1, 3 );
    pPack[0].PAD[1] = VIF_Unpack( VU1_XYZ,    nVectors, VIF_V4_32, TRUE, FALSE, FALSE );

    // Upload diffuse texture coordinates
    nBytes   = ALIGN_16( sizeof( s16 ) * 2 * nVerts );
    nVectors = nBytes  / (sizeof( s16 ) * 2);
    pPack[1].SetRef( nBytes, (u32)pUV );
    pPack[1].PAD[0] = VIF_SkipWrite( 1, 3 );
    pPack[1].PAD[1] = VIF_Unpack( VU1_UV0,    nVectors, VIF_V2_16, TRUE,  TRUE, FALSE );

    // upload colors
    nBytes   = ALIGN_16( sizeof( u32 ) * nVerts );
    nVectors = nBytes / sizeof( u32 );
    pPack[2].SetRef( nBytes, (u32)pColor );
    pPack[2].PAD[0] = VIF_SkipWrite( 1, 3 );
    pPack[2].PAD[1] = VIF_Unpack( VU1_RGB,    nVectors, VIF_V4_8, TRUE, FALSE, FALSE );
}

//=============================================================================

void vu1_interface::UploadSpriteVertChunk( const vector4* pPos,
                                           const vector2* pRotScale,
                                           const u32*     pColor,
                                           s32            nSprites ) const
{
    CONFIRM_ALIGNMENT( pPos      );
    CONFIRM_ALIGNMENT( pRotScale );
    CONFIRM_ALIGNMENT( pColor    );

    s32     nBytes;
    s32     nVectors;

    DLArrayAlloc( pPack, dmatag, 3 );

    // upload positions
    nBytes   = ALIGN_16( sizeof(vector4) * nSprites );
    nVectors = nBytes / sizeof(vector4);
    pPack[0].SetRef( nBytes, (u32)pPos );
    pPack[0].PAD[0] = VIF_SkipWrite( 1, 15 );
    pPack[0].PAD[1] = VIF_Unpack( VU1_XYZ, nVectors, VIF_V4_32, TRUE, FALSE, FALSE );

    // upload rotations and scales
    nBytes   = ALIGN_16( sizeof(vector2) * nSprites );
    nVectors = nBytes / sizeof(vector2);
    pPack[1].SetRef( nBytes, (u32)pRotScale );
    pPack[1].PAD[0] = VIF_SkipWrite( 1, 15 );
    pPack[1].PAD[1] = VIF_Unpack( VU1_UV0, nVectors, VIF_V2_32, TRUE, TRUE, FALSE );

    // upload colors
    nBytes   = ALIGN_16( sizeof(u32) * nSprites );
    nVectors = nBytes / sizeof(u32);
    pPack[2].SetRef( nBytes, (u32)pColor );
    pPack[2].PAD[0] = VIF_SkipWrite( 1, 15 );
    pPack[2].PAD[1] = VIF_Unpack( VU1_RGB, nVectors, VIF_V4_8, FALSE, FALSE, FALSE );
}

//=============================================================================

void vu1_interface::UploadSpriteVertChunk( const vector4* pPos,
                                           const vector4* pVelocity,
                                           const u32*     pColor,
                                           s32            nSprites ) const
{
    CONFIRM_ALIGNMENT( pPos      );
    CONFIRM_ALIGNMENT( pVelocity );
    CONFIRM_ALIGNMENT( pColor    );

    s32     nBytes;
    s32     nVectors;

    DLArrayAlloc( pPack, dmatag, 3 );

    // upload positions
    nBytes   = ALIGN_16( sizeof(vector4) * nSprites );
    nVectors = nBytes / sizeof(vector4);
    pPack[0].SetRef( nBytes, (u32)pPos );
    pPack[0].PAD[0] = VIF_SkipWrite( 1, 15 );
    pPack[0].PAD[1] = VIF_Unpack( VU1_XYZ, nVectors, VIF_V4_32, TRUE, FALSE, FALSE );

    // upload velocities
    nBytes   = ALIGN_16( sizeof(vector4) * nSprites );
    nVectors = nBytes / sizeof(vector4);
    pPack[1].SetRef( nBytes, (u32)pVelocity );
    pPack[1].PAD[0] = VIF_SkipWrite( 1, 15 );
    pPack[1].PAD[1] = VIF_Unpack( VU1_UV0, nVectors, VIF_V4_32, TRUE, FALSE, FALSE );

    // upload colors
    nBytes   = ALIGN_16( sizeof(u32) * nSprites );
    nVectors = nBytes / sizeof(u32);
    pPack[2].SetRef( nBytes, (u32)pColor );
    pPack[2].PAD[0] = VIF_SkipWrite( 1, 15 );
    pPack[2].PAD[1] = VIF_Unpack( VU1_RGB, nVectors, VIF_V4_8, FALSE, FALSE, FALSE );
}

//=============================================================================

void vu1_interface::UploadShadReceiveVertChunk( const vector4* pVert,
                                                const s16*     pUV,
                                                s32            nVerts ) const
{
    CONFIRM_ALIGNMENT( pVert );
    CONFIRM_ALIGNMENT( pUV   );

    // ADC bits MUST be present on first 2 vertices of each chunk!
    ASSERT( *((u32*)&pVert[ m_Offset + 0 ] + 3) & 0x8000 );
    ASSERT( *((u32*)&pVert[ m_Offset + 1 ] + 3) & 0x8000 );

    s32     nBytes;
    s32     nVectors;

    DLArrayAlloc( pPack, dmatag, 2 );

    // Upload vertices
    nBytes   = ALIGN_16( sizeof( vector4 ) * nVerts );
    nVectors = nBytes / 16;
    pPack[0].SetRef( nBytes, (u32)(pVert+m_Offset) );
    pPack[0].PAD[0] = VIF_SkipWrite( 1, 3 );
    pPack[0].PAD[1] = VIF_Unpack( VU1_XYZ, nVectors, VIF_V4_32, TRUE, FALSE, FALSE );

    // TODO: Only do this for punch-through otherwise we don't need them
    // Upload diffuse texture coordinates
    nBytes   = ALIGN_16( (sizeof( s16 ) * 2) * nVerts );
    nVectors = nBytes  / (sizeof( s16 ) * 2);
    pPack[1].SetRef( nBytes, (u32)(pUV + (m_Offset * 2)) );
    pPack[1].PAD[0] = VIF_SkipWrite( 1, 3 );
    pPack[1].PAD[1] = VIF_Unpack( VU1_UV0, nVectors, VIF_V2_16, TRUE,  TRUE, FALSE );
}

//=============================================================================

void vu1_interface::UploadRGBChunk( const u16* pRGB, s32 nVerts ) const
{
    CONFIRM_ALIGNMENT( pRGB );

    s32     nBytes;
    s32     nVectors;

    // Upload RGB values
    DLPtrAlloc( pPack, dmatag );
    nBytes   = ALIGN_16( sizeof( u16 ) * nVerts );
    nVectors = nBytes  / sizeof( u16 );
    if (pRGB)
    {
        pPack->SetRef( nBytes, (u32)(pRGB + m_Offset) );
    }
    else
    {
        pPack->SetRef( nBytes, (u32)(s_UnlitColor) );
    }
    pPack->PAD[0] = VIF_SkipWrite( 1, 3 );
    pPack->PAD[1] = VIF_Unpack( VU1_RGB, nVectors, VIF_V4_5, FALSE, FALSE, FALSE );
}

//=============================================================================

void vu1_interface::UploadSkinVertChunk( const s16* pUV,
                                         const s16* pPos,
                                         const u8*  pBoneIndices,
                                         const u8*  pNormals,
                                         s32        nVerts ) const
{
    CONFIRM_ALIGNMENT( pUV          );
    CONFIRM_ALIGNMENT( pPos         );
    CONFIRM_ALIGNMENT( pBoneIndices );
    CONFIRM_ALIGNMENT( pNormals     );

    // ADC bits MUST be present on first 2 vertices of each chunk!
    ASSERT( *(u16*)&pPos[ 0*4 + 3 ] & 0x8000 );
    ASSERT( *(u16*)&pPos[ 1*4 + 3 ] & 0x8000 );

    s32     nBytes;
    s32     nVectors;

    DLArrayAlloc( pPack, dmatag, 4 );

    // Upload diffuse texture coordinates
    nBytes   = ALIGN_16( (sizeof( s16 ) * 2) * nVerts );
    nVectors = nBytes  / (sizeof( s16 ) * 2);
    pPack[0].SetRef( nBytes, (u32)pUV );
    pPack[0].PAD[0] = VIF_SkipWrite( 1, 3 );
    pPack[0].PAD[1] = VIF_Unpack( VU1_UV0, nVectors, VIF_V2_16, TRUE, TRUE, FALSE );

    // Upload positions
    nBytes   = ALIGN_16( (sizeof( u16 ) * 4) * nVerts );
    nVectors = nBytes  / (sizeof( u16 ) * 4);
    pPack[1].SetRef( nBytes, (u32)pPos );
    pPack[1].PAD[0] = SCE_VIF1_SET_STMOD( 1, 0 );
    pPack[1].PAD[1] = VIF_Unpack( VU1_XYZ, nVectors, VIF_V4_16, TRUE, FALSE, FALSE );

    // Upload normals
    nBytes   = ALIGN_16( (sizeof( u8  ) * 4) * nVerts );
    nVectors = nBytes  / (sizeof( u8  ) * 4);
    pPack[2].SetRef( nBytes, (u32)pNormals );
    pPack[2].PAD[0] = SCE_VIF1_SET_STMOD( 0, 0 );
    pPack[2].PAD[1] = VIF_Unpack( VU1_NORMAL, nVectors, VIF_V4_8, TRUE, FALSE, FALSE );

    // Upload bone indices
    nBytes   = ALIGN_16( (sizeof( u8  ) * 4) * nVerts );
    nVectors = nBytes  / (sizeof( u8  ) * 4);
    pPack[3].SetRef( nBytes, (u32)pBoneIndices );
    pPack[3].PAD[0] = VIF_SkipWrite( 1, 3 );
    pPack[3].PAD[1] = VIF_Unpack( VU1_BNIDX, nVectors, VIF_V4_8, FALSE, FALSE, FALSE );
}

//=============================================================================

void vu1_interface::UploadSkinVertChunk( const s16* pPos,
                                         const u8*  pBoneIndices,
                                         s32        nVerts ) const
{
    CONFIRM_ALIGNMENT( pPos         );
    CONFIRM_ALIGNMENT( pBoneIndices );

    // ADC bits MUST be present on first 2 vertices of each chunk!
    ASSERT( *(u16*)&pPos[ 0*4 + 3 ] & 0x8000 );
    ASSERT( *(u16*)&pPos[ 1*4 + 3 ] & 0x8000 );

    s32     nBytes;
    s32     nVectors;

    DLArrayAlloc( pPack, dmatag, 3 );

    // Upload bone indices
    nBytes   = ALIGN_16( (sizeof( u8  ) * 4) * nVerts );
    nVectors = nBytes  / (sizeof( u8  ) * 4);
    pPack[0].SetRef( nBytes, (u32)pBoneIndices );
    pPack[0].PAD[0] = VIF_SkipWrite( 1, 3 );
    pPack[0].PAD[1] = VIF_Unpack( VU1_BNIDX, nVectors, VIF_V4_8, FALSE, FALSE, FALSE );

    // Upload positions
    nBytes   = ALIGN_16( (sizeof( s16 ) * 4) * nVerts );
    nVectors = nBytes  / (sizeof( s16 ) * 4);
    pPack[1].SetRef( nBytes, (u32)pPos );
    pPack[1].PAD[0] = SCE_VIF1_SET_STMOD( 1, 0 );
    pPack[1].PAD[1] = VIF_Unpack( VU1_XYZ, nVectors, VIF_V4_16, TRUE, FALSE, FALSE );

    // reset stmode
    pPack[2].SetCont( 0 );
    pPack[2].PAD[0] = SCE_VIF1_SET_STMOD( 0, 0 );
    pPack[2].PAD[1] = 0;
}

//=============================================================================

void vu1_interface::RenderNonClipInstances( void )
{
    if( m_nNonClipInstances == 0 )
        return;

    s32 nVerts = m_nVerts;
    m_Offset   = 0;

    vu1_instance* pStart = (vu1_instance*)NONCLIPPED_SPAD_START;
    vu1_instance* pEnd   = pStart + m_nNonClipInstances;

    // Loop through all chunks
    while( nVerts > 0 )
    {
        // Set number of verts for this chunk
        s32 Num = MIN( nVerts, VU1_VERTS );

        // Upload the verts, uv's and normals
        UploadVertChunk( m_pVert, m_pUV, m_pNormal, Num );

        // Render all instances
        for( vu1_instance* pInstance = pStart; pInstance < pEnd; pInstance++ )
        {
            UploadVertCount( Num, pInstance->Flags, pInstance->UOffset, pInstance->VOffset, pInstance->Alpha );
            SetL2W( *pInstance->pL2W );
            UploadRGBChunk ( pInstance->pRGB, Num  );
            if ( pInstance->Flags & VU1_DYNAMICLIGHT )
                UploadLights( (vu1_lightinfo*)pInstance->pLightInfo );
            
            KickCont( (u32)VU1_ENTRY_RIGID_XFORM_FAST );
            NextBuffer();
        }
        
        Sync();

        // Move onto next chunk
        nVerts   -= Num;
        m_Offset += Num;
    }
}

//==============================================================================

void vu1_interface::RenderClippedInstances( void )
{
    if( m_nClippedInstances == 0 )
        return;

    s32 nVerts = m_nVerts;
    m_Offset   = 0;

    vu1_instance* pStart = (vu1_instance*)(CLIPPED_SPAD_START);
    vu1_instance* pEnd   = pStart + m_nClippedInstances;

    SyncAll();
    SetBuffer( 1 );
    InitClipper();

    // Loop through all chunks
    while( nVerts > 0 )
    {
        // Set number of verts for this chunk
        s32 Num = MIN( nVerts, VU1_VERTS );

        // Render all instances
        for( vu1_instance* pInstance = pStart; pInstance < pEnd; pInstance++ )
        {
            UploadVertChunk( m_pVert, m_pUV, m_pNormal, Num );
            UploadVertCount( Num, pInstance->Flags, pInstance->UOffset, pInstance->VOffset, pInstance->Alpha );
            SetL2W( *pInstance->pL2W );
            UploadRGBChunk( pInstance->pRGB, Num );
            if ( pInstance->Flags & VU1_DYNAMICLIGHT )
                UploadLights( (vu1_lightinfo*)pInstance->pLightInfo );
            Kick( (u32)VU1_ENTRY_RIGID_XFORM_CLIPPED );
            Sync();
        }

        // Move onto next chunk
        nVerts   -= Num;
        m_Offset += Num;
    }

    NextBuffer();
}

//==============================================================================

void vu1_interface::SendConstMaterial( void ) const
{
    // activate the textures (must be done before the material is alloc'ed
    // so that it works with MFIFO!)
    s32 i;
    for( i = 0; i < m_nShadowProjectors; i++ )
    {
        // activate the texture and store out the registers
        ASSERT( m_ProjShadowTextures[i].GetPointer() );
        const xbitmap& ShadBitmap = m_ProjShadowTextures[i].GetPointer()->m_Bitmap;
        eng_PushGSContext( 1 );
        vram_Activate( ShadBitmap, 3 );
        eng_PopGSContext();
    }

    texture* pSpotlight = m_SpotlightTexture.GetPointer();
    if ( pSpotlight )
    {
        eng_PushGSContext( 1 );
        vram_Activate( pSpotlight->m_Bitmap, 3 );
        eng_PopGSContext();
    }

    // set up the dma tag and unpack information
    DLPtrAlloc( pConstMat, vu1_const_material );
    pConstMat->DMA.SetCont( sizeof(vu1_const_material)-sizeof(dmatag) );
    pConstMat->DMA.PAD[0] = VIF_SkipWrite( 1, 0 );
    pConstMat->DMA.PAD[1] = VIF_Unpack( VU1_CONST_MAT,
                                        VU1_CONST_MAT_SIZE,
                                        VIF_V4_32,
                                        FALSE,
                                        FALSE,
                                        TRUE );

    // copy in the light multiplier (used for emergency lighting)
    pConstMat->LightMultiplier.GetX() = (f32)m_FilterColor.R/128.0f;
    pConstMat->LightMultiplier.GetY() = (f32)m_FilterColor.G/128.0f;
    pConstMat->LightMultiplier.GetZ() = (f32)m_FilterColor.B/128.0f;
    pConstMat->LightMultiplier.GetW() = (f32)m_FilterColor.A/128.0f;

    // set up the shadow texture
    pConstMat->ShadowTex.Data = SCE_GS_SET_TEX0( VRAM_ZBUFFER_START/256,
                                                 VRAM_FRAME_BUFFER_WIDTH/64,
                                                 SCE_GS_PSMCT32,
                                                 9, 9, 1, 1, 0, 0, 0, 0, 0 );
    pConstMat->ShadowTex.Addr = SCE_GS_TEX0_2;

    // set up the spotlight texture
    if ( pSpotlight )
    {
        eng_PushGSContext( 1 );
        SetTextureRegisters ( pSpotlight->m_Bitmap, pConstMat->SpotlightTex, FALSE );
        eng_PopGSContext();
    }

    // set up clamping
    pConstMat->Clamp.Data = SCE_GS_SET_CLAMP( 1, 1, 0, 0, 0, 0 );
    pConstMat->Clamp.Addr = SCE_GS_CLAMP_2;

    // set up the frame registers
    u64 BackAddr  = eng_GetFrameBufferAddr(0) / 2048;
    u64 FrontAddr = eng_GetFrameBufferAddr(1) / 2048;
    pConstMat->FrameFrontAlpha.Data = SCE_GS_SET_FRAME( FrontAddr, VRAM_FRAME_BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0x00FFFFFF );
    pConstMat->FrameBackAlpha.Data  = SCE_GS_SET_FRAME( BackAddr,  VRAM_FRAME_BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0x00FFFFFF );
    pConstMat->FrameBackAll.Data    = SCE_GS_SET_FRAME( BackAddr,  VRAM_FRAME_BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0x00000000 );
    pConstMat->FrameBackRGB.Data    = SCE_GS_SET_FRAME( BackAddr,  VRAM_FRAME_BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0xFF000000 );
    pConstMat->FrameFrontAlpha.Addr = SCE_GS_FRAME_1;
    pConstMat->FrameBackAlpha.Addr  = SCE_GS_FRAME_2;
    pConstMat->FrameBackAll.Addr    = SCE_GS_FRAME_1;
    pConstMat->FrameBackRGB.Addr    = SCE_GS_FRAME_2;

    // set up the constant blend modes
    pConstMat->IntensityAlpha.Data = SCE_GS_SET_ALPHA( C_DST, C_ZERO, A_SRC, C_ZERO, 0 );
    pConstMat->SpotlightAlpha.Data = SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_DST, C_DST,  0 );
    pConstMat->IntensityAlpha.Addr = SCE_GS_ALPHA_2;
    pConstMat->SpotlightAlpha.Addr = SCE_GS_ALPHA_1;

    // set up the giftags
    pConstMat->Context1Gif.Build2     ( 4, 0, GIF_PRIM_TRIANGLESTRIP, GIF_FLAG_SMOOTHSHADE |
                                                                      GIF_FLAG_TEXTURE     |
                                                                      GIF_FLAG_ALPHA );
    pConstMat->Context2Gif.Build2     ( 4, 0, GIF_PRIM_TRIANGLESTRIP, GIF_FLAG_SMOOTHSHADE |
                                                                      GIF_FLAG_TEXTURE     |
                                                                      GIF_FLAG_ALPHA       |
                                                                      GIF_FLAG_CONTEXT );
    pConstMat->Context2GifNoTex.Build2( 4, 0, GIF_PRIM_TRIANGLESTRIP, GIF_FLAG_SMOOTHSHADE |
                                                                      GIF_FLAG_ALPHA       |
                                                                      GIF_FLAG_CONTEXT );

    pConstMat->Context1Gif.Reg     ( GIF_REG_NOP, GIF_REG_NOP, GIF_REG_NOP, GIF_REG_NOP );
    pConstMat->Context2Gif.Reg     ( GIF_REG_NOP, GIF_REG_NOP, GIF_REG_NOP, GIF_REG_NOP );
    pConstMat->Context2GifNoTex.Reg( GIF_REG_NOP, GIF_REG_NOP, GIF_REG_NOP, GIF_REG_NOP );

    // set up the spotlight position
    pConstMat->SpotlightPos    = m_SpotlightPos;
    pConstMat->SpotlightMatrix = m_SpotlightMatrix;

    // set up the projected shadow information
    for( i = 0; i < m_nShadowProjectors; i++ )
    {
        // activate the texture and store out the registers
        ASSERT( m_ProjShadowTextures[i].GetPointer() );
        const xbitmap& ShadBitmap = m_ProjShadowTextures[i].GetPointer()->m_Bitmap;
        eng_PushGSContext( 1 );
        SetTextureRegisters( ShadBitmap, pConstMat->ProjShadowTex[i], FALSE );
        eng_PopGSContext();

        // store the projection matrix
        pConstMat->ProjShadowMatrix[i] = *m_pProjShadowMatrices[i];
    }

    // set up a register load/sync gs giftag
    pConstMat->RegLoadGif.BuildRegLoad( 0, TRUE );

    // finally, we have internal storage for the microcode
    pConstMat->Storage.Zero();
}

//==============================================================================
//=============================================================================
// Begin/End functions
//=============================================================================
//=============================================================================

void vu1_interface::BuildInitPackets( void )
{
    // grab out the matrices we'll need
    const view*    pView = eng_GetView();
    const matrix4& W2S   = pView->GetW2S();
    const matrix4& W2C   = pView->GetW2C();
    const matrix4& C2S   = pView->GetC2S();
    matrix4 C2W = W2C;
    C2W.Invert();

    // Wait for VU1 and VIF1 to go idle
    DLPtrAlloc( pSync, dmatag );
    pSync->SetCont( 0 );
    pSync->PAD[0] = SCE_VIF1_SET_FLUSH( 0 );

    // save off clipping information
    vu1_clip_data* pClip = (vu1_clip_data*)smem_BufferAlloc( sizeof(vu1_clip_data) );
    m_pClipData = pClip;
    ASSERT( pClip );
    pClip->W2C = W2C;
    pClip->C2W = C2W;
    pClip->C2S = C2S;
    pClip->Planes[0].Mask = CLIP_FLAG_NEG_Z;    // Front
    pClip->Planes[1].Mask = CLIP_FLAG_POS_Z;    // Back
    pClip->Planes[2].Mask = CLIP_FLAG_POS_X;    // Right
    pClip->Planes[3].Mask = CLIP_FLAG_NEG_X;    // Left
    pClip->Planes[4].Mask = CLIP_FLAG_POS_Y;    // Top
    pClip->Planes[5].Mask = CLIP_FLAG_NEG_Y;    // Bottom
    pClip->Planes[6].Mask = 0;                  // Terminate
    pClip->Planes[0].JumpAddr = 0;
    pClip->Planes[1].JumpAddr = 2;
    pClip->Planes[2].JumpAddr = 4;
    pClip->Planes[3].JumpAddr = 6;
    pClip->Planes[4].JumpAddr = 8;
    pClip->Planes[5].JumpAddr = 10;
    pClip->Planes[6].JumpAddr = 0;

    // set up the initialization dma
    DLPtrAlloc( pPack, vu1_init );
    pPack->DMA.SetCont( sizeof( vu1_init ) - sizeof( dmatag ) );
    pPack->DMA.PAD[0] = VIF_SkipWrite( 1, 0 );
    pPack->DMA.PAD[1] = SCE_VIF1_SET_FLUSH( 0 );

    // set up the initialization data
    VIF_Mask( &pPack->VIF1[0], VIF_ASIS, VIF_ASIS, VIF_COL,  VIF_COL,
                               VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS,
                               VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS,
                               VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS );
    pPack->VIF1[2]  = SCE_VIF1_SET_STCOL( 0 );
    pPack->VIF1[3]  = 0x3f800000;    // ==1.0f in hex
    pPack->VIF1[4]  = 0x3f800000;    // ==1.0f in hex
    pPack->VIF1[5]  = 0x3f800000;    // ==1.0f in hex
    pPack->VIF1[6]  = 0x3f800000;    // ==1.0f in hex
    pPack->VIF1[7]  = SCE_VIF1_SET_STROW( 0 );

    // NOTE ABOUT STROW: This is set up for the neat int to float trick for
    // vertex compression. We bias the fixed-point value by a large int
    // using the VIF decompression code, then we subtract a large number
    // to re-normalize. The subtraction can be done by pre-translating the
    // bone matrices so the microcode loops require no additional instructions.
    // So we add the int: (((23-exp)+0x7f)<<23) + 0x400000
    // Then we sub the float: -1.5f*(2^(23-exp))   (That's power, not xor!)
    pPack->VIF1[8]  = (((23-4)+0x7f)<<23) + 0x400000;
    pPack->VIF1[9]  = (((23-4)+0x7f)<<23) + 0x400000;
    pPack->VIF1[10] = (((23-4)+0x7f)<<23) + 0x400000;
    pPack->VIF1[11] = (((23-4)+0x7f)<<23) + 0x400000;
    pPack->VIF1[12] = 0;
    pPack->VIF1[13] = 0;
    pPack->VIF1[14] = 0;
    pPack->VIF1[15] = VIF_Unpack( 0, 1 * 4, VIF_V4_32, TRUE, FALSE, TRUE ); 
    pPack->W2S     = W2S;
    pPack->Kick[0] = SCE_VIF1_SET_FLUSH( 0 );
    pPack->Kick[1] = SCE_VIF1_SET_MSCAL( MCODE_START_INIT/8, 0 );
    pPack->Kick[2] = SCE_VIF1_SET_FLUSH( 0 );
    pPack->Kick[3] = 0;
}

//=============================================================================

void vu1_interface::Begin( void )
{
    // activate the sync, material, and clipper microcode
    LoadSyncMCode();
    LoadInitMCode();
    LoadClipperMCode();
    LoadMaterialMCode( MCODE_MATERIALS );

    ASSERT( (m_InBegin = !m_InBegin) == TRUE );

    // start up the vu1 display list
    BuildInitPackets();
    SendConstMaterial();

    // Initialise all vu1 variables
    ASSERT( !m_InBeginInst );
    m_VUBuffer    = 0;

    // build the precompiled packets for faster dma packet building
    m_NormalBuffers[0].SetCont(0);
    m_NormalBuffers[0].PAD[0] = SCE_VIF1_SET_BASE( m_VUBuffers[0], 0 );
    m_NormalBuffers[0].PAD[1] = SCE_VIF1_SET_OFFSET( 0, 0 );
    m_NormalBuffers[1].SetCont(0);
    m_NormalBuffers[1].PAD[0] = SCE_VIF1_SET_BASE( m_VUBuffers[1], 0 );
    m_NormalBuffers[1].PAD[1] = SCE_VIF1_SET_OFFSET( 0, 0 );
    m_NormalBuffers[2].SetCont(0);
    m_NormalBuffers[2].PAD[0] = SCE_VIF1_SET_BASE( m_VUBuffers[2], 0 );
    m_NormalBuffers[2].PAD[1] = SCE_VIF1_SET_OFFSET( 0, 0 );

    // cache off some things to avoid thrashing the instruction cache to fetch
    // them from entropy or other areas
    const view* pView = eng_GetView();
    const matrix4& W2V( eng_GetView( )->GetW2V() );
    m_ViewEnvMapMatrix0.Set( W2V(0,0), W2V(0,1), W2V(2,0), W2V(2,1) );
    m_ViewEnvMapMatrix1.Set( W2V(1,0), W2V(1,1), 0.0f,     0.0f     );
    m_DistortionTex0 = SCE_GS_SET_TEX0( vram_GetBankAddr(0),
                                        VRAM_FRAME_BUFFER_WIDTH/128,
                                        SCE_GS_PSMCT32,
                                        8, 8,
                                        0, 1, 0, 0, 0, 0, 0 );;
    s32 L, T, R, B;
    pView->GetViewport( L, T, R, B );
    m_DistortionClamp = SCE_GS_SET_CLAMP( 2, 2, (L>>1), (R>>1)-1, (T>>1), (B>>1)-1 );

    NextBuffer();
}

//=============================================================================

void vu1_interface::End( void )
{
    ASSERT( (m_InBegin = !m_InBegin) == FALSE );

    // Wait for VU1 and VIF1 to go idle
    // Stall until the VU1, PATH1 and PATH2 are idle
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_FLUSHA( 0 );
}

//=============================================================================

void vu1_interface::ClearShadowCache( void )
{
    // clear out the shadow cache
    m_NextShadowEntry         = 0;
    m_ShadowProjectorCache[0] = 0xff;
    m_ShadowProjectorCache[1] = 0xff;
    m_ShadowProjectorCache[2] = 0xff;
    m_ShadowProjectorCache[3] = 0xff;
    m_ShadowProjectorCache[4] = 0xff;
    for ( s32 iProj = 0; iProj < render::MAX_SHADOW_CASTERS; iProj++ )
    {
        m_ShadowInfo[iProj].CacheIndex = 0xff;
    }
}

//=============================================================================

s32 vu1_interface::AddReceiverToShadowCache( s32 iProj )
{
    ASSERT( (iProj>=0) && (iProj < render::MAX_SHADOW_CASTERS) );

    s32 CacheIndex = m_ShadowInfo[iProj].CacheIndex;
    if ( CacheIndex != 0xff )
    {
        // the projector is already in the cache, nothing to do
        ASSERT( m_ShadowProjectorCache[CacheIndex] == (u8)iProj );
        return m_ShadowInfo[iProj].CacheIndex;
    }
    else
    {
        CacheIndex = m_NextShadowEntry;

        // remove the projector we are to replace
        s32 OldProj = m_ShadowProjectorCache[CacheIndex];
        if ( OldProj != 0xff )
        {
            ASSERT( m_ShadowInfo[OldProj].CacheIndex == CacheIndex );
            m_ShadowInfo[OldProj].CacheIndex = 0xff;
        }

        // move the new one into the cache
        m_ShadowInfo[iProj].CacheIndex     = CacheIndex;
        m_ShadowProjectorCache[CacheIndex] = iProj;

        // TODO: Do we really need this flush?!?!? probably not
        if ( m_NextShadowEntry == 0 )
        {
            Sync();
        }

        // move the data to the cache
        DLPtrAlloc( pDMA, dmatag );
        pDMA->SetRef( sizeof(vu1_shadow_receive), (u32)m_ShadowInfo[iProj].pShadReceive );
        pDMA->PAD[0] = VIF_SkipWrite( 1, 0 );
        pDMA->PAD[1] = VIF_Unpack( VU1_MATERIAL + 3 + (CacheIndex*5), 5, VIF_V4_32, TRUE, FALSE, TRUE );

        m_NextShadowEntry++;
        if ( m_NextShadowEntry == SHADOW_CACHE_SIZE )
            m_NextShadowEntry = 0;

        return CacheIndex;
    }
}

//=============================================================================

void vu1_interface::ClearShadowProjectors( void )
{
    // clear out the shadow projection info
    for ( s32 iProj = 0; iProj < render::MAX_SHADOW_CASTERS; iProj++ )
    {
        m_ShadowInfo[iProj].pShadCast    = NULL;
        m_ShadowInfo[iProj].pShadReceive = NULL;
    }
}

//=============================================================================

void vu1_interface::SetShadowCastInfo( s32            ProjectorIndex,
                            u64            Frame,
                            const matrix4& ProjMat )
{
    ASSERT( (ProjectorIndex>=0) && (ProjectorIndex < render::MAX_SHADOW_CASTERS) );
    ASSERT( m_ShadowInfo[ProjectorIndex].pShadCast == NULL );

    vu1_shadow_cast* pShadCast = (vu1_shadow_cast*)smem_BufferAlloc( sizeof(vu1_shadow_cast) );
    pShadCast->AlphaChannel.Data = Frame;
    pShadCast->AlphaChannel.Addr = SCE_GS_FRAME_1;
    pShadCast->ProjectionMatrix            = ProjMat;
    m_ShadowInfo[ProjectorIndex].pShadCast = pShadCast;
}

//=============================================================================

void vu1_interface::SetShadowReceiveInfo( s32            ProjectorIndex,
                               u64            Tex0,
                               const matrix4& ProjMat )
{
    ASSERT( (ProjectorIndex>=0) && (ProjectorIndex < render::MAX_SHADOW_CASTERS) );
    ASSERT( m_ShadowInfo[ProjectorIndex].pShadReceive == NULL );

    vu1_shadow_receive* pShadReceive          = (vu1_shadow_receive*)smem_BufferAlloc( sizeof(vu1_shadow_receive) );
    pShadReceive->ShadowTex.TEX0              = *((sceGsTex0*)&Tex0);
    pShadReceive->ShadowTex.TEX0Addr          = SCE_GS_TEX0_2;
    pShadReceive->ProjectionMatrix            = ProjMat;
    m_ShadowInfo[ProjectorIndex].pShadReceive = pShadReceive;
}

//=============================================================================

void vu1_interface::SetShadowCastMaterial( void )
{
    // upload the basic parts of the very simple shadow material
    DLPtrAlloc( pShadMat, vu1_shadow_material );
    pShadMat->DMA.SetCont( sizeof(vu1_shadow_material) - sizeof(dmatag) );
    pShadMat->DMA.PAD[0] = VIF_SkipWrite( 1, 0 );
    pShadMat->DMA.PAD[1] = VIF_Unpack( VU1_MATERIAL, 3, VIF_V4_32, TRUE, FALSE, TRUE );
    pShadMat->Flags.Set( 0.0f, 0.0f, 0.0f, 0.0f );
    pShadMat->PassOneGIF.Build2( 4, 0, GIF_PRIM_TRIANGLESTRIP, GIF_FLAG_SMOOTHSHADE );
    pShadMat->PassOneGIF.Reg( GIF_REG_NOP, GIF_REG_NOP, GIF_REG_RGBAQ, GIF_REG_XYZ2 );
    pShadMat->PassTwoGIF.BuildRegLoad( 0, TRUE );

    // upload a reg load giftag for state management
    pShadMat->VIF[0] = 0;
    pShadMat->VIF[1] = 0;
    pShadMat->VIF[2] = 0;
    pShadMat->VIF[3] = VIF_Unpack( VU1C_RegLoadGif, 1, VIF_V4_32, TRUE, FALSE, TRUE );
    pShadMat->RegLoadGIF.BuildRegLoad( 0, TRUE );
}

//=============================================================================

void vu1_interface::SetShadowReceiveMaterial( void )
{
    // upload the basic parts of the very simple shadow material
    DLPtrAlloc( pShadMat, vu1_shadow_material );
    pShadMat->DMA.SetCont( sizeof(vu1_shadow_material) - sizeof(dmatag) );
    pShadMat->DMA.PAD[0] = VIF_SkipWrite( 1, 0 );
    pShadMat->DMA.PAD[1] = VIF_Unpack( VU1_MATERIAL, 3, VIF_V4_32, TRUE, FALSE, TRUE );
    pShadMat->Flags.Set( 0.0f, 0.0f, 0.0f, 0.0f );
    pShadMat->PassOneGIF.Build2( 4, 0, GIF_PRIM_TRIANGLESTRIP, GIF_FLAG_ALPHA | GIF_FLAG_SMOOTHSHADE );
    pShadMat->PassOneGIF.Reg( GIF_REG_NOP, GIF_REG_RGBAQ, GIF_REG_NOP, GIF_REG_XYZ2 );
    pShadMat->PassTwoGIF.Build2( 4, 0, GIF_PRIM_TRIANGLESTRIP, GIF_FLAG_ALPHA | GIF_FLAG_SMOOTHSHADE | GIF_FLAG_TEXTURE | GIF_FLAG_CONTEXT );
    pShadMat->PassTwoGIF.Reg( GIF_REG_NOP, GIF_REG_ST, GIF_REG_RGBAQ, GIF_REG_XYZ2 );

    // upload a reg load giftag for state management
    pShadMat->VIF[0] = 0;
    pShadMat->VIF[1] = 0;
    pShadMat->VIF[2] = 0;
    pShadMat->VIF[3] = VIF_Unpack( VU1C_RegLoadGif, 1, VIF_V4_32, TRUE, FALSE, TRUE );
    pShadMat->RegLoadGIF.BuildRegLoad( 0, TRUE );
}

//=============================================================================

void vu1_interface::BeginShadows( void )
{
    ASSERT( (m_InShadowBegin = !m_InShadowBegin) == TRUE );

    // activate the sync, material, and clipper microcode
    LoadSyncMCode();
    LoadInitMCode();
    LoadClipperMCode();
    LoadMaterialMCode( MCODE_SHADRECEIVE_PASSES );
    //NOTE: The transform microcode will depend on whether we're doing casting
    //(skinned) or receiving (rigid).

    // Build the initialization packets
    BuildInitPackets();

    // Initialize all vu1 variables
    ASSERT( !m_InBeginInst );
    m_VUBuffer = 0;

    // build the precompiled packets for faster dma packet building
    m_NormalBuffers[0].SetCont(0);
    m_NormalBuffers[0].PAD[0] = SCE_VIF1_SET_BASE( m_VUBuffers[0], 0 );
    m_NormalBuffers[0].PAD[1] = SCE_VIF1_SET_OFFSET( 0, 0 );
    m_NormalBuffers[1].SetCont(0);
    m_NormalBuffers[1].PAD[0] = SCE_VIF1_SET_BASE( m_VUBuffers[1], 0 );
    m_NormalBuffers[1].PAD[1] = SCE_VIF1_SET_OFFSET( 0, 0 );
    m_NormalBuffers[2].SetCont(0);
    m_NormalBuffers[2].PAD[0] = SCE_VIF1_SET_BASE( m_VUBuffers[2], 0 );
    m_NormalBuffers[2].PAD[1] = SCE_VIF1_SET_OFFSET( 0, 0 );
    ClearShadowCache();

    NextBuffer();
}

//=============================================================================

void vu1_interface::EndShadows( void )
{
    ASSERT( (m_InShadowBegin = !m_InShadowBegin) == FALSE );

    // Wait for VU1 and VIF1 to go idle
    // Stall until the VU1, PATH1 and PATH2 are idle
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_FLUSHA( 0 );
}

//=============================================================================
//=============================================================================
// instances rendering routines
//=============================================================================
//=============================================================================

void vu1_interface::BeginInstance( const vector4* pVert, const s16* pUV, const s8* pNormal, s32 nVerts )
{
    ASSERT( m_InBegin == TRUE  );
    ASSERT( (m_InBeginInst = !m_InBeginInst) == TRUE );
    CONFIRM_ALIGNMENT( pVert   );
    CONFIRM_ALIGNMENT( pUV     );
    CONFIRM_ALIGNMENT( pNormal );

    // load up the rigid xform microcode
    LoadXFormMCode( MCODE_RIGID_XFORM );

    // lock scratchpad
    SPAD.Lock();

    m_pVert       = pVert;
    m_pUV         = pUV;
    m_pNormal     = pNormal;
    m_nVerts      = nVerts;
    
    m_nNonClipInstances = 0;
    m_nClippedInstances = 0;
}

//=============================================================================

void vu1_interface::EndInstance( void )
{
    ASSERT( (m_InBeginInst = !m_InBeginInst) == FALSE );

    RenderNonClipInstances();
    RenderClippedInstances();

    // unlock scratchpad
    SPAD.Unlock();
}

//=============================================================================

void vu1_interface::AddInstance( const u16*     pRGB,
                                 const matrix4& L2W,
                                 u8             UOffset,
                                 u8             VOffset,
                                 u32            Flags,
                                 s32            Alpha,
                                 void*          pLightInfo )
{
    ASSERT( m_InBeginInst );
    CONFIRM_ALIGNMENT( pRGB );

    vu1_instance* pInstance;
    
    if( Flags & VU1_CLIPPED )
    {
        if ( m_nClippedInstances >= MAX_CLIPPED_INSTANCES )
        {
            EndInstance();
            BeginInstance( m_pVert, m_pUV, m_pNormal, m_nVerts );
        }
        pInstance = (vu1_instance*)(CLIPPED_SPAD_START+m_nClippedInstances*sizeof(vu1_instance));

        m_nClippedInstances++;
    }
    else
    {
        if ( m_nNonClipInstances >= MAX_NONCLIPPED_INSTANCES )
        {
            EndInstance();
            BeginInstance( m_pVert, m_pUV, m_pNormal, m_nVerts );
        }
        pInstance = (vu1_instance*)(NONCLIPPED_SPAD_START+m_nNonClipInstances*sizeof(vu1_instance));

        m_nNonClipInstances++;
    }

    pInstance->pRGB       = pRGB;
    pInstance->pL2W       = &L2W;
    pInstance->UOffset    = UOffset;
    pInstance->VOffset    = VOffset;
    pInstance->Flags      = Flags;
    pInstance->ShadFlags  = 0;
    pInstance->pLightInfo = (byte*)pLightInfo;
    pInstance->Alpha      = (!(Flags&VU1_FADING_ALPHA)) || (Alpha==255) ? 128 : (u8)(Alpha>>1);
}

//=============================================================================

void vu1_interface::RenderRawStrips( s32               nVerts,
                                     const matrix4&    L2W,
                                     const vector4*    pPos,
                                     const s16*        pUV,
                                     const u32*        pColor )
{
    // load up the rigid xform microcode
    LoadXFormMCode( MCODE_RIGID_XFORM );

    SyncAll();
    SetBuffer( 1 );
    InitClipper();

    // loop through all chunks
    while ( nVerts > 0 )
    {
        // set number of verts for this chunk
        s32 Num = MIN( nVerts, VU1_VERTS );

        // render these verts
        UploadRawVertChunk( pPos, pUV, pColor, Num );
        UploadVertCount( Num, VU1_CLIPPED, 0, 0, 0x80 );
        SetL2W( L2W );
        Kick( (u32)VU1_ENTRY_RIGID_XFORM_CLIPPED );
        Sync();

        // move onto next chunk
        nVerts -= Num;
        pPos   += Num;
        pUV    += Num*2;
        pColor += Num;
    }

    NextBuffer();
    SyncAll();
}

//=============================================================================

void vu1_interface::Render3dSprites( s32              nSprites,
                                     f32              UniScale,
                                     const matrix4*   pL2W,
                                     const vector4*   pPositions,
                                     const vector2*   pRotScales,
                                     const u32*       pColors )
{
    // load up the sprite xform code
    LoadXFormMCode( MCODE_SPRITE_XFORM );

    // Make sure the data is aligned by moving the pointers back and dma'ing
    // some garbage if necessary. We can skip over that garbage data in
    // microcode.
    s32 nSpritesToSkip = 0;
    while( ((u32)pColors) & 0xf )
    {
        --pPositions;
        --pColors;
        --pRotScales;
        ++nSprites;
        ++nSpritesToSkip;
    }
    ASSERT( nSpritesToSkip < (VU1_VERTS/4) );   // make sure we're not skipping more than a buffer
    CONFIRM_ALIGNMENT( pPositions );
    CONFIRM_ALIGNMENT( pColors );
    CONFIRM_ALIGNMENT( pRotScales );

    // send in the v2w matrix and sprite coefficients
    DLPtrAlloc( pInit, sprite_init_coeff );
    pInit->DMACont.SetCont( sizeof(matrix4) );
    pInit->DMACont.PAD[0] = VIF_SkipWrite( 1, 0 );
    pInit->DMACont.PAD[1] = VIF_Unpack( VU1C_W2V0, 4, VIF_V4_32, FALSE, FALSE, TRUE );
    pInit->W2V = eng_GetView()->GetW2V();
    pInit->DMARef.SetRef( sizeof(f32)*16, (u32)s_SinCosCoeff );
    pInit->DMARef.PAD[0] = VIF_SkipWrite( 1, 0 );
    pInit->DMARef.PAD[1] = VIF_Unpack( VU1C_SinCos0, 4, VIF_V4_32, FALSE, FALSE, TRUE );

    // setup buffers for sprite render
    SyncAll();
    SetBuffer( 1 );
    InitClipper();

    // if there wasn't an l2w specified, then just use identity
    if( pL2W == NULL )
        pL2W = &m_IdentityL2W;

    // loop through all the sprites
    while( nSprites > 0 )
    {
        // set number of sprites for this chunk
        s32 Num = MIN( nSprites, VU1_VERTS/4 );

        // render these sprites
        UploadSpriteVertChunk( pPositions, pRotScales, pColors, Num );
        UploadSpriteVertCount( nSpritesToSkip, Num, UniScale );
        SetL2W( *pL2W );
        Kick( (u32)VU1_ENTRY_SPRITE_XFORM );
        Sync();

        // move to the next sprites
        nSpritesToSkip = 0;
        pPositions += Num;
        pRotScales += Num;
        pColors    += Num;
        nSprites   -= Num;
    }

    // finish off everything and sync up
    NextBuffer();
    SyncAll();
}

//=============================================================================

void vu1_interface::RenderVelocitySprites( s32            nSprites,
                                           f32            UniScale,
                                           const matrix4* pL2W,
                                           const matrix4* pVelMatrix,
                                           const vector4* pPositions,
                                           const vector4* pVelocities,
                                           const u32*     pColors )
{
    // load up the sprite xform code
    LoadXFormMCode( MCODE_SPRITE_XFORM );

    // Make sure the data is aligned by moving the pointers back and dma'ing
    // some garbage if necessary. We can skip over that garbage data in
    // microcode.
    s32 nSpritesToSkip = 0;
    while( ((u32)pColors) & 0xf )
    {
        --pPositions;
        --pVelocities;
        --pColors;
        ++nSprites;
        ++nSpritesToSkip;
    }
    ASSERT( nSpritesToSkip < (VU1_VERTS/4) );   // make sure we're not skipping more than a buffer
    CONFIRM_ALIGNMENT( pPositions );
    CONFIRM_ALIGNMENT( pVelocities );
    CONFIRM_ALIGNMENT( pColors );

    // send in the v2w matrix
    DLPtrAlloc( pInit, sprite_init );
    pInit->DMACont.SetCont( sizeof(matrix4) );
    pInit->DMACont.PAD[0] = VIF_SkipWrite( 1, 0 );
    pInit->DMACont.PAD[1] = VIF_Unpack( VU1C_W2V0, 4, VIF_V4_32, FALSE, FALSE, TRUE );
    pInit->W2V = eng_GetView()->GetW2V();

    // setup buffers for sprite render
    SyncAll();
    SetBuffer( 1 );
    InitClipper();

    // if there wasn't an l2w specified, then just use identity
    if( pL2W == NULL )
        pL2W = &m_IdentityL2W;

    // loop through all the sprites
    while( nSprites > 0 )
    {
        // set number of sprites for this chunk
        s32 Num = MIN( nSprites, VU1_VERTS/4 );

        // render these sprites
        UploadSpriteVertChunk( pPositions, pVelocities, pColors, Num );
        UploadSpriteVertCount( nSpritesToSkip, Num, UniScale );
        SetL2W( *pL2W );
        SetVelMatrix( *pVelMatrix );
        Kick( (u32)VU1_ENTRY_VEL_SPRITE_XFORM );
        Sync();

        // move to the next sprites
        nSpritesToSkip = 0;
        pPositions  += Num;
        pVelocities += Num;
        pColors     += Num;
        nSprites    -= Num;
    }

    // finish off everything and sync up
    NextBuffer();
    SyncAll();
}

//=============================================================================

void vu1_interface::RenderHeatHazeSprites( s32            nSprites,
                                           f32            UniScale,
                                           const matrix4* pL2W,
                                           const vector4* pPositions,
                                           const vector2* pRotScales,
                                           const u32*     pColors )
{
(void)nSprites;
(void)UniScale;
(void)pL2W;
(void)pPositions;
(void)pRotScales;
(void)pColors;
#if 0
    if( pL2W == NULL )
        pL2W = &m_IdentityL2W;
    const matrix4& V2W = eng_GetView()->GetV2W();
    const matrix4& W2V = eng_GetView()->GetW2V();
    matrix4 L2V = W2V * L2W;
    const matrix4& V2C = eng_GetView()->GetV2C();

    draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_NO_ZWRITE | DRAW_UV_CLAMP | DRAW_CULL_NONE );
    draw_SetL2W( V2W );
    gsreg_Begin();
    gsreg_Set( SCE_GS_TEX0_1, vram_GetBankAddr(0), VRAM_FRAME_BUFFER_WIDTH/128, SCE_GS_PSMCT32, 8, 8, 0, 1, 0, 0, 0, 0, 0 );
    gsreg_End();

    s32 i;
    for( i = 0; i < nSprites; i++ )
    {
        // calc the five points in view space
        f32 Sine, Cosine;
        vector3 Points[5];
        vector3 Points[0] = L2V * vector3( pPositions[i].GetX(), pPositions[i].GetY(), pPositions[i].GetZ() );
        
        /*
        vector3 Center( pPositions[i].GetX(), pPositions[i].GetY(), pPositions[i].GetZ() );
        Center = S2V * Center;

        // calc the four sprite corners
        vector3 Corners[4];
        f32 Sine, Cosine;
        x_sincos( -pRotScales[i].X, Sine, Cosine );

        vector3 v0( Cosine - Sine, Sine + Cosine, 0.0f );
        vector3 v1( Cosine + Sine, Sine - Cosine, 0.0f );
        Corners[0] = v0;
        Corners[1] = v1;
        Corners[2] = -v0;
        Corners[3] = -v1;
        for( j = 0; j < 4; j++ )
        {
        Corners[j].Scale( pRotScales[i].Y * UniScale );
        Corners[j] += Center;
        }

        // now render it through draw
        xcolor C( pColors[i]&0xff, (pColors[i]&0xff00)>>8, (pColors[i]&0xff0000)>>16, (pColors[i]&0xff000000)>>24 );
        C.R = (C.R==0x80) ? 255 : (C.R<<1);
        C.G = (C.G==0x80) ? 255 : (C.G<<1);
        C.B = (C.B==0x80) ? 255 : (C.B<<1);
        C.A = (C.A==0x80) ? 255 : (C.A<<1);
        draw_Color( C );
        draw_UV( 0.0f, 0.0f );  draw_Vertex( Corners[0] );
        draw_UV( 1.0f, 0.0f );  draw_Vertex( Corners[3] );
        draw_UV( 0.0f, 1.0f );  draw_Vertex( Corners[1] );
        draw_UV( 1.0f, 0.0f );  draw_Vertex( Corners[3] );
        draw_UV( 0.0f, 1.0f );  draw_Vertex( Corners[1] );
        draw_UV( 1.0f, 1.0f );  draw_Vertex( Corners[2] );
        */
    }

    draw_End();

#if 0
    // load up the sprite xform code
    LoadXFormMCode( MCODE_SPRITE_XFORM );

    // send in the v2w matrix and sprite coefficients
    struct sprite_init
    {
        dmatag      DMACont;
        matrix4     W2V;
        dmatag      DMARef;
    };
    sprite_init* pInit = DLStruct( sprite_init );
    pInit->DMACont.SetCont( sizeof(matrix4) );
    pInit->DMACont.PAD[0] = VIF_SkipWrite( 1, 0 );
    pInit->DMACont.PAD[1] = VIF_Unpack( VU1C_W2V0, 4, VIF_V4_32, FALSE, FALSE, TRUE );
    pInit->W2V = eng_GetView()->GetW2V();
    pInit->DMARef.SetRef( sizeof(f32)*16, (u32)s_SinCosCoeff );
    pInit->DMARef.PAD[0] = VIF_SkipWrite( 1, 0 );
    pInit->DMARef.PAD[1] = VIF_Unpack( VU1C_SinCos0, 4, VIF_V4_32, FALSE, FALSE, TRUE );

    // setup buffers for sprite render
    SyncAll();
    SetBuffer( 1 );
    InitClipper();

    // if there wasn't an l2w specified, then just use identity
    if( pL2W == NULL )
        pL2W = &m_IdentityL2W;

    // loop through all the sprites
    while( nSprites > 0 )
    {
        // set number of sprites for this chunk
        s32 Num = MIN( nSprites, VU1_VERTS/4 );

        // render these sprites
        UploadHeatHazeSpriteVertChunk( pPositions, pRotScales, pColors, Num );
        UploadSpriteVertCount( Num, UniScale );
        SetL2W( *pL2W );
        Kick( (u32)VU1_ENTRY_SPRITE_XFORM );
        Sync();

        // move to the next sprites
        pPositions += Num;
        pRotScales += Num;
        nSprites   -= Num;
    }

    // finish off everything and sync up
    NextBuffer();
    SyncAll();
#endif
#endif
}

//=============================================================================
//=============================================================================
// skin rendering routines
//=============================================================================
//=============================================================================

void vu1_interface::UploadSkinMatrix( const matrix4& Mat, s16 CacheIndex )
{
    CONFIRM_ALIGNMENT(&Mat);

    if( m_SkinFlags & render::INSTFLAG_CLIPPED )
    {
        DLPtrAlloc( pDma, dmatag );
        pDma->SetRef( sizeof(matrix4), (u32)&Mat );
        pDma->PAD[0] = VIF_SkipWrite( 1, 0 );
        pDma->PAD[1] = VIF_Unpack( VU1_SKIN_BONE_CACHE + (CacheIndex*4), 4, VIF_V4_32, TRUE, FALSE, TRUE );
    }
    else
    {
        DLPtrAlloc( pMat, mat_struct );
        pMat->DMARef.SetRef( sizeof(matrix4), (u32)&Mat );
        pMat->DMARef.PAD[0] = VIF_SkipWrite( 1, 0 );
        pMat->DMARef.PAD[1] = VIF_Unpack( VU1_SKIN_BONE_CACHE + (CacheIndex*4), 4, VIF_V4_32, TRUE, FALSE, TRUE );
        pMat->DMACont.SetCont( 0 );
        pMat->DMACont.PAD[0] = 0;
        pMat->DMACont.PAD[1] = SCE_VIF1_SET_MSCAL( (u32)(VU1_ENTRY_SKIN_SETUP_MATRIX + 2*CacheIndex), 0 );
    }
}

//=============================================================================

void vu1_interface::BeginSkinRenderNormal( void )
{
    ASSERT( m_InBegin     == TRUE );
    ASSERT( m_InBeginInst == FALSE );
    ASSERT( (m_InBeginSkin = !m_InBeginSkin) == TRUE );

    // load up the skinning microcode
    LoadXFormMCode( MCODE_SKIN_XFORM );

    // TODO: If this sync causes big stalls, try to figure a way around it.
    SyncAll();
    SetSkinBuffer(0);
    m_VUSkinBuffer = 1;
}

//=============================================================================

void vu1_interface::RenderSkinVertsNormal( s32            NVerts,
                                           const s16*     pUV,
                                           const s16*     pPos,
                                           const u8*      pBoneIndices,
                                           const u8*      pNormals,
                                           u32            MCodeAddress )
{
    ASSERT( m_InBeginSkin == TRUE );

    s32 nVerts = NVerts;

    // Loop through all chunks
    while ( nVerts > 0 )
    {
        // Set number of verts for this chunk
        s32 Num = MIN( nVerts, VU1_SKIN_VERTS );

        // Upload the verts, uvs, normals, and bone indices
        UploadVertCount( Num, m_SkinFlags, m_SkinUOffset, m_SkinVOffset, m_Alpha );
        SetL2W( m_IdentityL2W );
        UploadSkinVertChunk( pUV, pPos, pBoneIndices, pNormals, Num );
        UploadSkinLights( (vu1_lightinfo*)m_pSkinLightInfo );

        // kick it
        KickCont( MCodeAddress );   // this does the soft-skinning followed by a gs-sync
                                    // the extra sync/kick pair means its safe to move to the next buffer

        // move to the next buffer
        NextSkinBuffer();
        nVerts       -= Num;
        pUV          += Num*2;
        pPos         += Num*4;
        pBoneIndices += Num*4;
        pNormals     += Num*4;
    }

    ASSERT( nVerts == 0 );
}

//=============================================================================

void vu1_interface::EndSkinRenderNormal( void )
{
    ASSERT( (m_InBeginSkin = !m_InBeginSkin) == FALSE );

    // TODO: If this sync causes big stalls, try to figure a way around it.
    SyncAll();
    NextBuffer();    // force the BASE/OFFSET to get reset for normal rendering
}

//=============================================================================

void vu1_interface::BeginSkinRenderClipped( void )
{
    ASSERT( m_InBegin );
    ASSERT( (m_InBeginSkin = !m_InBeginSkin) == TRUE );

    // load up the skinning microcode
    LoadXFormMCode( MCODE_SKIN_XFORM );

    // TODO: If this sync causes big stalls, try to figure a way around it.
    SyncAll();
    SetSkinBuffer(1);
    InitSkinClipper();
}

//=============================================================================

void vu1_interface::RenderSkinVertsClipped( s32            NVerts,
                                            const s16*     pUV,
                                            const s16*     pPos,
                                            const u8*      pBoneIndices,
                                            const u8*      pNormals ) const
{
    ASSERT( m_InBeginSkin );

    s32 nVerts = NVerts;

    // Loop through all chunks
    while ( nVerts > 0 )
    {
        // Set number of verts for this chunk
        s32 Num = MIN( nVerts, VU1_SKIN_VERTS );

        // Upload the verts, uvs, normals, and bone indices
        UploadVertCount( Num, m_SkinFlags, m_SkinUOffset, m_SkinVOffset, m_Alpha );
        SetL2W( m_IdentityL2W );
        UploadSkinVertChunk( pUV, pPos, pBoneIndices, pNormals, Num );
        UploadSkinLights( (vu1_lightinfo*)m_pSkinLightInfo );

        // kick it
        Kick( (u32)VU1_ENTRY_SKIN_XFORM_CLIPPED );

        nVerts       -= Num;
        pUV          += Num*2;
        pPos         += Num*4;
        pBoneIndices += Num*4;
        pNormals     += Num*4;

        Sync();
    }
    
    ASSERT( nVerts == 0 );
}

//=============================================================================

void vu1_interface::EndSkinRenderClipped( void )
{
    ASSERT( (m_InBeginSkin = !m_InBeginSkin) == FALSE );

    // TODO: If this sync causes big stalls, try to figure a way around it.
    SyncAll();
    NextBuffer();    // force the BASE/OFFSET to get reset for normal rendering
}

//=============================================================================
//=============================================================================
// lighting functions
//=============================================================================
//=============================================================================

void vu1_interface::SetLight( void*          pLightInfo,
                              s32            iLight,
                              const matrix4& L2W,
                              const vector3& Pos,
                              f32            Radius,
                              xcolor         Col ) const
{
    ASSERT( (iLight>=0) && (iLight < MAX_LIGHTS) );

    // gather some useful info
    vu1_lightinfo* pLI = (vu1_lightinfo*)pLightInfo;
    vu1_light& Light = pLI->Lights[iLight];
    vector3 LPos     = Pos - L2W.GetTranslation();
    f32 radius_sqr   = Radius*Radius;
    if ( radius_sqr < 1.0f )
        radius_sqr = 1.0f;

    // set up the light in vu friendly format
    Light.X      = LPos.GetX()*L2W(0,0) + LPos.GetY()*L2W(0,1) + LPos.GetZ()*L2W(0,2);
    Light.Y      = LPos.GetX()*L2W(1,0) + LPos.GetY()*L2W(1,1) + LPos.GetZ()*L2W(1,2);
    Light.Z      = LPos.GetX()*L2W(2,0) + LPos.GetY()*L2W(2,1) + LPos.GetZ()*L2W(2,2);
    Light.Radius = 1.0f / radius_sqr;
    Light.R      = (f32)(Col.R);
    Light.G      = (f32)(Col.G);
    Light.B      = (f32)(Col.B);
}

//=============================================================================

void* vu1_interface::NewLightingSetup( void ) const
{
    // rigid and skinned can share the same structures
    vu1_lightinfo* pLightInfo = (vu1_lightinfo*)smem_BufferAlloc( sizeof(vu1_lightinfo) );

    // set up the mask and unpack for the lights
    pLightInfo->SkipWrite = VIF_SkipWrite( 1, 3 );
    VIF_Mask( pLightInfo->LightMask, VIF_BLOCKED, VIF_BLOCKED, VIF_ASIS, VIF_ASIS,
                                     VIF_BLOCKED, VIF_BLOCKED, VIF_ASIS, VIF_ASIS,
                                     VIF_BLOCKED, VIF_BLOCKED, VIF_ASIS, VIF_ASIS,
                                     VIF_BLOCKED, VIF_BLOCKED, VIF_ASIS, VIF_ASIS );
    pLightInfo->Unpack = VIF_Unpack( VU1_UV0, MAX_LIGHTS * 4, VIF_V4_32, FALSE, TRUE, FALSE );
   
    // clear out the lights
    x_memset( pLightInfo->Lights, 0, sizeof(vu1_light)*MAX_LIGHTS );

    // restore the mask used by the normal pipeline
    VIF_Mask( pLightInfo->RestoreMask, VIF_ASIS, VIF_ASIS, VIF_ROW,  VIF_ROW,
                                       VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS,
                                       VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS,
                                       VIF_ASIS, VIF_ASIS, VIF_ASIS, VIF_ASIS );

    // and finally, the padding set to VIF NOPs
    pLightInfo->Pad[0] = 0;
    pLightInfo->Pad[1] = 0;

    return pLightInfo;
}

//=============================================================================

void vu1_interface::SetSkinAmbient( void* pLightInfo, xcolor Ambient, u32 InstFlags ) const
{
    vu1_lightinfo* pLI = (vu1_lightinfo*)pLightInfo;

    // the fourth light is reserved for ambient
    pLI->Lights[3].R = Ambient.R;
    pLI->Lights[3].G = Ambient.G;
    pLI->Lights[3].B = Ambient.B;

    // Take filter light into consideration
    ApplyFilterLighting( pLI->Lights[3], InstFlags );
}

//=============================================================================

void vu1_interface::SetSkinLight( void*          pLightInfo,
                                  s32            iLight,
                                  const vector3& Dir,
                                  xcolor         Col,
                                  u32            InstFlags ) const
{
    ASSERT( (iLight >= 0) && (iLight < MAX_LIGHTS-1) );

    vu1_lightinfo* pLI = (vu1_lightinfo*)pLightInfo;

    pLI->Lights[iLight].R = Col.R;
    pLI->Lights[iLight].G = Col.G;
    pLI->Lights[iLight].B = Col.B;

    // Take filter light into consideration
    ApplyFilterLighting( pLI->Lights[iLight], InstFlags );

    // the direction is transposed and pre-multiplied by 32.0 to make vu1 happy
    switch ( iLight )
    {
    case 0:
        pLI->Lights[0].X = -Dir.GetX() * 32.0f;
        pLI->Lights[1].X = -Dir.GetY() * 32.0f;
        pLI->Lights[2].X = -Dir.GetZ() * 32.0f;
        break;
    case 1:
        pLI->Lights[0].Y = -Dir.GetX() * 32.0f;
        pLI->Lights[1].Y = -Dir.GetY() * 32.0f;
        pLI->Lights[2].Y = -Dir.GetZ() * 32.0f;
        break;
    case 2:
        pLI->Lights[0].Z = -Dir.GetX() * 32.0f;
        pLI->Lights[1].Z = -Dir.GetY() * 32.0f;
        pLI->Lights[2].Z = -Dir.GetZ() * 32.0f;
        break;
    default:
        ASSERT( FALSE );
        break;
    }
}

//=============================================================================
//=============================================================================
// shadow skins
//=============================================================================
//=============================================================================

void vu1_interface::BeginSkinShadow( s32 iProj )
{
    ASSERT( m_InShadowBegin == TRUE  );
    ASSERT( m_InBeginInst   == FALSE );
    ASSERT( (m_InBeginSkin = !m_InBeginSkin) == TRUE );

    // load up the skin shadow casting mcode
    LoadXFormMCode( MCODE_SHADCAST_XFORM );

    SyncAll();
    SetSkinBuffer(0);

    // move the cast matrix into vu
    DLPtrAlloc( pDMA, dmatag );
    pDMA->SetRef( sizeof(vu1_shadow_cast), (u32)m_ShadowInfo[iProj].pShadCast );
    pDMA->PAD[0] = VIF_SkipWrite( 1, 0 );
    pDMA->PAD[1] = VIF_Unpack( VU1_MATERIAL + 3, 5, VIF_V4_32, TRUE, FALSE, TRUE );
}

//=============================================================================

void vu1_interface::RenderSkinShadowTwoBones( s32            NVerts,
                                              const s16*     pPos,
                                              const u8*      pBoneIndices )
{
    ASSERT( m_InBeginSkin == TRUE );
    s32 nVerts = NVerts;
    NextSkinShadowCastBuffer();

    // Loop through all chunks
    while ( nVerts > 0 )
    {
        // Set number of verts for this chunk
        s32 Num = MIN( nVerts, VU1_SKIN_VERTS );

        // upload the verts, uvs, normals, and bone indices
        UploadShadowVertCount( Num, (m_SkinFlags&0xFF80), 0, 0, 0 );
        SetShadowL2W( m_IdentityL2W );
        UploadSkinVertChunk( pPos, pBoneIndices, Num );

        // kick it
        KickCont( (u32)VU1_ENTRY_SHAD_CAST_2BONES );    // the extra sync/kick pair means its safe to move to the next buffer

        // move to the next buffer
        NextSkinShadowCastBuffer();
        nVerts       -= Num;
        pPos         += Num*4;
        pBoneIndices += Num*4;
    }

    ASSERT( nVerts == 0 );
    Sync();
}

//=============================================================================

void vu1_interface::RenderSkinShadowOneBone( s32            NVerts,
                                             const s16*     pPos,
                                             const u8*      pBoneIndices )
{
    ASSERT( m_InBeginSkin == TRUE );
    s32 nVerts = NVerts;
    NextSkinShadowCastBuffer();

    // Loop through all chunks
    while ( nVerts > 0 )
    {
        // Set number of verts for this chunk
        s32 Num = MIN( nVerts, VU1_SKIN_VERTS );

        // upload the verts, uvs, normals, and bone indices
        UploadShadowVertCount( Num, (m_SkinFlags&0xFF80), 0, 0, 0 );
        SetShadowL2W( m_IdentityL2W );
        UploadSkinVertChunk( pPos, pBoneIndices, Num );

        // kick it
        KickCont( (u32)VU1_ENTRY_SHAD_CAST_1BONE ); // this does the soft-skinning followed by a gs-sync
                                                        // the extra sync/kick pair means its safe to move to the next buffer

        // move to the next buffer
        NextSkinShadowCastBuffer();
        nVerts       -= Num;
        pPos         += Num*4;
        pBoneIndices += Num*4;
    }

    ASSERT( nVerts == 0 );
    Sync();
}

//=============================================================================

void vu1_interface::EndSkinShadow( void )
{
    ASSERT( (m_InBeginSkin = !m_InBeginSkin) == FALSE );

    // TODO: If this sync causes big stalls, try to figure a way around it.
    SyncAll();
    NextBuffer();    // force the BASE/OFFSET to get reset for normal rendering
}

//=============================================================================

void vu1_interface::RenderShadNonClipInstances( void )
{
    if ( m_nNonClipInstances == 0 )
        return;

    s32 nVerts = m_nVerts;
    m_Offset   = 0;

    vu1_instance* pStart = (vu1_instance*)NONCLIPPED_SPAD_START;
    vu1_instance* pEnd   = pStart + m_nNonClipInstances;

    // Loop through all chunks
    while ( nVerts > 0 )
    {
        // Set number of verts for this chunk
        s32 Num = MIN( nVerts, VU1_VERTS );

        UploadShadReceiveVertChunk( m_pVert, m_pUV, Num );

        // Render all instances
        for ( vu1_instance* pInstance = pStart; pInstance < pEnd; pInstance++ )
        {
            // TODO: Figure out a better way to do this
            s32 CacheIndex = AddReceiverToShadowCache( pInstance->ShadFlags );

            UploadShadowVertCount( Num, pInstance->Flags, (1<<CacheIndex), pInstance->UOffset, pInstance->VOffset );
            SetShadowL2W( *pInstance->pL2W );
            
            KickCont( (u32)VU1_ENTRY_SHAD_RECEIVE_FAST );
            NextBuffer();
        }

        // TODO: If this stall takes too much, figure out another way
        Sync();

        // Move on to next chunk
        nVerts   -= Num;
        m_Offset += Num;
    }
}

//=============================================================================

void vu1_interface::RenderShadClippedInstances( void )
{
    if ( m_nClippedInstances == 0 )
        return;

    s32 nVerts = m_nVerts;
    m_Offset   = 0;

    vu1_instance* pStart = (vu1_instance*)(CLIPPED_SPAD_START);
    vu1_instance* pEnd   = pStart + m_nClippedInstances;

    SyncAll();
    SetBuffer( 1 );
    InitClipper();

    // Loop through all chunks
    while( nVerts > 0 )
    {
        // Set number of verts for this chunk
        s32 Num = MIN( nVerts, VU1_VERTS );

        // Render all instances
        for ( vu1_instance* pInstance = pStart; pInstance < pEnd; pInstance++ )
        {
            // TODO: Figure out a better way to do this
            s32 CacheIndex = AddReceiverToShadowCache( pInstance->ShadFlags );

            UploadShadReceiveVertChunk( m_pVert, m_pUV, Num );
            UploadShadowVertCount( Num, pInstance->Flags, 1<<CacheIndex, pInstance->UOffset, pInstance->VOffset );
            SetShadowL2W( *pInstance->pL2W );
            Kick( (u32)VU1_ENTRY_SHAD_RECEIVE_SLOW );
            Sync();
        }

        // Move onto next chunk
        nVerts   -= Num;
        m_Offset += Num;
    }

    NextBuffer();
}

//=============================================================================

void vu1_interface::BeginShadReceiveInstance( const vector4* pPos,
                                              const s16*     pUV,
                                              s32            nVerts )
{
    ASSERT( m_InShadowBegin );
    ASSERT( (m_InBeginInst = !m_InBeginInst) == TRUE );
    CONFIRM_ALIGNMENT( pPos    );
    CONFIRM_ALIGNMENT( pUV     );

    // load up the rigid shadow receiving mcode
    LoadXFormMCode( MCODE_SHADRECEIVE_XFORM );

    m_pVert       = pPos;
    m_pUV         = pUV;
    m_nVerts      = nVerts;
    
    m_nNonClipInstances = 0;
    m_nClippedInstances = 0;
}

//=============================================================================

void vu1_interface::AddShadReceiveInstance( const matrix4* pL2W,
                                            u8             UOffset,
                                            u8             VOffset,
                                            s32            Flags,
                                            s32            iProj   )
{
    ASSERT( m_InBeginInst == TRUE );

    vu1_instance* pInstance;
    if ( Flags & VU1_CLIPPED )
    {
        if ( m_nClippedInstances > MAX_CLIPPED_INSTANCES )
        {
            EndShadReceiveInstance();
            BeginShadReceiveInstance( m_pVert, m_pUV, m_nVerts );
        }
        pInstance = (vu1_instance*)(CLIPPED_SPAD_START+m_nClippedInstances*sizeof(vu1_instance));

        m_nClippedInstances++;
    }
    else
    {
        if ( m_nNonClipInstances > MAX_NONCLIPPED_INSTANCES )
        {
            EndShadReceiveInstance();
            BeginShadReceiveInstance( m_pVert, m_pUV, m_nVerts );
        }
        pInstance = (vu1_instance*)(NONCLIPPED_SPAD_START+m_nNonClipInstances*sizeof(vu1_instance));

        m_nNonClipInstances++;
    }

    pInstance->pRGB       = NULL;
    pInstance->pL2W       = pL2W;
    pInstance->UOffset    = UOffset;
    pInstance->VOffset    = VOffset;
    pInstance->Flags      = Flags & 0xFF80;  // Keep only the flags relevant to the VU
    pInstance->ShadFlags  = iProj;
    pInstance->pLightInfo = NULL;
}

//=============================================================================

void vu1_interface::EndShadReceiveInstance( void )
{
    ASSERT( (m_InBeginInst = !m_InBeginInst) == FALSE );

    RenderShadNonClipInstances();
    RenderShadClippedInstances();
}


