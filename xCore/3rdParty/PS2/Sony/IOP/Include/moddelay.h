/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                     I/O Processor Library
 *                          Version 0.60
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                       All Rights Reserved.
 *
 *                      moddelay - moddelay.h
 *                      IOP MIDI Stream Delay
 *
 *     Version   Date          Design     Log
 *  --------------------------------------------------------------------
 *     0.60      Oct.12.1999   katayama   first checked in.
 */
#ifndef _moddelay_h_
#define _moddelay_h_
typedef struct {
	unsigned int	delayBfSize;
	unsigned int	rp, wp;
	unsigned short	curTime;
	unsigned short	delayTime;
	unsigned char	delayBf[0];
} sceMidiDelay_DelayBuffer;
#define sceMidiDelay_GetDelayBuffer(x,y) \
	((sceMidiDelay_DelayBuffer*)(x)->buffGrp[0].buffCtx[((y)*2)+1].buff)

#define	sceMidiDelayNoError	0
#define	sceMidiDelayError	(-1)

int sceMidiDelay_Init(sceCslCtx*);
int sceMidiDelay_ATick(sceCslCtx*);
int sceMidiDelay_Flush(sceCslCtx*);
#endif /*!_moddelay_h_*/
/* $Id: moddelay.h,v 1.6 2003/09/12 05:26:31 tokiwa Exp $ */

