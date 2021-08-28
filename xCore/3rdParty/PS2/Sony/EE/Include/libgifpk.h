/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 0.01
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                        libpkt - libgifpk.h
 *                        gif packet support
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.01           Mar,26,1999     shibuya
 */

#ifndef __libgifpk__
#define __libgifpk__

#include <eekernel.h>
#include <eestruct.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
  u_int *pCurrent;
  u_long128 *pBase;
  u_long128 *pDmaTag;
  u_long *pGifTag;
}sceGifPacket;


void sceGifPkInit(sceGifPacket *pPacket, u_long128 *pBase);

void sceGifPkReset(sceGifPacket *pPacket);
u_long128 *sceGifPkTerminate(sceGifPacket *pPacket);

u_int sceGifPkSize(sceGifPacket *pPacket);

void sceGifPkCnt(sceGifPacket *pPacket, u_int opt1, u_int opt2, u_int flag);
void sceGifPkRet(sceGifPacket *pPacket, u_int opt1, u_int opt2, u_int flag);
void sceGifPkEnd(sceGifPacket *pPacket, u_int opt1, u_int opt2, u_int flag);

void sceGifPkNext(sceGifPacket *pPacket, u_long128 *pNext, u_int opt1, u_int opt2, u_int flag);
void sceGifPkCall(sceGifPacket *pPacket, u_long128 *pCall, u_int opt1, u_int opt2, u_int flag);

void sceGifPkRefe(sceGifPacket *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);
void sceGifPkRef(sceGifPacket *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);
void sceGifPkRefs(sceGifPacket *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);

u_int *sceGifPkReserve(sceGifPacket *pPacket, u_int count);

void sceGifPkOpenGifTag(sceGifPacket *pPacket, u_long128 gifTag);
void sceGifPkCloseGifTag(sceGifPacket *pPacket);

void sceGifPkAddGsData(sceGifPacket *pPacket, u_long data);
void sceGifPkAddGsDataN(sceGifPacket *pPacket, u_long *pData, u_int count);

void sceGifPkAddGsPacked(sceGifPacket* pPacket, u_long128 data);
void sceGifPkAddGsPackedN(sceGifPacket* pPacket, u_long128* pData, u_int count);

void sceGifPkAddGsAD(sceGifPacket *pPacket, u_int addr, u_long data);

void sceGifPkRefLoadImage(sceGifPacket *pPacket, u_short bp, u_char psm, u_short bw, u_long128 *image, u_int size, u_int x, u_int y, u_int w, u_int h);


void sceGifPkDump(sceGifPacket *pPacket);

#ifdef __cplusplus
}
#endif

#endif /* __libgifpk__ */
