/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: memory.h,v 1.3 1999/10/12 09:20:50 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         memory.h
 *                         
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 */

#ifndef _MEMORY_H
#define _MEMORY_H

#ifndef _TYPES_H
#include <sys/types.h>
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

extern void *memchr(const void *p, int c, size_t n);
extern int  memcmp(const void *s1, const void *s2, size_t n);
extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memmove(void *dest, const void *src, size_t n);
extern void *memset(void *s, int c, size_t n);

extern int  bcmp(const void *s1, const void *s2, size_t n);
extern void bcopy(const void *src, void *dest, size_t n);
extern void bzero(void *s, size_t n);

/* IOP orignal functions -- word(32bit) operation */
extern void  *wmemcopy(u_long *dst, const u_long *src, u_long bytes);
extern void  *wmemset(u_long *dst, u_long c, u_long bytes);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif  /* _MEMORY_H */





