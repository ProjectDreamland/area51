/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 * Emotion Engine / I/O Processor Common Header
 *
 * Copyright (C) 2000, 2001 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * modsein.h
 *	IOP SE Stream Message Input
 */
#ifndef _modsein_h_
#define _modsein_h_

#ifndef _eetypes_h_
#include <sys/types.h>
#endif

#define sceSEInNoError	0
#define sceSEInError	(-1)

#ifndef _cslse_h_
#include <cslse.h>
#endif

/* backward compatibility */
#define sceSEInCommandVersion      sceSEMsg_VERSION
#define sceSEInCommandPrefix       sceSEMsg_PREFIX
#define sceSEInCommandPrefixLength sceSEMsg_PREFIX_LENGTH

#define	sceSEIn_MakeMsg(stat,d1,d2) sceSEIn_MakeMsg4(stat,d1,d2,0)
#define	sceSEIn_MakeMsg4(stat,d1,d2,d3) \
	((((unsigned int)(stat))&0xff)|\
	((((unsigned int)(d1))&0x7f)<<8)|\
	((((unsigned int)(d2))&0x7f)<<16)|\
	((((unsigned int)(d3))&0x7f)<<24))
#define	sceSEIn_MakeMsg7bitPad3(pan,pitch) \
	(((((unsigned int)(pan))&0xff))|\
	 ((((unsigned int)(pitch))&0x7f)<<8)|\
	 ((((unsigned int)(pitch))&0x3f80)<<9))
#define sceSEIn_MakeMsg2(stat,d1) \
	((((unsigned int)(stat))&0xff)|((((unsigned int)(d1))&0x7f)<<8))
#define	sceSEIn_MakeMsgData(d0,d1,d2,d3) \
	((((unsigned int)(d0))&0xff)|\
	((((unsigned int)(d1))&0xff)<<8)|\
	((((unsigned int)(d2))&0xff)<<16)|\
	((((unsigned int)(d3))&0xff)<<24))
#define	sceSEIn_MakeMsgData2LSB(d) (((unsigned int)(d))&0xff)
#define	sceSEIn_MakeMsgData2MSB(d) ((((unsigned int)(d))&0xff00)>>8)

/* 0xAn: Note on/off */
#ifdef sceSEInCommandVersion_0
#define sceSEIn_NoteOn(ctx,port,id,bank,prog,note,vel) \
	sceSEIn_PutMsg((ctx),(port),(id),\
			sceSEIn_MakeMsg4((unsigned int)(bank)|0xa0,(prog),(note),(vel)),\
			sceSEIn_MakeMsgData(64,0,0,0))
#define sceSEIn_PitchOn(ctx,port,id,bank,prog,note,vel,pitch) \
	sceSEIn_PutMsg((ctx),(port),(id),\
			sceSEIn_MakeMsg4((unsigned int)(bank)|0xa0,(prog),(note),(vel)),\
			sceSEIn_MakeMsg7bitPad3(64,(pitch)))
#else
#define sceSEIn_NoteOn(ctx,port,id,bank,prog,note,vel,pan) \
	sceSEIn_PutMsg((ctx),(port),(id),\
			sceSEIn_MakeMsg4((unsigned int)(bank)|0xa0,(prog),(note),(vel)),\
			sceSEIn_MakeMsgData((pan),0,0,0))
#define sceSEIn_PitchOn(ctx,port,id,bank,prog,note,vel,pan,pitch) \
	sceSEIn_PutMsg((ctx),(port),(id),\
			sceSEIn_MakeMsg4((unsigned int)(bank)|0xa0,(prog),(note),(vel)),\
			sceSEIn_MakeMsg7bitPad3((pan),(pitch)))
#endif
#define	sceSEIn_NoteOff(ctx,port,id,bank,prog,note) \
	sceSEIn_PutMsg((ctx),(port),(id),\
			sceSEIn_MakeMsg4((unsigned int)(bank)|0xa0,(prog),(note),0),\
			0)

#ifdef __cplusplus
extern "C" {
#endif
extern int sceSEIn_Init (sceCslCtx *);
extern int sceSEIn_ATick (sceCslCtx *);
extern int sceSEIn_Load (sceCslCtx *);
extern int sceSEIn_PutMsg (sceCslCtx *, unsigned int, unsigned int, unsigned int, unsigned int);
extern int sceSEIn_PutSEMsg (sceCslCtx*, unsigned int, unsigned int, unsigned char *, unsigned int);
extern int sceSEIn_MakeNoteOn (sceCslCtx*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int);
extern int sceSEIn_MakePitchOn (sceCslCtx*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, unsigned int);
extern int sceSEIn_MakeTimeLfoCycle (sceCslCtx*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
extern int sceSEIn_MakeTimeVolume (sceCslCtx*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
extern int sceSEIn_MakeTimePanpot (sceCslCtx*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, unsigned int);
extern int sceSEIn_MakeTimePitch (sceCslCtx*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
extern int sceSEIn_MakePitchLFO (sceCslCtx*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
extern int sceSEIn_MakeAmpLFO (sceCslCtx*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
extern int sceSEIn_MakeAllNoteOff (sceCslCtx*, unsigned int, unsigned int);
extern int sceSEIn_MakeAllNoteOffMask (sceCslCtx*, unsigned int, unsigned int, unsigned int, unsigned int);
extern int sceSEIn_MakeNoteOnZero (sceCslCtx*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int);
extern int sceSEIn_MakePitchOnZero (sceCslCtx*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, unsigned int);
#ifdef __cplusplus
}
#endif
#endif /* !_modsein_h_ */
/* $Id: modsein.h,v 1.16 2003/04/28 08:17:57 kaol Exp $ */
