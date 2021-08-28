//=========================================================================
//
//  ANIMDECOMPRESS.CPP
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "animdata.hpp"
//#include "parsing/bitstream.hpp"
#include "x_bitstream.hpp"


//=========================================================================
//=========================================================================
//=========================================================================
// DECOMPRESSOR PROTOTYPES AND FUNCTION TABLES
//=========================================================================
//=========================================================================
//=========================================================================

#define ANIM_ALIGNED(n)      ( (((u32)(n)) +   3) & (  -4) )

typedef void decomp_fn  ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );

//=========================================================================

void Decomp_Const       ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );

void DecompS_Delta      ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );
void DecompQ_Delta      ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );
void DecompQ_Delta2     ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );
void DecompQ_Delta3     ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );
void DecompT_Delta      ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );

void DecompS_Single     ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );
void DecompQ_Single     ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );
void DecompT_Single     ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );

void DecompV_32         ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );
void DecompQ_32         ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );
void DecompT_LocalTrans ( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData );

//=========================================================================

decomp_fn* s_ScaleDecompFnptr[] = 
{
    Decomp_Const,
    DecompS_Single,
    DecompS_Delta,
    DecompV_32,
};

//=========================================================================

decomp_fn* s_RotationDecompFnptr[] = 
{
    Decomp_Const,
    DecompQ_Single,
    DecompQ_Delta,
    DecompQ_32,
    DecompQ_Delta2,
    DecompQ_Delta3,
};

//=========================================================================

decomp_fn* s_TranslationDecompFnptr[] = 
{
    Decomp_Const,
    DecompT_LocalTrans,
    DecompT_Single,
    DecompT_Delta,
    DecompV_32,
};

//=========================================================================
//=========================================================================
//=========================================================================
// DECOMPRESSORS
//=========================================================================
//=========================================================================
//=========================================================================

void Decomp_Const( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)BS;
    (void)AG;
    (void)iStream;
    (void)nFrames;
    (void)pData;
}

//=========================================================================

void DecompV_32( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)AG;
    (void)iStream;
    s32 i;
    for( i=0; i<nFrames; i++ )
    {
        vector3 V;
        BS.ReadVector( V );
        *((vector3p*)pData) = V;
        pData += sizeof(vector3p);
    }
}

//=========================================================================

void DecompQ_32( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)AG;
    (void)iStream;

    s32 i;
    for( i=0; i<nFrames; i++ )
    {
        BS.ReadQuaternion( *((quaternion*)pData) );
        pData += sizeof(quaternion);
    }
}

//=========================================================================

void DecompS_Single( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)AG;
    (void)iStream;
    (void)nFrames;

    s32 iX,iY,iZ;
    vector3p V;

    BS.ReadVariableLenS32( iX );
    BS.ReadVariableLenS32( iY );
    BS.ReadVariableLenS32( iZ );
    V.X = iX / 128.0f;
    V.Y = iY / 128.0f;
    V.Z = iZ / 128.0f;

    *((vector3p*)pData) = V;
    pData += sizeof(vector3p);
}

//=========================================================================

void DecompT_LocalTrans( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)BS;
    (void)nFrames;

    ASSERT( iStream < AG.GetNBones() );
    vector3 V = AG.GetBone(iStream).LocalTranslation;

    *((vector3p*)pData) = V;
    pData += sizeof(vector3p);
}

//=========================================================================

void DecompT_Single( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)AG;
    (void)iStream;
    (void)nFrames;

    s32 iX,iY,iZ;
    vector3p V;

    BS.ReadVariableLenS32( iX );
    BS.ReadVariableLenS32( iY );
    BS.ReadVariableLenS32( iZ );
    V.X = iX / 128.0f;
    V.Y = iY / 128.0f;
    V.Z = iZ / 128.0f;

    *((vector3p*)pData) = V;
    pData += sizeof(vector3p);
}

//=========================================================================

void DecompQ_Single( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)AG;
    (void)iStream;
    (void)nFrames;

    s32 iX,iY,iZ,iW;
    quaternion Q;

    BS.ReadVariableLenS32( iX );
    BS.ReadVariableLenS32( iY );
    BS.ReadVariableLenS32( iZ );
    BS.ReadVariableLenS32( iW );
    Q.X = iX / 2048.0f;
    Q.Y = iY / 2048.0f;
    Q.Z = iZ / 2048.0f;
    Q.W = iW / 2048.0f;

    *((quaternion*)pData) = Q;
    pData += sizeof(quaternion);
}

//=========================================================================
//=========================================================================
//=========================================================================
// DELTA COMPRESSION
//=========================================================================
//=========================================================================
//=========================================================================

void DeltaDecompress( bitstream& BS, f32* pSample, s32 Stride, s32 nSamples, f32 Prec )
{
    s32 FirstSample;
    s32 MinDelta;
    u32 nDeltaBits;
    f32 OneOverPrec = 1.0f / Prec;

    BS.ReadVariableLenS32( FirstSample );
    BS.ReadVariableLenS32( MinDelta );
    BS.ReadU32( nDeltaBits, 5 );

    // Create first sample
    pSample[0] = FirstSample * OneOverPrec;

    // Create other samples
    for( s32 i=1; i<nSamples; i++ )
    {
        u32 D;
        BS.ReadU32( D, nDeltaBits );
        pSample[i*Stride] = pSample[(i-1)*Stride] + (((s32)D + MinDelta)*OneOverPrec);
    }
}

//=========================================================================

void DeltaDecompressQ( bitstream& BS, u16* pSample, s32 Stride, s32 nSamples, f32 Prec )
{
    s32 FirstSample;
    s32 MinDelta;
    u32 nDeltaBits;
    f32 OneOverPrec = 1.0f / Prec;
    BS.ReadVariableLenS32( FirstSample );
    BS.ReadVariableLenS32( MinDelta );
    BS.ReadU32( nDeltaBits, 5 );

    // Create first sample
    f32 S = FirstSample * OneOverPrec;
    ASSERT( (S>=-1.0f) && (S<=1.0f) );
    *pSample = (u16)((S+1.0f)*0.5f*65535.0f);
    pSample += Stride;
    f32 PrevSample = S;
    // Create other samples
    for( s32 i=1; i<nSamples; i++ )
    {
        u32 D;
        BS.ReadU32( D, nDeltaBits );

        S = PrevSample + (((s32)D + MinDelta)*OneOverPrec);
        ASSERT( (S>=-1.0f) && (S<=1.0f) );
        *pSample = (u16)((S+1.0f)*0.5f*65535.0f);
        PrevSample = S;

        pSample += Stride;
    }
}

//=========================================================================

void DecompS_Delta( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)AG;
    (void)iStream;

    DeltaDecompress( BS, &((vector3p*)pData)->X, 3, nFrames, 128.0f );
    DeltaDecompress( BS, &((vector3p*)pData)->Y, 3, nFrames, 128.0f );
    DeltaDecompress( BS, &((vector3p*)pData)->Z, 3, nFrames, 128.0f );
    pData += sizeof(vector3p)*nFrames;
}

//=========================================================================

void DecompT_Delta( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)AG;
    (void)iStream;

    DeltaDecompress( BS, &((vector3p*)pData)->X, 3, nFrames, 128.0f );
    DeltaDecompress( BS, &((vector3p*)pData)->Y, 3, nFrames, 128.0f );
    DeltaDecompress( BS, &((vector3p*)pData)->Z, 3, nFrames, 128.0f );
    pData += sizeof(vector3p)*nFrames;
}

//=========================================================================

void DecompQ_Delta( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)AG;
    (void)iStream;
    DeltaDecompressQ( BS, &((u16*)pData)[0], 4, nFrames, 2048.0f );
    DeltaDecompressQ( BS, &((u16*)pData)[1], 4, nFrames, 2048.0f );
    DeltaDecompressQ( BS, &((u16*)pData)[2], 4, nFrames, 2048.0f );
    DeltaDecompressQ( BS, &((u16*)pData)[3], 4, nFrames, 2048.0f );

    //((quaternion*)pData)->Normalize();

    pData += 8*nFrames;
}

//=========================================================================

void DecompQ_Delta2( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)AG;
    (void)iStream;
    DeltaDecompressQ( BS, &((u16*)pData)[0], 4, nFrames, (f32)(1<<14) );
    DeltaDecompressQ( BS, &((u16*)pData)[1], 4, nFrames, (f32)(1<<14) );
    DeltaDecompressQ( BS, &((u16*)pData)[2], 4, nFrames, (f32)(1<<14) );
    DeltaDecompressQ( BS, &((u16*)pData)[3], 4, nFrames, (f32)(1<<14) );

    //((quaternion*)pData)->Normalize();

    pData += 8*nFrames;
}

//=========================================================================

void DecompQ_Delta3( bitstream& BS, const anim_group& AG, s32 iStream, s32 nFrames, byte*& pData )
{
    (void)AG;
    (void)iStream;
    DeltaDecompressQ( BS, &((u16*)pData)[0], 4, nFrames, (f32)(1<<16) );
    DeltaDecompressQ( BS, &((u16*)pData)[1], 4, nFrames, (f32)(1<<16) );
    DeltaDecompressQ( BS, &((u16*)pData)[2], 4, nFrames, (f32)(1<<16) );
    DeltaDecompressQ( BS, &((u16*)pData)[3], 4, nFrames, (f32)(1<<16) );

    //((quaternion*)pData)->Normalize();

    pData += 8*nFrames;
}

//=========================================================================
//=========================================================================
//=========================================================================
// RLE DECOMPRESS
//=========================================================================
//=========================================================================
//=========================================================================

void RLEDecompress( bitstream& BS, u32* pData, s32& nSamples )
{
    u32 nBitsPerCount;
    u32 nBitsPerSample;
    u32 C;
    u32 V;

    //
    // Unpack bitcounts from bitstream
    //
    BS.ReadRangedU32( nBitsPerCount,  3, 8  );   
    BS.ReadRangedU32( nBitsPerSample, 0, 32 );
    
    //
    // Loop until all samples are decompressed
    //
    nSamples = 0;
    while( 1 )
    {
        // Read Count
        BS.ReadU32( C, nBitsPerCount );

        // Increment total samples
        nSamples += C;

        // If we hit terminator break out
        if( C==0 ) break;

        // Read Value and duplicate
        BS.ReadU32( V, nBitsPerSample );
        while( C-- ) *pData++ = V;
    }

}

//=========================================================================

void RLEDecompressOffsetInfo( bitstream& BS, anim_key_stream* pStream, u32 Mask, u32 Shift )
{
    u32 nBitsPerCount;
    u32 nBitsPerSample;
    u32 C;
    u32 V;

    //
    // Unpack bitcounts from bitstream
    //
    BS.ReadRangedU32( nBitsPerCount,  3, 8  );   
    BS.ReadRangedU32( nBitsPerSample, 0, 32 );
    
    //
    // Loop until all samples are decompressed
    //
    while( 1 )
    {
        // Read Count
        BS.ReadU32( C, nBitsPerCount );

        // If we hit terminator break out
        if( C==0 ) break;

        // Read Value and duplicate
        BS.ReadU32( V, nBitsPerSample );

        while( C-- )
        {
            pStream->Offset &= ~(Mask<<Shift);
            pStream->Offset |= ( V & Mask ) << Shift;
            pStream++;
        }
    }
}

//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_DECOMPRESS
//=========================================================================
//=========================================================================
//=========================================================================

void AnimationDecompress( const anim_group& AG,
                          const byte*       pCompressedData, 
                          anim_key_stream*  pStream, 
                          s32               DecompressedSize )
{
    (void) DecompressedSize;  // SKS: Prevent compiler from complaining that variable is not used (release builds)
    s32 i;

    // Wrap bitstream around compressed data
    bitstream BS;
    BS.Init( pCompressedData, 1024*1024 );

    // Get cursor position of header information and cursor position of
    // the keyframe data.  Then jump to the header information
    u32 KeyDataCursor;
    u32 HeaderCursor;
    BS.ReadU32( HeaderCursor );
    KeyDataCursor = BS.GetCursor();
    BS.SetCursor( HeaderCursor );

    //
    // Read Header info
    //
    u32 TotalStreams;
    u32 nFrames;
    BS.ReadU32( TotalStreams, 10 );
    BS.ReadRangedU32( nFrames, 2, MAX_KEYS_PER_BLOCK+1 );

    // Decompress rle info into stream structures
    RLEDecompressOffsetInfo( BS, pStream, STREAM_SCL_MASK, STREAM_SCL_SHIFT );
    RLEDecompressOffsetInfo( BS, pStream, STREAM_ROT_MASK, STREAM_ROT_SHIFT );
    RLEDecompressOffsetInfo( BS, pStream, STREAM_TRS_MASK, STREAM_TRS_SHIFT );
    RLEDecompressOffsetInfo( BS, pStream, STREAM_FLG_MASK, STREAM_FLG_SHIFT );

    //
    // Decompress iDecompressor
    //
    s32  nStreams;
    u32* pSDIndex = (u32*)x_malloc( sizeof(u32)*TotalStreams );
    u32* pRDIndex = (u32*)x_malloc( sizeof(u32)*TotalStreams );
    u32* pTDIndex = (u32*)x_malloc( sizeof(u32)*TotalStreams );
    RLEDecompress( BS, pSDIndex, nStreams );
    RLEDecompress( BS, pRDIndex, nStreams );
    RLEDecompress( BS, pTDIndex, nStreams );

    //
    // Jump back to compressed key data
    //
    BS.SetCursor(KeyDataCursor);

    //
    // Initialize ptr to the destination keyframe data
    //
    byte* pData = (byte*)pStream + sizeof(anim_key_stream)*TotalStreams;

    //
    // Read in the individual streams
    //
    for( i=0; i<(s32)TotalStreams; i++ )
    {
        // Be sure we haven't overun the decompressed area
        ASSERT( (u32)pData <= ((u32)pStream + DecompressedSize) );

        // Compute offset for this stream
        pStream[i].SetOffset( (u32)pData - (u32)pStream );

        //
        // Call the decompressors for S,R,T
        // These functions advance the pData pointer
        //
        s_ScaleDecompFnptr      [ pSDIndex[i] ]( BS, AG, i, nFrames, pData );
        s_RotationDecompFnptr   [ pRDIndex[i] ]( BS, AG, i, nFrames, pData );
        s_TranslationDecompFnptr[ pTDIndex[i] ]( BS, AG, i, nFrames, pData );
    }

    //
    // Verify we decompressed into the size we expected
    //
    ASSERT( (u32)pData == ((u32)pStream + DecompressedSize) );

    //
    // Free temporary decompressor indices
    //
    x_free( pSDIndex );
    x_free( pRDIndex );
    x_free( pTDIndex );

}

//=========================================================================
