/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 $Id: libtimer.h,v 1.13 2003/08/31 05:56:42 kono Exp $
 */
/* 
 *                      Emotion Engine Library
 *                          Version 0.00
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libtimer - libtimer.h
 *                     header file of libtimer
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      0.00          2001-11-24        kono        the first version
 *      0.01          2002-06-14        kono        counter virtulize
 *      0.02          2003-02-18        kono        handler support
 *                                                  alarm support
 *                                                  thread delaying support
 *      0.03          2003-03-17        kono        add more 'i' version func
 *      0.04          2003-04-10        kono        fix erx export
 *      0.05          2003-04-25        kono        apply to official error code
 *      0.06          2003-05-01        kono        add handler func's arg
 *                                                  interrupted program counter
 */

#ifndef __LIBTIMER_H__
#define __LIBTIMER_H__

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

#include <sys/types.h>


#define SCE_TIMER_PRESCALE1			0
#define SCE_TIMER_PRESCALE16		1
#define SCE_TIMER_PRESCALE256		2

#define SCE_TIMER_MAX_SOFTCOUNTER	128
#define SCE_TIMER_MAX_SOFTALARM		64

/* プロトタイプ宣言 */
int    sceTimerInit(u_int uiMode);
int    sceTimerEnd(void);

int    sceTimerGetPreScaleFactor(void);

u_long sceTimerUSec2BusClock(u_int uiSec, u_int uiUsec);
void   sceTimerBusClock2USec(u_long ulClock, u_int *puiSec, u_int *puiUsec);
float  sceTimerBusClock2Freq(u_long ulClock);
u_long sceTimerFreq2BusClock(float fFreq);

int    sceTimerStartSystemTime(void);
int    sceTimerStopSystemTime(void);
u_long sceTimerGetSystemTime(void);
u_long isceTimerGetSystemTime(void);

int    sceTimerAllocCounter(void);
int    isceTimerAllocCounter(void);
int    sceTimerFreeCounter(int id);
int    isceTimerFreeCounter(int id);
int    sceTimerStartCounter(int id);
int    isceTimerStartCounter(int id);
int    sceTimerStopCounter(int id);
int    isceTimerStopCounter(int id);
u_long sceTimerSetCount(int id, u_long ulNewCount);
u_long sceTimerGetBaseTime(int id);
u_long isceTimerGetBaseTime(int id);
u_long sceTimerGetCount(int id);
u_long isceTimerGetCount(int id);
int    sceTimerSetHandler(int id, u_long ulSchedule, u_long (*cbHandler)(int, u_long, u_long, void *, void *), void *arg);
int    isceTimerSetHandler(int id, u_long ulSchedule, u_long (*cbHandler)(int, u_long, u_long, void *, void *), void *arg);

int    sceTimerGetUsedUnusedCounters(int *pnUsed, int *pnUnused);
int    isceTimerGetUsedUnusedCounters(int *pnUsed, int *pnUnused);

int    sceTimerSetAlarm(u_long ulSchedule, u_long (*cbHandler)(int, u_long, u_long, void *, void *), void *arg);
int    isceTimerSetAlarm(u_long ulSchedule, u_long (*cbHandler)(int, u_long, u_long, void *, void *), void *arg);
int    sceTimerReleaseAlarm(int id);
int    isceTimerReleaseAlarm(int id);

int    sceTimerDelayThread(u_int uiUsec);

void  *sceTimerGetErxEntries(void);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif	/* __LIBTIMER_H__ */
