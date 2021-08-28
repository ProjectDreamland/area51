/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.20
 *                           Shift-JIS
 *
 *      Copyright (C) 2002-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       <netglue - netglue.h>
 *               <glue library header for protocol stack >
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Sep,21,2001     komaki      first version
 *       1.10           Jan,29,2002     komaki      move errno function
 *                                                  inside extern "C"{}
 *       1.20           Apr,24,2003     ksh         enable/disable
 */

#ifndef _NETGLUE_H_
#define _NETGLUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 0

/* default -> disabled */
#if !defined(sceNetGlueDisableSocketSymbolAliases) \
	&& !defined(sceNetGlueEnableSocketSymbolAliases)
#define sceNetGlueDisableSocketSymbolAliases
#endif

#if defined(sceNetGlueEnableSocketSymbolAliases)
#undef	sceNetGlueDisableSocketSymbolAliases
#endif	/* sceNetGlueEnableSocketSymbolAliases */

#endif	/* 0 */

#include <netglue/netdb.h>
#include <netglue/sys/socket.h>
#include <netglue/netinet/in.h>
#include <netglue/netinet/tcp.h>
#include <netglue/arpa/inet.h>

int *__sceNetGlueErrnoLoc(void);
int *__sceNetGlueHErrnoLoc(void);

#define	sceNetGlueErrno		(*__sceNetGlueErrnoLoc())
#define	sceNetGlueHErrno	(*__sceNetGlueHErrnoLoc())

void *sceNetGlueGetErxEntries(void);

int sceNetGlueAbort(int sockfd);
int sceNetGlueAbortResolver(int thread_id);
int sceNetGlueThreadInit(int thread_id);
int sceNetGlueThreadTerminate(int thread_id);

#ifdef __cplusplus
}
#endif

#endif	/*  _NETGLUE_H_ */

