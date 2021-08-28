//=========================================================================
//
//  ANIMKEYDATA.CPP
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "animdata.hpp"
//#include "parsing/bitstream.hpp"
#include "x_bitstream.hpp"

//
// Based on anim_key_formats
//
//=========================================================================
//                                            C    S  16   32 
static s32 s_ScaleFormatOverhead[]        = { 0,  12, 24,   0 };
static s32 s_ScaleFormatSize[]            = { 0,   0,  2,  12 };
static s32 s_RotationFormatOverhead[]     = { 0,  16,  0,   0 };
static s32 s_RotationFormatSize[]         = { 0,   0,  8,  16 };

//=========================================================================

static anim_key_block*  s_pMRUBlock = NULL;
static anim_key_block*  s_pLRUBlock = NULL;
static s32              s_nBlocksDecompressed = 0;
static s32              s_nBlockBytesDecompressed = 0;
static s32              s_MaxAllowedDecompressedBytes = 300*1024;

//=========================================================================

s32     anim_key_stream::s_SF;
s32     anim_key_stream::s_RF;
s32     anim_key_stream::s_TF;
s32     anim_key_stream::s_SO;
s32     anim_key_stream::s_RO;
s32     anim_key_stream::s_TO;
byte*   anim_key_stream::s_pData;

//=========================================================================

extern 
void AnimationDecompress( const anim_group& AG,
                          const byte*       pCompressedData, 
                          anim_key_stream*  pStream, 
                          s32               DecompressedSize );

//=========================================================================

void anim_SetMaxAllowedDecompressedBytes( s32 NBytes )
{
    s_MaxAllowedDecompressedBytes = NBytes;
}

//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_KEY
//=========================================================================
//=========================================================================
//=========================================================================

/*
void anim_key::Interpolate( const anim_key& K0, const anim_key& K1, f32 T )
{
#if USE_SCALE_KEYS    
    Scale       = K0.Scale       + T*(K1.Scale       - K0.Scale);
#endif

    Rotation    = Blend( K0.Rotation, K1.Rotation, T );
    Translation = K0.Translation + T*(K1.Translation - K0.Translation);
}
*/

//=========================================================================

void anim_key::Identity( void )
{
#if USE_SCALE_KEYS    
    Scale.Set(1,1,1) ;
#endif
    Translation.Zero() ;
    Rotation.Identity() ;
}

//=========================================================================

void anim_key::Setup( matrix4& M )
{
#if USE_SCALE_KEYS
    M.Setup( Scale, Rotation, Translation ) ;
#else
    // Fill out 3x3 rotations.
    f32 tx  = 2.0f * Rotation.X;   // 2x
    f32 ty  = 2.0f * Rotation.Y;   // 2y
    f32 tz  = 2.0f * Rotation.Z;   // 2z
    f32 txw =   tx * Rotation.W;   // 2x * w
    f32 tyw =   ty * Rotation.W;   // 2y * w
    f32 tzw =   tz * Rotation.W;   // 2z * w
    f32 txx =   tx * Rotation.X;   // 2x * x
    f32 tyx =   ty * Rotation.X;   // 2y * x
    f32 tzx =   tz * Rotation.X;   // 2z * x
    f32 tyy =   ty * Rotation.Y;   // 2y * y
    f32 tzy =   tz * Rotation.Y;   // 2z * y
    f32 tzz =   tz * Rotation.Z;   // 2z * z
    M(0,0) = 1.0f-(tyy+tzz); M(0,1) = tyx + tzw;      M(0,2) = tzx - tyw;           
    M(1,0) = tyx - tzw;      M(1,1) = 1.0f-(txx+tzz); M(1,2) = tzy + txw;           
    M(2,0) = tzx + tyw;      M(2,1) = tzy - txw;      M(2,2) = 1.0f-(txx+tyy);    

    // Fill out translation
    M.SetTranslation( Translation );

    // Fill out last column
    M(0,3) = 0.0f;
    M(1,3) = 0.0f;
    M(2,3) = 0.0f;
    M(3,3) = 1.0f;
#endif
}

//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_KEY_STREAM
//=========================================================================
//=========================================================================
//=========================================================================

void anim_key_stream::GetOffsetsAndFormats   (  s32  nFrames,
                                                s32& SO, 
                                                s32& RO, 
                                                s32& TO,
                                                anim_key_format& SF, 
                                                anim_key_format& RF, 
                                                anim_key_format& TF ) const
{
    SF = (anim_key_format)((Offset >> STREAM_SCL_SHIFT) & STREAM_SCL_MASK);
    RF = (anim_key_format)((Offset >> STREAM_ROT_SHIFT) & STREAM_ROT_MASK);
    TF = (anim_key_format)((Offset >> STREAM_TRS_SHIFT) & STREAM_TRS_MASK);
    SO = (Offset >> STREAM_OFT_SHIFT) & STREAM_OFT_MASK;
    RO = SO + s_ScaleFormatOverhead[SF]     + s_ScaleFormatSize[SF]*nFrames;
    TO = RO + s_RotationFormatOverhead[RF]  + s_RotationFormatSize[RF]*nFrames;
}

//=========================================================================

inline void anim_key_stream::GrabKey( s32 iFrame, anim_key& Key )
{
#if USE_SCALE_KEYS    

    // Decompress scale
    {
        if( s_SF == CONSTANT_VALUE )
        {
            Key.Scale.Set( 1.0f, 1.0f, 1.0f );
        }
        else
        if( s_SF == SINGLE_VALUE )
        {
            Key.Scale = ((vector3p*)(s_pData + s_SO))[ 0 ];
        }
        else
        if( s_SF == PRECISION_32 )
        {
            Key.Scale = ((vector3p*)(s_pData + s_SO))[ iFrame ];
        }
        else
        {
            ASSERT(FALSE);
        }
    }
#endif

    // Decompress rotation
    {
        if( s_RF == PRECISION_16 )
        {   
            // I'm using temp variables to tell the compiler that pR doesn't
            // point to Key.Rotation so it can do the math out of order.
            u16* pR = &((u16*)(s_pData + s_RO))[ iFrame<<2 ];
            f32 TempX = ((f32)pR[0] * (2.0f / 65535.0f)) - 1.0f;
            f32 TempY = ((f32)pR[1] * (2.0f / 65535.0f)) - 1.0f;
            f32 TempZ = ((f32)pR[2] * (2.0f / 65535.0f)) - 1.0f;
            f32 TempW = ((f32)pR[3] * (2.0f / 65535.0f)) - 1.0f;
            Key.Rotation.X = TempX;
            Key.Rotation.Y = TempY;
            Key.Rotation.Z = TempZ;
            Key.Rotation.W = TempW;
        }
        else
        if( s_RF == CONSTANT_VALUE )
        {
            Key.Rotation.Identity();
        }
        else
        if( s_RF == SINGLE_VALUE )
        {
            Key.Rotation = ((quaternion*)(s_pData + s_RO))[ 0 ];
        }
        else
        if( s_RF == PRECISION_32 )
        {
            Key.Rotation = ((quaternion*)(s_pData + s_RO))[ iFrame ];
        }
        else
        {
            ASSERT( FALSE );
        }
    }

    // Decompress translation
    {
        if( s_TF == CONSTANT_VALUE )
        {
            Key.Translation.Set(0.0f,0.0f,0.0f);// = vector3(0,0,0);
        }
        else
        if( s_TF == SINGLE_VALUE )
        {
            Key.Translation = ((vector3p*)(s_pData + s_TO))[ 0 ];
        }
        else
        if( s_TF == PRECISION_32 )
        {
            Key.Translation = ((vector3p*)(s_pData + s_TO))[ iFrame ];
        }
        else
        {
            ASSERT(FALSE);
        }
    }
}

//=========================================================================

void anim_key_stream::GetRawKey( byte* pData, s32 nFrames, s32 iFrame, anim_key& Key )
{
    s_SF = (Offset >> STREAM_SCL_SHIFT) & STREAM_SCL_MASK;
    s_RF = (Offset >> STREAM_ROT_SHIFT) & STREAM_ROT_MASK;
    s_TF = (Offset >> STREAM_TRS_SHIFT) & STREAM_TRS_MASK;
    s_SO = (Offset >> STREAM_OFT_SHIFT) & STREAM_OFT_MASK;
    s_RO = s_SO + s_ScaleFormatOverhead[s_SF]     + s_ScaleFormatSize[s_SF]*nFrames;
    s_TO = s_RO + s_RotationFormatOverhead[s_RF]  + s_RotationFormatSize[s_RF]*nFrames;
    s_pData = pData;

    GrabKey( iFrame, Key );
}

//=========================================================================

void anim_key_stream::GetInterpKey( byte* pData, s32 nFrames, s32 iFrame, f32 T, anim_key& Key )
{
    ASSERT( iFrame < nFrames-1 );

    s_pData = pData;
    s_SF = (Offset >> STREAM_SCL_SHIFT) & STREAM_SCL_MASK;
    s_RF = (Offset >> STREAM_ROT_SHIFT) & STREAM_ROT_MASK;
    s_TF = (Offset >> STREAM_TRS_SHIFT) & STREAM_TRS_MASK;
    s_SO = (Offset >> STREAM_OFT_SHIFT) & STREAM_OFT_MASK;
    s_RO = s_SO + s_ScaleFormatOverhead[s_SF]     + s_ScaleFormatSize[s_SF]*nFrames;
    s_TO = s_RO + s_RotationFormatOverhead[s_RF]  + s_RotationFormatSize[s_RF]*nFrames;
    anim_key K0;
    anim_key K1;

    GrabKey( iFrame+0, K0 );
    GrabKey( iFrame+1, K1 );

    Key.Interpolate( K0, K1, T );
}

//=========================================================================

u32 anim_key_stream::GetFlags( void ) const
{
    return (Offset >> STREAM_FLG_SHIFT) & STREAM_FLG_MASK;
}

//=========================================================================

void anim_key_stream::SetFlags( u32 Flags )
{
    // Clear current flags
    Offset &= ~(STREAM_FLG_MASK<<STREAM_FLG_SHIFT);

    // Write new flags
    Offset |= (Flags<<STREAM_FLG_SHIFT);
}

//=========================================================================

void anim_key_stream::SetFormats( anim_key_format SF, anim_key_format RF, anim_key_format TF )
{
    // Clear current formats
    Offset &= ~(STREAM_SCL_MASK<<STREAM_SCL_SHIFT);
    Offset &= ~(STREAM_ROT_MASK<<STREAM_ROT_SHIFT);
    Offset &= ~(STREAM_TRS_MASK<<STREAM_TRS_SHIFT);

    // Write new formats
    Offset |= ((u32)SF<<STREAM_SCL_SHIFT);
    Offset |= ((u32)RF<<STREAM_ROT_SHIFT);
    Offset |= ((u32)TF<<STREAM_TRS_SHIFT);
}

//=========================================================================

void anim_key_stream::SetOffset( s32 ScaleOffset )
{
    // Clear current offset
    Offset &= ~(STREAM_OFT_MASK<<STREAM_OFT_SHIFT);

    // Write new offset
    Offset |= (ScaleOffset<<STREAM_OFT_SHIFT);
}

//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_KEY_BLOCK
//=========================================================================
//=========================================================================
//=========================================================================

void anim_key_block::AttachToList( void )
{
    // Add to beginning of list
    pNext = s_pMRUBlock;
    pPrev = NULL;
    if( s_pMRUBlock ) s_pMRUBlock->pPrev = this;
    if( s_pLRUBlock == NULL ) s_pLRUBlock = this;
    s_pMRUBlock = this;
}

//=========================================================================

void anim_key_block::DetachFromList( void )
{
    // Remove from list
    if( pNext ) pNext->pPrev = pPrev;
    if( pPrev ) pPrev->pNext = pNext;
    if( s_pMRUBlock == this ) s_pMRUBlock = pNext;
    if( s_pLRUBlock == this ) s_pLRUBlock = pPrev;
    pNext = NULL;
    pPrev = NULL;
}

//=========================================================================

anim_key_stream* anim_key_block::AcquireStreams( const anim_group& AG )
{
    MEMORY_OWNER( "ANIMATION CACHE" );
    // Be sure we can fit this decompressed data into the cache
    ASSERT( DecompressedDataSize <= s_MaxAllowedDecompressedBytes );

    // Move block to beginning of list
    DetachFromList();
    AttachToList();

    // Check if we've already decompressed
    if( pStream )
        return pStream;

    // Limit the maximum number of decompressed bytes
    while( (s_nBlockBytesDecompressed+DecompressedDataSize) > s_MaxAllowedDecompressedBytes )
    {
        ASSERT( s_pLRUBlock != this );
        s_pLRUBlock->ReleaseStreams();

        // Force us to hold at least one
        if( s_pMRUBlock == NULL )
            break;
    }

    // Increment number of decompressed bytes
    s_nBlockBytesDecompressed += DecompressedDataSize;
    s_nBlocksDecompressed++;

    // Allocate destination of decompressed data
    pStream = (anim_key_stream*)x_malloc( DecompressedDataSize );
    ASSERT( pStream );

    xtimer Timer;
    Timer.Start();

    // Kick off decompression
    AnimationDecompress( AG,
                         //AG.GetCompressedDataPtr()+CompressedDataOffset, 
                         pFactoredCompressedData,
                         pStream, 
                         DecompressedDataSize );

    Timer.Stop();

    //x_DebugMsg("DecompressedBlock: (%1.3f ms) (%d bytes) (%d blocks) (%d bytes total) <%s>\n",
    //    Timer.ReadMs(), DecompressedDataSize, s_nBlocksDecompressed, s_nBlockBytesDecompressed, AG.GetFileName() );

    return pStream;
}

//=========================================================================

void anim_key_block::ReleaseStreams( void )
{
    if( pStream == NULL )
        return;

    // Detach from list
    DetachFromList();

    // Deallocate decompressed data
    x_free( pStream );
    pStream = NULL;

    // Decrement number of decompressed bytes
    s_nBlockBytesDecompressed -= DecompressedDataSize;
    s_nBlocksDecompressed--;
}

//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_KEYS
//=========================================================================
//=========================================================================
//=========================================================================

anim_keys::anim_keys( void )
{
}

//=========================================================================

anim_keys::~anim_keys( void )
{
}

//=========================================================================

xbool anim_keys::IsBoneMasked( const anim_group& AnimGroup, s32 iBone ) const
{
    ASSERT( (iBone>=0) && (iBone<m_nBones) );
    anim_key_block& KeyBlock = AnimGroup.m_pKeyBlock[ m_iKeyBlock ];
    anim_key_stream* pStream = KeyBlock.AcquireStreams( AnimGroup );
    return (pStream[iBone].GetFlags() & STREAM_FLAG_MASKED) ? (TRUE):(FALSE);
}

//=========================================================================

void anim_keys::GetRawKey( const anim_group& AnimGroup, s32 iFrame, s32 iStream, anim_key& Key ) const
{
    ASSERT( (iStream>=0) && (iStream<(m_nBones+m_nProps)) );

    s32 iBlock      = iFrame >> MAX_KEYS_PER_BLOCK_SHIFT;
    s32 iBlockFrame = iFrame & MAX_KEYS_PER_BLOCK_MASK;
    if( (iBlock==m_nKeyBlocks) && (iBlockFrame==0) )
    {
        iBlock--;
        iBlockFrame = MAX_KEYS_PER_BLOCK;
    }
    ASSERT( (iBlock>=0) && (iBlock<m_nKeyBlocks) );

    anim_key_block& KeyBlock = AnimGroup.m_pKeyBlock[ m_iKeyBlock + iBlock ];
    anim_key_stream* pStream = KeyBlock.AcquireStreams( AnimGroup );

    pStream[iStream].GetRawKey( (byte*)pStream, KeyBlock.nFrames, iBlockFrame, Key );
}

//=========================================================================

void anim_keys::GetInterpKey( const anim_group& AnimGroup, f32  Frame, s32 iStream, anim_key& Key ) const
{
    ASSERT( (iStream>=0) && (iStream<(m_nBones+m_nProps)) );

    s32 iFrame      = (s32)Frame;
    f32 fFrac       = Frame - (f32)iFrame;
    s32 iBlock      = iFrame >> MAX_KEYS_PER_BLOCK_SHIFT;
    s32 iBlockFrame = iFrame & MAX_KEYS_PER_BLOCK_MASK;
    if( (iBlock==m_nKeyBlocks) && (iBlockFrame==0) )
    {
        iBlock--;
        iBlockFrame = MAX_KEYS_PER_BLOCK;
    }
    ASSERT( (iBlock>=0) && (iBlock<m_nKeyBlocks) );

    anim_key_block& KeyBlock = AnimGroup.m_pKeyBlock[ m_iKeyBlock + iBlock ];
    anim_key_stream* pStream = KeyBlock.AcquireStreams( AnimGroup );

    pStream[iStream].GetInterpKey( (byte*)pStream, KeyBlock.nFrames, iBlockFrame, fFrac, Key );
}

//=========================================================================

void anim_keys::GetRawKeys( const anim_group& AnimGroup, s32 iFrame, anim_key* pKey ) const
{
    s32 iBlock      = iFrame >> MAX_KEYS_PER_BLOCK_SHIFT;
    s32 iBlockFrame = iFrame & MAX_KEYS_PER_BLOCK_MASK;
    if( (iBlock==m_nKeyBlocks) && (iBlockFrame==0) )
    {
        iBlock--;
        iBlockFrame = MAX_KEYS_PER_BLOCK;
    }
    ASSERT( (iBlock>=0) && (iBlock<m_nKeyBlocks) );

    anim_key_block& KeyBlock = AnimGroup.m_pKeyBlock[ m_iKeyBlock + iBlock ];
    anim_key_stream* pStream = KeyBlock.AcquireStreams( AnimGroup );

    for( s32 i=0; i<m_nBones; i++ )
        pStream[i].GetRawKey( (byte*)pStream, KeyBlock.nFrames, iBlockFrame, pKey[i] );
}

//=========================================================================

void anim_keys::GetInterpKeys( const anim_group& AnimGroup, f32  Frame, anim_key* pKey ) const
{
    s32 iFrame      = (s32)Frame;
    f32 fFrac       = Frame - (f32)iFrame;
    s32 iBlock      = iFrame >> MAX_KEYS_PER_BLOCK_SHIFT;
    s32 iBlockFrame = iFrame & MAX_KEYS_PER_BLOCK_MASK;
    if( (iBlock==m_nKeyBlocks) && (iBlockFrame==0) )
    {
        iBlock--;
        iBlockFrame = MAX_KEYS_PER_BLOCK;
    }
    ASSERT( (iBlock>=0) && (iBlock<m_nKeyBlocks) );

    anim_key_block& KeyBlock = AnimGroup.m_pKeyBlock[ m_iKeyBlock + iBlock ];
    anim_key_stream* pStream = KeyBlock.AcquireStreams( AnimGroup );

    for( s32 i=0; i<m_nBones; i++ )
        pStream[i].GetInterpKey( (byte*)pStream, KeyBlock.nFrames, iBlockFrame, fFrac, pKey[i] );
}

//=========================================================================

void anim_keys::GetInterpKeys( const anim_group& AnimGroup, f32  Frame, anim_key* pKey, s32 nBones ) const
{
    s32 iFrame      = (s32)Frame;
    f32 fFrac       = Frame - (f32)iFrame;
    s32 iBlock      = iFrame >> MAX_KEYS_PER_BLOCK_SHIFT;
    s32 iBlockFrame = iFrame & MAX_KEYS_PER_BLOCK_MASK;
    if( (iBlock==m_nKeyBlocks) && (iBlockFrame==0) )
    {
        iBlock--;
        iBlockFrame = MAX_KEYS_PER_BLOCK;
    }
    ASSERT( (iBlock>=0) && (iBlock<m_nKeyBlocks) );

    anim_key_block& KeyBlock = AnimGroup.m_pKeyBlock[ m_iKeyBlock + iBlock ];
    anim_key_stream* pStream = KeyBlock.AcquireStreams( AnimGroup );

    ASSERT(nBones <= m_nBones) ;
    for( s32 i=0; i<nBones; i++ )
        pStream[i].GetInterpKey( (byte*)pStream, KeyBlock.nFrames, iBlockFrame, fFrac, pKey[i] );
}

//=========================================================================



/*




//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_KEY_SET
//=========================================================================
//=========================================================================
//=========================================================================




/*


//=========================================================================

void anim_key_set::GetRawKey( s32 iFrame, anim_key& Key ) const
{
    //
    // Get Scale
    //
    {

        if( m_ScaleFormat == KNOWN_CONSTANT )
        {
            Key.Scale.X = 1.0f;
            Key.Scale.Y = 1.0f;
            Key.Scale.Z = 1.0f;
        }
        else
        if( m_ScaleFormat == FULL_PRECISION )
        {
            vector3* pD = (vector3*)m_pScale;
            Key.Scale = pD[iFrame];
            #ifdef TARGET_GCN   
            SwapEndian( Key.Scale );  
            #endif
        }
        else
        if( m_ScaleFormat == SINGLE_VALUE )
        {
            vector3* pD = (vector3*)m_pScale;
            Key.Scale = pD[0];
            #ifdef TARGET_GCN   
            SwapEndian( Key.Scale );  
            #endif
        }
    }

    //
    // Get Rotation
    //
    {
        if( m_RotationFormat == QUAT_8_PACK )
        {
            static f32 ITable[16] = {0,0.142857f,0.28571f,0.428571f,0.571428f,0.714285f,0.857143f,1.0f,
                                    0.1f,0.2f,0.3f,0.5f,0.6f,0.7f,0.8f,0.9f};

            s32 iPack = iFrame >> 3;
            s32 iQ    = iFrame & 7;
            u64 PA = *((u64*)(m_pRotation + sizeof(u64)*(iPack<<1) + 0));
            u64 PB = *((u64*)(m_pRotation + sizeof(u64)*(iPack<<1) + sizeof(u64)));

            #ifdef TARGET_GCN
            SwapEndian(PA);
            SwapEndian(PB);
            #endif

            // unpack boundary quaternions
            quaternion QA,QB;
            QA.X = (((s64)PA<<(13*0))>>(64-(13*1))) * (1.0f/4095.0f);
            QA.Y = (((s64)PA<<(13*1))>>(64-(13*1))) * (1.0f/4095.0f);
            QA.Z = (((s64)PA<<(13*2))>>(64-(13*1))) * (1.0f/4095.0f);
            QA.W = (((s64)PA<<(13*3))>>(64-(13*1))) * (1.0f/4095.0f);
            QB.X = (((s64)PB<<(13*0))>>(64-(13*1))) * (1.0f/4095.0f);
            QB.Y = (((s64)PB<<(13*1))>>(64-(13*1))) * (1.0f/4095.0f);
            QB.Z = (((s64)PB<<(13*2))>>(64-(13*1))) * (1.0f/4095.0f);
            QB.W = (((s64)PB<<(13*3))>>(64-(13*1))) * (1.0f/4095.0f);

            // Blend between the boundary quaternions
            f32 T;
            if( iQ==0 ) T = 0.0f;
            else
            if( iQ==7 ) T = 1.0f;
            else
            if( iQ<4 ) T = ITable[((PA>>((3-iQ)<<2)) & 0xF)];
            else       T = ITable[((PB>>((6-iQ)<<2)) & 0xF)];

            Key.Rotation = Blend(QA,QB,T);

        }
        else
        if( m_RotationFormat == PRECISION_8 )
        {
            f32 Min[4];
            f32 Max[4];
            Min[0] = ((f32*)m_pRotation)[0];
            Min[1] = ((f32*)m_pRotation)[1];
            Min[2] = ((f32*)m_pRotation)[2];
            Min[3] = ((f32*)m_pRotation)[3];
            Max[0] = ((f32*)m_pRotation)[4];
            Max[1] = ((f32*)m_pRotation)[5];
            Max[2] = ((f32*)m_pRotation)[6];
            Max[3] = ((f32*)m_pRotation)[7];

            #ifdef TARGET_GCN
            SwapEndian( Min[0] );
            SwapEndian( Min[1] );
            SwapEndian( Min[2] );
            SwapEndian( Min[3] );
            SwapEndian( Max[0] );
            SwapEndian( Max[1] );
            SwapEndian( Max[2] );
            SwapEndian( Max[3] );
            #endif

            byte* pI = m_pRotation + (sizeof(f32)*8) + (iFrame<<2);

            Key.Rotation.X = Min[0] + (pI[0]/255.0f)*(Max[0]-Min[0]);
            Key.Rotation.Y = Min[1] + (pI[1]/255.0f)*(Max[1]-Min[1]);
            Key.Rotation.Z = Min[2] + (pI[2]/255.0f)*(Max[2]-Min[2]);
            Key.Rotation.W = Min[3] + (pI[3]/255.0f)*(Max[3]-Min[3]);
        }
        else
        if( m_RotationFormat == PRECISION_16 )
        {
            s16* pD = (s16*)m_pRotation;
            pD += (iFrame<<2);
            s16 D[4];
            D[0] = pD[0];
            D[1] = pD[1];
            D[2] = pD[2];
            D[3] = pD[3];

            #ifdef TARGET_GCN
            SwapEndian(D[0]);
            SwapEndian(D[1]);
            SwapEndian(D[2]);
            SwapEndian(D[3]);
            #endif

            Key.Rotation.X = (f32)D[0] * (1.0f/16384.0f);
            Key.Rotation.Y = (f32)D[1] * (1.0f/16384.0f);
            Key.Rotation.Z = (f32)D[2] * (1.0f/16384.0f);
            Key.Rotation.W = (f32)D[3] * (1.0f/16384.0f);
        }
        else
        if( m_RotationFormat == FULL_PRECISION )
        {
            quaternion* pD = (quaternion*)m_pRotation;
            Key.Rotation = pD[iFrame];
            #ifdef TARGET_GCN   
            SwapEndian( Key.Rotation );  
            #endif
        }
        else
        if( m_RotationFormat == SINGLE_VALUE )
        {
            quaternion* pD = (quaternion*)m_pRotation;
            Key.Rotation = pD[0];
            #ifdef TARGET_GCN
            SwapEndian( Key.Rotation );
            #endif
        }

    }

    //
    // Get Translation
    //
    {
        if( m_TranslationFormat == SINGLE_VALUE_16 )
        {
            s16* pD = (s16*)m_pTranslation;
            s16 D[3];
            D[0] = pD[0];
            D[1] = pD[1];
            D[2] = pD[2];

            #ifdef TARGET_GCN
            SwapEndian(D[0]);
            SwapEndian(D[1]);
            SwapEndian(D[2]);
            #endif

            Key.Translation.X = (f32)D[0]*(1.0f/16.0f);
            Key.Translation.Y = (f32)D[1]*(1.0f/16.0f);
            Key.Translation.Z = (f32)D[2]*(1.0f/16.0f);
        }
        else
        if( m_TranslationFormat == PRECISION_16 )
        {
            s16* pD = (s16*)m_pTranslation;
            pD += (iFrame*3);
            s16 D[3];
            D[0] = pD[0];
            D[1] = pD[1];
            D[2] = pD[2];

            #ifdef TARGET_GCN
            SwapEndian(D[0]);
            SwapEndian(D[1]);
            SwapEndian(D[2]);
            #endif

            Key.Translation.X = (f32)D[0]*(1.0f/16.0f);
            Key.Translation.Y = (f32)D[1]*(1.0f/16.0f);
            Key.Translation.Z = (f32)D[2]*(1.0f/16.0f);
        }
        else
        if( m_TranslationFormat == FULL_PRECISION )
        {
            vector3* pD = (vector3*)m_pTranslation;
            Key.Translation = pD[iFrame];
            #ifdef TARGET_GCN   
            SwapEndian( Key.Translation );  
            #endif
        }
        else
        if( m_TranslationFormat == SINGLE_VALUE )
        {
            vector3* pD = (vector3*)m_pTranslation;
            Key.Translation = pD[0];
            #ifdef TARGET_GCN   
            SwapEndian( Key.Translation );  
            #endif
        }
    }
}


*/

//=========================================================================
