/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *
 *      Copyright (C) 1998-2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libsdr - sdrcmd.h
 *                    common constant for libsdr
 *
 */

#ifndef _SDRCMD_H_
#define _SDRCMD_H_

typedef int (*sceSdrUserCommandFunction)(unsigned int, void *, int);
#define rSdUserCommandMaxNum 0x10
#define _rSdUserCommandMask 0x00f0
#define _rSdUserCommandNumberSiftBit 4

#define sceSdrGetUserCommandNumber(cmd)	\
	(((cmd) & _rSdUserCommandMask) >> _rSdUserCommandNumberSiftBit)

/* ----------------------------------------------------------------
 *	COMMAND 
 * ---------------------------------------------------------------- */
#define rSdInit                  0x8000
#define rSdSetParam              0x8010
#define rSdGetParam              0x8020
#define rSdSetSwitch             0x8030
#define rSdGetSwitch             0x8040
#define rSdSetAddr               0x8050
#define rSdGetAddr               0x8060
#define rSdSetCoreAttr           0x8070
#define rSdGetCoreAttr           0x8080
#define rSdNote2Pitch            0x8090
#define rSdPitch2Note            0x80a0
#define rSdProcBatch             0x80b0
#define rSdProcBatchEx           0x80c0
#define rSdVoiceTrans            0x80d0
#define rSdBlockTrans            0x80e0
#define rSdVoiceTransStatus      0x80f0
#define rSdBlockTransStatus      0x8100
#ifdef SCE_OBSOLETE /* obsoleted */
#define rSdSetTransCallback      0x8110
#define rSdSetIRQCallback        0x8120
#endif
#define rSdSetEffectAttr         0x8130
#define rSdGetEffectAttr         0x8140
#define rSdClearEffectWorkArea   0x8150
#define rSdSetTransIntrHandler   0x8160
#define rSdSetSpu2IntrHandler    0x8170
#define rSdStopTrans             0x8180
#define rSdCleanEffectWorkArea   0x8190
#define rSdSetEffectMode         0x81a0
#define rSdSetEffectModeParams   0x81b0
#define rSdProcBatch2            0x81c0
#define rSdProcBatchEx2          0x81d0

#define rSdChangeThreadPriority  0x8f10

#define rSdUserCommand0		 0x9000
#define rSdUserCommand1		 0x9010
#define rSdUserCommand2		 0x9020
#define rSdUserCommand3		 0x9030
#define rSdUserCommand4		 0x9040
#define rSdUserCommand5		 0x9050
#define rSdUserCommand6		 0x9060
#define rSdUserCommand7		 0x9070
#define rSdUserCommand8		 0x9080
#define rSdUserCommand9		 0x9090
#define rSdUserCommandA		 0x90a0
#define rSdUserCommandB		 0x90b0
#define rSdUserCommandC		 0x90c0
#define rSdUserCommandD		 0x90d0
#define rSdUserCommandE		 0x90e0
#define rSdUserCommandF		 0x90f0
#define rSdUserCommandMin	rSdUserCommand0
#define rSdUserCommandMax	rSdUserCommandF

extern sceSdrUserCommandFunction sceSdrSetUserCommandFunction (int, sceSdrUserCommandFunction);

/* ----------------------------------------------------------------
 *	End on File
 * ---------------------------------------------------------------- */
#endif /* _SDRCMD_H_ */
/* DON'T ADD STUFF AFTER THIS */
