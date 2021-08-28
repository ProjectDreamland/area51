#ifndef __ENCODE_H
#define __ENCODE_H

#include "x_types.hpp"
#include "decode.hpp"

s32 ps2_EncodeToAdpcm(s32 Length,s16 *pOutBuffer,t_DecodeHeader *pHeader);
s32 gcn_EncodeToAdpcm(s32 Length,s16 *pOutBuffer,t_DecodeHeader *pHeader);
s32 pc_Encode(s32 Length, u8* pOutBuffer, s8* pFilename);

void gcn_EncodeInit(void);
void gcn_EncodeKill(void);

void ps2_EncodeInit(void);
void ps2_EncodeKill(void);

#endif