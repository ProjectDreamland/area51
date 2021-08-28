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
 *                        libdev - devvu1.h
 *                         develop library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.01           Mar,29,1999     shibuya
 */

#ifndef __devvu1__
#define __devvu1__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
  u_long128 vf[32];
  u_int status;
  u_int mac;
  u_int clipping;
  u_int r, i, q, p;
  u_short vi[16];
}sceDevVu1Cnd;

void sceDevVu1Reset(void);

int sceDevVu1Pause(void);

int sceDevVu1Continue(void);

void sceDevVu1PutDBit(int bit);

void sceDevVu1PutTBit(int bit);

int sceDevVu1GetDBit(void);

int sceDevVu1GetTBit(void);

void sceDevVu1Exec(u_short addr);

u_short sceDevVu1GetTpc(void);

int sceDevVu1GetCnd(sceDevVu1Cnd *cnd);

int sceDevVu1PutCnd(sceDevVu1Cnd *cnd);

#ifdef __cplusplus
}
#endif

#endif
