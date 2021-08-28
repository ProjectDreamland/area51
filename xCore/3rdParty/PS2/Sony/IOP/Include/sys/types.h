/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: types.h,v 1.2 1999/10/12 09:10:06 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         sys/types.h
 *                         
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 */

#ifndef _TYPES_H
#define _TYPES_H

#ifndef NULL
# define NULL		((void *) 0)
#endif

/*
 * The following typedefs are also protected by individual ifdefs for
 * historical reasons:
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int		size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef int			ssize_t;
#endif

#ifndef _CADDR_T
#define _CADDR_T
typedef char *			caddr_t;
#endif

/* bsd */
typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned int		u_int;
typedef unsigned long		u_long;

/* sysv */
typedef unsigned char		unchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

#endif /* _TYPES_H */
