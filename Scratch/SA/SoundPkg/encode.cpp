#include "x_types.hpp"
#include "x_files.hpp"
#include "encode.hpp"
#include "library/encvag.h"
#include "memory.h"
#include "../../support/audiomgr/audio.hpp"
#include "iopaudio.h"


#define SAMPLES_PER_BLOCK 28
#define ENCODED_BLOCK_SIZE  8       // Size of encoded block in 16 bit words.
#define ENC_VAG_1_SHOT		0
#define ENC_VAG_1_SHOT_END	1
#define ENC_VAG_LOOP_START	2
#define ENC_VAG_LOOP_BODY	3
#define ENC_VAG_LOOP_END	4

//
// Encodes from PCM to sony format ADPCM. It's encoded in
// 2KB blocks with a 'BlockSkip' byte jump between them.
// Returns the length of data encoded.
//
s32 EncodeStream(s16 *pSource,s16 *pDest,s32 BlockSkip,s32 Length,s32 LoopStart,s32 LoopEnd)
{
    s32 EncodeLength;

    s16 TempBuffer[SAMPLES_PER_BLOCK];
    s16 *pIn;
    s16  *pOut;

    s32 Position,LoopStarted,Remain;

    pIn = pSource;
    pOut = pDest;
    EncodeLength = 0;
    Remain = 2048 / sizeof(s16);

    // Blank block for BRR filter initializing
    memset(pOut,0,ENCODED_BLOCK_SIZE * sizeof(s16));
    pOut+=ENCODED_BLOCK_SIZE;
    Remain-= ENCODED_BLOCK_SIZE;
    EncodeLength += ENCODED_BLOCK_SIZE * sizeof(s16);

    EncVagInit(ENC_VAG_MODE_NORMAL);

    // It's just easier dealing with samples instead of bytes
    Length = Length / 2;
    LoopStart = LoopStart / 2;
    LoopEnd = LoopEnd / 2;

    Position = 0;
    LoopStarted = 0;

    while (Length)
    {
        if (Length > SAMPLES_PER_BLOCK)
        {
            if ( (Position >= LoopStart) && (LoopStart != LoopEnd) )
            {
                if (Position < LoopEnd)
                {
                    if (LoopStarted)
                    {
                        EncVag(pIn,pOut,ENC_VAG_LOOP_BODY);
                    }
                    else
                    {
                        EncVag(pIn,pOut,ENC_VAG_LOOP_START);
                        LoopStarted = TRUE;
                    }
                }
                else
                {
                    if (LoopStarted)
                    {
                        EncVag(pIn,pOut,ENC_VAG_LOOP_END);
                        LoopStarted = FALSE;
                    }
                    else
                    {
                        EncVag(pIn,pOut,ENC_VAG_1_SHOT);
                    }
                }
            }
            else
            {
                EncVag(pIn,pOut,ENC_VAG_1_SHOT);
            }
            pIn += SAMPLES_PER_BLOCK;
            Length -= SAMPLES_PER_BLOCK;
            Position += SAMPLES_PER_BLOCK;
        }
        else
        {
            memset(TempBuffer,0,SAMPLES_PER_BLOCK*sizeof(s16));
            memcpy(TempBuffer,pIn,Length * sizeof(s16));
            if ( (LoopStart != LoopEnd) && LoopStarted )
            {
                EncVag(TempBuffer, pOut, ENC_VAG_LOOP_END );
            }
            else
            {
                EncVag(TempBuffer, pOut, ENC_VAG_1_SHOT_END );
            }
            Position += Length;
            Length = 0;
        }
        pOut += ENCODED_BLOCK_SIZE;
        EncodeLength += ENCODED_BLOCK_SIZE * sizeof(s16);
        Remain -= ENCODED_BLOCK_SIZE;
        if (Remain<=0)
        {
            ASSERT(Remain==0);      // We must be on a 16 byte boundary
            Remain = 2048/sizeof(s16);
            pOut  += (BlockSkip/sizeof(s16));
        }
    }

#if 0
    if ((EncodeLength & 2047)==0)
    {
        pOut-=ENCODED_BLOCK_SIZE;
        EncodeLength-=ENCODED_BLOCK_SIZE * sizeof(s16);
    }
#endif
    EncVagFin(pOut);
    EncodeLength+=ENCODED_BLOCK_SIZE * sizeof(s16);
    pOut+= ENCODED_BLOCK_SIZE;
    while ( (EncodeLength & 2047) && BlockSkip)
    {
        x_memset(pOut,0,ENCODED_BLOCK_SIZE*sizeof(s16));
        pOut+=ENCODED_BLOCK_SIZE;
        EncodeLength += ENCODED_BLOCK_SIZE * sizeof(s16);
    }

    return EncodeLength;
}

//-----------------------------------------------------------------------------
s32 EncodeToSonyAdpcm(s32 Length,s16 *pOutBuffer,t_DecodeHeader *pHeader)
{
    s32 length;
    s32 AlignedLength;

    if ( (pHeader->Type == CFXTYPE_ELEMENT_STREAM) ||
         (pHeader->Type == CFXTYPE_ELEMENT_HYBRID))
    {
        AlignedLength = (Length + 4095) & ~4095;
        if (AlignedLength - Length > 1)
        {
            if (pHeader->Flags & AUDFLAG_STEREO)
            {
                x_memset((s8 *)pHeader->pLeft+(Length/2),0,(AlignedLength - Length)/2);
                x_memset((s8 *)pHeader->pRight+(Length/2),0,(AlignedLength - Length)/2);

            }
            else
            {
                x_memset((s8 *)pHeader->pLeft+Length,0,(AlignedLength - Length)/2);
            }
        }
    }

    if (pHeader->Flags & AUDFLAG_STEREO)
    {
        length = EncodeStream(pHeader->pLeft,pOutBuffer,2048,pHeader->Length / 2,pHeader->LoopStart,pHeader->LoopEnd);
        length += EncodeStream(pHeader->pRight,pOutBuffer+1024,2048,pHeader->Length / 2,pHeader->LoopStart,pHeader->LoopEnd);
        x_printf("Stereo sample, length=%08x, encoded length=%08x\n",pHeader->Length,length);
    }
    else
    {
        length = EncodeStream(pHeader->pLeft,pOutBuffer,0,pHeader->Length,pHeader->LoopStart,pHeader->LoopEnd);
    }

    // return the length of the encoded output stream
    return length;

}
