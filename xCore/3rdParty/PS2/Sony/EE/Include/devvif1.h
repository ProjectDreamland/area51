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
 *                        libdev - devvif1.h
 *                         develop library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.01           Mar,29,1999     shibuya
 */

#ifndef __devvif1__
#define __devvif1__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
  u_int		row[4];
  u_int		col[4];
  u_int		mask;
  u_int		code;
  u_int		stat;
  u_short	itop,itops;
  u_short	base,offset;
  u_short	top,tops;
  u_short	mark;
  u_short	num;
  u_char	error;
  u_char	cl,wl;
  u_char	cmod;
  u_char	pad;
}sceDevVif1Cnd;


void sceDevVif1Reset(void);

int sceDevVif1Pause(int mode);

int sceDevVif1Continue(void);

u_int sceDevVif1PutErr(int interrupt, int miss1, int miss2);

u_int sceDevVif1GetErr(void);

int sceDevVif1GetCnd(sceDevVif1Cnd *);

int sceDevVif1PutFifo(u_long128 *addr, int n);

int sceDevVif1GetFifo(u_long128 *addr, int n);


#ifdef __cplusplus
}
#endif

#endif
