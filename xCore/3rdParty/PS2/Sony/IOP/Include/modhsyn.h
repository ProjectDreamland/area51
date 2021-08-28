/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.1
 */
/*
 * Emotion Engine / I/O Processor Common Header
 *
 * Copyright (C) 1998-1999,2001 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * modhsyn.h
 *	IOP SPU2 Synthesizer
 */
#ifndef _modhsyn_h_
#define _modhsyn_h_

#ifdef __cplusplus
extern "C" {
#endif

#define sceHSynNoError	0
#define sceHSynError		-1

typedef struct {
	unsigned char	id;
	unsigned short	waveLen;	/* in sample */
	short			*wave;
} sceHSynUserLfoWave;

#define sceHSynMinVelocity	1
#define sceHSynNumVelocity	126
typedef struct {
	unsigned char	velMap[sceHSynNumVelocity];
} sceHSynUserVelocityMap;

#define sceHSynEnvSize	1348

typedef struct {
	unsigned char			priority;
	unsigned char			maxPolyphony;
#define sceHSynModeHSyn  0
#define sceHSynModeSESyn 1
#define sceHSynTypeHSyn  0
#define sceHSynTypeSESyn 1
#define sceHSynTypeProgram 0
#define sceHSynTypeTimbre  1
#define sceHSynModeNone  0xff
        unsigned char			portMode;
        unsigned char			waveType;
	int						lfoWaveNum;
	sceHSynUserLfoWave		*lfoWaveTbl;
	int						velocityMapNum;
	sceHSynUserVelocityMap	*velocityMapTbl;
	unsigned char			system[sceHSynEnvSize];
} sceHSynEnv;

#define sceHSyn_GetEnv(x,y) \
	((sceHSynEnv*)(x)->buffGrp[0].buffCtx[((y)*2)+1].buff)

int sceHSyn_Init(sceCslCtx*,unsigned int);
int sceHSyn_ATick(sceCslCtx*);
#define sceHSynMaxBank	15
int sceHSyn_Load(sceCslCtx*,unsigned int,void*,void*,unsigned int);
int sceHSyn_Unload(sceCslCtx*,unsigned int,unsigned int);
int sceHSyn_VoiceTrans(short,unsigned char*,unsigned char*,unsigned int);
#define sceHSyn_VoisTrans(w,x,y,z) sceHSyn_VoiceTrans(w,x,y,z)
int sceHSyn_SetReservVoice(unsigned int*);
int sceHSyn_GetReservVoice(unsigned int*);
#define sceHSyn_Volume_0db	0x100
int sceHSyn_SetVolume(sceCslCtx*,unsigned int,unsigned short);
unsigned short sceHSyn_GetVolume(sceCslCtx*,unsigned int);
int sceHSyn_AllNoteOff(sceCslCtx*,unsigned int);
int sceHSyn_AllSoundOff(sceCslCtx*,unsigned int);
int sceHSyn_ResetAllControler(sceCslCtx*,unsigned int);

int sceHSyn_SetIdMonitor(sceCslIdMonitor *idCtx); /* for sesq2 */

#define sceHSyn_NumCore 2
#define sceHSyn_NumVoice 24
/*
 * Voice State
 *	bit 0-3:	operation port number
 *	bit 4-6:	voice state
 *  bit 7:		status valid bit
 */
#define sceHSyn_VoiceStat_Free 		(0<<4)
#define sceHSyn_VoiceStat_Pending	(1<<4)
#define sceHSyn_VoiceStat_KeyOn		(2<<4)
#define sceHSyn_VoiceStat_KeyOff	(3<<4)
typedef struct {
	int	pendingVoiceCount;
	int	workVoiceCount;
	unsigned char	voice_state[sceHSyn_NumCore][sceHSyn_NumVoice];
	unsigned short	voice_env[sceHSyn_NumCore][sceHSyn_NumVoice];
} sceHSyn_VoiceStat;
#define sceHSyn_GetVoiceStat(x) ((((int)(x))&0x80)?(((int)(x))&0x70):-1)
#define sceHSyn_GetVoiceCtrlPort(x) ((((int)(x))&0x80)?(((int)(x))&0xf):-1)
int sceHSyn_SetVoiceStatBuffer(sceHSyn_VoiceStat*);

typedef struct {
	int		core;
#define SCEHS_REV_MODE_OFF		0
#define SCEHS_REV_MODE_ROOM		1
#define SCEHS_REV_MODE_STUDIO_A	2
#define SCEHS_REV_MODE_STUDIO_B	3
#define SCEHS_REV_MODE_STUDIO_C	4
#define SCEHS_REV_MODE_HALL		5
#define SCEHS_REV_MODE_SPACE	6
#define SCEHS_REV_MODE_ECHO		7
#define SCEHS_REV_MODE_DELAY	8
#define SCEHS_REV_MODE_PIPE		9
#define SCEHS_REV_MODE_MAX		10
#define SCEHS_REV_MODE_CLEAR_WA	(1<<8)
	int		mode;
	short	depth_L, depth_R;
	int		delay;
	int		feedback;
	short	vol_l, vol_r;
} sceHSyn_EffectAttr;
int sceHSyn_SetEffectAttr(sceHSyn_EffectAttr*);
typedef struct {
#define sceHSynChStat_KeyOn		(1<<0)
#define sceHSynChStat_Hold		(1<<1)
#define sceHSynChStat_KeyOff	(1<<2)
	unsigned char	ch[16];
} sceHSynChStat;
int sceHSyn_GetChStat(sceCslCtx*,unsigned int,sceHSynChStat*);
#define sceHSynOutputMode_Mono		0
#define sceHSynOutputMode_Stereo	1
int sceHSyn_SetOutputMode(int);
int sceHSyn_GetOutputMode(void);
/****** DEBUG SUPPORT *********/
#define sceHSyn_SdCall_inProcess	0x80000000
#define sceHSyn_sceSdInit						0
#define sceHSyn_sceSdSetParam					1
#define sceHSyn_sceSdGetParam					2
#define sceHSyn_sceSdSetSwitch				3
#define sceHSyn_sceSdGetSwitch				4
#define sceHSyn_sceSdSetAddr					5
#define sceHSyn_sceSdGetAddr					6
#define sceHSyn_sceSdSetCoreAttr				7
#define sceHSyn_sceSdGetCoreAttr				8
#define sceHSyn_sceSdNote2Pitch				9
#define sceHSyn_sceSdPitch2Note				10
#define sceHSyn_sceSdProcBatch				11
#define sceHSyn_sceSdProcBatchEx				12
#define sceHSyn_sceSdVoiceTrans				13
#define sceHSyn_sceSdBlockTrans				14
#define sceHSyn_sceSdVoiceTransStatus			15
#define sceHSyn_sceSdBlockTransStatus			16
#define sceHSyn_sceSdSetTransCallback			17
#define sceHSyn_sceSdSetIRQCallback			18
#define sceHSyn_sceSdSetEffectAttr			19
#define sceHSyn_sceSdGetEffectAttr			20
#define sceHSyn_sceSdClearEffectWorkArea		21
typedef struct {
	unsigned int	func;
	unsigned int	retVal;
	unsigned int	arg[5];
} sceHSyn_SdCall;
typedef struct {
	unsigned int	infoBlkNum;
	unsigned int	readIndex;
	unsigned int	writeIndex;
	sceHSyn_SdCall	sdCall[0];
} sceHSyn_DebugInfo;
int sceHSyn_SetDebugInfoBuffer(sceHSyn_DebugInfo*);

extern int sceHSyn_SESetMaxVoices(unsigned char);
extern int sceHSyn_SEAllNoteOff(sceCslCtx *, unsigned int);
extern int sceHSyn_SEAllSoundOff(sceCslCtx *, unsigned int);
extern int sceHSyn_SERetrieveVoiceNumberByID(sceCslCtx *, unsigned int, unsigned int, char *, char);
extern int sceHSyn_SERetrieveAllSEMsgIDs(sceCslCtx *, unsigned int, unsigned int *, int);
extern int sceHSyn_MSGetVoiceStateByID(sceCslCtx *, unsigned int, unsigned char, unsigned char *, char);
extern int sceHSyn_MSGetVoiceEnvelopeByID(sceCslCtx *, unsigned int, unsigned char, unsigned short *, char);

#ifdef __cplusplus
}
#endif

/*****************************/	
#endif /* !_modhsyn_h_ */
/* $Id: modhsyn.h,v 1.19 2003/12/11 10:27:30 tokiwa Exp $ */

