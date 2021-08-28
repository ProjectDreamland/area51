/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.1
 */
/*
 *                     I/O Processor Library
 *                          Version 2.0.6
 *                           Shift-JIS
 *
 *      Copyright (C) 2004 Sony Computer Entertainment Inc.
 *                       All Rights Reserved.
 *
 *                       modsesq2 - modsesq2.h
 *                         IOP SE Sequencer
 *
 *     Version   Date          Design     Log
 *  --------------------------------------------------------------------
 *     1.00      Oct.2000      kaol       first release
 *  --------------------------------------------------------------------
 *                             tokiwa     sesq2
 */
#ifndef _modsesq2_h_
#define _modsesq2_h_

#ifndef Bool
#define Bool int
#define True 1
#define False 0
#endif

#define sceSESq2CommandVersion		0x01
#define sceSESq2StreamPrefixSize	6
#define sceSESq2MessageIdPrefix		0xf8

#define sceSESq2NumSeqStream		32

#define sceSESq2Env_NoSeSongNum		(-1)
#define sceSESq2_Volume0db		127
#define sceSESq2_BaseTimeScale		1000
#define sceSESq2_PanpotCenter		64

#define sceSESq2_AllStop                0x00
#define sceSESq2_Stop                   0x01

typedef struct {
#define sceSESq2Env_NoSeqSet		0xff
    unsigned char	setNo;
#define sceSESq2Env_NoOutPortAssignment	0xff
	unsigned char	port;
} sceSESq2PortAssignment;

#define sceSESq2Stat_ready	         (1<<0)
#define sceSESq2Stat_inPlay	         (1<<1)
#define sceSESq2Stat_dataEnd	         (1<<2)
#define	sceSESq2Stat_noLoop	         (1<<3)
#define	sceSESq2Stat_finalTick	         (1<<4)
#define sceSESq2Stat_voiceLook           (1<<5)
#define sceSESq2Stat_pause               (1<<6)
#define sceSESq2Stat_pauseStart          (1<<7)  
#define	sceSESq2Stat_seqIDAutoUnselect   (1<<8)
#define sceSESq2Stat_allStop             (1<<9)
#define sceSESq2Stat_noteFirst           (1<<10)
#define	sceSESq2Stat_loop	         (1<<11)

typedef struct {
    unsigned int	songNum;
    unsigned char       masterVolume;
    char                masterPanpot;
    unsigned short	masterTimeScale;
    unsigned int	status;
    int                 defaultOutPort;
    sceSESq2PortAssignment outPort [sceSESq2NumSeqStream];

#define sceSESq2EnvSize                4216

    unsigned char	system [sceSESq2EnvSize];
} sceSESq2Env;

#define sceSESq2_SeqPlayStop	0
#define sceSESq2_SeqPlayStart	1
#define sceSESq2_SeqPlayTerminate 2
#define sceSESq2_SeqPlaySeqIDAutoUnselect	(1 << 8)

#define sceSESq2GetEnv(x,y) ((sceSESq2Env*)(x)->buffGrp[0].buffCtx[((y)*2)+1].buff)

extern int sceSESq2Init (sceCslCtx*,u_int);
extern int sceSESq2ATick(sceCslCtx*);
extern int sceSESq2Load (sceCslCtx*,u_int);

/*** for SE sequence ***/
extern int sceSESq2GetStatus  (sceCslCtx *, u_int, int);
extern Bool sceSESq2IsInPlay  (sceCslCtx *, u_int, int);
extern Bool sceSESq2IsDataEnd (sceCslCtx *, u_int, int);

/* for modsesq2 */
extern int sceSESq2SetIdMonitor(sceCslIdMonitor *id_monitor);
extern int sceSESq2Play(sceCslCtx *pCtx, u_int port, u_char setNo,
			u_char seqNo, u_char volume, u_char panpot, 
			short pitch);

extern int sceSESq2PlayDefault(sceCslCtx *pCtx, u_int port, u_char setNo, 
				    u_char seqNo);

extern int sceSESq2Stop(sceCslCtx *pCtx, u_int message_id);
extern int sceSESq2Pause(sceCslCtx *pCtx, u_int message_id, u_int mode);
extern int sceSESq2Resume(sceCslCtx *pCtx, u_int message_id);

extern int sceSESq2SetVolume(sceCslCtx *pCtx, u_int message_id, 
			     u_char id_volume, u_int time);

extern int sceSESq2SetPanpot(sceCslCtx *pCtx, u_int message_id, 
			     u_char id_panpot, u_int time);

extern int sceSESq2SetPitch(sceCslCtx *pCtx, u_int message_id, 
			    short id_pitch, u_int time);

#endif /*!_modsesq2_h_*/
/* $Id: modsesq2.h,v 1.27 2004/02/06 09:59:01 tokiwa Exp $ */
















