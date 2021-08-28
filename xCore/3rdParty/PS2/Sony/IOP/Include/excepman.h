/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 2.8
 */
/* $Id: excepman.h,v 1.5 2003/04/15 11:13:19 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         excepman.h
 *                         exception manager
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 */

#ifndef _EXCEPMAN_H
#define _EXCEPMAN_H

#ifdef __LANGUAGE_ASSEMBLY
#define EXCEP_Int	0
#define EXCEP_MOD	1
#define EXCEP_TLBL	2
#define EXCEP_TLBS	3
#define EXCEP_AdEL	4
#define EXCEP_AdES	5
#define EXCEP_IBE	6
#define EXCEP_DBE	7
#define EXCEP_Sys	8
#define EXCEP_Bp	9
#define EXCEP_RI	10
#define EXCEP_CpU	11
#define EXCEP_Ovf	12
#define EXCEP_reserv1	13
#define EXCEP_reserv2	14
#define EXCEP_HDB	15
#define EXCEP_MAX	16
#endif

#if defined(__LANGUAGE_C)
enum EXCEP {
    /* 0,1,2,3 */
    EXCEP_Int=0,    EXCEP_MOD,      EXCEP_TLBL,     EXCEP_TLBS,
    /* 4,5,6,7 */
    EXCEP_AdEL,     EXCEP_AdES,     EXCEP_IBE,      EXCEP_DBE,
    /* 8,9,10,11 */
    EXCEP_Sys,      EXCEP_Bp,       EXCEP_RI,       EXCEP_CpU,
    /* 12,13,14,15 */
    EXCEP_Ovf,      EXCEP_reserv1,  EXCEP_reserv2,  EXCEP_HDB,
    EXCEP_MAX
};

extern int RegisterExceptionHandler(int excode, void (*handler)(void));
extern int RegisterPriorityExceptionHandler(int excode,
				     int priority, void (*handler)(void));
extern int RegisterDefaultExceptionHandler(void (*handler)(void));

extern int ReleaseExceptionHandler(int excode, void (*handler)(void));
extern int ReleaseDefaultExceptionHandler(void (*handler)(void));
#endif

#endif /* _EXCEPMAN_H */
