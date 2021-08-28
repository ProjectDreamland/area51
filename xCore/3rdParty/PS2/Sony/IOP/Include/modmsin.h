/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 * Emotion Engine / I/O Processor Common Header
 *
 * Copyright (C) 1998,1999,2001 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * modmsin.h
 *	IOP Midi Stream Message Input
 */
#ifndef _modmsin_h_
#define _modmsin_h_

#ifdef __cplusplus
extern "C" {
#endif

#define sceMSInNoError	0
#define sceMSInError	-1

int sceMSIn_Init(sceCslCtx*);
int sceMSIn_ATick(sceCslCtx*);
int sceMSIn_Load(sceCslCtx*);
int sceMSIn_PutMsg(sceCslCtx*,unsigned int,unsigned int);
int sceMSIn_PutExcMsg(sceCslCtx*,unsigned int,unsigned char*,unsigned int);
#define	sceMSIn_MakeMsg(stat,d1,d2) sceMSIn_MakeMsg3(stat,d1,d2)
#define	sceMSIn_MakeMsg3(stat,d1,d2) \
	((((unsigned int)(stat))&0xff)|((((unsigned int)(d1))&0x7f)<<8)\
	|((((unsigned int)(d2))&0x7f)<<16))
#define sceMSIn_MakeMsg2(stat,d1) \
	((((unsigned int)(stat))&0xff)|((((unsigned int)(d1))&0x7f)<<8))
#define sceMSIn_NoteOn(ctx,port,ch,key,vel) \
	sceMSIn_PutMsg(ctx,port,sceMSIn_MakeMsg3((unsigned int)(ch)|0x90,key,vel))
#define	sceMSIn_NoteOff(ctx,port,ch,key) \
	sceMSIn_PutMsg(ctx,port,sceMSIn_MakeMsg2((unsigned int)(ch)|0x80,key))
#define sceMSIn_ProgramChange(ctx,port,ch,prg) \
	sceMSIn_PutMsg(ctx,port,sceMSIn_MakeMsg2((unsigned int)(ch)|0xc0,prg))
#define sceMSIn_BankSelect(ctx,port,ch,bno) \
	sceMSIn_PutMsg(ctx,port,sceMSIn_MakeMsg3((unsigned int)(ch)|0xb0,0,bno))
#define	sceMSIn_NoteOnEx(ctx,port,ch,key,vel,prg) \
	((sceMSIn_ProgramChange(ctx,port,ch,prg)==sceMSInNoError)?\
	sceMSIn_NoteOn(ctx,port,ch,key,vel):sceMSInError)
#define sceMSIn_ControlChange(ctx,port,ch,no,v) \
	sceMSIn_PutMsg(ctx,port,sceMSIn_MakeMsg3((unsigned int)(ch)|0xb0,no,v))
typedef struct {
	unsigned char	d[7];
} sceMSInHsMsg;
int sceMSIn_PutHsMsg(sceCslCtx*,unsigned int,sceMSInHsMsg*);
#define sceMSIn_MakeHsMsg1(buf,op,ch,dH,dL) {\
	(buf)->d[0]=sceCslMidiHsStatusEx1;(buf)->d[1]=(op);(buf)->d[2]=(ch);\
	(buf)->d[3]=(dH);(buf)->d[4]=(dL);}
#define sceMSIn_MakeHsMsg2(buf,op,ch,key,id,dH,dL) {\
	(buf)->d[0]=sceCslMidiHsStatusEx2;(buf)->d[1]=(op);(buf)->d[2]=(ch);\
	(buf)->d[3]=(key);(buf)->d[4]=(id);(buf)->d[5]=(dH);(buf)->d[6]=(dL);}
#define sceMSIn_MakeHsPreExpression(buf,ch,d) \
	sceMSIn_MakeHsMsg1(buf,sceCslMidiHsCmdExpression,ch,d,0)
#define sceMSIn_MakeHsPrePanpot(buf,ch,d) \
	sceMSIn_MakeHsMsg1(buf,sceCslMidiHsCmdPanpot,ch,d,0)
#define sceMSIn_MakeHsPrePitchBend(buf,ch,dH,dL) \
	sceMSIn_MakeHsMsg1(buf,sceCslMidiHsCmdPitchBend,ch,dH,dL)
#define sceMSIn_MakeHsExpression(buf,ch,key,id,d) \
	sceMSIn_MakeHsMsg2(buf,sceCslMidiHsCmdExpression,ch,key,id,d,0)
#define sceMSIn_MakeHsPanpot(buf,ch,key,id,d) \
	sceMSIn_MakeHsMsg2(buf,sceCslMidiHsCmdPanpot,ch,key,id,d,0)
#define sceMSIn_MakeHsPitchBend(buf,ch,key,id,dH,dL) \
	sceMSIn_MakeHsMsg2(buf,sceCslMidiHsCmdPitchBend,ch,key,id,dH,dL)
#define sceMSIn_MakeHsNoteOn(buf,ch,key,id,d) \
	sceMSIn_MakeHsMsg2(buf,sceCslMidiHsCmdNoteCtrl,ch,key,id,d,0)
#define sceMSIn_MakeHsNoteOff(buf,ch,key,id) \
	sceMSIn_MakeHsNoteOn(buf,ch,key,id,0)

#ifdef __cplusplus
}
#endif
#endif /* !_modmsin_h_ */
/* $Id: modmsin.h,v 1.6 2002/03/27 02:01:47 xokano Exp $ */
