/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: stdlib.h,v 1.4 2002/02/15 07:47:08 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         stdlib.h
 *                         
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 */

#ifndef	_STDLIB_H
#define	_STDLIB_H

#ifndef _TYPES_H
#include <sys/types.h>
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

extern          long strtol(const char *,char**, int);
extern unsigned long strtoul(const char *, char **, int);

#define atoi(p) (int)strtol(p,NULL, 10)
#define atol(p) strtol(p,NULL, 10)

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif	/* _STDLIB_H */

