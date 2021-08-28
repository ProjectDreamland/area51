/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                     <libeenet - ent_ppp.h>
 *                  <header for eenet device "ppp">
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Jan,24,2002     komaki      first version
 */

#ifndef _ENT_PPP_H_
#define _ENT_PPP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* erx export */
void *sceEENetPppGetErxEntries(void);

extern int sceEENetDevicePPPReg(int comm_stacksize, int rpc_stacksize);
extern int sceEENetDevicePPPUnreg(void);

#ifdef __cplusplus
}
#endif

#endif /* _ENT_PPP_H_ */
