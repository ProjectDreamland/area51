/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                     I/O Processor Library
 *                          Version 2.0.6
 *                           Shift-JIS
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                       All Rights Reserved.
 *
 *                       modsesq - modsesq.h
 *                         IOP SE Sequencer
 *
 *     Version   Date          Design     Log
 *  --------------------------------------------------------------------
 *     1.00      Oct.2000      kaol       first release
 */
#ifndef _modsesq_h_
#define _modsesq_h_

#ifndef Bool
#define Bool int
#define True 1
#define False 0
#endif

#define sceSESqNoError	sceCslNoError
#define sceSESqError	sceCslError

#define sceSESqCommandVersion		0x01
#define sceSESqStreamPrefixSize		6
#define sceSESqMessageIdPrefix		0xf1

#define sceSESqNumSeqStream		32

#define sceSESqEnv_NoSeSongNum		(-1)
#define sceSESq_Volume0db		127
#define sceSESq_BaseTimeScale		1000
#define sceSESq_PanpotCenter		64

typedef struct {
#define sceSESqEnv_NoSeqSet		0xff
	unsigned char	setNo;
#define sceSESqEnv_NoOutPortAssignment	0xff
	unsigned char	port;
} sceSESqPortAssignment;

#define sceSESqStat_ready	(1<<0)
#define sceSESqStat_inPlay	(1<<1)
#define sceSESqStat_dataEnd	(1<<2)
#define	sceSESqStat_noLoop	(1<<3)
#define	sceSESqStat_finalTick	(1<<4)
#define	sceSESqStat_seqIDAutoUnselect	(1<<8)

typedef struct {
    unsigned int	songNum;
    unsigned char       masterVolume;
    char                masterPanpot;
    unsigned short	masterTimeScale;
    unsigned int	status;
    int                 defaultOutPort;
    sceSESqPortAssignment outPort [sceSESqNumSeqStream];

#define sceSESqEnvSize		2552

    unsigned char	system [sceSESqEnvSize];
} sceSESqEnv;

#define sceSESq_SeqPlayStop	0
#define sceSESq_SeqPlayStart	1
#define sceSESq_SeqPlayTerminate 2
#define sceSESq_SeqPlaySeqIDAutoUnselect	(1 << 8)

#define sceSESq_GetEnv(x,y) ((sceSESqEnv*)(x)->buffGrp[0].buffCtx[((y)*2)+1].buff)

extern int sceSESq_Init (sceCslCtx*,unsigned int);
extern int sceSESq_ATick(sceCslCtx*);
extern int sceSESq_Load (sceCslCtx*,unsigned int);

/*** for SE sequence ***/
extern int sceSESq_SelectSeq (sceCslCtx*,unsigned int, unsigned char, unsigned char);
extern int sceSESq_UnselectSeq (sceCslCtx *, unsigned int, int);
extern int sceSESq_SeqPlaySwitch (sceCslCtx *, unsigned int, int, int);
extern int sceSESq_SeqGetStatus  (sceCslCtx *, unsigned int, int);
extern Bool sceSESq_SeqIsInPlay  (sceCslCtx *, unsigned int, int);
extern Bool sceSESq_SeqIsDataEnd (sceCslCtx *, unsigned int, int);
extern int sceSESq_SeqSetSEMsgID (sceCslCtx *, unsigned int, int, unsigned int);
extern int sceSESq_SeqTerminateVoice (sceCslCtx *, unsigned int, unsigned int, unsigned int, unsigned int);

#endif /*!_modsesq_h_*/
/* $Id: modsesq.h,v 1.17 2003/09/12 05:27:57 tokiwa Exp $ */











