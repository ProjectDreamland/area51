/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 0.02
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         libdev - libdev.h
 *       include devvu0.h,devvu1.h,devgif.h devvif0.h devvif1.h devfont.h
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.01           Mar,29,1999     shibuya
 *       0.02           Apr,16,2003     hana        add sceDevGetErxEntries
 */


#ifndef __libdev__
#define __libdev__

#include <devvu0.h>
#include <devvu1.h>
#include <devgif.h>
#include <devvif0.h>
#include <devvif1.h>
#include <devfont.h>

#ifdef __cplusplus
extern "C" {
#endif

void *sceDevGetErxEntries(void);

#ifdef __cplusplus
}
#endif

#endif
