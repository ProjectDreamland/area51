/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: setjmp.h,v 1.2 1999/10/12 09:06:47 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         setjmp.h
 *                         
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 */

#ifndef _SETJMP_H
#define _SETJMP_H

/* jmp_buf indices */
#define	JB_PC		0
#define	JB_SP		1
#define	JB_FP		2
#define	JB_S0		3
#define	JB_S1		4
#define	JB_S2		5
#define	JB_S3		6
#define	JB_S4		7
#define	JB_S5		8
#define	JB_S6		9
#define	JB_S7		10
#define	JB_GP		11

#define	JB_SIZE		12

#if defined(LANGUAGE_C)||defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
typedef int jmp_buf[JB_SIZE];
#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif
extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int);
#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif
#endif

#endif /* _SETJMP_H */
