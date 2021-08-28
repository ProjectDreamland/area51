/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: scrtchpd.h,v 1.2 2002/03/27 05:10:07 xokano Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         scratchpad.h
 *                         ScratchPad allocation services
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           2000/12/20      tei
 */
#ifndef _SCRATCHPAD_H_
#define _SCRATCHPAD_H_

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

extern void *AllocScratchPad(int mode);
extern int FreeScratchPad(void *alloced_addr);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _SCRATCHPAD_H_ */
