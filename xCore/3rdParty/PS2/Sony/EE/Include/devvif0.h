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
 *                        libdev - devvif0.h
 *                         develop library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.01           Mar,29,1999     shibuya
 */

#ifndef __devvif0__
#define __devvif0__

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
  u_short	mark;
  u_short	num;
  u_char	error;
  u_char	cl,wl;
  u_char	cmod;
  u_char	pad;
}sceDevVif0Cnd;


void sceDevVif0Reset(void);

int sceDevVif0Pause(int mode);

int sceDevVif0Continue(void);

u_int sceDevVif0PutErr(int interrupt, int miss1, int miss2);

u_int sceDevVif0GetErr(void);

int sceDevVif0GetCnd(sceDevVif0Cnd *);

int sceDevVif0PutFifo(u_long128 *addr, int n);



#ifdef __cplusplus
}
#endif

#endif
