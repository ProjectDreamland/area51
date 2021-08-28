/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: intrman.h,v 1.14 2003/06/03 08:48:03 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         intrman.h
 *                         interrupt manager
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 *       1.01           1999/11/13      tei
 */

#ifndef _INTRMAN_H
#define _INTRMAN_H

#ifndef NULL
#define NULL ((void*)0)
#endif

#if defined(__LANGUAGE_C)

extern int CpuSuspendIntr(int *oldstat);
extern int CpuResumeIntr(int oldstat);
extern int QueryIntrContext(void);

#endif /* if defined(__LANGUAGE_C) */

#endif /* _INTRMAN_H */
