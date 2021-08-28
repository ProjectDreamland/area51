/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 * Emotion Engine Library
 *
 * Copyright (C) 1998-1999, 2002, 2003 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * liblout - liblout.h
 *     EE PCM Line Out
 */
#ifndef _SCE_LIBLOUT_H
#define _SCE_LIBLOUT_H

#include <csl.h>
#include <sif.h>

#define sceLoutNoError	0
#define sceLoutError	-1

#define sceLoutInputUnit	512
#define sceLoutMaxLine		4
#define	sceLoutNoOutPut		0xff
#define	sceLoutDmaPreWait	(1<<7)
typedef struct {
	unsigned char	attrib;
	unsigned char	lineAssign[sceLoutMaxLine];
	unsigned int	iopBufSize;
	void*			iopBuf[2];
	unsigned int	nDmaBuf;
	sceSifDmaData	dma[0];
} sceLoutConf;

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif
int sceLout_Init(sceCslCtx*,unsigned int);
int sceLout_Quit(sceCslCtx*);
int sceLout_ATick(sceCslCtx*);
/*int sceLout_Load(sceCslCtx*);*/
#define sceLout_Load(x)
void *sceLout_GetErxEntries(void);
#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif
#endif /* !_SCE_LIBLOUT_H */
/* $Id: liblout.h,v 1.10 2003/05/29 06:48:35 kaol Exp $ */

