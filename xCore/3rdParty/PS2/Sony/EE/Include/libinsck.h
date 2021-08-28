/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/*
 *                      Emotion Engine Library
 *                          Version 1.10
 *                           Shift-JIS
 *
 *      Copyright (C) 2001-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       <libinsck - libinsck.h>
 *                      <libinsck general header>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           May,17,2001     komaki      first version
 *       1.10           Apr,24,2003     mka, ksh    enable/disable
 */

#ifndef _LIBINSCK_H_
#define _LIBINSCK_H_

#if defined(__cplusplus)
extern "C" {
#endif

#if 0

/* default -> disabled */
#if !defined(sceInsockDisableSocketSymbolAliases) \
	&& !defined(sceInsockEnableSocketSymbolAliases)
#define sceInsockDisableSocketSymbolAliases
#endif

#if defined(sceInsockEnableSocketSymbolAliases)
#undef	sceInsockDisableSocketSymbolAliases
#endif	/* sceInsockEnableSocketSymbolAliases */

#endif /* 0 */

#include <libinsck/netdb.h>
#include <libinsck/sys/socket.h>
#define sceLibnetDisableSocketSymbolAliases
#include <libnet.h>
#include <libinsck/netinet/in.h>
#include <libinsck/netinet/tcp.h>
#include <libinsck/arpa/inet.h>

int *__sceInsockErrnoLoc(void);
int *__sceInsockHErrnoLoc(void);

#define	sceInsockErrno		(*__sceInsockErrnoLoc())
#define	sceInsockHErrno		(*__sceInsockHErrnoLoc())

void *sceInsockGetErxEntries(void);

int sceInsockSetSifMBindRpcValue(u_int buffersize, u_int stacksize,
	int priority);

int sceInsockSetRecvTimeout(int s, int ms);

int sceInsockSetSendTimeout(int s, int ms);

int sceInsockSetShutdownTimeout(int s, int ms);

int sceInsockAbort(int s, int flags);

int sceInsockTerminate(int thread_id);

int sceInsockPoll(sceInetPollFd_t *fds, int nfds, int ms);

int sceInsockSendFin(int s, int ms);

void *(*sceInsockSetMallocFunction(void *(*alloc_func)(size_t size)))
	(size_t size);

void (*sceInsockSetFreeFunction(void (*free_func)(void *ptr)))(void *ptr);

int sceInsockInitMemory(void *poolbase, unsigned int poolsize);

int sceInsockAbortGethostbyname(const char *name);

int sceInsockAbortGethostbyaddr(const char *addr, int len, int type);

void sceInsockSetErrnoMode(int mode);

int sceInsockGetErrnoMode(void);

#if defined(__cplusplus)
}
#endif

#endif	/*  _LIBINSCK_H_ */
