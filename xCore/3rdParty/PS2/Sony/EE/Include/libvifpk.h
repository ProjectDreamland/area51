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
 *                        libpkt - libvifpk.h
 *                        vif packet support
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.01           Mar,26,1999     shibuya
 */

#ifndef __libvifpk__
#define __libvifpk__

#include <eekernel.h>
#include <eestruct.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct{
  u_int *pCurrent;
  u_long128 *pBase;
  u_long128 *pDmaTag;
  u_int *pVifCode;

  u_int numlen;
  u_int pad11;
  u_int pad12;
  u_int pad13;
}sceVif0Packet;

typedef struct{
  u_int *pCurrent;
  u_long128 *pBase;
  u_long128 *pDmaTag;
  u_int *pVifCode;

  u_int numlen;
  u_long *pGifTag;
  u_int pad12;
  u_int pad13;
}sceVif1Packet;



/*------------------------------------------------------------------*/

void sceVif0PkInit(sceVif0Packet *pPacket, u_long128 *pBase);

void sceVif0PkReset(sceVif0Packet *pPacket);
u_long128 *sceVif0PkTerminate(sceVif0Packet *pPacket);

u_int sceVif0PkSize(sceVif0Packet *pPacket);

void sceVif0PkCnt(sceVif0Packet *pPacket, u_int flag);
void sceVif0PkRet(sceVif0Packet *pPacket, u_int flag);
void sceVif0PkEnd(sceVif0Packet *pPacket, u_int flag);

void sceVif0PkNext(sceVif0Packet *pPacket, u_long128 *pNext, u_int flag);
void sceVif0PkCall(sceVif0Packet *pPacket, u_long128 *pCall, u_int flag);

void sceVif0PkRefe(sceVif0Packet *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);
void sceVif0PkRef(sceVif0Packet *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);
void sceVif0PkRefs(sceVif0Packet *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);

void sceVif0PkOpenUpkCode(sceVif0Packet *pPacket, u_short vuaddr, u_int upkcmd, u_int cl, u_int wl);
void sceVif0PkCloseUpkCode(sceVif0Packet *pPacket);

u_int *sceVif0PkReserve(sceVif0Packet *pPacket, u_int count);

void sceVif0PkAddCode(sceVif0Packet *pPacket, u_int code);
void sceVif0PkAddData(sceVif0Packet *pPacket, u_int data);
void sceVif0PkAddDataN(sceVif0Packet *pPacket, u_int* pData, u_int count);

void sceVif0PkAddUpkData32(sceVif0Packet *pPacket, u_int data);
void sceVif0PkAddUpkData64(sceVif0Packet *pPacket, u_long data);
void sceVif0PkAddUpkData128(sceVif0Packet *pPacket, u_long128 data);
void sceVif0PkAddUpkData32N(sceVif0Packet *pPacket, u_int *pData, u_int count);
void sceVif0PkAddUpkData64N(sceVif0Packet *pPacket, u_long *pData, u_int count);
void sceVif0PkAddUpkData128N(sceVif0Packet *pPacket, u_long128 *pData, u_int count);

void sceVif0PkAlign(sceVif0Packet *pPacket, u_int bit, u_int pos);

void sceVif0PkRefMpg(sceVif0Packet *pPacket, u_short vuaddr, u_long128 *pMicro, u_int size, u_int opt1);


void sceVif0PkDump(sceVif0Packet *pPacket);

/*------------------------------------------------------------------*/

void sceVif1PkInit(sceVif1Packet *pPacket, u_long128 *pBase);

void sceVif1PkReset(sceVif1Packet *pPacket);
u_long128 *sceVif1PkTerminate(sceVif1Packet *pPacket);

u_int sceVif1PkSize(sceVif1Packet *pPacket);

void sceVif1PkCnt(sceVif1Packet *pPacket, u_int flag);
void sceVif1PkRet(sceVif1Packet *pPacket, u_int flag);
void sceVif1PkEnd(sceVif1Packet *pPacket, u_int flag);

void sceVif1PkNext(sceVif1Packet *pPacket, u_long128 *pNext, u_int flag);
void sceVif1PkCall(sceVif1Packet *pPacket, u_long128 *pCall, u_int flag);

void sceVif1PkRefe(sceVif1Packet *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);
void sceVif1PkRef(sceVif1Packet *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);
void sceVif1PkRefs(sceVif1Packet *pPacket, u_long128 *pRef, u_int size, u_int opt1, u_int opt2, u_int flag);

void sceVif1PkOpenUpkCode(sceVif1Packet *pPacket, u_short vuaddr, u_int upkcmd, u_int cl, u_int wl);
void sceVif1PkCloseUpkCode(sceVif1Packet *pPacket);

void sceVif1PkOpenDirectCode(sceVif1Packet *pPacket, int stall);
void sceVif1PkCloseDirectCode(sceVif1Packet *pPacket);

void sceVif1PkOpenDirectHLCode(sceVif1Packet *pPacket, int stall);
void sceVif1PkCloseDirectHLCode(sceVif1Packet *pPacket);

void sceVif1PkOpenGifTag(sceVif1Packet *pPacket, u_long128 gifTag);
void sceVif1PkCloseGifTag(sceVif1Packet *pPacket);

u_int *sceVif1PkReserve(sceVif1Packet *pPacket, u_int count);
void sceVif1PkAlign(sceVif1Packet *pPacket, u_int bit, u_int pos);

void sceVif1PkAddCode(sceVif1Packet *pPacket, u_int code);
void sceVif1PkAddData(sceVif1Packet *pPacket, u_int data);
void sceVif1PkAddDataN(sceVif1Packet *pPacket, u_int *pData, u_int count);

void sceVif1PkAddUpkData32(sceVif1Packet *pPacket, u_int data);
void sceVif1PkAddUpkData64(sceVif1Packet *pPacket, u_long data);
void sceVif1PkAddUpkData128(sceVif1Packet *pPacket, u_long128 data);
void sceVif1PkAddUpkData32N(sceVif1Packet *pPacket, u_int *pData, u_int count);
void sceVif1PkAddUpkData64N(sceVif1Packet *pPacket, u_long *pData, u_int count);
void sceVif1PkAddUpkData128N(sceVif1Packet *pPacket, u_long128 *pData, u_int count);

void sceVif1PkAddDirectData(sceVif1Packet *pPacket, u_long128 data);
void sceVif1PkAddDirectDataN(sceVif1Packet *pPacket, u_long128* pData, u_int count);

void sceVif1PkAddGsData(sceVif1Packet *pPacket, u_long data);
void sceVif1PkAddGsDataN(sceVif1Packet *pPacket, u_long *pData, u_int count);

void sceVif1PkAddGsPacked(sceVif1Packet* pPacket, u_long128 data);
void sceVif1PkAddGsPackedN(sceVif1Packet* pPacket, u_long128* pData, u_int count);

void sceVif1PkAddGsAD(sceVif1Packet *pPacket, u_int addr, u_long data);

void sceVif1PkRefLoadImage(sceVif1Packet *pPacket, u_short bp, u_char psm, u_short bw, u_long128 *image, u_int size, u_int x, u_int y, u_int w, u_int h);

void sceVif1PkRefMpg(sceVif1Packet *pPacket, u_short vuaddr, u_long128 *pMicro, u_int size, u_int opt1);

void sceVif1PkDump(sceVif1Packet *pPacket);

#ifdef __cplusplus
}
#endif

#endif /* __libvifpk__ */
