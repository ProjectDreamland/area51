/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 0.10
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libmc - libmc.h
 *                     header file of libmc
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      0.00          2000- 5-25     T.Honda     the first version
 */

#ifndef _LIBMCX_H
#define _LIBMCX_H


#define sceMcxFuncGetInfo		1
#define sceMcxFuncSetInfo		2
#define sceMcxFuncGetMem		3
#define sceMcxFuncSetMem		4
#define sceMcxFuncShowTrans		5
#define sceMcxFuncHideTrans		6
#define sceMcxFuncReadDev		7
#define sceMcxFuncWriteDev		8
#define sceMcxFuncGetUifs		9
#define sceMcxFuncSetUifs		10
#define sceMcxFuncSetLed		11
#define sceMcxFuncChgPrior		12


#define sceMcxSyncRun	0
#define sceMcxSyncNone	(-1)
#define sceMcxSyncFin	1


#define sceMcxBitAppli		0x01
#define sceMcxBitSpeaker	0x02
#define sceMcxBitInfrared	0x04
#define sceMcxBitFlash		0x08
#define sceMcxBitLed		0x10
#define sceMcxBitDate		0x20

#define sceMcxDevRtc		0
#define sceMcxDevMem		1
#define sceMcxDevUifs		2

#define sceMcxResSucceed	0
#define sceMcxResNoDevice	(-12)

#define sceMcxIniSucceed		0
#define sceMcxIniErrKernel	(-101)
#define sceMcxIniOldMcxserv	(-120)
#define sceMcxIniOldMcxman	(-121)



typedef struct {
	short AppliNo;
	short Reserve1;
	int AplArg;
	unsigned char Speaker;
	unsigned char Infrared;
	unsigned char Flash;
	unsigned char Led;
	struct {
		unsigned char Week, Sec, Min, Hour;
		unsigned char Day, Month;
		unsigned short Year;
		} _Rtc;
	unsigned Serial;
	} sceMcxTblInfo;


#define PWeek	_Rtc.Week
#define PSec	_Rtc.Sec
#define PMin	_Rtc.Min
#define PHour	_Rtc.Hour
#define PDay	_Rtc.Day
#define PMonth	_Rtc.Month
#define PYear	_Rtc.Year


typedef struct {
	unsigned char AMin, AHour;
	unsigned Alarm:1;
	unsigned KeyLock:1;
	unsigned Volume:2;
	unsigned AreaCode:3;
	unsigned RtcSet:1;
	unsigned char Reserve1;
	unsigned short Font;
	short Reserve2;
	} sceMcxTblUifs;


/*
 * Prototypes
 */


#ifdef __cplusplus
extern "C" {
#endif

int sceMcxInit(void);
int sceMcxGetInfo(int, int, sceMcxTblInfo *);
int sceMcxSetInfo(int, int, const sceMcxTblInfo *, unsigned);
int sceMcxGetMem(int, int, void *, unsigned, unsigned);
int sceMcxSetMem(int, int, const void *, unsigned, unsigned);
int sceMcxShowTrans(int, int, int, int);
int sceMcxHideTrans(int, int);
int sceMcxReadDev(int, int, int, const void *, unsigned, void *, unsigned);
int sceMcxWriteDev(int, int, int, const void *, unsigned, const void *, unsigned);
int sceMcxGetUifs(int, int, sceMcxTblUifs *);
int sceMcxSetUifs(int, int, const sceMcxTblUifs *);
int sceMcxSetLed(int, int, int);
int sceMcxChangeThreadPriority(int);
int sceMcxSync(int, int *, int *);
void *sceMcxGetErxEntries(void);
int sceMcxEnd(void);


#ifdef __cplusplus
}
#endif
#endif		/* ndef _LIBMCX_H_ */
