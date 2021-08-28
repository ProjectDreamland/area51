/*                 -*- mode: c-mode; tab-width: 4; indent-tabs-mode: nil; -*-
 SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 $Id: heaplib.h,v 1.1 2003/09/17 09:56:33 tei Exp $
 */
/*
 * I/O Processor Library
 *
 * Copyright (C) 2003 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * heaplib.h
 *   heap memory service
 *
 *      Date            Design      Log
 *  ----------------------------------------------------
 *      2003-09-17      isii        first open
 */

#ifndef _HEAPLIB_H
#define _HEAPLIB_H

#ifdef __cplusplus
extern "C" {
#endif

extern void	*CreateHeap(int heapblocksize, int flag);
extern void	DeleteHeap(void *heap);
extern void	*AllocHeapMemory(void *heap, size_t nbytes);
extern int	FreeHeapMemory(void *heap, void *ptr);
extern int	HeapTotalFreeSize(void *heap);

#define HEAP_AUTO_EXTEND	1
#define HEAP_ALLOC_MEM_BOTTOM	2

#ifdef __cplusplus
}
#endif

#endif /* _HEAPLIB_H */
