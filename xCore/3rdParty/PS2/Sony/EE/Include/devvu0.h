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
 *                        libdev - devvu0.h
 *                         develop library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.01           Mar,29,1999     shibuya
 */

#ifndef __devvu0__
#define __devvu0__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
  u_long128 vf[32];
  u_int status;
  u_int mac;
  u_int clipping;
  u_int r, i, q;
  u_short vi[16];
}sceDevVu0Cnd;

void sceDevVu0Reset(void);

int sceDevVu0Pause(void);

int sceDevVu0Continue(void);

void sceDevVu0PutDBit(int bit);

void sceDevVu0PutTBit(int bit);

int sceDevVu0GetDBit(void);

int sceDevVu0GetTBit(void);

void sceDevVu0Exec(u_short addr);

u_short sceDevVu0GetTpc(void);

int sceDevVu0GetCnd(sceDevVu0Cnd *cnd);

int sceDevVu0PutCnd(sceDevVu0Cnd *cnd);

#ifdef __cplusplus
}
#endif

#endif
