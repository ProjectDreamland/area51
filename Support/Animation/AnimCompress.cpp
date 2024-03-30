#ifndef DISABLE_LEGACY_CODE
//=========================================================================
//
//  ANIMCOMPRESS.CPP
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "animdata.hpp"
#include "x_bitstream.hpp"

//=========================================================================

s32 s_ScaleCompUses[8] = {0};
s32 s_ScaleCompSize[8] = {0};
s32 s_ScaleCompSamples[8] = {0};
s32 s_RotationCompUses[8] = {0};
s32 s_RotationCompSize[8] = {0};
s32 s_RotationCompSamples[8] = {0};
s32 s_TranslationCompUses[8] = {0};
s32 s_TranslationCompSize[8] = {0};
s32 s_TranslationCompSamples[8] = {0};
s32 s_TotalNBones = 0;
s32 s_TotalFrames = 0;

//=========================================================================
//=========================================================================
//=========================================================================
// SUPPORT FUNCTIONS
//=========================================================================
//=========================================================================
//=========================================================================

s32 F32ToS32( f32 F )
{
    ASSERT(F >= (-S32_MAX)) ;
    ASSERT(F <= (+S32_MAX)) ;

    if( F >= 0 ) 
        return (s32)(F+0.5f);
    else
        return -((s32)((-F)+0.5f));
}

//=========================================================================

s32 ComputeNBitsNeeded( u32 N )
{
    s32 B=0;
    while( N ) { B++; N>>=1; }
    return B;
}

//=========================================================================
/*
void DisplayAnimCompressStats( void )
{
    s32 i;
    s32 Sum[3]={0};

    for( i=0; i<8; i++ )
    {
        if( s_ScaleCompSamples[i] == 0 ) s_ScaleCompSamples[i] = 1;
        if( s_RotationCompSamples[i] == 0 ) s_RotationCompSamples[i] = 1;
        if( s_TranslationCompSamples[i] == 0 ) s_TranslationCompSamples[i] = 1;
    }


    x_DebugMsg("------------------------------------------------------------------------\n");
    x_DebugMsg("--                       ANIM COMPRESS STATS                          --\n");
    x_DebugMsg("------------------------------------------------------------------------\n");

    for( i=0; i<6; i++ )
    {
        Sum[0] += s_ScaleCompSize[i];
        x_DebugMsg("%1d Scale       %5d = %8d bytes = %5.2f bytes per key\n", 
            i, 
            s_ScaleCompUses[i], 
            (s_ScaleCompSize[i]+7)/8, 
            ((s_ScaleCompSize[i]+7)/8)/(f32)s_ScaleCompSamples[i]);
    }

    x_DebugMsg("------------------------------------------------------------------------\n");

    for( i=0; i<6; i++ )
    {
        Sum[1] += s_RotationCompSize[i];
        x_DebugMsg("%1d Rotation    %5d = %8d bytes = %5.2f bytes per key\n", 
            i, 
            s_RotationCompUses[i], 
            (s_RotationCompSize[i]+7)/8, 
            ((s_RotationCompSize[i]+7)/8)/(f32)s_RotationCompSamples[i] );
    }

    x_DebugMsg("------------------------------------------------------------------------\n");

    for( i=0; i<6; i++ )
    {
        Sum[2] += s_TranslationCompSize[i];
        x_DebugMsg("%1d Translation %5d = %8d bytes = %5.2f bytes per key\n", 
            i, 
            s_TranslationCompUses[i], 
            (s_TranslationCompSize[i]+7)/8, 
            ((s_TranslationCompSize[i]+7)/8)/(f32)s_TranslationCompSamples[i] );
    }

    x_DebugMsg("------------------------------------------------------------------------\n");
    x_DebugMsg("Scale       bytes = %8d\n",(Sum[0]+7)/8);
    x_DebugMsg("Rotation    bytes = %8d\n",(Sum[1]+7)/8);
    x_DebugMsg("Translation bytes = %8d\n",(Sum[2]+7)/8);
    x_DebugMsg("------------------------------------------------------------------------\n");
}
*/
//=========================================================================
/*
void DisplayAnimCompressStats( const char* pFileName )
{
    s32 i;
    s32 Sum[3]={0};
    X_FILE* fp = x_fopen(pFileName,"wt");
    ASSERT(fp);
    if( !fp )
        return;


    x_fprintf(fp,"------------------------------------------------------------------------\n");
    x_fprintf(fp,"--                       ANIM COMPRESS STATS                          --\n");
    x_fprintf(fp,"------------------------------------------------------------------------\n");

    for( i=0; i<6; i++ )
    {
        Sum[0] += s_ScaleCompSize[i];
        x_fprintf(fp,"%1d Scale       %5d = %8d bytes = %5.2f bytes per key\n", 
            i, 
            s_ScaleCompUses[i], 
            (s_ScaleCompSize[i]+7)/8, 
            (s_ScaleCompSamples[i])?(((s_ScaleCompSize[i]+7)/8)/(f32)s_ScaleCompSamples[i]):(0));
    }

    x_fprintf(fp,"------------------------------------------------------------------------\n");

    for( i=0; i<6; i++ )
    {
        Sum[1] += s_RotationCompSize[i];
        x_fprintf(fp,"%1d Rotation    %5d = %8d bytes = %5.2f bytes per key\n", 
            i, 
            s_RotationCompUses[i], 
            (s_RotationCompSize[i]+7)/8, 
            (s_RotationCompSamples[i])?(((s_RotationCompSize[i]+7)/8)/(f32)s_RotationCompSamples[i]):(0) );
    }

    x_fprintf(fp,"------------------------------------------------------------------------\n");

    for( i=0; i<6; i++ )
    {
        Sum[2] += s_TranslationCompSize[i];
        x_fprintf(fp,"%1d Translation %5d = %8d bytes = %5.2f bytes per key\n", 
            i, 
            s_TranslationCompUses[i], 
            (s_TranslationCompSize[i]+7)/8, 
            (s_TranslationCompSamples[i])?(((s_TranslationCompSize[i]+7)/8)/(f32)s_TranslationCompSamples[i]):(0));
    }

    x_fprintf(fp,"------------------------------------------------------------------------\n");
    x_fprintf(fp,"Scale       bytes = %8d\n",(Sum[0]+7)/8);
    x_fprintf(fp,"Rotation    bytes = %8d\n",(Sum[1]+7)/8);
    x_fprintf(fp,"Translation bytes = %8d\n",(Sum[2]+7)/8);
    x_fprintf(fp,"------------------------------------------------------------------------\n");
    x_fprintf(fp,"nBones            = %8d\n",s_TotalNBones);
    x_fprintf(fp,"nFrames           = %8d\n",s_TotalFrames);
    x_fprintf(fp,"Bytes per Key     = %5.2f\n",(Sum[0]+Sum[1]+Sum[2])/(f32)(8*s_TotalFrames*s_TotalNBones));
    x_fprintf(fp,"------------------------------------------------------------------------\n");
    x_fclose(fp);
}
*/

//=========================================================================

static
void RLECompress( const u32* pData, s32 nSamples, bitstream& BS )
{
    s32 i;
    s32 nBitsPerSample;
    s32 nBitsPerCount;
    s32 MaxRunLength;

    //
    // Compute number of bits per sample
    //
    {
        u32 MaxSample=0;
        
        for( i=0; i<nSamples; i++ )
            MaxSample = MAX(MaxSample,pData[i]);

        nBitsPerSample = ComputeNBitsNeeded( MaxSample );
    }
    ASSERT( nBitsPerSample <= 8 );

    //
    // Compute number of bits per count and max run length
    //
    {
        s32 BestCountBits  = 0;
        s32 BestBitsNeeded = S32_MAX;
        for( i=3; i<8; i++ )
        {
            s32 MaxRunLength = (1<<i)-1;
            s32 nRuns=0;
            s32 RunStartI=0;
            s32 I=0;
            while( I < nSamples )
            {
                // Start a new run if we need to
                if( (I==0) || 
                    (pData[I] != pData[RunStartI]) ||
                    ((I-RunStartI+1) > MaxRunLength) )
                {
                    nRuns++;
                    RunStartI = I;
                }

                I++;
            }

            // Compute number of bits needed. Remember to add on bits for
            // terminator.
            s32 nBitsNeeded = ((nRuns+1) * i) + (nBitsPerSample*nSamples);
            if( nBitsNeeded < BestBitsNeeded )
            {
                BestBitsNeeded = nBitsNeeded;
                BestCountBits  = i;
            }
        }

        MaxRunLength = (1<<BestCountBits)-1;
        nBitsPerCount = BestCountBits;
    }

    //
    // Pack bitcounts into bitstream
    //
    BS.WriteRangedU32( (u32)nBitsPerCount,  3, 8  );   
    BS.WriteRangedU32( (u32)nBitsPerSample, 0, 32 );

    //
    // Pack samples into bitstream
    //
    {
        s32 I = 0;
        s32 TotalBitsWritten = BS.GetCursor();

        while( I < nSamples )
        {
            // Start a new run
            s32 RunStartI = I;
            s32 RunLength = 0;

            // Determine run length
            while( 1 )
            {
                if( I == nSamples ) break;
                if( pData[I] != pData[RunStartI] ) break;
                if( (I-RunStartI+1) > MaxRunLength) break;
                I++;
                RunLength++;
            }
            ASSERT( RunLength <= MaxRunLength );
            
            // Pack Count
            BS.WriteU32( RunLength, nBitsPerCount );
            BS.WriteU32( pData[RunStartI], nBitsPerSample );
        }

        // Add terminator
        BS.WriteU32( 0, nBitsPerCount );

        TotalBitsWritten = BS.GetCursor() - TotalBitsWritten;
        //x_DebugMsg("RLECompress: %d samples into %1.2f bytes\n",nSamples,TotalBitsWritten/(f32)8);
    }
    
}

//=========================================================================

s32 TOTAL_BITS = 0;
s32 TOTAL_OVERHEAD = 0;

static
void DeltaCompress( bitstream&  BS, 
                    const f32*  pSample, 
                    s32         Stride, 
                    s32         nSamples, 
                    f32         Precision )
{
    s32 i;
    s32 TotalBitsWritten = BS.GetCursor();

    //
    // Get min and max of deltas
    //
    s32 MinDelta =  S32_MAX;
    s32 MaxDelta = -S32_MAX;
    for( i=1; i<nSamples; i++ )
    {
        s32 S0 = F32ToS32( pSample[Stride*(i-1)] * Precision );
        s32 S1 = F32ToS32( pSample[Stride*(i+0)] * Precision );
        s32 Delta = S1 - S0;

        MinDelta = MIN(MinDelta,Delta);
        MaxDelta = MAX(MaxDelta,Delta);
    }

    //
    // Compute number of bits necessary to hold deltas
    //
    s32 nDeltaBits = ComputeNBitsNeeded( MaxDelta - MinDelta );    

    //
    // Pack initial sample
    //
    {
        // Convert first sample to integer
        s32 S = F32ToS32( pSample[0] * Precision );
        BS.WriteVariableLenS32( S );
    }

    //
    // Pack min delta
    //
    BS.WriteVariableLenS32( MinDelta );

    //
    // Pack number of bits for deltas
    //
    BS.WriteU32( nDeltaBits, 5 );

    s32 OverheadBits = BS.GetCursor() - TotalBitsWritten;

    //
    // Pack the deltas
    //
    //x_DebugMsg("[%2d] ",nDeltaBits);
    for( i=1; i<nSamples; i++ )
    {
        s32 S0 = F32ToS32( pSample[Stride*(i-1)] * Precision );
        s32 S1 = F32ToS32( pSample[Stride*(i+0)] * Precision );
        s32 Delta = (S1 - S0) - MinDelta;
        ASSERT( Delta >= 0 );
        BS.WriteU32( Delta, nDeltaBits );
        //x_DebugMsg("%2d ",ComputeNBitsNeeded( Delta ));
        //x_DebugMsg("%4d ",Delta);
    }
    //x_DebugMsg("\n");

    TotalBitsWritten = BS.GetCursor() - TotalBitsWritten;
    //x_DebugMsg("DeltaCompress: %d samples into %1.2f bytes. Overhead %1.2f\n",nSamples,(TotalBitsWritten+7)/(f32)8,(OverheadBits+7)/(f32)8);
    TOTAL_BITS += TotalBitsWritten;
    TOTAL_OVERHEAD += OverheadBits;
}

//=========================================================================
//=========================================================================
//=========================================================================
// SCALE
//=========================================================================
//=========================================================================
//=========================================================================

void CompressScale( bitstream&          BS,
                    s32                 iStream,
                    const anim_key*     pKeys,
                    s32                 StartFrame,
                    s32                 EndFrame,
                    s32                 TotalFrames,
                    f32                 DistToChilden,
                    s32&                DecompressedDataSize,
                    u32&                DecompressedFormat,
                    u32&                iDecompressor)
{
    (void)DistToChilden;
    s32     i,j;
    s32     nFrames = EndFrame - StartFrame + 1;

    //
    // Compute range of values
    //
    f32 MinValue[3] = {+F32_MAX,+F32_MAX,+F32_MAX};
    f32 MaxValue[3] = {-F32_MAX,-F32_MAX,-F32_MAX};
    for( i=StartFrame; i<=EndFrame; i++ )
    {
        f32* pF = (f32*)&pKeys[ iStream*TotalFrames + i ].Scale;
        for( j=0; j<3; j++ )
        {
            ASSERT(pF[j] >= (-S32_MAX)) ;
            ASSERT(pF[j] <= (+S32_MAX)) ;

            MinValue[j] = MIN(MinValue[j],pF[j]);
            MaxValue[j] = MAX(MaxValue[j],pF[j]);
        }
    }

    //
    // Check if all scales = 1.0f
    //
    if( 1 )
    {
        f32 E = 0.005f;
        for( i=0; i<3; i++ )
        {
            if( x_abs(1.0f - MinValue[i]) > E ) break;
            if( x_abs(1.0f - MaxValue[i]) > E ) break;
        }

        if( i==3 )
        {
            DecompressedFormat    = CONSTANT_VALUE;
            DecompressedDataSize  = 0;
            iDecompressor         = 0;
            return;
        }
    }

    //
    // Check if all scales are a single value
    //
    if( 1 )
    {
        f32 E = 0.01f;
        for( i=0; i<3; i++ )
            if( (MaxValue[i] - MinValue[i]) > E ) break;

        if( i==3 )
        {
            BS.WriteVariableLenS32( F32ToS32( (MaxValue[0] + MinValue[0]) * 0.5f * 128.0f ) );
            BS.WriteVariableLenS32( F32ToS32( (MaxValue[1] + MinValue[1]) * 0.5f * 128.0f ) );
            BS.WriteVariableLenS32( F32ToS32( (MaxValue[2] + MinValue[2]) * 0.5f * 128.0f ) );

            DecompressedFormat    = SINGLE_VALUE;
            DecompressedDataSize  = sizeof(vector3p);
            iDecompressor         = 1;
            return;
        }
    }

    //
    // Use delta compression
    //
    if( 1 )
    {
        const vector3* pScale = &pKeys[ iStream*TotalFrames + StartFrame ].Scale;
        s32      Stride = sizeof(anim_key)/sizeof(f32);

        DeltaCompress( BS, &((f32*)pScale)[0], Stride, nFrames, 128.0f );
        DeltaCompress( BS, &((f32*)pScale)[1], Stride, nFrames, 128.0f );
        DeltaCompress( BS, &((f32*)pScale)[2], Stride, nFrames, 128.0f );

        DecompressedFormat    = PRECISION_32;
        DecompressedDataSize  = sizeof(vector3p)*nFrames;
        iDecompressor         = 2;
        return;
    }

    //
    // Store as full precision
    //
    if( 1 )
    {
        for( i=StartFrame; i<=EndFrame; i++ )
        {
            const anim_key& Key = pKeys[ iStream*TotalFrames + i ];
            BS.WriteVector( Key.Scale );
        }

        DecompressedFormat    = PRECISION_32;
        DecompressedDataSize  = sizeof(vector3p)*nFrames;
        iDecompressor         = 3;
        return;
    }

}

//=========================================================================
//=========================================================================
//=========================================================================
// ROTATION
//=========================================================================
//=========================================================================
//=========================================================================

void CompressRotation( bitstream&          BS,
                    s32                 iStream,
                    const anim_key*     pKeys,
                    s32                 StartFrame,
                    s32                 EndFrame,
                    s32                 TotalFrames,
                    f32                 DistToChildren,
                    s32&                DecompressedDataSize,
                    u32&                DecompressedFormat,
                    u32&                iDecompressor)
{
    s32     i,j;
    s32     nFrames = EndFrame - StartFrame + 1;

    //
    // Compute range of values
    //
    f32 MinValue[4] = {+F32_MAX,+F32_MAX,+F32_MAX,+F32_MAX};
    f32 MaxValue[4] = {-F32_MAX,-F32_MAX,-F32_MAX,-F32_MAX};
    for( i=StartFrame; i<=EndFrame; i++ )
    {
        f32* pF = (f32*)&pKeys[ iStream*TotalFrames + i ].Rotation;
        for( j=0; j<4; j++ )
        {
            MinValue[j] = MIN(MinValue[j],pF[j]);
            MaxValue[j] = MAX(MaxValue[j],pF[j]);
        }
    }

    //
    // Check if all rotations are the identity
    //
    if( 1 )
    {
        f32 QV[4] = {0,0,0,1};
        f32 E = 0.001f;

        for( i=StartFrame; i<=EndFrame; i++ )   
        {
            const quaternion& KQ = pKeys[ iStream*TotalFrames + i ].Rotation;
            if( x_abs(KQ.X-QV[0]) > E ) break;
            if( x_abs(KQ.Y-QV[1]) > E ) break;
            if( x_abs(KQ.Z-QV[2]) > E ) break;
            if( x_abs(KQ.W-QV[3]) > E ) break;
        }

        if( i > EndFrame )
        {
            DecompressedFormat    = CONSTANT_VALUE;
            DecompressedDataSize  = 0;
            iDecompressor         = 0;
            return;
        }
    }

    //
    // Check if all quaternions are a single value
    //
    if( 1 )
    {
        f32 E = 0.001f;
        for( i=0; i<4; i++ )
            if( (MaxValue[i] - MinValue[i]) > E ) break;

        if( i==4 )
        {
            quaternion Q;
            Q.X = (MaxValue[0] + MinValue[0]) * 0.5f;
            Q.Y = (MaxValue[1] + MinValue[1]) * 0.5f;
            Q.Z = (MaxValue[2] + MinValue[2]) * 0.5f;
            Q.W = (MaxValue[3] + MinValue[3]) * 0.5f;
            Q.Normalize();

            BS.WriteVariableLenS32( F32ToS32( Q.X * 2048.0f ) );
            BS.WriteVariableLenS32( F32ToS32( Q.Y * 2048.0f ) );
            BS.WriteVariableLenS32( F32ToS32( Q.Z * 2048.0f ) );
            BS.WriteVariableLenS32( F32ToS32( Q.W * 2048.0f ) );

            DecompressedFormat    = SINGLE_VALUE;
            DecompressedDataSize  = sizeof(quaternion);
            iDecompressor         = 1;
            return;
        }
    }


    //
    // Use delta compression
    //
    if( 1 )//&& (DistToChildren < 800) )
    {
        f32 Prec;

        // Decide on Precision
        if( DistToChildren < 300 )
        {
            Prec = 2048.0f;
            iDecompressor = 2;
        }
        else
        if( DistToChildren < 500 )
        {
            Prec = (f32)(1<<14);
            iDecompressor = 4;
        }
        else
        {
            Prec = (f32)(1<<16);
            iDecompressor = 5;
        }

        const quaternion* pQ = &pKeys[ iStream*TotalFrames + StartFrame ].Rotation;
        s32      Stride = sizeof(anim_key)/sizeof(f32);

        DeltaCompress( BS, &pQ->X, Stride, nFrames, Prec );
        DeltaCompress( BS, &pQ->Y, Stride, nFrames, Prec );
        DeltaCompress( BS, &pQ->Z, Stride, nFrames, Prec );
        DeltaCompress( BS, &pQ->W, Stride, nFrames, Prec );

        DecompressedFormat    = PRECISION_16;
        DecompressedDataSize  = 8*nFrames;
        return;
    }

    //
    // Pack with full precision
    //
    if( 1 )
    {
        for( i=StartFrame; i<=EndFrame; i++ )
        {
            const anim_key& Key = pKeys[ iStream*TotalFrames + i ];
            BS.WriteQuaternion( Key.Rotation );
        }

        DecompressedFormat    = PRECISION_32;
        DecompressedDataSize  = sizeof(quaternion)*nFrames;
        iDecompressor         = 3;
        return;
    }
}

//=========================================================================
//=========================================================================
//=========================================================================
// TRANSLATION
//=========================================================================
//=========================================================================
//=========================================================================

void CompressTranslation( bitstream&          BS,
                    const anim_bone*    pBone,
                    s32                 nBones,
                    s32                 iStream,
                    const anim_key*     pKeys,
                    s32                 StartFrame,
                    s32                 EndFrame,
                    s32                 TotalFrames,
                    f32                 DistToChilden,
                    s32&                DecompressedDataSize,
                    u32&                DecompressedFormat,
                    u32&                iDecompressor)
{
    (void)DistToChilden;
    s32     i,j;
    s32     nFrames = EndFrame - StartFrame + 1;

    //
    // Compute range of values
    //
    f32 MinValue[3] = {+F32_MAX,+F32_MAX,+F32_MAX};
    f32 MaxValue[3] = {-F32_MAX,-F32_MAX,-F32_MAX};
    for( i=StartFrame; i<=EndFrame; i++ )
    {
        f32* pF = (f32*)&pKeys[ iStream*TotalFrames + i ].Translation;
        for( j=0; j<3; j++ )
        {
            MinValue[j] = MIN(MinValue[j],pF[j]);
            MaxValue[j] = MAX(MaxValue[j],pF[j]);
        }
    }

    //
    // Check if all translations are 0
    //
    if( 1 )
    {
        f32 E = 0.01f;
        for( i=0; i<3; i++ )
        {
            if( x_abs(0.0f - MinValue[i]) > E ) break;
            if( x_abs(0.0f - MaxValue[i]) > E ) break;
        }

        if( i==3 )
        {
            DecompressedFormat    = CONSTANT_VALUE;
            DecompressedDataSize  = 0;
            iDecompressor         = 0;
            return;
        }
    }

    //
    // Check if all translations are equal to the local translation
    //
    if( 1 )
    {
        if( iStream < nBones )
        {
            vector3 LT = pBone[iStream].LocalTranslation;
            f32 E = 0.1f;
            for( i=0; i<3; i++ )
            {
                if( x_abs(LT[i] - MinValue[i]) > E ) break;
                if( x_abs(LT[i] - MaxValue[i]) > E ) break;
            }

            if( i==3 )
            {
                DecompressedFormat    = SINGLE_VALUE;
                DecompressedDataSize  = sizeof(vector3p);
                iDecompressor         = 1;
                return;
            }
        }
    }

    //
    // Check if all translations are a single value
    //
    if( 1 )
    {
        f32 E = 0.1f;
        for( i=0; i<3; i++ )
            if( (MaxValue[i] - MinValue[i]) > E ) break;

        if( i==3 )
        {
            DecompressedFormat    = SINGLE_VALUE;
            DecompressedDataSize  = sizeof(vector3p);
            iDecompressor         = 2;

            BS.WriteVariableLenS32( F32ToS32( (MaxValue[0] + MinValue[0]) * 0.5f * 128.0f ) );
            BS.WriteVariableLenS32( F32ToS32( (MaxValue[1] + MinValue[1]) * 0.5f * 128.0f ) );
            BS.WriteVariableLenS32( F32ToS32( (MaxValue[2] + MinValue[2]) * 0.5f * 128.0f ) );
            return;
        }
    }

    //
    // Use delta compression
    //
    if( 1 )
    {
        const vector3* pTrans = &pKeys[ iStream*TotalFrames + StartFrame ].Translation;
        s32      Stride = sizeof(anim_key)/sizeof(f32);

        DeltaCompress( BS, &((f32*)pTrans)[0], Stride, nFrames, 128 );
        DeltaCompress( BS, &((f32*)pTrans)[1], Stride, nFrames, 128 );
        DeltaCompress( BS, &((f32*)pTrans)[2], Stride, nFrames, 128 );

        DecompressedFormat    = PRECISION_32;
        DecompressedDataSize  = sizeof(vector3p)*nFrames;
        iDecompressor         = 3;
        return;
    }

    //
    // Store as full precision
    //
    if( 1 )
    {
        for( i=StartFrame; i<=EndFrame; i++ )
        {
            const anim_key& Key = pKeys[ iStream*TotalFrames + i ];
            BS.WriteVector( Key.Translation );
        }

        DecompressedFormat    = PRECISION_32;
        DecompressedDataSize  = sizeof(vector3p)*nFrames;
        iDecompressor         = 4;
        return;
    }
}

//=========================================================================

void ComputeLenToChildren( const anim_bone*    pBone,
                           s32                 nBones,
                           f32*                pLen )
{
    s32 i;

    // Clear lengths
    for( i=0; i<nBones; i++ )
        pLen[i] = 0;

    i = nBones-1;
    while( i > 0 )
    {
        if( pBone[i].iParent != -1 )
        {
            f32 CurLen   = pLen[ pBone[i].iParent ];
            f32 ChildLen = pBone[i].LocalTranslation.Length() + pLen[i];
            pLen[ pBone[i].iParent ] = MAX( CurLen, ChildLen );
        }

        i--;
    }
}

//=========================================================================
//=========================================================================
//=========================================================================
// ANIM_COMPRESS
//=========================================================================
//=========================================================================
//=========================================================================

void CompressAnimationData( bitstream&          aBS,
                            const anim_key*     pKeys,
                            const anim_bone*    pBone,
                            s32                 nBones,
                            const u32*          pStreamFlags,
                            s32                 TotalFrames,
                            s32                 TotalStreams,
                            s32                 StartFrame,
                            s32                 EndFrame,
                            f32*                pLenOfBonesToVerts,
                            s32&                DecompressedDataSize,
                            s32&                iAnimBoneMin,
                            s32&                iAnimBoneMax )
{
    s32 i;
    bitstream BS;

    // Confirm we are on a byte boundary
    ASSERT( (BS.GetCursor()%8) == 0 );
    s_TotalNBones = nBones;
    s_TotalFrames += TotalFrames;

    // Allocate room for storing formats 
    u32* pSFormat = (u32*)x_malloc( sizeof(u32)*TotalStreams );
    u32* pRFormat = (u32*)x_malloc( sizeof(u32)*TotalStreams );
    u32* pTFormat = (u32*)x_malloc( sizeof(u32)*TotalStreams );
    u32* pSDIndex = (u32*)x_malloc( sizeof(u32)*TotalStreams );
    u32* pRDIndex = (u32*)x_malloc( sizeof(u32)*TotalStreams );
    u32* pTDIndex = (u32*)x_malloc( sizeof(u32)*TotalStreams );
    ASSERT( pSFormat && pRFormat && pTFormat );
    x_memset( pSFormat, 0, sizeof(u32)*TotalStreams );
    x_memset( pRFormat, 0, sizeof(u32)*TotalStreams );
    x_memset( pTFormat, 0, sizeof(u32)*TotalStreams );
    ASSERT( pSDIndex && pRDIndex && pTDIndex );
    x_memset( pSDIndex, 0, sizeof(u32)*TotalStreams );
    x_memset( pRDIndex, 0, sizeof(u32)*TotalStreams );
    x_memset( pTDIndex, 0, sizeof(u32)*TotalStreams );

    // Setup jump to header info
    s32 HeaderJumpCursor = BS.GetCursor();
    BS.WriteU32( 0 );

    //
    // Write out compression information for each stream
    //
    {
        // Start accumulating decompressed data size
        DecompressedDataSize = 0;

        //
        // Decide and compress each of the individual streams.  
        //
        for( i=0; i<TotalStreams; i++ )
        {
            s32 SizeOfDecompressed;
            s32 Cursor;
            s32 nFrames = EndFrame - StartFrame + 1;

            f32 LenToChildren = (i < nBones) ? (pLenOfBonesToVerts[i]) : (pLenOfBonesToVerts[0]);

            Cursor = BS.GetCursor();
            CompressScale       ( BS, i, pKeys, StartFrame, EndFrame, TotalFrames, LenToChildren, SizeOfDecompressed, pSFormat[i], pSDIndex[i] );
            DecompressedDataSize += SizeOfDecompressed;
            s_ScaleCompUses[ pSDIndex[i] ]++;
            s_ScaleCompSize[ pSDIndex[i] ] += BS.GetCursor() - Cursor;
            s_ScaleCompSamples[ pSDIndex[i] ] += nFrames;

            Cursor = BS.GetCursor();
            CompressRotation    ( BS, i, pKeys, StartFrame, EndFrame, TotalFrames, LenToChildren, SizeOfDecompressed, pRFormat[i], pRDIndex[i] );
            DecompressedDataSize += SizeOfDecompressed;
            s_RotationCompUses[ pRDIndex[i] ]++;
            s_RotationCompSize[ pRDIndex[i] ] += BS.GetCursor() - Cursor;
            s_RotationCompSamples[ pRDIndex[i] ] += nFrames;

            Cursor = BS.GetCursor();
            CompressTranslation ( BS, pBone, nBones, i, pKeys, StartFrame, EndFrame, TotalFrames, LenToChildren, SizeOfDecompressed, pTFormat[i], pTDIndex[i] );
            DecompressedDataSize += SizeOfDecompressed;
            s_TranslationCompUses[ pTDIndex[i] ]++;
            s_TranslationCompSize[ pTDIndex[i] ] += BS.GetCursor() - Cursor;
            s_TranslationCompSamples[ pTDIndex[i] ] += nFrames;

            // Is this a bone stream? (as opposed to being a prop stream)
            if( i < nBones )
            {
                xbool bAnimated = FALSE;
                
                // Is scale animated?
                if( ( pSFormat[i] != CONSTANT_VALUE ) && ( pSFormat[i] != SINGLE_VALUE ) )
                    bAnimated = TRUE;

                // Is rotation animated?
                if( ( pRFormat[i] != CONSTANT_VALUE ) && ( pRFormat[i] != SINGLE_VALUE ) )
                    bAnimated = TRUE;

                // Is translation animated?
                if( ( pTFormat[i] != CONSTANT_VALUE ) && ( pTFormat[i] != SINGLE_VALUE ) )
                    bAnimated = TRUE;

                // Update anim info?
                if( bAnimated )
                {
                    // Update min
                    if( ( iAnimBoneMin == -1 ) || ( i < iAnimBoneMin ) )
                        iAnimBoneMin = i;

                    // Update max
                    if( i > iAnimBoneMax )
                        iAnimBoneMax = i;
                }
            }
        }
    }

    //x_DebugMsg("TOTAL_BITS %d\n",TOTAL_BITS);
    //x_DebugMsg("TOTAL_OVERHEAD %d\n",TOTAL_OVERHEAD);
    //x_DebugMsg("PERC_OVERHEAD  %f\n",100.0f*TOTAL_OVERHEAD/TOTAL_BITS);

    //
    // Patch the header Jump 
    //
    {
        u32 Cursor = BS.GetCursor();
        BS.SetCursor( HeaderJumpCursor );
        BS.WriteU32( Cursor );
        BS.SetCursor( Cursor );
    }

    //
    // Write out general header info
    //
    BS.WriteU32( TotalStreams, 10 );
    BS.WriteRangedU32( EndFrame - StartFrame + 1, 2, MAX_KEYS_PER_BLOCK+1 );

    //
    // Write out the information for the stream structures
    //
    RLECompress( pSFormat,      TotalStreams, BS );
    RLECompress( pRFormat,      TotalStreams, BS );
    RLECompress( pTFormat,      TotalStreams, BS );
    RLECompress( pStreamFlags,  TotalStreams, BS );
    RLECompress( pSDIndex,      TotalStreams, BS );
    RLECompress( pRDIndex,      TotalStreams, BS );
    RLECompress( pTDIndex,      TotalStreams, BS );

    //
    // Release format buffers
    //
    x_free( pSFormat );
    x_free( pRFormat );
    x_free( pTFormat );
    x_free( pSDIndex );
    x_free( pRDIndex );
    x_free( pTDIndex );

    //
    // We are done so add padding to reach the next byte boundary
    //
    BS.AlignCursor(3);
    ASSERT( (BS.GetCursor()%8) == 0 );

    //
    // Add on stream overhead to decompressed size
    //
    DecompressedDataSize += TotalStreams * sizeof(anim_key_stream);
    //DecompressedDataSize += (12+16+12)*(EndFrame - StartFrame + 1)*TotalStreams;

    //
    // Add our sub-bitstream to the bitstream passed in.  For this to
    // work the cursor must be positioned at the end of BS
    //
    aBS.WriteBits( BS.GetDataPtr(), BS.GetCursor() );

}

//=========================================================================


#endif // !DISABLE_LEGACY_CODE
