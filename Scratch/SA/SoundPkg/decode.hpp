#ifndef __DECODE_H
#define __DECODE_H

#include "x_types.hpp"

typedef struct s_DecodeHeader
{
    s32             Length;
    s32             SampleRate;
    s16             Type;
    s16             Flags;
    s32             LoopStart;
    s32             LoopEnd;
    s16             *pLeft;
    s16             *pRight;
} t_DecodeHeader;

// Input a filename. Returns a ptr to a block of decoded data
// in pcm format. NULL if it can't decode it.
void DecodeToPcm(char *pFilename,t_DecodeHeader *pHeader);
#endif