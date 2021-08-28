/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 * Emotion Engine Library
 *
 * Copyright (C) 2003 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * libsdr.h
 */

#ifndef _SCE_LIBSDR_H
#define _SCE_LIBSDR_H

#include <sdmacro.h>	/* common/include */
#include <sdrcmd.h>	/* common/include */
#include <sifrpc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------
 *	Proto Types
 * ---------------------------------------------------------------- */

extern int sceSdRemoteInit(void);
extern int sceSdRemote(int arg, ...);
extern int sceSdRemoteCallbackInit(int priority);
extern int sceSdRemoteCallbackQuit(void);
extern sceSifEndFunc sceSdCallBack(sceSifEndFunc end_func);
extern int sceSdTransToIOP(void *buff, u_int sendAddr, u_int size, u_int isBlock);

extern void *sceSdGetErxEntries(void);

#ifdef __cplusplus
}
#endif

#endif /* !_SCE_LIBSDR_H */
