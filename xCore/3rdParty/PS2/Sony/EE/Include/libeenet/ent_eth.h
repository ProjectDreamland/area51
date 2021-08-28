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
 *                     <libeenet - ent_eth.h>
 *                  <header for eenet device "eth">
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Apr,12,2002     komaki      first version
 */

#ifndef _ENT_ETH_H_
#define _ENT_ETH_H_

#ifdef __cplusplus
extern "C" {
#endif

/* erx export */
void *sceEENetEthGetErxEntries(void);

extern int sceEENetDeviceETHReg(int comm_stacksize, int rpc_stacksize);
extern int sceEENetDeviceETHUnreg(void);

#ifdef __cplusplus
}
#endif

#endif /* _ENT_ETH_H_ */
