/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                      Emotion Engine Library
 *                          Version 2.4.0
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         eekernel.h
 *                         develop library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.3.0          Jun.18.1999     horikawa    add syscall
 *       0.4.0          Jul.15.1999     horikawa    add syscall
 *       0.5.0          Aug.12.1999     horikawa    add syscall
 *       1.1.0          Oct.12.1999     horikawa    add syscall
 *       1.2.0          Nov.03.1999     horikawa    add syscall
 *       1.4.0          Jan.19.2000     horikawa    modified to 'DI()'
 *       1.5.0          Feb.29.2000     horikawa    modified to syscalls
 *       1.6.0          Feb.29.2000     horikawa    AddIntcHandler2
 *                                                  AddDmacHandler2
 *                                                  SetDebugHandler
 *       2.0.0          Aug.17.2000     horikawa    ExpandScratchPad
 *       2.2.0          Dec.12.2000     horikawa    DIntr/EIntr
 *       2.2.1          Jan.23.2001     akiyuki     added void, const
 *                                                   and __asm__
 *       2.3.0          Jan.22,2003     kumagae     remove system use
 *       2.4.0          Aug.25,2003     hana        add sceVprintf, sceSnprintf, sceVsnprintf
 */

#ifndef _eekernel_h_
#define _eekernel_h_

#include <stdarg.h>
#include <stddef.h>
#include <eetypes.h>

#define THREAD_ERROR            (-1)
#define MAX_THREADS             256
#define MAX_SEMAPHORES          256
#define MAX_PRIORITY            128
#define MAX_HANDLERS            128
#define MAX_ALARMS              64

#ifdef __cplusplus
extern "C" {
#endif
int EIntr(void);
int DIntr(void);
#ifdef __cplusplus
}
#endif

#define EI      EIntr
#define DI      DIntr

#ifndef EI
#define EI()                    __asm__ volatile("ei")
#endif
#ifndef DI
#define DI() \
{ \
      u_int stat; \
      do { \
        __asm__ volatile (".p2align 3"); \
        __asm__ volatile ("di"); \
        __asm__ volatile ("sync.p"); \
        __asm__ volatile ("mfc0 %0, $12" : "=r"(stat):); \
      } while (stat & 0x00010000); \
}
#endif

#define ExitHandler()           __asm__ volatile("sync.l; ei")

#define WRITEBACK_DCACHE        0
#define INVALIDATE_DCACHE       1
#define INVALIDATE_ICACHE       2
#define INVALIDATE_CACHE        3

#define INTC_GS                 0
#define INTC_SBUS               1
#define INTC_VBON               2       /* old fashioned */
#define INTC_VBLANK_S           2
#define INTC_VBOF               3       /* old fashioned */
#define INTC_VBLANK_E           3
#define INTC_VIF0               4
#define INTC_VIF1               5
#define INTC_VU0                6
#define INTC_VU1                7
#define INTC_IPU                8
#define INTC_TIM0               9
#define INTC_TIM1               10

#define DMAC_VIF0               0
#define DMAC_VIF1               1
#define DMAC_GIF                2
#define DMAC_FROM_IPU           3
#define DMAC_TO_IPU             4
#define DMAC_FROM_SPR           8
#define DMAC_TO_SPR             9

#define DMAC_CIS                13      /* Channel interrupt */
#define DMAC_MEIS               14      /* MemFIFO empty interrupt */
#define DMAC_BEIS               15      /* Bus error interrupt */

#define THS_RUN                 0x01
#define THS_READY               0x02
#define THS_WAIT                0x04
#define THS_SUSPEND             0x08
#define THS_WAITSUSPEND         0x0c
#define THS_DORMANT             0x10

#define INIT_DMAC               0x01
#define INIT_VU1                0x02
#define INIT_VIF1               0x04
#define INIT_GIF                0x08
#define INIT_VU0                0x10
#define INIT_VIF0               0x20
#define INIT_IPU                0x40


#ifdef __cplusplus
extern "C" {
#endif

struct ThreadParam {
    int     status;
    void    (*entry)(void *);
    void    *stack;
    int     stackSize;
    void    *gpReg;
    int     initPriority;
    int     currentPriority;
    u_int   attr;
    u_int   option;
    int     waitType;
    int     waitId;
    int     wakeupCount;
};

struct SemaParam {
    int     currentCount;
    int     maxCount;
    int     initCount;
    int     numWaitThreads;
    u_int   attr;
    u_int   option;
};

extern void *_gp;
extern __inline__ void *GetGp(void);
extern __inline__ void *GetGp(void)
{
        void *gp;
        __asm__ volatile (
                "move           %0, $28\n"
                : "=r"(gp)
                :
                : "memory"
        );
        return(gp);
}
extern __inline__ void *SetGp(void *newgp);
extern __inline__ void *SetGp(void *newgp)
{
        void *oldgp;
        __asm__ volatile (
                "move           %0, $28\n"
                "move           $28, %1\n"
                : "=&r"(oldgp)
                : "r"(newgp)
                : "memory"
        );
        return(oldgp);
}
extern __inline__ void *SetModuleGp(void);
extern __inline__ void *SetModuleGp(void)
{
        void *oldgp;
        __asm__ volatile (
                "move           %0, $28\n"
                "move           $28, %1\n"
                : "=&r"(oldgp)
                : "r"(&_gp)
                : "memory"
        );
        return(oldgp);
}

/*
 * COP0
 */

void FlushCache(int);
void iFlushCache(int);
void SyncDCache(void *, void *);
void iSyncDCache(void *, void *);
void InvalidDCache(void *, void *);
void iInvalidDCache(void *, void *);
u_int GetCop0(int);
u_int iGetCop0(int);


/*
 * Multi Thread
 */


int CreateThread(struct ThreadParam *);
int DeleteThread(int);
int StartThread(int, void *arg);
void ExitThread(void);
void ExitDeleteThread(void);
int TerminateThread(int);
int iTerminateThread(int);
int ChangeThreadPriority(int, int);
int iChangeThreadPriority(int, int);
int RotateThreadReadyQueue(int);
int iRotateThreadReadyQueue(int);
int ReleaseWaitThread(int);     
int iReleaseWaitThread(int);
int GetThreadId(void);
int ReferThreadStatus(int, struct ThreadParam *);
int iReferThreadStatus(int, struct ThreadParam *);
int SleepThread(void);
int WakeupThread(int);
int iWakeupThread(int);
int CancelWakeupThread(int);
int iCancelWakeupThread(int);
int SuspendThread(int);
int iSuspendThread(int);
int ResumeThread(int);
int iResumeThread(int);
int DelayThread(u_int);

/*
 * Semaphore
 */

int CreateSema(struct SemaParam *);
int DeleteSema(int);
int SignalSema(int);
int iSignalSema(int);
int WaitSema(int);
int PollSema(int);
int iPollSema(int);
int ReferSemaStatus(int, struct SemaParam *);
int iReferSemaStatus(int, struct SemaParam *);


/*
 * Interrupt
 */

int EnableIntc(int);
int iEnableIntc(int);
int DisableIntc(int);
int iDisableIntc(int);
int EnableDmac(int);
int iEnableDmac(int);
int DisableDmac(int);
int iDisableDmac(int);
int SetAlarm(u_short, void (*)(int, u_short, void *), void *);
int iSetAlarm(u_short, void (*)(int, u_short, void *), void *);
int ReleaseAlarm(int);
int iReleaseAlarm(int);
int TimerSetAlarm(u_long ulSchedule, u_long (*cbHandler)(int, u_long, u_long, void *), void *arg);
int iTimerSetAlarm(u_long ulSchedule, u_long (*cbHandler)(int, u_long, u_long, void *), void *arg);
int TimerReleaseAlarm(int id);
int iTimerReleaseAlarm(int id);

int AddIntcHandler(int, int (*)(int), int);
int AddIntcHandler2(int, int (*)(int, void *, void *), int, void *);
int RemoveIntcHandler(int, int);
int AddDmacHandler(int, int (*)(int), int);
int AddDmacHandler2(int, int (*)(int, void *, void *), int, void *);
int RemoveDmacHandler(int, int);
int QueryIntrContext(void);

void *SetDebugHandler(
        int,
        void (*h)(u_int, u_int, u_int, u_int, u_int, u_long128 *)
);

int ExpandScratchPad(u_int);

u_long TimerUSec2BusClock(u_int uiSec, u_int uiUsec);
void   TimerBusClock2USec(u_long ulClock, u_int *puiSec, u_int *puiUsec);
float  TimerBusClock2Freq(u_long ulClock);
u_long TimerFreq2BusClock(float fFreq);

int SetTimerAlarm(u_long ulClock, u_long (*cbHandler)(int, u_long, u_long, void *, void *), void *arg);
int iSetTimerAlarm(u_long ulClock, u_long (*cbHandler)(int, u_long, u_long, void *, void *), void *arg);

int ReleaseTimerAlarm(int id);
int iReleaseTimerAlarm(int id);
u_long GetTimerSystemTime(void);
u_long iGetTimerSystemTime(void);
int    AllocTimerCounter(void);
int    iAllocTimerCounter(void);
int    FreeTimerCounter(int id);
int    iFreeTimerCounter(int id);
int    StartTimerCounter(int id);
int    iStartTimerCounter(int id);
int    StopTimerCounter(int id);
int    iStopTimerCounter(int id);
u_long SetTimerCount(int id, u_long ulNewCount);
u_long GetTimerBaseTime(int id);
u_long iGetTimerBaseTime(int id);
u_long GetTimerCount(int id);
u_long iGetTimerCount(int id);
int    SetTimerHandler(int id, u_long ulSchedule, u_long (*cbHandler)(int, u_long, u_long, void *, void *), void *arg);
int    iSetTimerHandler(int id, u_long ulSchedule, u_long (*cbHandler)(int, u_long, u_long, void *, void *), void *arg);

int    GetTimerUsedUnusedCounters(int *pnUsed, int *pnUnused);
int    iGetTimerUsedUnusedCounters(int *pnUsed, int *pnUnused);


/*
 * Others
 */

void LoadExecPS2(const char *, int, char *[]);
void Exit(int n);

void scePrintf(const char *fmt, ...);
void sceVprintf(const char *fmt, va_list ap);
int sceSnprintf(char *buffer, size_t count, const char *fmt, ...);
int sceVsnprintf(char *buffer, size_t count, const char *fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif /* _eekernel_h_ */
