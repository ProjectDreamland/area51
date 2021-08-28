/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: thread.h,v 1.14 2002/02/17 19:25:34 tei Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         thread.h
 *                         thread manager defines
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       1.00           1999/10/12      tei
 *       1.01           1999/11/20      tei
 */

#ifndef _THREAD_H_
#define _THREAD_H_

#include <sys/types.h>

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

#define TH_SELF		0
#define TPRI_RUN	0
 
/*
 * Thread priority convention
 *
 *  HIGHEST .. HIGHSYS_RESERVED		reserved for debug system
 *  MODULE_INIT-3 .. MODULE_INIT	for module manager
 *  MODULE_INIT				for each module initialize routine
 *
 *  USER_HIGHEST..USER_LOWEST		for user & drivers
 *
 *  LOWSYS_RESERVED .. LOWEST		reserved for debug system
 */

#define	LOWEST_PRIORITY			(32*4-2)
#define	HIGHEST_PRIORITY		1

#define	HIGHSYS_RESERVED_PRIORITY	4
#define	MODULE_INIT_PRIORITY		8
#define	USER_HIGHEST_PRIORITY		9

#define	LOWSYS_RESERVED_PRIORITY	(LOWEST_PRIORITY-2)
#define	USER_LOWEST_PRIORITY		(LOWSYS_RESERVED_PRIORITY-1)


struct SysClock {
    u_int	low;
    u_int	hi;
};

/* For Thread Management */
struct ThreadParam {
    u_int	attr;		/* 属性 			*/
    u_int	option;		/* ユーザ用付加情報  		*/
    void	*entry;		/* エントリアドレス 		*/
    int		stackSize;	/* スタックサイズ 		*/
    int		initPriority;	/* 優先順位 初期値 		*/
};

struct ThreadInfo {
    u_int	attr;		/* 属性 			*/
    u_int	option;		/* ユーザ用付加情報  		*/
    int		status;		/* スレッドの状態 		*/
    void	*entry;		/* エントリアドレス 		*/
    void	*stack;		/* スタック領域の先頭 		*/
    int		stackSize;	/* スタックサイズ 		*/
    void	*gpReg;		/* gp レジスタ値 		*/
    int		initPriority;	/* 優先順位 初期値 		*/
    int		currentPriority;/* 優先順位 現在値 		*/
    int		waitType;	/* 待ち状態の種類		*/
    int		waitId;		/* waittype の待ち対象の ID	*/
    int		wakeupCount;	/* 未処理の wakeup 回数		*/
    u_long	*regContext;
    int		reserved1;
    int		reserved2;
    int		reserved3;
    int		reserved4;
};

typedef struct ThreadRunStatus {
    int		status;		/* スレッドの状態 		*/
    int		currentPriority;/* 優先順位 現在値 		*/
    int		waitType;	/* 待ち状態の種類		*/
    int		waitId;		/* waittype の待ち対象の ID	*/
    int		wakeupCount;	/* 未処理の wakeup 回数		*/
    u_long	*regContext;
    struct SysClock runClocks;		/* 実行時間                     */
    u_int	intrPreemptCount;	/* 割込み処理で実行権を奪われた回数 */
    u_int	threadPreemptCount;	/* 実行権を奪われた回数         */
    u_int	releaseCount;		/* 実行権を手放した回数         */
} ThreadRunStatus;

/* ThreadInfo.status */
#define	THS_RUN		0x01
#define THS_READY	0x02
#define THS_WAIT	0x04
#define THS_SUSPEND	0x08
#define THS_WAITSUSPEND	0x0c
#define THS_DORMANT	0x10
#define THS_DEAD	0x20

/* ThreadInfo.waitType */
#define TSW_SLEEP	1	/* SleepThread()による待ち状態		*/
#define TSW_DELAY	2	/* DelayThread()による待ち状態		*/
#define TSW_SEMA	3	/* セマフォの待ち状態			*/
#define TSW_EVENTFLAG	4	/* イベントフラグの待ち状態		*/
#define TSW_MBX		5	/* メッセージボックスの待ち状態		*/
#define TSW_VPL		6	/* 可変長メモリブロック獲得の待ち状態	*/
#define TSW_FPL		7	/* 固定長メモリブロック獲得の待ち状態	*/

/* ThreadParam.attr, ThreadInfo.attr */
#define TH_ASM		0x01000000
#define TH_C		0x02000000
#define TH_UMODE	0x00000008
#define TH_NO_FILLSTACK	0x00100000
#define TH_CLEAR_STACK	0x00200000

extern int CreateThread( struct ThreadParam *param );
extern int DeleteThread( int thid );
extern int StartThread( int thid, u_long arg );
extern int StartThreadArgs( int thid, int args, void *argp );
extern int ExitThread(void);
extern int ExitDeleteThread(void);
extern int TerminateThread( int thid );
extern int iTerminateThread( int thid );
/*
 *	extern int DisableDispatchThread(void);
 *	extern int EnableDispatchThread(void);
 */
extern int ChangeThreadPriority( int thid, int priority );
extern int iChangeThreadPriority( int thid, int priority );
extern int RotateThreadReadyQueue( int priority );
extern int iRotateThreadReadyQueue( int priority );
extern int ReleaseWaitThread( int thid );
extern int iReleaseWaitThread( int thid );
extern int GetThreadId(void);
extern int GetThreadCurrentPriority(void);
extern int CheckThreadStack(void);
extern int GetThreadStackFreeSize(int thid);
extern int ReferThreadStatus( int thid, struct ThreadInfo *info );
extern int iReferThreadStatus( int thid, struct ThreadInfo *info );
extern int ReferThreadRunStatus( int thid, struct ThreadRunStatus *stat,
				 size_t statsize );
/* ------ 同期・通信機能 ------ */

/* XXXX->attr common */
#define AT_THFIFO	0
#define AT_THPRI	1
#define AT_SINGLE	0
#define AT_MULTI	2
#define AT_MSFIFO	0
#define AT_MSPRI	4

/* --- Thread direct --- */
extern int SleepThread(void);
extern int WakeupThread( int thid );
extern int iWakeupThread( int thid );
extern int CancelWakeupThread( int thid );
extern int iCancelWakeupThread( int thid );
/*
 *	extern int SuspendThread( int thid );
 *	extern int iSuspendThread( int thid );
 *	extern int ResumeThread( int thid );
 *	extern int iResumeThread( int thid );
 */

/* --- Semaphore --- */
struct SemaParam {
    u_int	attr;		/* セマフォの属性		*/
    u_int	option;		/* ユーザ用付加情報  		*/
    int		initCount;	/* セマフォの初期値		*/
    int		maxCount;	/* セマフォの最大値		*/
};

struct SemaInfo {
    u_int	attr;		/* セマフォの属性		*/
    u_int	option;		/* ユーザ用付加情報  		*/
    int		initCount;	/* セマフォの初期値		*/
    int		maxCount;	/* セマフォの最大値		*/
    int		currentCount;	/* セマフォの現在値		*/
    int		numWaitThreads;	/* セマフォ待ちスレッド数	*/
    int		reserved1;
    int		reserved2;
};

/* SemaParam->attr, SemaInfo->attr */
#define SA_THFIFO	AT_THFIFO
#define SA_THPRI	AT_THPRI
#define SA_IHTHPRI	0x100

extern int CreateSema( struct SemaParam *param );
extern int DeleteSema( int semid );
extern int SignalSema( int semid );
extern int iSignalSema( int semid );
extern int WaitSema( int semid );
extern int PollSema( int semid );
extern int ReferSemaStatus( int semid, struct SemaInfo *info );
extern int iReferSemaStatus( int semid, struct SemaInfo *info );

/* --- EventFlag --- */
struct EventFlagParam {
    u_int	attr;		/* イベントフラグの属性		*/
    u_int	option;		/* ユーザ用付加情報  		*/
    u_int	initPattern;	/* イベントフラグの初期値	*/
};

struct EventFlagInfo {
    u_int	attr;		/* イベントフラグの属性		*/
    u_int	option;		/* ユーザ用付加情報  		*/
    u_int	initPattern;	/* イベントフラグの初期値	*/
    u_int	currentPattern;	/* イベントフラグの現在値	*/
    int		numWaitThreads;	/* イベントフラグ待ちスレッド数 */
    int		reserved1;
    int		reserved2;
};

/* EventFlagParam->attr, EventFlagInfo->attr */
#define EA_SINGLE	AT_SINGLE
#define EA_MULTI	AT_MULTI

/* waitmode */
#define EW_AND		0
#define EW_OR		1
#define EW_CLEAR	0x10

extern int CreateEventFlag( struct EventFlagParam *param );
extern int DeleteEventFlag( int evfid );
extern int SetEventFlag( int evfid, u_long bitpattern );
extern int iSetEventFlag( int evfid, u_long bitpattern );
extern int ClearEventFlag( int evfid, u_long bitpattern );
extern int iClearEventFlag( int evfid, u_long bitpattern );
extern int WaitEventFlag( int evfid, u_long bitpattern, int waitmode,
			   u_long *resultpat );
extern int PollEventFlag( int evfid, u_long bitpattern, int waitmode,
		   u_long *resultpat );
extern int ReferEventFlagStatus( int evfid, struct EventFlagInfo *info );
extern int iReferEventFlagStatus( int evfid, struct EventFlagInfo *info );

/* --- MessageBox --- */
struct MbxParam {
    u_int	attr;		/* メッセージボックスの属性	*/
    u_int	option;		/* ユーザ用付加情報  		*/
};

struct MbxInfo {
    u_int	attr;		/* メッセージボックスの属性	*/
    u_int	option;		/* ユーザ用付加情報  		*/
    int		numWaitThreads;	/* メッセージ待ちスレッド数	*/
    int		numMessage;	/* 受取り待ちメッセージ数	*/
    struct MsgPacket	*topPacket; /* 受取り待ち先頭メッセージ	*/
    int		reserved1;
    int		reserved2;
};

struct MsgPacket {
    struct MsgPacket *next;
    u_char	      msgPriority;
    u_char	      dummy[3];
};

/* MbxParam->attr, MbxInfo->attr */
#define MBA_THFIFO	AT_THFIFO
#define MBA_THPRI	AT_THPRI
#define MBA_MSFIFO	AT_MSFIFO
#define MBA_MSPRI	AT_MSPRI

extern int CreateMbx( struct MbxParam *param );
extern int DeleteMbx( int mbxid );
extern int SendMbx( int mbxid, struct MsgPacket *sendmsg );
extern int iSendMbx( int mbxid, struct MsgPacket *sendmsg );
extern int ReceiveMbx( struct MsgPacket **recvmsg, int mbxid );
extern int PollMbx( struct MsgPacket **recvmsg, int mbxid );
extern int ReferMbxStatus( int mbxid, struct MbxInfo *info );
extern int iReferMbxStatus( int mbxid, struct MbxInfo *info );

/* --- Variable zize memory pool --- */
struct VplParam {
    u_int	attr;		/* 可変長メモリプールの属性		*/
    u_int	option;		/* ユーザ用付加情報			*/
    int		size;		/* 可変長メモリプールのサイズ		*/
};

struct VplInfo {
    u_int	attr;		/* 可変長メモリプールの属性		*/
    u_int	option;		/* ユーザ用付加情報  			*/
    int		size;		/* 可変長メモリプールのサイズ		*/
    int		freeSize;	/* 可変長メモリプールの未使用サイズ	*/
    int		numWaitThreads;	/* メモリ待ちスレッド数			*/
    int		reserved1;
    int		reserved2;
    int		reserved3;
};

/* VplParam.attr, VplInfo.attr */
#define VA_THFIFO	AT_THFIFO
#define VA_THPRI	AT_THPRI
#define VA_MEMBTM	0x200

extern int  CreateVpl( struct VplParam *param );
extern int  DeleteVpl( int vplid );
extern void *AllocateVpl( int vplid, int size );
extern void *pAllocateVpl( int vplid, int size );
extern void *ipAllocateVpl( int vplid, int size );
extern int  FreeVpl( int vplid, void *block );
extern int  ReferVplStatus( int vplid, struct VplInfo *info );
extern int  iReferVplStatus( int vplid, struct VplInfo *info );

/* --- Fixed zize memory pool --- */
struct FplParam {
    u_int	attr;		/* 固定長メモリプールの属性		*/
    u_int	option;		/* ユーザ用付加情報  			*/
    int		blockSize;	/* 固定長メモリプールのブロックサイズ	*/
    int		numBlocks;	/* 固定長メモリプールのブロック数	*/
};

struct FplInfo {
    u_int	attr;		/* 固定長メモリプールの属性		*/
    u_int	option;		/* ユーザ用付加情報  			*/
    int		blockSize;	/* 固定長メモリプールのブロックサイズ	*/
    int		numBlocks;	/* 固定長メモリプールのブロック数	*/
    int		freeBlocks;	/* 固定長メモリプールの未使用ブロック	*/
    int		numWaitThreads;	/* メモリ待ちスレッド数			*/
    int		reserved1;
    int		reserved2;
    int		reserved3;
    int		reserved4;
};

/* FplParam.attr, FplInfo.attr */
#define FA_THFIFO	AT_THFIFO
#define FA_THPRI	AT_THPRI
#define FA_MEMBTM	0x200

extern int  CreateFpl( struct FplParam *param );
extern int  DeleteFpl( int fplid );
extern void *AllocateFpl( int fplid );
extern void *pAllocateFpl( int fplid );
extern void *ipAllocateFpl( int fplid );
extern int FreeFpl( int fplid, void *block );
extern int ReferFplStatus( int fplid, struct FplInfo *info );
extern int iReferFplStatus( int fplid, struct FplInfo *info );

/* 時間/タイマ管理機能 */
extern int DelayThread(unsigned int usec);

typedef u_int (*AlarmHandler)(void*);

extern int GetSystemTime(struct SysClock *clock);
extern u_int GetSystemTimeLow(void);

extern int SetAlarm( struct SysClock *clock, AlarmHandler handler,
		     void *common);
extern int iSetAlarm( struct SysClock *clock, AlarmHandler handler,
		      void *common);
extern int CancelAlarm( AlarmHandler handler, void *common );
extern int iCancelAlarm( AlarmHandler handler, void *common );

extern void USec2SysClock(unsigned int usec, struct SysClock *clock);
extern void SysClock2USec(struct SysClock *clock, int *sec, int *usec);

/* システム状態の取得機能 */

typedef struct SystemStatus {
    u_int		status;
    int			systemLowTimerWidth;	/* 32 bit or 16 bit */
    struct SysClock	idleClocks;
    struct SysClock	kernelClocks;		/* not yet. reserved */
    u_int		comesOutOfIdleCount;	/* comes out of idle count */
    u_int		threadSwitchCount;	/* total thread switch count */
    u_int		reserved[8];
} SystemStatus;

/* SystemStatus->status */
#define	TSS_THREAD		0
#define	TSS_DISABLEDISPATCH	1
#define	TSS_DISABLEINTR		3
#define	TSS_NOTHREAD		4

extern int ReferSystemStatus( struct SystemStatus *info, size_t infosize );
extern int GetThreadmanIdList(int type,
			      int *readbuf, int readbufsize, int *objectcount);
enum IdListType {
    TMID_Thread = 1,
    TMID_Semaphore,
    TMID_EventFlag,
    TMID_Mbox,
    TMID_Vpl,
    TMID_Fpl,
    TMID_SleepThread,
    TMID_DelayThread,
    TMID_DormantThread
};


#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _THREAD_H_ */
