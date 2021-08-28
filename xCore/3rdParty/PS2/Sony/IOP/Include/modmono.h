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
 *                       modmono - modmono.h
 *                    IOP MIDI Stream Mono Assign
 *
 *     Version   Date          Design     Log
 *  --------------------------------------------------------------------
 *     0.60      Oct.12.1999   katayama   first checked in.
 */
#ifndef _modmono_h_
#define _modmono_h_
typedef struct {
#define sceMidiMono_MaxKey	128
#define sceMidiMono_MaxCh	16
	unsigned char	mono[sceMidiMono_MaxCh];
#define sceMidiMono_NoKey (0xff)
	unsigned char	onKey[sceMidiMono_MaxCh];
	unsigned char	velocity[sceMidiMono_MaxCh];
	unsigned char	key[sceMidiMono_MaxCh][sceMidiMono_MaxKey];
} sceMidiMono_Env;
#define sceMidiMono_GetEnv(x,y) \
	((sceMidiMono_Env*)(x)->buffGrp[0].buffCtx[((y)*2)+1].buff)

#define	sceMidiMonoNoError	0
#define	sceMidiMonoError	(-1)
#define sceMidiMonoOn	1
#define sceMidiMonoOff	0

int sceMidiMono_Init(sceCslCtx*);
int sceMidiMono_ATick(sceCslCtx*);
int sceMidiMono_SetMono(sceCslCtx*,unsigned int,unsigned char,int);
#endif /*!_modmono_h_*/
/* $Id: modmono.h,v 1.6 2003/09/12 05:27:24 tokiwa Exp $ */
