/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: modload.h,v 1.12 2002/02/15 07:51:46 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         modload.h
 *                         Module loader
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 *       2.00           2001/04/24      tei
 */

#ifndef _MODLOAD_H
#define _MODLOAD_H
#include <sys/types.h>

typedef struct _modulestatus {
    char	name[56];    /* モジュール名 */
    u_short	version;     /* モジュールのバージョン */
    u_short     flags;	     /* モジュールの状態 */
    int		id;          /* モジュールを識別する ID 番号 */
    u_long	entry_addr;  /* モジュールの実行開始アドレス */
    u_long	gp_value;    /* モジュールの GP レジスタ値 */
    u_long	text_addr;   /* モジュールの先頭アドレス */
    u_long	text_size;   /* モジュールのテキストセグメントサイズ */
    u_long	data_size;   /* モジュールのデータセグメントサイズ */
    u_long	bss_size;    /* モジュールの未初期化データセグメントサイズ */
    u_long	lreserve[2];
} ModuleStatus;

/* ModuleStatus.flags */
#define MSTF_LOADED		(0x0001)
#define MSTF_EXEC		(0x0002)
#define MSTF_RESIDENT		(0x0003)
#define MSTF_STOPPING		(0x0004)
#define MSTF_SelfSTOPPING	(0x0005)
#define MSTF_STOPPED		(0x0006)
#define MSTF_SelfSTOPPED	(0x0007)
#define MSTF_MASK(x)		((x)&0xf)

#define MSTF_REMOVABLE		(0x0010)
#define MSTF_NOSYSALLOC		(0x0020)
#define MSTF_CLEARMOD		(0x0040)

struct _lmwooption;

typedef struct _ldfilefunc {
    int (*beforeOpen)(void *opt, const char *filename, int flag);
    int (*afterOpen)(void *opt, int fd);
    int (*close)(void *opt, int fd);
    int (*setBufSize)(void *opt, int fd, size_t nbyte);
    int (*beforeRead)(void *opt, int fd, size_t nbyte);
    int (*read)(void *opt, int fd, void *buf, size_t nbyte);
    int (*lseek)(void *opt, int fd, long offset, int whence);
    int (*getfsize)(void *opt, int fd);
} LDfilefunc;

typedef struct _lmwooption {
    char	position;
    char	access;
    char	creserved[2];
    void	*distaddr;
    int		distoffset;
    LDfilefunc	*functable;
    void	*funcopt;
    int		ireserved[3];
} LMWOoption;

/* LMWOoption.position     */
/* -- module allocate type */
#define LMWO_POS_Low		SMEM_Low
#define LMWO_POS_High		SMEM_High
#define LMWO_POS_Addr		SMEM_Addr
/* LMWOoption.access       */
/* -- file access type     */
#define LMWO_ACCESS_Noseek	(1)
#define LMWO_ACCESS_Seekfew	(2)
#define LMWO_ACCESS_Seekmany	(4)

extern int  LoadStartModule(const char *filename,
		     int args, const char *argp, int *result);

extern int  LoadModule(const char *filename);
extern int  LoadModuleBuffer(const u_int *inbuf);
extern int  StartModule(int modid, const char *filename,
			int args, const char *argp, int *result);

extern int  StopModule(int modid, int args, const char *argp, int *result);
extern int  UnloadModule(int modid);

extern int  SelfStopModule(int args, const char *argp, int *result);
extern void SelfUnloadModule(void);

extern int  SearchModuleByName(const char *modulename);
extern int  SearchModuleByAddress(const void *addr);
extern int  GetModuleIdList(int *readbuf, int readbufsize, int *modulecount);
extern int  ReferModuleStatus(int modid, ModuleStatus *status);
extern int  GetModuleIdListByName(const char *modulename,
			     int *readbuf, int readbufsize, int *modulecount);

extern void *AllocLoadMemory(int type, unsigned long size, void *addr);
extern int  FreeLoadMemory(void *area);
extern int  LoadModuleAddress(const char *filename, void *addr, int offset);
extern int  LoadModuleBufferAddress(const u_int *inbuf, void *addr, int offset);
extern int  LoadModuleWithOption(const char *filename, const LMWOoption *option);

extern int SetModuleFlags(int modid, int flag);

#endif /* _MODLOAD_H */
