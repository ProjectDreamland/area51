/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                     <libeenet - ent_smap.h>
 *                  <header for eenet device "smap">
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Dec,15,2001     komaki      first version
 */

#ifndef _ENT_SMAP_H_
#define _ENT_SMAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* erx export */
void *sceEENetSmapGetErxEntries(void);

extern int sceEENetDeviceSMAPReg(int comm_stacksize, int rpc_stacksize);
extern int sceEENetDeviceSMAPUnreg(void);

#ifdef __cplusplus
}
#endif

#endif /* _ENT_SMAP_H_ */
