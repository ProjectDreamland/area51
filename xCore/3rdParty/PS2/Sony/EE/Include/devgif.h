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
 *                        libdev - devgif.h
 *                         develop library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.01           Mar,29,1999     shibuya
 */

#ifndef __devgif__
#define __devgif__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
  u_long128 tag;
  u_int stat;
  u_int count;
  u_int p3count;
  u_int p3tag;
  u_int pad;
}sceDevGifCnd;

void sceDevGifReset(void);

int sceDevGifPause(void);

int sceDevGifContinue(void);

void sceDevGifPutImtMode(int);

u_int sceDevGifGetImtMode(void);

int sceDevGifPutP3msk(int enable);

int sceDevGifGetP3msk(void);

int sceDevGifGetCnd(sceDevGifCnd *);

int sceDevGifPutFifo(u_long128 *addr, int n);

#ifdef __cplusplus
}
#endif

#endif
