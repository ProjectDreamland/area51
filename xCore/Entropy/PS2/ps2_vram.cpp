//==============================================================================
// VRAM Manager
//
// The PS2 uses block address to access textures in VRAM. Unfortunately these
// blocks are not laid out in linear fashion, and the image format and size will
// determine how the blocks are used. (Read the GS User's Manual.) For our
// purposes, we will assume that a texture takes up "x" amount of blocks and
// not try to get fancy filling up holes. The vram manager itself can be kindof
// nasty to read (by-product of the PS2 being nasty hardware), but looking at
// a high-level point of view, here's what's going on...
//
// We will define a texture bank to be a range of vram blocks that are available
// for use. There must always be at least one bank, but there could be multiple
// ones in use. For example, bank 0 could be reserved for diffuse textures, and
// bank 1 could be reserved for environment maps, etc. Textures are added and
// removed from the banks in round-robin fashion using linked lists. (They are
// not actually removed, but must be thought of that way to avoid clashing.) We
// won't try to have any fancy schemes to keep textures in memory, cause that
// adds more CPU overhead that you can't afford, and VRAM is so small it won't
// buy you much anyway. The linked lists are circular, so all we need is a
// "current" pointer. Here's a diagram of what vram might look like...
//
//   /-------------------------------------------------------------------------\
//   |  Bank One       | Bank Two         | Bank Three       | Bank Four       |
//   |-------------------------------------------------------------------------|
//   |                 |                  |                  |                 |
//   |  Curr==1        |  Curr==10        | Curr==-1         | Curr==13        |
//   |                 |                  |                  |                 |
//   |     *           |  *               | *                |         *       |
//   |  3->1->2->0->9  |  10->14->1       | NULL             | 11->12->13      |
//   |  ^\_________/   |  ^\_____/        |                  | ^\______/       |
//   |                 |                  |                  |                 |
//   \-------------------------------------------------------------------------/
//
//==============================================================================

#include "ps2_vram.hpp"
#include "ps2_misc.hpp"

//==============================================================================
// Defines
//==============================================================================

#define VRAM_START_BLOCK            (VRAM_FREE_MEMORY_START/256)
#define VRAM_END_BLOCK              (16384)

#define NUM_DIMENSION_VARIATIONS    (49)    // 512x512 to 8x8 in all variations
#define NUM_BIT_DEPTHS              (4)     // 32, 16, 8, and 4-bit are supported
#define MAX_WIDTH                   (512)
#define MAX_HEIGHT                  (512)
#define MIN_WIDTH                   (8)
#define MIN_HEIGHT                  (8)
#define INITIAL_TEXTURE_CAPACITY    (768)
#define MAX_BANKS                   (8)

//==============================================================================
// Typedefs and structs
//==============================================================================

struct texture_ref
{
    enum
    {
        FLAG_IN_USE   = 0x01,
        FLAG_ACTIVE   = 0x02,
        FLAG_CLUT4BIT = 0x04,
        FLAG_CLUT8BIT = 0x08,
        FLAG_CLUTONLY = 0x10,
        FLAG_LOCKED   = 0x20,
    };

    u64     miptbp1;        // partially pre-calculated mip table register
    u64     miptbp2;        // partially pre-calculated mip table register
    u64     tex0;           // partially pre-calculated tex0 register
    u32     RefAddr0;       // context 0 ref back into display list for already loaded texture
    u32     RefAddr1;       // context 1 ref back into display list for already loaded texture
    u8      Flags;          // self-explanatory
    s8      Bank;           // which bank this texture is active in
    s8      nMips;          // # of mips this texture has
    u8      MipLoad;        // index into the array of common upload data for these dimensions
    void*   pData;          // ptr to the xbitmap or registered clut/texture data
    s16     Next;           // next texture active in this bank
    s16     Prev;           // prev texture active in this bank
    s16     StartAddr;      // start block for the texture
    s16     nBlocks;        // number of blocks this texture uses
    s16     MipOffsets[7];  // block offsets for each mip
    s16     ClutOffset;     // block offset for clut
};

//==============================================================================

struct common_upload_data
{
    u64     Bitbltbuf;      // partially pre-calculated bitbltbuf register
    u64     Trxreg;         // pre-calculated trxreg register
    giftag  Giftag;         // pre-calculated image giftag
    s32     NQWords;        // number of qwords we need to send
};

//==============================================================================

struct texture_bank
{
    s16     StartBlock;
    s16     EndBlock;
    s16     CurrentRef;
};

//==============================================================================

union upload_packet
{
    struct
    {
        dmatag  DMACont;
        giftag  GifTagReg;
        u64     BitBltBuf;
        u64     BitBltBufAddr;
        u64     TrxPos;
        u64     TrxPosAddr;
        u64     TrxReg;
        u64     TrxRegAddr;
        u64     TrxDir;
        u64     TrxDirAddr;
        giftag  GifTagImage;
        dmatag  DMARef;
    };

    u_long128   ULongArray[8];
};

//==============================================================================

struct upload_packet_ext
{
    dmatag  DMACont;
    giftag  GifTagImage;
    dmatag  DMARef;
};

//==============================================================================

union activate_packet
{
    struct
    {
        dmatag          DMA;
        giftag          GIF;
        u64             texflush;
        u64             texflushaddr;
        u64             tex1;
        u64             tex1addr;
        u64             tex0;
        u64             tex0addr;
        u64             miptbp1;
        u64             miptbp1addr;
        u64             miptbp2;
        u64             miptbp2addr;
    };

    u_long128   ULongArray[7];
};

//==============================================================================
// Data accessed directly by entropy
//==============================================================================
f32 VRAM_LogTable[256];
s32 VRAM_BytesLoaded = 0;

//==============================================================================
// Data
//==============================================================================

static upload_packet        s_BasicUploadPacket     PS2_ALIGNMENT(16);
static activate_packet      s_BasicActivatePacket   PS2_ALIGNMENT(16);

#ifdef X_ASSERT
static xbool                s_VramInitted = FALSE;
#endif
static s32                  s_VramStart           = VRAM_START_BLOCK;
static s32                  s_VramEnd             = VRAM_END_BLOCK;
static s32                  s_nTextureRefsAlloced = 0;
static texture_ref*         s_TextureRefs         = NULL;
//WARNING: The MipLoad index may not have enough bits if this grows any more...
static common_upload_data   s_CommonUploadData[NUM_DIMENSION_VARIATIONS*NUM_BIT_DEPTHS];
static s32                  s_nTextureBanks = 0;
static texture_bank         s_TextureBanks[MAX_BANKS];
static f32                  s_MipK[2] = { 0.0f, 0.0f };
//==============================================================================
// Data used for packing mips together (isn't the most efficient way to pack
// mips, but handles the general case pretty well and avoids writing an
// intricate texture packing routine--dimensions that waste blocks have been
// noted with parentheses)
//==============================================================================
static s16 s_nBlocksRequired[NUM_DIMENSION_VARIATIONS*NUM_BIT_DEPTHS] = 
{
//  32-bit
//  512x512     256x256     128x128     64x64       32x32       16x16       8x8
    32*128,     32*32,      32*8,       32*2,       16,         4,          1,
//  512x256     256x128     128x64      64x32       32x16       16x8        
    32*64,      32*16,      32*4,       32,         8,          2,
//  512x128     256x64      128x32      (64x16)     (32x8)
    32*32,      32*8,       32*2,       24,         6,
//  512x64      256x32      (128x16)    (64x8)
    32*16,      32*4,       32*1+24,    22,
//  512x32      (256x16)    (128x8)
    32*8,       32*3+24,    32*1+22,
//  (512x16)    (256x8)
    32*7+24,    32*3+22,
//  (512x8)
    32*7+22,
//  256x512     128x256     64x128      (32x64)     (16x32)     (8x16)
    32*64,      32*16,      32*4,       32*1+16,    12,         3,
//  128x512     64x256      (32x128)    (16x64)     (8x32)
    32*32,      32*8,       32*3+16,    32*1+12,    11,
//  64x512      (32x256)    (16x128)    (8x64)
    32*16,      32*7+16,    32*3+12,    32*1+11,
//  (32x512)    (16x256)    (8x128)
    32*15+16,   32*7+12,    32*3+11,
//  (16x512)    (8x256)
    32*15+12,   32*7+11,
//  (8x512)
    32*15+11,

//  16-bit
//  512x512     256x256     128x128     64x64       32x32       16x16       8x8
    32*64,      32*16,      32*4,       32,         8,          2,          1,
//  512x256     256x128     128x64      64x32       32x16       16x8
    32*32,      32*8,       32*2,       16,         4,          1,
//  512x128     256x64      (128x32)    (64x16)     (32x8)
    32*16,      32*4,       32*1+16,    12,         3,
//  512x64      (256x32)    (128x16)    (64x8)
    32*8,       32*3+16,    32*1+12,    11,
//  (512x32)    (256x16)    (128x8)
    32*7+16,    32*3+12,    32*1+11,
//  (512x16)    (256x8)
    32*7+12,    32*3+11,
//  (512x8)
    32*7+11,
//  256x512     128x256     64x128      (32x64)     (16*32)     8*16
    32*32,      32*8,       32*2,       24,         6,          2,
//  128x512     64x256      (32x128)    (16x64)     (8x32)
    32*16,      32*4,       32*1+24,    22,         6,
//  64x512      (32x256)    (16x128)    (8x64)
    32*8,       32*3+24,    32*1+22,    22,
//  (32x512)    (16x256)    (8x128)
    32*7+24,    32*3+22,    32*1+22,
//  (16x512)    (8x256)
    32*7+22,    32*3+22,
//  (8x512)
    32*7+22,

//  8-bit
//  512x512     256x256     128x128     64x64       32x32       16x16       8x8
    32*32,      32*8,       32*2,       16,         4,          1,          1,
//  512x256     256x128     128x64      64x32       32x16       16x8
    32*16,      32*4,       32*1,       8,          2,          1,
//  512x128     256x64      (128x32)    (64x16)     32x8
    32*8,       32*2,       24,         6,          2,
//  512x64      (256x32)    (128x16)    (64x8)
    32*4,       32*1+24,    22,         6,
//  (512x32)    (256x16)    (128x8)
    32*3+24,    32*1+22,    22,
//  (512x16)    (256x8)
    32*3+22,    32*1+22,
//  (512x8)
    32*3+22,
//  256x512     128x256     (64x128)    (32x64)     (16x32)     8x16
    32*16,      32*4,       32*1+16,    12,         3,          1,
//  128x512     (64x256)    (32x128)    (16x64)     (8x32)
    32*8,       32*3+16,    32*1+12,    11,         3,
//  (64x512)    (32x256)    (16x128)    (8x64)
    32*7+16,    32*3+12,    32*1+11,    11,
//  (32x512)    (16x256)    (8x128)
    32*7+12,    32*3+11,    32*1+11,
//  (16x512)    (8x256)
    32*7+11,    32*3+11,
//  (8x512)
    32*7+11,

//  4-bit
//  512x512     256x256     128x128     64x64       32x32       16x16       8x8
    32*16,      32*4,       32*1,       8,          2,          1,          1,
//  512x256     256x128     128x64      64x32       32x16       16x8
    32*8,       32*2,       16,         4,          1,          1,
//  512x128     (256x64)    (128x32)    (64x16)     32x8
    32*4,       32*1+16,    12,         3,          1,
//  (512x64)    (256x32)    (128x16)    (64x8)
    32*3+16,    32*1+12,    11,         3,
//  (512x32)    (256x16)    (128x8)
    32*3+12,    32*1+11,    11,
//  (512x16)    (256x8)
    32*3+11,    32*1+11,
//  (512x8)
    32*3+11,
//  256x512     128x256     (64x128)    (32x64)     16x32       8x16
    32*8,       32*2,       24,         6,          2,          1,
//  128x512     (64x256)    (32x128)    (16x64)     8x32,
    32*4,       32*1+24,    22,         6,          2,
//  (64x512)    (32x256)    (16x128)    (8x64)
    32*3+24,    32*1+22,    22,         6,
//  (32x512)    (16x256)    (8x128)
    32*3+22,    32*1+22,    22,
//  (16x512)    (8x256)
    32*3+22,    32*1+22,
//  (8x512)
    32*3+22
};

//==============================================================================
// Prototypes for doing x_section
//==============================================================================

static void DeActivate          ( s32                   VramId  )   X_SECTION(render_deferred);
static void UploadClut          ( texture_ref*          pTRef,
                                  const byte*           pClut   )   X_SECTION(render_deferred);
static void UploadTexture       ( texture_ref*          pTRef,
                                  const xbitmap&        Bitmap  )   X_SECTION(render_deferred);
static void BuildActivatePacket ( s32                   Context,
                                  DLPtr( pPacket, activate_packet ),
                                  const texture_ref*    pTRef   );
static void AddToBank           ( s32                   VramId,
                                  texture_ref*          pTRef,
                                  s32                   Bank    )   X_SECTION(render_deferred);
static void UploadTexture       ( s32                   VramId,
                                  s32                   Bank,
                                  const xbitmap*        pBitmap )   X_SECTION(render_deferred);
static void ActivateTexture     ( s32                   VramId, 
                                  s32                   Bank, 
                                  const xbitmap*        pBitmap );

//==============================================================================
// Internal functions
//==============================================================================

static
s32 CalcCommonUploadIndex( s32 Width, s32 Height, s32 BitDepth )
{
    // verify size
    ASSERT( (Width >=MIN_WIDTH ) && (Width <=MAX_WIDTH ) );
    ASSERT( (Height>=MIN_HEIGHT) && (Height<=MAX_HEIGHT) );

    // verify power of two
    ASSERT( ((Width-1)  & Width ) == 0 );
    ASSERT( ((Height-1) & Height) == 0 );

    // the upload data is laid out like this: (makes the mip access happy)
    // 512x512  256x256 128x128 64x64   32x32   16x16   8x8
    // 512x256  256x128 128x64  64x32   32x16   16x8
    // 512x128  256x64  128x32  64x16   32x8
    // 512x64   256x32  128x16  64x8
    // 512x32   256x16  128x8
    // 512x16   256x8
    // 512x8
    // 256x512  128x256 64x128  32x64   16x32   8x16
    // 128x512  64x256  32x128  16x64   8x32
    // 64x512   32x256  16x128  8x64
    // 32x512   16x256  8x128
    // 16x512   8x256
    // 8x512

    // which table?
    s32 Offset = 0;
    switch ( BitDepth )
    {
    default:    ASSERT( FALSE );
    case 32:    Offset += (NUM_DIMENSION_VARIATIONS*0); break;
    case 16:    Offset += (NUM_DIMENSION_VARIATIONS*1); break;
    case 8:     Offset += (NUM_DIMENSION_VARIATIONS*2); break;
    case 4:     Offset += (NUM_DIMENSION_VARIATIONS*3); break;
    }

    if ( Width != Height )
    {
        // rectangular texture...we use the triangular form
        Offset += 7;

        // if it's taller than it is wide, jump to the next triangle
        if ( Height > Width )
        {
            s32 Temp = Width;
            Width    = Height;
            Height   = Temp;
            Offset  += 6+5+4+3+2+1;
        }

        // the scale determines which row
        s32 Scale = Width / Height;
        switch ( Scale )
        {
        default:    ASSERT( FALSE );
        case 2:     Offset += 0;                break;
        case 4:     Offset += 6;                break;
        case 8:     Offset += (6+5);            break;
        case 16:    Offset += (6+5+4);          break;
        case 32:    Offset += (6+5+4+3);        break;
        case 64:    Offset += (6+5+4+3+2);      break;
        }
    }

    // now the offset just depends on the width
    switch ( Width )
    {
        default:    ASSERT( FALSE );
        case 512:   Offset += 0;                break;
        case 256:   Offset += 1;                break;
        case 128:   Offset += 2;                break;
        case 64:    Offset += 3;                break;
        case 32:    Offset += 4;                break;
        case 16:    Offset += 5;                break;
        case 8:     Offset += 6;                break;
    }

    ASSERT( (Offset>=0) && (Offset<(NUM_DIMENSION_VARIATIONS*NUM_BIT_DEPTHS)) );

    return Offset;
}

//==============================================================================

static
s32 GetGSFormat( s32 BitDepth )
{
    switch ( BitDepth )
    {
    default:    ASSERT( FALSE );
    case 32:    return SCE_GS_PSMCT32;
    case 16:    return SCE_GS_PSMCT16;
    case 8:     return SCE_GS_PSMT8;
    case 4:     return SCE_GS_PSMT4;
    }
}

//==============================================================================

static
s32 GetNewVramID( void )
{
    // return the first available slot
    for ( s32 i = 1; i < s_nTextureRefsAlloced; i++ )
    {
        if ( (s_TextureRefs[i].Flags & texture_ref::FLAG_IN_USE) == 0 )
        {
            return i;
        }
    }

    // no slots available? re-alloc time
    ASSERTS( FALSE, "Registration leak? Too many texture registrations." );
    const s32 kAdditionalTextures = 128;
    s_TextureRefs = (texture_ref*)x_realloc( s_TextureRefs,
                                              (s_nTextureRefsAlloced+kAdditionalTextures)*sizeof(texture_ref) );
    ASSERT( s_TextureRefs );
    x_memset( &s_TextureRefs[s_nTextureRefsAlloced], 0, kAdditionalTextures*sizeof(texture_ref) );
    s_nTextureRefsAlloced += kAdditionalTextures;
    return (s_nTextureRefsAlloced-kAdditionalTextures);
}

//==============================================================================

static
s32 PackTexture( s32 Width, s32 Height, s32 nMips, s32 BPP, s32 CommonUploadIndex, s16 MipOffsets[], s16& ClutOffset )
{
    // figure out where the mips should land
    s32 i;
    s32 nBlocksUsed = 0;
    for ( i = 0; i <= nMips; i++ )
    {
        MipOffsets[i] = nBlocksUsed;
        nBlocksUsed += s_nBlocksRequired[CommonUploadIndex];
        Width  >>= 1;
        Height >>= 1;
        CommonUploadIndex++;
    }

    // clear the other offsets
    for ( i = nMips+1; i <= 6; i++ )
    {   
        MipOffsets[i] = -1;
    }

    // figure out where the clut should land
    if ( BPP == 8 )
    {
        ClutOffset   = nBlocksUsed;
        nBlocksUsed += 4;
    }
    else
    if ( BPP == 4 )
    {
        ClutOffset   = nBlocksUsed;
        nBlocksUsed++; 
    }
    else
    {
        ClutOffset = -1;
    }

    return nBlocksUsed;
}

//==============================================================================

static
void DeActivate( s32 VramId )
{
    ASSERT( (VramId >= 1) && (VramId < s_nTextureRefsAlloced) );
    texture_ref* pTRef = &s_TextureRefs[VramId];
    
    ASSERT( pTRef->Flags & texture_ref::FLAG_IN_USE );
    ASSERT( pTRef->Flags & texture_ref::FLAG_ACTIVE );
    s32          Bank  = pTRef->Bank;
    
    ASSERT( (Bank>=0) && (Bank < s_nTextureBanks) );
    texture_bank* pBank = &s_TextureBanks[Bank];
    
    // remove this guy from the linked-list
    s32 Next = pTRef->Next;
    s32 Prev = pTRef->Prev;
    if ( Next == VramId )
    {
        // only one item in the list?
        ASSERT( Prev == VramId );
        pBank->CurrentRef = -1;
    }
    else
    {
        // fix up the bank information
        if ( pBank->CurrentRef == VramId )
            pBank->CurrentRef = Prev;

        // remove the texture from the bank's circular linked list
        // (this may cause a texture ref that links to itself, but
        // we can handle that case, so it's okay)
        s_TextureRefs[Next].Prev = pTRef->Prev;
        s_TextureRefs[Prev].Next = pTRef->Next;
    }

    // this texture is no longer active
    pTRef->Flags &= ~texture_ref::FLAG_ACTIVE;
}

//==============================================================================

static
void UploadClut( texture_ref* pTRef, const byte* pClut )
{
    DLPtrAlloc( pPacket, upload_packet );

    // copy the basic data over (using u128's to be efficient)
    u_long128* pSrc128 = (u_long128*)&s_BasicUploadPacket;
    pPacket->ULongArray[0] = pSrc128[0];
    pPacket->ULongArray[1] = pSrc128[1];
    pPacket->ULongArray[2] = pSrc128[2];
    pPacket->ULongArray[3] = pSrc128[3];
    pPacket->ULongArray[4] = pSrc128[4];
    pPacket->ULongArray[5] = pSrc128[5];
    //pPacket->ULongArray[6] = pSrc128[6];  // giftag will get copied anyway
    //pPacket->ULongArray[7] = pSrc128[7];  // dmatag will get built anyway

    // fill in the data specific to a clut
    if ( pTRef->Flags&texture_ref::FLAG_CLUT4BIT )
    {
        pPacket->BitBltBuf   = SCE_GS_SET_BITBLTBUF( 0, 0, 0, pTRef->ClutOffset+pTRef->StartAddr, 1, SCE_GS_PSMCT32 );
        pPacket->TrxReg      = SCE_GS_SET_TRXREG( 8, 2 );
        pPacket->GifTagImage.BuildImageLoad( 16*4, TRUE );
        pPacket->DMARef.SetRef( 16*4, (u32)pClut );
        pPacket->DMARef.MakeDirect( 16*4 );
        #ifdef X_DEBUG
        VRAM_BytesLoaded += 16*4;
        #endif
    }
    else
    {
        pPacket->BitBltBuf   = SCE_GS_SET_BITBLTBUF( 0, 0, 0, pTRef->ClutOffset+pTRef->StartAddr, 1, SCE_GS_PSMCT32 );
        pPacket->TrxReg      = SCE_GS_SET_TRXREG( 16, 16 );
        pPacket->GifTagImage.BuildImageLoad( 16*16*4, TRUE );
        pPacket->DMARef.SetRef( 256*4, (u32)pClut );
        pPacket->DMARef.MakeDirect( 256*4 );
        #ifdef X_DEBUG
        VRAM_BytesLoaded += 256*4;
        #endif
    }
}

//==============================================================================

static
void UploadTexture( texture_ref* pTRef, const xbitmap& Bitmap )
{
    // upload each of the mips
    for ( s32 i = 0; i <= pTRef->nMips; i++ )
    {
        DLPtrAlloc( pPacket, upload_packet );

        // copy the basic data over (using u128's to be efficient)
        u_long128* pSrc128 = (u_long128*)&s_BasicUploadPacket;
        pPacket->ULongArray[0] = pSrc128[0];
        pPacket->ULongArray[1] = pSrc128[1];
        pPacket->ULongArray[2] = pSrc128[2];
        pPacket->ULongArray[3] = pSrc128[3];
        pPacket->ULongArray[4] = pSrc128[4];
        pPacket->ULongArray[5] = pSrc128[5];
        //pPacket->ULongArray[6] = pSrc128[6];  // giftag will get copied anyway
        //pPacket->ULongArray[7] = pSrc128[7];  // dmatag will get built anyway

        // fill in the data specific to this mip
        common_upload_data* pCommonData = &s_CommonUploadData[pTRef->MipLoad+i];
        pPacket->BitBltBuf   = pCommonData->Bitbltbuf | SCE_GS_SET_BITBLTBUF( 0, 0, 0, pTRef->MipOffsets[i]+pTRef->StartAddr, 0, 0 );
        pPacket->TrxReg      = pCommonData->Trxreg;
        pPacket->GifTagImage = pCommonData->Giftag;
        s32 nQWordsLeft = pCommonData->NQWords;
        if ( nQWordsLeft <= 32767 )
        {
            pPacket->DMARef.SetRef( pCommonData->NQWords<<4, (u32)Bitmap.GetPixelData(i) );
            pPacket->DMARef.MakeDirect( pCommonData->NQWords<<4 );
        }
        else
        {
            // send the max amount initially
            u32 PixelData = (u32)Bitmap.GetPixelData(i);
            pPacket->DMARef.SetRef( 32767<<4, PixelData );
            pPacket->DMARef.MakeDirect( 32767<<4 );
            nQWordsLeft -= 32767;
            PixelData += 32767<<4;

            // keep sending image packets until we're done
            while ( nQWordsLeft )
            {
                s32 nQWordsToSend = MIN( 32767, nQWordsLeft );

                DLPtrAlloc( pExtPacket, upload_packet_ext );
                pExtPacket->DMACont.SetCont( sizeof(upload_packet_ext)-sizeof(dmatag)*2 );
                pExtPacket->DMACont.MakeDirect( sizeof(upload_packet_ext)-sizeof(dmatag)*2 );
                pExtPacket->GifTagImage.BuildImageLoad( nQWordsToSend<<4, TRUE );
                pExtPacket->DMARef.SetRef( nQWordsToSend<<4, PixelData );
                pExtPacket->DMARef.MakeDirect( nQWordsToSend<<4 );
                nQWordsLeft -= nQWordsToSend;
                PixelData   += nQWordsToSend<<4;
            }
        }

        #ifdef X_DEBUG
        VRAM_BytesLoaded += pCommonData->NQWords<<4;
        #endif
    }

    // upload the clut
    if ( (pTRef->Flags&texture_ref::FLAG_CLUT4BIT) || (pTRef->Flags&texture_ref::FLAG_CLUT8BIT) )
    {
        UploadClut( pTRef, Bitmap.GetClutData() );
    }
}

//==============================================================================

static
void BuildActivatePacket( s32 Context, DLPtr( pPacket, activate_packet ), const texture_ref* pTRef )
{
    // copy the basic data over (using u128's to be efficient)
    u_long128* pSrc128 = (u_long128*)&s_BasicActivatePacket;
    pPacket->ULongArray[0] = pSrc128[0];
    pPacket->ULongArray[1] = pSrc128[1];
    pPacket->ULongArray[2] = pSrc128[2];
    pPacket->ULongArray[3] = pSrc128[3];
    pPacket->ULongArray[4] = pSrc128[4];
    pPacket->ULongArray[5] = pSrc128[5];
    pPacket->ULongArray[6] = pSrc128[6];

    // set up the data specific to this texture (some of these registers were
    // partially pre-computed which is why we're doing all this business with
    // the or's)
    u64 tex0;
    if ( (pTRef->Flags&texture_ref::FLAG_CLUT4BIT) ||
         (pTRef->Flags&texture_ref::FLAG_CLUT8BIT) )
    {
        tex0 = pTRef->tex0 | SCE_GS_SET_TEX0( pTRef->MipOffsets[0]+pTRef->StartAddr,
                                              0, 0, 0, 0, 0, 0,
                                              pTRef->ClutOffset+pTRef->StartAddr,
                                              0, 0, 0, 0 );
    }
    else
    {
        tex0 = pTRef->tex0 | SCE_GS_SET_TEX0( pTRef->MipOffsets[0]+pTRef->StartAddr,
                                              0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0 );
    }
    pPacket->tex0    = tex0;
    pPacket->miptbp1 = pTRef->miptbp1;
    pPacket->miptbp2 = pTRef->miptbp2;
    if ( pTRef->nMips > 0 ) pPacket->miptbp1 |= ((u64)(pTRef->MipOffsets[1]+pTRef->StartAddr))<<0;
    if ( pTRef->nMips > 1 ) pPacket->miptbp1 |= ((u64)(pTRef->MipOffsets[2]+pTRef->StartAddr))<<20;
    if ( pTRef->nMips > 2 ) pPacket->miptbp1 |= ((u64)(pTRef->MipOffsets[3]+pTRef->StartAddr))<<40;
    if ( pTRef->nMips > 3 ) pPacket->miptbp2 |= ((u64)(pTRef->MipOffsets[4]+pTRef->StartAddr))<<0;
    if ( pTRef->nMips > 4 ) pPacket->miptbp2 |= ((u64)(pTRef->MipOffsets[5]+pTRef->StartAddr))<<20;
    if ( pTRef->nMips > 5 ) pPacket->miptbp2 |= ((u64)(pTRef->MipOffsets[6]+pTRef->StartAddr))<<40;
    if ( pTRef->nMips > 0 )
    {
        pPacket->tex1    = SCE_GS_SET_TEX1( 0, pTRef->nMips, 1, 4, 0, 0, (s32)(s_MipK[Context]*16.0f) );
    }
    else
    {
        pPacket->tex1    = SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 );
    }

    // are we on the other context?
    if ( Context == 1 )
    {
        pPacket->tex0addr++;
        pPacket->tex1addr++;
        pPacket->miptbp1addr++;
        pPacket->miptbp2addr++;
    }
}

//==============================================================================

static
void AddToBank( s32 VramId, texture_ref* pTRef, s32 Bank )
{
    // is there room for this texture?
    ASSERT( (Bank>=0) && (Bank<s_nTextureBanks) );
    texture_bank* pBank = &s_TextureBanks[Bank];
    ASSERT( pTRef->nBlocks <= (pBank->EndBlock-pBank->StartBlock) );

    // find some space for this item
    #ifdef X_ASSERT
    s32 LoopCount = 0;
    #endif
    do
    {
        ASSERT( ++LoopCount < 200 );
        if ( pBank->CurrentRef == -1 )
        {
            // easy case, the bank is completely empty
            pTRef->Next       = VramId;
            pTRef->Prev       = VramId;
            pTRef->StartAddr  = pBank->StartBlock;
            pBank->CurrentRef = VramId;
            return;
        }
        else
        {
            // figure out the insertion point (between current and next)
            texture_ref* pCurrRef = &s_TextureRefs[pBank->CurrentRef];
            texture_ref* pNextRef = &s_TextureRefs[pCurrRef->Next];

            // is there enough blocks to insert the texture?
            s16 StartBlock = pCurrRef->StartAddr+pCurrRef->nBlocks;
            s16 EndBlock   = pNextRef->StartAddr;
            if ( EndBlock < StartBlock )
            {
                // wrapping around the buffer...
                EndBlock = pBank->EndBlock;
                if ( pTRef->nBlocks > EndBlock-StartBlock )
                {
                    // won't fit at the end of the bank, what about at the beginning?
                    StartBlock = pBank->StartBlock;
                    EndBlock   = pNextRef->StartAddr;
                    ASSERT( EndBlock >= StartBlock );
                }
            }
            
            if ( pTRef->nBlocks <= EndBlock-StartBlock )
            {
                // we have enough room, insert this guy into our circular
                // linked-list
                pTRef->Next       = pCurrRef->Next;
                pTRef->Prev       = pBank->CurrentRef;
                pTRef->StartAddr  = StartBlock;
                pBank->CurrentRef = VramId;
                pCurrRef->Next    = VramId;
                pNextRef->Prev    = VramId;
                return;
            }
            else
            {
                // Locked?
                if (pNextRef->Flags & texture_ref::FLAG_LOCKED)
                {
                    // Skip over reference
                    pBank->CurrentRef = pCurrRef->Next ;
                }
                else
                {
                    // we don't have enough room, dump out the next texture
                    // to create some space
                    DeActivate( pCurrRef->Next );
                }
            }
        }
    } while ( 1 );
}

//==============================================================================

static
void UploadTexture( s32 VramId, s32 Bank, const xbitmap* pBitmap )
{
    ASSERT( (VramId >= 1) && (VramId < s_nTextureRefsAlloced) );

    texture_ref* pTRef = &s_TextureRefs[VramId];
    ASSERT( pTRef->Flags & texture_ref::FLAG_IN_USE );
    ASSERT( (pTRef->Flags & texture_ref::FLAG_CLUTONLY) == FALSE );

    if ( ((pTRef->Flags & texture_ref::FLAG_ACTIVE) == 0) ||
         (pTRef->Bank != Bank) )
    {
        // do we need to kick this texture out of another bank?
        if ( pTRef->Flags & texture_ref::FLAG_ACTIVE )
            DeActivate( VramId );

        // add it to the bank's linked list
        AddToBank( VramId, pTRef, Bank );

        // now that we have space reserved for this texture, upload it
        if ( (pBitmap) && ((pTRef->Flags & texture_ref::FLAG_LOCKED) == 0) )
            UploadTexture( pTRef, *pBitmap );

        // mark this texture as active (but we haven't sent the registers yet)
        pTRef->Bank      = Bank;
        pTRef->RefAddr0  = 0;
        pTRef->RefAddr1  = 0;
        pTRef->Flags    |= texture_ref::FLAG_ACTIVE;
    }
}

//==============================================================================

static
void ActivateTexture( s32 VramId, s32 Bank, const xbitmap* pBitmap )
{
    texture_ref* pTRef = &s_TextureRefs[VramId];
    ASSERT( pTRef->Flags & texture_ref::FLAG_IN_USE );
    ASSERT( (pTRef->Flags & texture_ref::FLAG_CLUTONLY) == FALSE );

    // upload the texture to vram
    UploadTexture( VramId, Bank, pBitmap );

    // set the texture registers
    s32 GsContext = eng_GetGSContext();
    if ( (GsContext == 0) && (pTRef->RefAddr0!=0) )
    {
        // easy out, we can ref back to the previous activation packet
        DLPtrAlloc( pTag, dmatag );
        pTag->SetRef( sizeof(activate_packet)-sizeof(dmatag), pTRef->RefAddr0 );
        pTag->MakeDirect( sizeof(activate_packet)-sizeof(dmatag) );
    }
    else
    if ( (GsContext == 1) && (pTRef->RefAddr1!=0) )
    {
        // easy out, we can ref back to the previous activation packet
        DLPtrAlloc( pTag, dmatag );
        pTag->SetRef( sizeof(activate_packet)-sizeof(dmatag), pTRef->RefAddr1 );
        pTag->MakeDirect( sizeof(activate_packet)-sizeof(dmatag) );
    }
    else
    {
        // we haven't activated this texture for this context before, we'll
        // need to build an activation packet
        DLPtrAlloc( pPacket, activate_packet );
        BuildActivatePacket( GsContext, pPacket, pTRef );
        
        // now we can set the ref address for future activations
        // NOTE: CAN'T DO THIS IF WE ARE USING MFIFO!!!
        //if ( GsContext == 0 )   pTRef->RefAddr0 = ((u32)pPacket)+16;    // +16 to skip the dma tag
        //else                    pTRef->RefAddr1 = ((u32)pPacket)+16;    // +16 to skip the dma tag
    }
}

//==============================================================================

static
void FlushBank( s32 Bank )
{
    ASSERT( (Bank>=0) && (Bank < s_nTextureBanks) );
    
    while ( s_TextureBanks[Bank].CurrentRef != -1 )
    {
        DeActivate( s_TextureBanks[Bank].CurrentRef );
    }
}

//==============================================================================

static
void FlushAllBanks( void )
{
    for ( s32 i = 0; i < s_nTextureBanks; i++ )
        vram_FlushBank( i );

    #ifdef X_DEBUG
    for ( s32 VramId = 1; VramId < s_nTextureRefsAlloced; VramId++ )
    {
        if ( s_TextureRefs[VramId].Flags & texture_ref::FLAG_IN_USE )
        {
            ASSERT( (s_TextureRefs[VramId].Flags & texture_ref::FLAG_ACTIVE) == 0 );
        }
    }
    #endif
}

//==============================================================================

static
void SetupDefaultBanks( void )
{
    s_nTextureBanks = 1;
    s_TextureBanks[0].CurrentRef = -1;
    s_TextureBanks[0].StartBlock = s_VramStart;
    s_TextureBanks[0].EndBlock   = s_VramEnd;
}

//==============================================================================
// Public functions
//==============================================================================

void vram_Init( void )
{
    MEMORY_OWNER( "vram_Init()" );

    // allocate an initial amount of textures we can handle
    s_TextureRefs = (texture_ref*)x_malloc( sizeof(texture_ref) * INITIAL_TEXTURE_CAPACITY );
    x_memset( s_TextureRefs, 0, sizeof(texture_ref) * INITIAL_TEXTURE_CAPACITY );
    s_nTextureRefsAlloced = INITIAL_TEXTURE_CAPACITY;
    ASSERT( s_TextureRefs );

    // initialize the common upload index data
    for ( s32 Height = MAX_HEIGHT; Height >= MIN_HEIGHT; Height >>= 1 )
    {
        for ( s32 Width = MAX_WIDTH; Width >= MIN_WIDTH; Width >>= 1 )
        {
            for ( s32 BitDepth = 32; BitDepth >= 4; BitDepth >>= 1 )
            {
                // which gs format?
                s32 PSM = GetGSFormat(BitDepth);

                // buffer width in gs terms?
                s32 BW;
                if ( (BitDepth == 32) || (BitDepth == 16) )
                    BW = MAX( 1, Width / 64 );
                else
                    BW = 2*MAX( 1, Width / 128 );

                // how many bytes is the image?
                s32 nBytes = Width*Height;
                switch ( BitDepth )
                {
                    default: ASSERT( FALSE );
                    case 32:    nBytes *= 4;    break;
                    case 16:    nBytes *= 2;    break;
                    case 8:     nBytes *= 1;    break;
                    case 4:     nBytes /= 2;    break;
                }

                // fill in the common data
                s32 Idx = CalcCommonUploadIndex( Width, Height, BitDepth );
                common_upload_data* pData = &s_CommonUploadData[Idx];
                pData->Bitbltbuf = SCE_GS_SET_BITBLTBUF( 0, BW, PSM, 0, BW, PSM );
                pData->Trxreg    = SCE_GS_SET_TRXREG   ( Width, Height );
                pData->NQWords   = (nBytes>>4);
                pData->Giftag.BuildImageLoad( MIN(nBytes, 32767<<4), TRUE );
            }
        }
    }

    // by default, we only have one main texture bank
    FlushAllBanks();
    SetupDefaultBanks();

    // pre-compute some upload packet data
    s_BasicUploadPacket.DMACont.SetCont( sizeof(upload_packet)-sizeof(dmatag)*2 );
    s_BasicUploadPacket.DMACont.MakeDirect( sizeof(upload_packet)-sizeof(dmatag)*2 );
    s_BasicUploadPacket.GifTagReg.BuildRegLoad( 4, FALSE );
    s_BasicUploadPacket.BitBltBuf     = 0;
    s_BasicUploadPacket.BitBltBufAddr = SCE_GS_BITBLTBUF;
    s_BasicUploadPacket.TrxPos        = SCE_GS_SET_TRXPOS( 0, 0, 0, 0, 0x00 );
    s_BasicUploadPacket.TrxPosAddr    = SCE_GS_TRXPOS;
    s_BasicUploadPacket.TrxReg        = 0;
    s_BasicUploadPacket.TrxRegAddr    = SCE_GS_TRXREG;
    s_BasicUploadPacket.TrxDir        = SCE_GS_SET_TRXDIR( 0 );
    s_BasicUploadPacket.TrxDirAddr    = SCE_GS_TRXDIR;
    s_BasicUploadPacket.GifTagImage.BuildImageLoad( 0, TRUE );
    s_BasicUploadPacket.DMARef.SetRef( 0, 0 );
    s_BasicUploadPacket.DMARef.MakeDirect( 0 );

    // pre-compute some activate packet data
    s_BasicActivatePacket.DMA.SetCont( sizeof(activate_packet)-sizeof(dmatag) );
    s_BasicActivatePacket.DMA.MakeDirect( sizeof(activate_packet)-sizeof(dmatag) );
    s_BasicActivatePacket.GIF.BuildRegLoad( 5, TRUE );
    s_BasicActivatePacket.texflush     = 0;
    s_BasicActivatePacket.texflushaddr = SCE_GS_TEXFLUSH;
    s_BasicActivatePacket.tex1         = 0;
    s_BasicActivatePacket.tex1addr     = SCE_GS_TEX1_1;
    s_BasicActivatePacket.tex0         = 0;
    s_BasicActivatePacket.tex0addr     = SCE_GS_TEX0_1;
    s_BasicActivatePacket.miptbp1      = 0;
    s_BasicActivatePacket.miptbp1addr  = SCE_GS_MIPTBP1_1;
    s_BasicActivatePacket.miptbp2      = 0;
    s_BasicActivatePacket.miptbp2addr  = SCE_GS_MIPTBP2_1;

    // Init log table
    VRAM_LogTable[0] = -127.0f;
    for( s32 i=1; i<256; i++ )
    {
        ieee32 F;
        F.s.sign = 0;
        F.s.exp  = 127;
        F.s.mant = i<<15;
        VRAM_LogTable[i] = x_log2(F.v) - 127.0f;
    }

    // done
    #ifdef X_ASSERT
    s_VramInitted = TRUE;
    #endif
}

//==============================================================================

void vram_Kill( void )
{
    ASSERT( s_VramInitted );

    // flush any textures
    vram_Flush();

    // free any memory
    if ( s_TextureRefs )
    {
        x_free( s_TextureRefs );
        s_TextureRefs        = 0;
        s_nTextureRefsAlloced = 0;
    }

    // done
    #ifdef X_ASSERT
    s_VramInitted = FALSE;
    #endif
}

//==============================================================================

s32 vram_AllocatePermanent( s32 nPages )
{
    ASSERTS( s_VramEnd == VRAM_END_BLOCK, "Only one permanent allocation allowed!" );

    // flush all the banks to make sure we're not corrupting textures
    vram_Flush();

    // reserve space for the user to manage on his own
    s_VramEnd -= (nPages * 32);

    // return the block address
    return s_VramEnd;
}

//==============================================================================

void vram_FreePermanent( void )
{
    s_VramEnd = VRAM_END_BLOCK;
}

//==============================================================================

s32 vram_GetPermanentArea( void )
{
    return s_VramEnd;
}

//==============================================================================

s32 vram_GetPermanentSize( void )
{
    s32 BlockSize = VRAM_END_BLOCK - s_VramEnd;
    return BlockSize / 32;
}

//==============================================================================

void vram_SetStartAddress( s32 BlockAddr )
{
    s_VramStart = BlockAddr;
}

//==============================================================================

s32 vram_RegisterLocked( s32 Width, s32 Height, s32 BPP )
{
    // Make sure there is no clut!
    ASSERT(BPP > 8) ;

    ASSERT( s_VramInitted );

    // find an open texture slot
    s32 VramID = GetNewVramID();

    // figure out the data needed to fill in the texture ref info
    s32 nMips  = 0;
    s32 PSM    = GetGSFormat( BPP );
    s32 LogW   = vram_GetLog2( Width  );
    s32 LogH   = vram_GetLog2( Height );
    s32 CUIdx  = CalcCommonUploadIndex( Width, Height, BPP );
    u16 Flags  = texture_ref::FLAG_IN_USE | texture_ref::FLAG_LOCKED |
                 ((BPP==4) ? texture_ref::FLAG_CLUT4BIT : 0) |
                 ((BPP==8) ? texture_ref::FLAG_CLUT8BIT : 0);

    s32 TBW0, TBW1, TBW2, TBW3, TBW4, TBW5, TBW6;
    switch ( BPP )
    {
    default:
    case 32:
    case 16:
        TBW0 = MAX(1,(Width>>0)/64);
        TBW1 = MAX(1,(Width>>1)/64);
        TBW2 = MAX(1,(Width>>2)/64);
        TBW3 = MAX(1,(Width>>3)/64);
        TBW4 = MAX(1,(Width>>4)/64);
        TBW5 = MAX(1,(Width>>5)/64);
        TBW6 = MAX(1,(Width>>6)/64);
        break;
    case 8:
    case 4:
        TBW0 = 2*MAX(1,(Width>>0)/128);
        TBW1 = 2*MAX(1,(Width>>1)/128);
        TBW2 = 2*MAX(1,(Width>>2)/128);
        TBW3 = 2*MAX(1,(Width>>3)/128);
        TBW4 = 2*MAX(1,(Width>>4)/128);
        TBW5 = 2*MAX(1,(Width>>5)/128);
        TBW6 = 2*MAX(1,(Width>>6)/128);
        break;
    }
    
    // fill in the texture ref information
    texture_ref* pTRef = &s_TextureRefs[VramID];
    pTRef->miptbp1     = SCE_GS_SET_MIPTBP1( 0, TBW1, 0, TBW2, 0, TBW3 );
    pTRef->miptbp2     = SCE_GS_SET_MIPTBP2( 0, TBW4, 0, TBW5, 0, TBW6 );
    pTRef->tex0        = SCE_GS_SET_TEX0( 0, TBW0, PSM, LogW, LogH, 1, 0x00, 0, SCE_GS_PSMCT32, 0, 0, 1 );
    pTRef->RefAddr0    = 0;
    pTRef->RefAddr1    = 0;
    pTRef->Flags       = Flags;
    pTRef->Bank        = -1;
    pTRef->nMips       = nMips;
    pTRef->MipLoad     = CUIdx;
    pTRef->pData       = NULL ;
    pTRef->Next        = -1;
    pTRef->Prev        = -1;
    pTRef->StartAddr   = -1;
    pTRef->nBlocks     = PackTexture( Width, Height, nMips, BPP, CUIdx, pTRef->MipOffsets, pTRef->ClutOffset );

    return VramID;
}

//==============================================================================

s32 vram_Register( const xbitmap& Bitmap )
{
    ASSERT( s_VramInitted );

    // make sure we're in a supported format
    #if defined(X_DEBUG) && defined(TARGET_DEV)
    xbitmap::format Format = Bitmap.GetFormat();
    ASSERT( (Format == xbitmap::FMT_32_ABGR_8888) ||
            (Format == xbitmap::FMT_32_UBGR_8888) ||
            (Format == xbitmap::FMT_16_ABGR_1555) ||
            (Format == xbitmap::FMT_16_UBGR_1555) ||
            (Format == xbitmap::FMT_P8_ABGR_8888) ||
            (Format == xbitmap::FMT_P8_UBGR_8888) ||
            (Format == xbitmap::FMT_P4_ABGR_8888) ||
            (Format == xbitmap::FMT_P4_UBGR_8888) );
    #endif

    // make sure we're not registering multiple times
    ASSERT( Bitmap.GetVRAMID() == 0 );
    if ( Bitmap.GetVRAMID() != 0 )
    {
        vram_Unregister( Bitmap );
    }

    // find an open texture slot
    s32 VramID = GetNewVramID();

    // figure out the data needed to fill in the texture ref info
    s32 Width  = Bitmap.GetWidth();
    s32 Height = Bitmap.GetHeight();
    s32 nMips  = Bitmap.GetNMips();
    s32 PSM    = GetGSFormat( Bitmap.GetBPP() );
    s32 LogW   = vram_GetLog2( Width  );
    s32 LogH   = vram_GetLog2( Height );
    s32 BPP    = Bitmap.GetBPP();
    s32 CUIdx  = CalcCommonUploadIndex( Width, Height, BPP );
    u16 Flags  = texture_ref::FLAG_IN_USE                    |
                 ((BPP==4) ? texture_ref::FLAG_CLUT4BIT : 0) |
                 ((BPP==8) ? texture_ref::FLAG_CLUT8BIT : 0);

    s32 TBW0, TBW1, TBW2, TBW3, TBW4, TBW5, TBW6;
    switch ( BPP )
    {
    default:
    case 32:
    case 16:
        TBW0 = MAX(1,(Width>>0)/64);
        TBW1 = MAX(1,(Width>>1)/64);
        TBW2 = MAX(1,(Width>>2)/64);
        TBW3 = MAX(1,(Width>>3)/64);
        TBW4 = MAX(1,(Width>>4)/64);
        TBW5 = MAX(1,(Width>>5)/64);
        TBW6 = MAX(1,(Width>>6)/64);
        break;
    case 8:
    case 4:
        TBW0 = 2*MAX(1,(Width>>0)/128);
        TBW1 = 2*MAX(1,(Width>>1)/128);
        TBW2 = 2*MAX(1,(Width>>2)/128);
        TBW3 = 2*MAX(1,(Width>>3)/128);
        TBW4 = 2*MAX(1,(Width>>4)/128);
        TBW5 = 2*MAX(1,(Width>>5)/128);
        TBW6 = 2*MAX(1,(Width>>6)/128);
        break;
    }
    
    // fill in the texture ref information
    texture_ref* pTRef = &s_TextureRefs[VramID];
    pTRef->miptbp1     = SCE_GS_SET_MIPTBP1( 0, TBW1, 0, TBW2, 0, TBW3 );
    pTRef->miptbp2     = SCE_GS_SET_MIPTBP2( 0, TBW4, 0, TBW5, 0, TBW6 );
    pTRef->tex0        = SCE_GS_SET_TEX0( 0, TBW0, PSM, LogW, LogH, 1, 0x00, 0, SCE_GS_PSMCT32, 0, 0, 1 );
    pTRef->RefAddr0    = 0;
    pTRef->RefAddr1    = 0;
    pTRef->Flags       = Flags;
    pTRef->Bank        = -1;
    pTRef->nMips       = nMips;
    pTRef->MipLoad     = CUIdx;
    pTRef->pData       = (void*)&Bitmap;
    pTRef->Next        = -1;
    pTRef->Prev        = -1;
    pTRef->StartAddr   = -1;
    pTRef->nBlocks     = PackTexture( Width, Height, nMips, BPP, CUIdx, pTRef->MipOffsets, pTRef->ClutOffset );

    // done
    Bitmap.SetVRAMID( VramID );

    return VramID;
}

//==============================================================================

s32 vram_RegisterClut( const byte* pClut, s32 nColors )
{
    ASSERT( s_VramInitted );
    ASSERT( (nColors==256) || (nColors==16) );
    // find an open texture slot
    s32 VramID = GetNewVramID();

    // figure out the data needed to fill in the texture ref info
    s32 BPP    = (nColors==256) ? 8 : 4;
    u16 Flags  = texture_ref::FLAG_IN_USE                    |
                 texture_ref::FLAG_CLUTONLY                  |
                 ((BPP==4) ? texture_ref::FLAG_CLUT4BIT : 0) |
                 ((BPP==8) ? texture_ref::FLAG_CLUT8BIT : 0);

    // fill in the texture ref information
    texture_ref* pTRef = &s_TextureRefs[VramID];
    pTRef->miptbp1     = 0;
    pTRef->miptbp2     = 0;
    pTRef->tex0        = 0;
    pTRef->RefAddr0    = 0;
    pTRef->RefAddr1    = 0;
    pTRef->Flags       = Flags;
    pTRef->Bank        = -1;
    pTRef->nMips       = 0;
    pTRef->MipLoad     = 0;
    pTRef->pData       = (void*)pClut;
    pTRef->Next        = -1;
    pTRef->Prev        = -1;
    pTRef->StartAddr   = -1;
    pTRef->nBlocks     = (BPP==8) ? 4 : 1;

    // done
    return VramID;
}

//==============================================================================

void vram_Unregister( s32 VramId )
{
    ASSERT( s_VramInitted );

    ASSERT( (VramId >= 1) && (VramId < s_nTextureRefsAlloced) );
    
    texture_ref* pTRef = &s_TextureRefs[VramId];
    ASSERT( pTRef->Flags & texture_ref::FLAG_IN_USE );

    // if the texture is active, de-activate it
    if ( pTRef->Flags & texture_ref::FLAG_ACTIVE )
    {
        DeActivate( VramId );
    }

    // clear the in-use flag
    pTRef->Flags = 0;
}

//==============================================================================

void vram_Unregister( const xbitmap& Bitmap )
{
    // Unregister
    vram_Unregister(Bitmap.GetVRAMID()) ;

    // clear the vram id
    Bitmap.SetVRAMID( 0 );
}

//==============================================================================

void vram_UnRegisterClut( s32 Handle )
{
    ASSERT( s_VramInitted );
    
    ASSERT( (Handle >= 1) && (Handle < s_nTextureRefsAlloced) );
    texture_ref* pTRef = &s_TextureRefs[Handle];
    ASSERT( pTRef->Flags & texture_ref::FLAG_IN_USE );
    ASSERT( pTRef->Flags & texture_ref::FLAG_CLUTONLY );

    // if the clut is active, de-activate it
    if ( pTRef->Flags & texture_ref::FLAG_ACTIVE )
    {
        DeActivate( Handle );
    }

    // clear the in-use flag
    pTRef->Flags = 0;
}

//==============================================================================

void vram_FlushBank( s32 Bank )
{
    ASSERT( (Bank>=0) && (Bank<s_nTextureBanks) );
    FlushBank(Bank);
}

//==============================================================================

void vram_Flush( void )
{
    ASSERT( s_VramInitted );

    FlushAllBanks();
    SetupDefaultBanks();
}

//==============================================================================

void vram_Activate( s32 VramId )
{
    ASSERT( s_VramInitted );

    ActivateTexture( VramId, 0, NULL );
}

//==============================================================================

void vram_Activate( s32 VramId, s32 Bank )
{
    ASSERT( s_VramInitted );

    ActivateTexture( VramId, Bank, NULL );
}

//==============================================================================

void vram_Activate( const xbitmap& Bitmap )
{
    ASSERT( s_VramInitted );

    ActivateTexture( Bitmap.GetVRAMID(), 0, &Bitmap ) ;
}

//==============================================================================

void vram_Upload( const xbitmap& Bitmap, s32 Bank )
{
    ASSERT( s_VramInitted );
    ASSERT( (Bank>=0) && (Bank<s_nTextureBanks) );

    UploadTexture( Bitmap.GetVRAMID(), Bank, &Bitmap );
}

//==============================================================================

void vram_Activate( const xbitmap& Bitmap, s32 Bank )
{
    ASSERT( s_VramInitted );
    ASSERT( (Bank>=0) && (Bank<s_nTextureBanks) );

    ActivateTexture( Bitmap.GetVRAMID(), Bank, &Bitmap );
}

//==============================================================================

s32 vram_AllocateBank( s32 nPages )
{
    ASSERT( s_VramInitted );

    // flush the default bank before stealing vram from it
    vram_FlushBank( 0 );

    // which bank do we assign?
    ASSERT( s_nTextureBanks < MAX_BANKS );
    s32 AllocedBank = s_nTextureBanks;
    s_nTextureBanks++;

    // steal the ram from the bottom of the default bank
    s32 nBlocks = nPages*32;
    texture_bank* pDefault = &s_TextureBanks[0];
    texture_bank* pNew     = &s_TextureBanks[AllocedBank];
    ASSERT( nBlocks < (pDefault->EndBlock - pDefault->StartBlock) );
    pNew->StartBlock     = pDefault->StartBlock;
    pNew->EndBlock       = pDefault->StartBlock + nBlocks;
    pDefault->StartBlock = pNew->EndBlock;

    // and there are no textures active in the new bank
    pNew->CurrentRef = -1;

    // done
    return AllocedBank;
}

//==============================================================================

s32 vram_GetBankAddr( s32 Bank )
{
    ASSERT( (Bank>=0) && (Bank<s_nTextureBanks) );
    return s_TextureBanks[Bank].StartBlock;
}

//==============================================================================

void vram_SetMipK( f32 MipK )
{
    s32 C = eng_GetGSContext();
    s_MipK[C] = MipK;
}

//==============================================================================

void vram_Activate( void )
{
    ASSERT( s_VramInitted );
}

//==============================================================================

s32 vram_GetLog2( s32 Value )
{
    // this function only works for small ranges that are power of two already
    ASSERT( (Value>=8) && (Value<=512) );
    ASSERT( ((Value-1) & Value) == 0 );

    switch ( Value )
    {
    default:    ASSERT( FALSE );
    case 512:   return 9;
    case 256:   return 8;
    case 128:   return 7;
    case 64:    return 6;
    case 32:    return 5;
    case 16:    return 4;
    case 8:     return 3;
    }
}

//==============================================================================

void vram_LoadClut( s32 Handle, s32 Bank )
{
    ASSERT( (Handle>=1) && (Handle<s_nTextureRefsAlloced) );
    
    texture_ref* pTRef = &s_TextureRefs[Handle];
    ASSERT( pTRef->Flags & texture_ref::FLAG_IN_USE );
    ASSERT( pTRef->Flags & texture_ref::FLAG_CLUTONLY );

    // easy out? is the clut already active?
    if ( (pTRef->Flags & texture_ref::FLAG_ACTIVE) &&
         (pTRef->Bank == Bank) )
    {
        return;
    }

    // de-activate it if the clut is in another bank
    if ( pTRef->Flags & texture_ref::FLAG_ACTIVE )
    {
        DeActivate( Handle );
    }

    // add it to a bank's linked list
    AddToBank( Handle, pTRef, Bank );

    // upload the clut
    UploadClut( pTRef, (const byte*)pTRef->pData );

    // mark this clut as active
    pTRef->Bank      = Bank;
    pTRef->RefAddr0  = 0;
    pTRef->RefAddr1  = 0;
    pTRef->Flags    |= texture_ref::FLAG_ACTIVE;

}

//==============================================================================

s32 vram_GetClutBaseAddr( const xbitmap& Bitmap )
{
    ASSERT( s_VramInitted );

    s32 VramId = Bitmap.GetVRAMID();
    ASSERT( (VramId >= 1) && (VramId < s_nTextureRefsAlloced) );

    texture_ref* pTRef = &s_TextureRefs[VramId];
    ASSERT( pTRef->Flags & texture_ref::FLAG_IN_USE );
    ASSERT( pTRef->Flags & texture_ref::FLAG_ACTIVE );
    ASSERT( (pTRef->Flags & texture_ref::FLAG_CLUTONLY) == FALSE );

    return (pTRef->StartAddr+pTRef->ClutOffset);
}

//==============================================================================

s32  vram_GetClutBaseAddr( s32 Handle )
{
    ASSERT( s_VramInitted );

    texture_ref* pTRef = &s_TextureRefs[Handle];
    ASSERT( pTRef->Flags & texture_ref::FLAG_IN_USE );
    ASSERT( pTRef->Flags & texture_ref::FLAG_CLUTONLY );
    ASSERT( pTRef->Flags & texture_ref::FLAG_ACTIVE );

    return pTRef->StartAddr;
}

//==============================================================================

s32  vram_GetPixelBaseAddr( s32 VramId )
{
    ASSERT( s_VramInitted );

    texture_ref* pTRef = &s_TextureRefs[VramId];
    ASSERT( pTRef->Flags & texture_ref::FLAG_IN_USE );
    ASSERT( pTRef->Flags & texture_ref::FLAG_ACTIVE );
    ASSERT( pTRef->Flags & texture_ref::FLAG_LOCKED );

    return pTRef->StartAddr;
}

//==============================================================================

void vram_LoadTextureImage( s32 Width, s32 Height,
                            s32 PageWidth,
                            const byte* pPixel, s32 PixelFormat,
                            s32 VRAMPixelAddr )
{
    ASSERT( s_VramInitted );

    DLPtrAlloc( pPacket, upload_packet );

    // copy the basic data over (using u128's to be efficient)
    u_long128* pSrc128 = (u_long128*)&s_BasicUploadPacket;
    pPacket->ULongArray[0] = pSrc128[0];
    pPacket->ULongArray[1] = pSrc128[1];
    pPacket->ULongArray[2] = pSrc128[2];
    pPacket->ULongArray[3] = pSrc128[3];
    pPacket->ULongArray[4] = pSrc128[4];
    pPacket->ULongArray[5] = pSrc128[5];
    //pPacket->ULongArray[6] = pSrc128[6];  // giftag will get copied anyway
    //pPacket->ULongArray[7] = pSrc128[7];  // dmatag will get built anyway

    // fill in the data specific to this upload
    s32 nBytes = Width*Height;
    switch( PixelFormat )
    {
    default:               ASSERT( FALSE );
    case SCE_GS_PSMCT32:   nBytes   <<= 2;  break;
    case SCE_GS_PSMCT16:   nBytes   <<= 1;  break;
    case SCE_GS_PSMT8H:
    case SCE_GS_PSMT8:     nBytes   <<= 0;  break;
    case SCE_GS_PSMT4:     nBytes   >>= 1;  break;
    }
    ASSERT( (nBytes&0xf) == 0 );
    
    pPacket->BitBltBuf   = SCE_GS_SET_BITBLTBUF( VRAMPixelAddr, PageWidth, PixelFormat, VRAMPixelAddr, PageWidth, PixelFormat );
    pPacket->TrxReg      = SCE_GS_SET_TRXREG( Width, Height );
    pPacket->GifTagImage.BuildImageLoad( MIN(nBytes,32767<<4), TRUE );

    s32 nQWordsLeft = nBytes/16;
    if ( nQWordsLeft <= 32767 )
    {
        pPacket->DMARef.SetRef( nQWordsLeft<<4, (u32)pPixel );
        pPacket->DMARef.MakeDirect( nQWordsLeft<<4 );
    }
    else
    {
        // send the max amount initially
        u32 PixelData = (u32)pPixel;
        pPacket->DMARef.SetRef( 32767<<4, PixelData );
        pPacket->DMARef.MakeDirect( 32767<<4 );
        nQWordsLeft -= 32767;
        PixelData += 32767<<4;

        // keep sending image packets until we're done
        while ( nQWordsLeft )
        {
            s32 nQWordsToSend = MIN( 32767, nQWordsLeft );

            DLPtrAlloc( pExtPacket, upload_packet_ext );
            pExtPacket->DMACont.SetCont( sizeof(upload_packet_ext)-sizeof(dmatag)*2 );
            pExtPacket->DMACont.MakeDirect( sizeof(upload_packet_ext)-sizeof(dmatag)*2 );
            pExtPacket->GifTagImage.BuildImageLoad( nQWordsToSend<<4, TRUE );
            pExtPacket->DMARef.SetRef( nQWordsToSend<<4, PixelData );
            pExtPacket->DMARef.MakeDirect( nQWordsToSend<<4 );
            nQWordsLeft -= nQWordsToSend;
            PixelData   += nQWordsToSend<<4;
        }
    }

    #ifdef X_DEBUG
    VRAM_BytesLoaded += nBytes;
    #endif
}

//==============================================================================

void vram_LoadClutData( s32 BPP, s32 VRAMAddr, const byte* pClut )
{
    DLPtrAlloc( pPacket, upload_packet );

    // copy the basic data over (using u128's to be efficient)
    u_long128* pSrc128 = (u_long128*)&s_BasicUploadPacket;
    pPacket->ULongArray[0] = pSrc128[0];
    pPacket->ULongArray[1] = pSrc128[1];
    pPacket->ULongArray[2] = pSrc128[2];
    pPacket->ULongArray[3] = pSrc128[3];
    pPacket->ULongArray[4] = pSrc128[4];
    pPacket->ULongArray[5] = pSrc128[5];
    //pPacket->ULongArray[6] = pSrc128[6];  // giftag will get copied anyway
    //pPacket->ULongArray[7] = pSrc128[7];  // dmatag will get built anyway

    // fill in the data specific to a clut
    if ( BPP == 4 )
    {
        pPacket->BitBltBuf   = SCE_GS_SET_BITBLTBUF( VRAMAddr, 1, SCE_GS_PSMCT32, VRAMAddr, 1, SCE_GS_PSMCT32 );
        pPacket->TrxReg      = SCE_GS_SET_TRXREG( 8, 2 );
        pPacket->GifTagImage.BuildImageLoad( 16*4, TRUE );
        pPacket->DMARef.SetRef( 16*4, (u32)pClut );
        pPacket->DMARef.MakeDirect( 16*4 );
        #ifdef X_DEBUG
        VRAM_BytesLoaded += 16*4;
        #endif
    }
    else
    {
        pPacket->BitBltBuf   = SCE_GS_SET_BITBLTBUF( VRAMAddr, 1, SCE_GS_PSMCT32, VRAMAddr, 1, SCE_GS_PSMCT32 );
        pPacket->TrxReg      = SCE_GS_SET_TRXREG( 16, 16 );
        pPacket->GifTagImage.BuildImageLoad( 16*16*4, TRUE );
        pPacket->DMARef.SetRef( 256*4, (u32)pClut );
        pPacket->DMARef.MakeDirect( 256*4 );
        #ifdef X_DEBUG
        VRAM_BytesLoaded += 256*4;
        #endif
    }
}

//==============================================================================

xbool vram_IsActive( const xbitmap& BMP )
{
    s32 VramId = BMP.GetVRAMID();
    ASSERT( (VramId >= 1) && (VramId < s_nTextureRefsAlloced) );

    texture_ref* pTRef = &s_TextureRefs[VramId];
    ASSERT( pTRef->Flags & texture_ref::FLAG_IN_USE );

    return ( (pTRef->Flags & texture_ref::FLAG_ACTIVE) ? TRUE : FALSE );
}

//==============================================================================

u64  vram_GetActiveTex0( const xbitmap& BMP )
{
    // get a texture ref
    s32 VramId = BMP.GetVRAMID();
    ASSERT( (VramId >= 1) && (VramId < s_nTextureRefsAlloced) );

    texture_ref* pTRef = &s_TextureRefs[VramId];
    ASSERT( (pTRef->Flags & texture_ref::FLAG_IN_USE) &&
            (pTRef->Flags & texture_ref::FLAG_ACTIVE) );

    // just add the vram address onto our pre-compiled tex0 register
    u64 tex0;
    if ( (pTRef->Flags&texture_ref::FLAG_CLUT4BIT) ||
         (pTRef->Flags&texture_ref::FLAG_CLUT8BIT) )
    {
        tex0 = pTRef->tex0 | SCE_GS_SET_TEX0( pTRef->MipOffsets[0]+pTRef->StartAddr,
            0, 0, 0, 0, 0, 0,
            pTRef->ClutOffset+pTRef->StartAddr,
            0, 0, 0, 0 );
    }
    else
    {
        tex0 = pTRef->tex0 | SCE_GS_SET_TEX0( pTRef->MipOffsets[0]+pTRef->StartAddr,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0 );
    }

    return tex0;
}

//==============================================================================

u64  vram_GetActiveTex1( const xbitmap& BMP )
{
    // get a texture ref
    s32 VramId = BMP.GetVRAMID();
    ASSERT( (VramId >= 1) && (VramId < s_nTextureRefsAlloced) );

    texture_ref* pTRef = &s_TextureRefs[VramId];
    ASSERT( (pTRef->Flags & texture_ref::FLAG_IN_USE) &&
            (pTRef->Flags & texture_ref::FLAG_ACTIVE) );

    // calculate a default tex1 register (mipk may not be currect...consider
    // using some other method if you want that)
    u64 tex1;
    if ( pTRef->nMips > 0 )
    {
        tex1 = SCE_GS_SET_TEX1( 0, pTRef->nMips, 1, 4, 0, 0, (s32)(-160.0f) );
    }
    else
    {
        tex1 = SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 );
    }

    return tex1;
}

//==============================================================================

u64  vram_GetActiveMip1( const xbitmap& BMP )
{
    // get a texture ref
    s32 VramId = BMP.GetVRAMID();
    ASSERT( (VramId >= 1) && (VramId < s_nTextureRefsAlloced) );

    texture_ref* pTRef = &s_TextureRefs[VramId];
    ASSERT( (pTRef->Flags & texture_ref::FLAG_IN_USE) &&
            (pTRef->Flags & texture_ref::FLAG_ACTIVE) );

    // build the mip register
    u64 miptbp1 = pTRef->miptbp1;
    if( pTRef->nMips > 0 )
    {
        miptbp1 |= ((u64)(pTRef->MipOffsets[1]+pTRef->StartAddr))<<0;
        if( pTRef->nMips > 1 )
        {
            miptbp1 |= ((u64)(pTRef->MipOffsets[2]+pTRef->StartAddr))<<20;
            if( pTRef->nMips > 2 )
                miptbp1 |= ((u64)(pTRef->MipOffsets[3]+pTRef->StartAddr))<<40;
        }
    }

    return miptbp1;
}

//==============================================================================

u64  vram_GetActiveMip2( const xbitmap& BMP )
{
    // get a texture ref
    s32 VramId = BMP.GetVRAMID();
    ASSERT( (VramId >= 1) && (VramId < s_nTextureRefsAlloced) );

    texture_ref* pTRef = &s_TextureRefs[VramId];
    ASSERT( (pTRef->Flags & texture_ref::FLAG_IN_USE) &&
            (pTRef->Flags & texture_ref::FLAG_ACTIVE) );

    // build the mip register
    u64 miptbp2 = pTRef->miptbp2;
    if( pTRef->nMips > 3 )
    {
        miptbp2 |= ((u64)(pTRef->MipOffsets[4]+pTRef->StartAddr))<<0;
        if( pTRef->nMips > 4 )
        {
            miptbp2 |= ((u64)(pTRef->MipOffsets[5]+pTRef->StartAddr))<<20;
            if( pTRef->nMips > 5 )
                miptbp2 |= ((u64)(pTRef->MipOffsets[6]+pTRef->StartAddr))<<40;
        }
    }

    return miptbp2;
}

