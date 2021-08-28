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
 *                       <libeenet - libeenet.h>
 *                       <eenet general header>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Dec,03,2001     komaki      first version
 */

#ifndef _LIBEENET_H_
#define _LIBEENET_H_

#include <sys/types.h>
#include <libeenet/eenettypes.h>
#include <libeenet/eeneterrno.h>
#if !defined(sceEENetDisableSocketSymbolAliases)
#include <libeenet/eenet_name.h>
#endif
#include <libeenet/ent_smap.h>
#include <libeenet/ent_eth.h>
#include <libeenet/ent_ppp.h>
#include <libeenet/netdb.h>
#include <sys/time.h>

/*
 * malloc statistics
 */
typedef struct sceEENetMallocStat {
	u_int	nmalloc;	/* # of sceEENetMalloc() called times */
	u_int	nrealloc;	/* # of sceEENetRealloc() called times */
	u_int	nfree;		/* # of sceEENetFree() called times */
	size_t	upages;		/* # of pages in use */
	size_t	ubytes;		/* # of bytes in use */
	size_t	max_upages;	/* maximum # of pages in use */
	size_t	max_ubytes;	/* maximum # of bytes in use */
} sceEENetMallocStat_t;

#ifdef __cplusplus
extern "C" {
#endif

/* erx export */
void *sceEENetGetErxEntries(void);

int *__sceEENetThreadErrno(void);
int *__sceEENetThreadHErrno(void);

#define	sceEENetErrno		(*__sceEENetThreadErrno())
#define	sceEENetHErrno		(*__sceEENetThreadHErrno())

int sceEENetInit(void *pool, int poolsize, int eenet_tpl, int stacksize, int priority);
int sceEENetTerm(void);

int sceEENetGetDNSAddr(char *nameserver1, char *nameserver2);
int sceEENetSetDNSAddr(const char *nameserver1, const char *nameserver2);

int sceEENetFreeThreadinfo(int tid);

/* BSD socket API */
int sceEENetClose(int s);
int sceEENetCloseWithRST(int s);
int sceEENetSocketAbort(int s);
int sceEENetThreadAbort(int tid);
struct sceEENetHostent *sceEENetGethostbynameTO(const char *name, int retrans,
	int retry);
struct sceEENetHostent *sceEENetGethostbyname2TO(const char *name, int af,
	int retrans, int retry);
struct sceEENetHostent *sceEENetGethostbyaddrTO(const char *addr, int len,
	int af, int retrans, int retry);
void sceEENetSetResolvTO(int retrans, int retry);
void sceEENetGetResolvTO(int *retrans, int *retry);

unsigned int sceEENetHtonl(unsigned int host32);
unsigned short sceEENetHtons(unsigned short host16);
unsigned int sceEENetNtohl(unsigned int net32);
unsigned short sceEENetNtohs(unsigned short net16);

/* utilities */
time_t sceEENetTime(time_t *t);
int sceEENetGettimeofday(struct timeval *tp, struct timezone *tzp);
unsigned int sceEENetSleep(unsigned int seconds);
int sceEENetUsleep(unsigned int seconds);

/* internal malloc */
void *sceEENetMalloc(size_t);
void sceEENetFree(void *);
void *sceEENetRealloc(void *, size_t);
void *sceEENetCalloc(size_t, size_t);
void *sceEENetMemalign(size_t, size_t);
void sceEENetMallocStat(struct sceEENetMallocStat *);

/* log */
int sceEENetOpenLog(const char *, int, u_short);
int sceEENetCloseLog(void);
void sceEENetSetLogLevel(int);
int sceEENetGetLogLevel(void);
#define EENET_LOG_EMERG		0
#define EENET_LOG_ALERT		1
#define EENET_LOG_CRIT		2
#define EENET_LOG_ERR		3
#define EENET_LOG_WARNING	4
#define EENET_LOG_NOTICE	5
#define EENET_LOG_INFO		6
#define EENET_LOG_DEBUG		7

int sceEENetDhcpClient(const char *ifname, const char *hostname, char *stackptr, int stacksize, int priority, int timeout);
int sceEENetPppoeClient(const char *ifname, const char *authname, const char *authkey, int authtype, char *stackptr, int stacksize, int priority, int timeout);

#ifdef __cplusplus
}
#endif

#endif /* _LIBEENET_H_ */
