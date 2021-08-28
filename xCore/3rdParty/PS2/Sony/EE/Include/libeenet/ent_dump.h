/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.00
 *                           Shift-JIS
 *
 *      Copyright (C) 2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                     <libeenet - ent_dump.h>
 *                    <header for eenet tcpdump>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           Apr,07,2003     komaki      first version
 */

#ifndef _ENT_DUMP_H_
#define _ENT_DUMP_H_

#include <sys/types.h>
#include <libeenet/eenettypes.h>
#include <libeenet/net/bpf.h>

typedef struct sceEENetTcpdumpParam {
	int term_sid;
	int killwait_sid;
	int tid;
} sceEENetTcpdumpParam_t;

#ifdef __cplusplus
extern "C" {
#endif

/* erx export */
void *sceEENetTcpdumpGetErxEntries(void);

int sceEENetTcpdump(const char *ifname, const char *WFileName,
	char *filebuf, int fbsize, int promisc,
	struct sceEENetBpfProgram *fcode, int snaplen, int cnt,
	char *stackptr, int stacksize, int priority,
	sceEENetTcpdumpParam_t *daemonp);

int sceEENetTcpdumpTerm(sceEENetTcpdumpParam_t *daemonp);

int sceEENetTcpdump2(const char *ifname, char *filebuf, int fbsize,
	int promisc, struct sceEENetBpfProgram *fcode, int snaplen, int cnt,
	char *stackptr, int stacksize, int priority,
	sceEENetTcpdumpParam_t *daemonp,
	int (*open_func)(const char *, int , ...),
	int (*write_func)(int , const void *, int),
	int (*close_func)(int));

#ifdef __cplusplus
}
#endif

#endif /* _ENT_DUMP_H_ */
