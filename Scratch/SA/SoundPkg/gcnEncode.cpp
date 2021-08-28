#include "x_types.hpp"
#include "x_files.hpp"
#include "encode.hpp"
#include "library/encvag.h"
#include "memory.h"
#include "../../support/audiomgr/audio.hpp"
#include "../../support/audiomgr/gcn/driver/gcnaud_audio.hpp"
#include "main.hpp"

typedef struct
{
    // start context
    u16 coef[16];
    u16 gain;
    u16 pred_scale;
    u16 yn1;
    u16 yn2;

    // loop context
    u16 loop_pred_scale;
    u16 loop_yn1;
    u16 loop_yn2;

} ADPCMINFO; 


typedef u32 (*lpFunc1)(u32);
typedef u32 (*lpFunc2)(void);
typedef void (*lpFunc3)(s16*, u8*, ADPCMINFO*, u32);
typedef void (*lpFunc4)(u8*, s16*, ADPCMINFO*, u32);
typedef void (*lpFunc5)(u8*, ADPCMINFO*, u32);

lpFunc1 getBytesForAdpcmBuffer;
lpFunc1 getBytesForAdpcmSamples;
lpFunc1 getBytesForPcmBuffer;
lpFunc1 getBytesForPcmSamples;
lpFunc1 getSampleForAdpcmNibble;
lpFunc1 getNibbleAddress;
lpFunc2 getBytesForAdpcmInfo;
lpFunc3 encode;
lpFunc4 decode;
lpFunc5 getLoopContext;


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
s32 gcnEncodeStream(s16 *pSource,s16 *pDest,s32 BlockSkip,s32 Length,s32 LoopStart,s32 LoopEnd)
{
    s32 EncodeLength,i;

    s16 *pIn;
    s16  *pOut;
    ADPCMINFO *pAdpcmInfo;

    s32 Remain;

    pIn = pSource;
    pOut = pDest;
    EncodeLength = 0;
    Remain = 2048 / sizeof(s16);

    // Do we need to store the ADPCM context? Is this just for looping?
    pAdpcmInfo = (ADPCMINFO *)pDest;
    pDest += sizeof(ADPCMINFO)/sizeof(s16);

    EncodeLength = sizeof(ADPCMINFO);
    EncodeLength += getBytesForAdpcmBuffer(Length /sizeof(s16));
    x_memset(pAdpcmInfo,0,sizeof(ADPCMINFO));

    encode(pSource,(u8*)pDest,pAdpcmInfo,Length / sizeof(s16));
    if (LoopStart)
    {
        x_printf("Loop context at %d end at %d, length=%d\n",LoopStart,LoopEnd,Length);
        getLoopContext((u8*)pDest,pAdpcmInfo,LoopStart / sizeof(s16) );
    }
    // Byteswap the adpcm header
    for (i=0;i<16;i++)
    {
        pAdpcmInfo->coef[i] = TargetEndian16(pAdpcmInfo->coef[i]);
    }
    pAdpcmInfo->gain = TargetEndian16(pAdpcmInfo->gain);
    pAdpcmInfo->pred_scale = TargetEndian16(pAdpcmInfo->pred_scale);
    pAdpcmInfo->yn1 = TargetEndian16(pAdpcmInfo->yn1);
    pAdpcmInfo->yn2 = TargetEndian16(pAdpcmInfo->yn2);
    pAdpcmInfo->loop_pred_scale = TargetEndian16(pAdpcmInfo->loop_pred_scale);
    pAdpcmInfo->loop_yn1 = TargetEndian16(pAdpcmInfo->loop_yn1);
    pAdpcmInfo->loop_yn2 = TargetEndian16(pAdpcmInfo->loop_yn2);


    return EncodeLength;
}

//-----------------------------------------------------------------------------
s32 gcn_EncodeToAdpcm(s32 Length,s16 *pOutBuffer,t_DecodeHeader *pHeader)
{
    s32 length;
    s32 AlignedLength;

    ASSERT(GetTargetPlatform()==TARGET_TYPE_GCN);

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
        length = gcnEncodeStream(pHeader->pLeft,pOutBuffer,2048,pHeader->Length / 2,pHeader->LoopStart,pHeader->LoopEnd);
        length += gcnEncodeStream(pHeader->pRight,pOutBuffer+1024,2048,pHeader->Length / 2,pHeader->LoopStart,pHeader->LoopEnd);
        x_printf("Stereo sample, length=%08x, encoded length=%08x\n",pHeader->Length,length);
    }
    else
    {
        length = gcnEncodeStream(pHeader->pLeft,pOutBuffer,0,pHeader->Length,pHeader->LoopStart,pHeader->LoopEnd);
    }

    // return the length of the encoded output stream
    return length;

}


static HINSTANCE hDll;

/*--------------------------------------------------------------------------*/
void gcn_EncodeKill(void)
{
    if (hDll)
        FreeLibrary(hDll);
    hDll = NULL;
}

void gcn_EncodeInit(void)
{
    xbool failed;

    hDll = LoadLibrary("dsptool.dll");
    failed = (hDll == NULL);
    if (hDll)
    {
        failed |= !(getBytesForAdpcmBuffer  = (lpFunc1)GetProcAddress(hDll,"getBytesForAdpcmBuffer" ));
        failed |= !(getBytesForAdpcmSamples = (lpFunc1)GetProcAddress(hDll,"getBytesForAdpcmSamples"));
        failed |= !(getBytesForPcmBuffer    = (lpFunc1)GetProcAddress(hDll,"getBytesForPcmBuffer"   ));
        failed |= !(getBytesForPcmSamples   = (lpFunc1)GetProcAddress(hDll,"getBytesForPcmSamples"  ));
        failed |= !(getNibbleAddress        = (lpFunc1)GetProcAddress(hDll,"getNibbleAddress"       ));
        failed |= !(getSampleForAdpcmNibble = (lpFunc1)GetProcAddress(hDll,"getSampleForAdpcmNibble"));
        failed |= !(getBytesForAdpcmInfo    = (lpFunc2)GetProcAddress(hDll,"getBytesForAdpcmInfo"   ));
        failed |= !(encode                  = (lpFunc3)GetProcAddress(hDll,"encode"                 ));
        failed |= !(decode                  = (lpFunc4)GetProcAddress(hDll,"decode"                 ));
        failed |= !(getLoopContext          = (lpFunc5)GetProcAddress(hDll,"getLoopContext"         ));
    }

    ASSERTS(!failed,"Unable to load dspadpcm.dll");
}

