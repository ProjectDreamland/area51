/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: timerman.h,v 1.7 2000/10/18 12:03:09 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         timerman.h
 *                         Hard timer manager defines
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 *       2.00           2000/10/04      tei
 *       2.01           2000/10/18      tei         remove old define
 */

#ifndef _TIMERMAN_H_
#define _TIMERMAN_H_

extern int AllocHardTimer(int source, int size, int prescale);
/* source */
#define TC_SYSCLOCK	(1)
#define	TC_PIXEL	(2)
#define	TC_HLINE	(4)

extern int FreeHardTimer(int timid);
extern unsigned long GetTimerCounter(int timid);
#define	iGetTimerCounter(timid)		GetTimerCounter(timid)

extern int SetTimerHandler(int timid, unsigned long comparevalue,
			   unsigned int (*timeuphandler)(void*), void *common);
extern int SetOverflowHandler(int timid, unsigned int (*handler)(void*),
			      void *common);
extern int SetupHardTimer(int timid, int source, int mode, int prescale);
/* mode */
#define TM_NO_GATE			(0)
#define TM_GATE_ON_Count		(1)
#define TM_GATE_ON_ClearStart		(3)
#define TM_GATE_ON_Clear_OFF_Start	(5)
#define TM_GATE_ON_Start		(7)

extern int StartHardTimer(int timid);
extern int StopHardTimer(int timid);

#endif /* _TIMERMAN_H_ */
