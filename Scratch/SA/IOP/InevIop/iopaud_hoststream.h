#ifndef __HOSTSTREAM_H
#define __HOSTSTREAM_H


void hoststream_Init(s32 Pitch);
void hoststream_Kill(void);
s32  hoststream_Update(u8 *pData,s32 Length,s32 Index);

#endif