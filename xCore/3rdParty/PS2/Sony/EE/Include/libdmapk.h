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
 *                        libpkt - libdmapk.h
 *                        dma packet support
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.01           Mar,26,1999     shibuya
 */

#ifndef __libdmapk__
#define __libdmapk__

#include <eekernel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
  u_int *pCurrent;
  u_long128 *pBase;
  u_long128 *pDmaTag;
  u_int pad03;
}sceDmaPacket;

void sceDmaPkInit(sceDmaPacket *pPacket, u_long128 *pBase);

void sceDmaPkReset(sceDmaPacket *pPacket);
u_long128 *sceDmaPkTerminate(sceDmaPacket *pPacket);

u_int sceDmaPkSize(sceDmaPacket *pPacket);

void sceDmaPkCnt(sceDmaPacket *pPacket, u_int opt1, u_int opt2, u_int flag);
void sceDmaPkRet(sceDmaPacket *pPacket, u_int opt1, u_int opt2, u_int flag);
void sceDmaPkEnd(sceDmaPacket *pPacket, u_int opt1, u_int opt2, u_int flag);

void sceDmaPkNext(sceDmaPacket *pPacket, u_long128 *pNext, u_int opt1, u_int opt2, u_int flag);
void sceDmaPkCall(sceDmaPacket *pPacket, u_long128 *pCall, u_int opt1, u_int opt2, u_int flag);

void sceDmaPkRefe(sceDmaPacket *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);
void sceDmaPkRef(sceDmaPacket *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);
void sceDmaPkRefs(sceDmaPacket *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);

u_int *sceDmaPkReserve(sceDmaPacket *pPacket, u_int count);

void sceDmaPkAddData(sceDmaPacket *pPacket, u_long128 data);
void sceDmaPkAddDataN(sceDmaPacket *pPacket, u_long128* pData, u_int count);

void sceDmaPkDump(sceDmaPacket *pPacket);

#ifdef __cplusplus
}
#endif

#endif /* __libdmapk__ */
