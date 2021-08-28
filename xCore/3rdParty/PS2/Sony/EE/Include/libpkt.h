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
 *                         libpkt - libpkt.h
 *                 include libdmapk.h,libvifpk.h,libgifpk.h
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.01           Mar,26,1999     shibuya
 *       0.02           Apr,16,2003     hana        add scePktGetErxEntries
 */

#ifndef __libpkt__
#define __libpkt__

#include <libdmapk.h>
#include <libvifpk.h>
#include <libgifpk.h>

#ifdef __cplusplus
extern "C" {
#endif

void *scePktGetErxEntries(void);

#ifdef __cplusplus
}
#endif

#endif /* __libpkt__ */
