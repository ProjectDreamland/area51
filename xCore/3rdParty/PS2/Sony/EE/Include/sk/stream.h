/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.1
 */
/*
 * Emotion Engine Library
 *
 * Copyright (C) 2001, 2002 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * sk/stradpcm.h
 *     Standard Kit / Sound System / ADPCM stream --- header file
 */

#ifndef _SCE_STANDARDKIT_SOUND_STREAM_H
#define _SCE_STANDARDKIT_SOUND_STREAM_H

/* maximum value */
#define SCESK_STRADPCM_MAX_STREAM	8
#define SCESK_STRADPCM_ALL_STREAM_BIT	0xff
#define SCESK_STRADPCM_VOLUME_MAX	0x3fff
#define SCESK_STRADPCM_PITCH_1X		0x1000

/* sceSkSsStrAdpcmEnv.buffer.validsize */
#define SCESK_STRADPCM_FLAG_BUFFER_QUEUED (1 << 31)
#define SCESK_STRADPCM_FLAG_BUFFER_SENT   (1 << 30)
#define SCESK_STRADPCM_FLAG_ALL  (SCESK_STRADPCM_FLAG_BUFFER_QUEUED | SCESK_STRADPCM_FLAG_BUFFER_SENT)
#define SCESK_STRADPCM_FLAG_MASK (~SCESK_STRADPCM_FLAG_ALL)

/* sceSkSsStrAdpcmEnv.adpcm.channel[].status */
#define SCESK_STRADPCM_STATUS_NOT_IN_USE	255
#define SCESK_STRADPCM_STATUS_IDLE		0
#define SCESK_STRADPCM_STATUS_INITIALIZE	10
#define SCESK_STRADPCM_STATUS_INITIALIZED	11
#define SCESK_STRADPCM_STATUS_PREPARE		20
#define SCESK_STRADPCM_STATUS_PREPARING		21
#define SCESK_STRADPCM_STATUS_PREPARED		22
#define SCESK_STRADPCM_STATUS_RUNNABLE		30
#define SCESK_STRADPCM_STATUS_RUNNING		31
#define SCESK_STRADPCM_STATUS_STOPPING		40
#define SCESK_STRADPCM_STATUS_SUSPENDED		49
										
/* sceSkSsStrAdpcmEnv.adpcm.channel[].voice.options */
#define SCESK_STRADPCM_OPTION_EFFECT	(1 << 0)
#define SCESK_STRADPCM_OPTION_AUTOPLAY	(1 << 1)
										
/* sceSkSsStrAdpcmEnv.adpcm.channel[].voice.mask */
#define SCESK_STRADPCM_ATTR_VOLUME_L	(1 << 0)
#define SCESK_STRADPCM_ATTR_VOLUME_R	(1 << 1)
#define SCESK_STRADPCM_ATTR_PITCH	(1 << 2)
#define SCESK_STRADPCM_ATTR_ADSR1	(1 << 3)
#define SCESK_STRADPCM_ATTR_ADSR2	(1 << 4)
#define SCESK_STRADPCM_ATTR_EFFECT	(1 << 5)

typedef struct {
    char core;			/* CORE # */
    char no;			/* Voice # */
    unsigned char options;	/* Voice Options */
    unsigned char mask;

    unsigned short voll, volr;
    /* unsigned int addr;	-> spu2.addr */
    unsigned short pitch;
    unsigned short pad10;	/* reserved */
    unsigned short adsr1;
    unsigned short adsr2;
} SceSkSsStrAdpcmVoiceAttr;

typedef struct {
    unsigned int addr;		/* The start address of stream buffer */
    unsigned int size;		/* The size of stream buffer */
             int validsize;	/* current valid size */
} SceSkSsStrAdpcmStreamBufferAttr;

typedef struct {
    unsigned int status;	/* The status of stream buffer */
    unsigned int addr;		/* The start address of stream buffer */
    unsigned int size;		/* The size of stream buffer */
             int validsize;	/* current valid size */
} SceSkSsStrAdpcmStreamTransferBufferAttr;

#define SCESK_STRADPCM_SYSTEM_SIZE 60

/* Attribute for one stream channel */
typedef struct {
    unsigned int status;		/* stream status */
    SceSkSsStrAdpcmVoiceAttr        voice;  /* voice attributes */
    SceSkSsStrAdpcmStreamBufferAttr spu2;   /* the stream buffer in SPU2 local memory */
    SceSkSsStrAdpcmStreamBufferAttr iop;    /* the stream buffer in IOP memory */
    SceSkSsStrAdpcmStreamBufferAttr buffer;	/* data to copy */
    unsigned char system [SCESK_STRADPCM_SYSTEM_SIZE];
} SceSkSsStrAdpcmStreamChannelAttr;

typedef struct {
    unsigned int version;	/* version # of specification */
    unsigned int trans_ch;	/* transfer channel # */
    SceSkSsStrAdpcmStreamChannelAttr channel [SCESK_STRADPCM_MAX_STREAM];
} SceSkSsStrAdpcmStreamAttr;

#define SCESK_STRADPCM_ENV_SYSTEM_SIZE 4
typedef struct {
    unsigned int reserved [SCESK_STRADPCM_ENV_SYSTEM_SIZE];
    unsigned int stream_bit;
    unsigned int transfer_status;
    SceSkSsStrAdpcmStreamAttr adpcm;
    SceSkSsStrAdpcmStreamTransferBufferAttr buffer [SCESK_STRADPCM_MAX_STREAM];
} SceSkSsStrAdpcmEnv __attribute__((aligned(64)));

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

int sceSkSsSTRADPCM_Init (void);
int sceSkSsSTRADPCM_Quit (void);

int sceSkSsSTRADPCM_Bind    (unsigned int, ...);
int sceSkSsSTRADPCM_Unbind  (unsigned int, ...);
int sceSkSsSTRADPCM_Control (unsigned int, unsigned int, ...);
int sceSkSsSTRADPCM_Status  (unsigned int, unsigned int, ...);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#define sceSkSsInitSTRADPCM() sceSkSsSTRADPCM_Init()
#define sceSkSsQuitSTRADPCM() sceSkSsSTRADPCM_Quit()

#define sceSkSsBindSTRADPCM(id)            sceSkSsSTRADPCM_Bind(SCESK_STRADPCM,(id))
#define sceSkSsUnbindSTRADPCM(id)          sceSkSsSTRADPCM_Unbind(SCESK_STRADPCM,(id))
#define sceSkSsBindTrackSTRADPCM(id,sa_id) sceSkSsSTRADPCM_Bind(SCESK_TRACK,(id),(sa_id))
#define sceSkSsUnbindTrackSTRADPCM(id) 	   sceSkSsSTRADPCM_Unbind(SCESK_TRACK,(id))

#define sceSkSsInitEnvironmentSTRADPCM(id,env)   sceSkSsSTRADPCM_Control((id),SCESK_INITENV,(env))

#define      sceSkSsSetupSTRADPCM(id,env,ch_bit) sceSkSsSTRADPCM_Control((id),SCESK_SETENV,(env),(ch_bit))
#define    sceSkSsUnsetupSTRADPCM(id,env)        sceSkSsSTRADPCM_Control((id),SCESK_UNSETENV,(env))
#define    sceSkSsPreloadSTRADPCM(id,env,ch_bit) sceSkSsSTRADPCM_Control((id),SCESK_PRELOAD,(env),(ch_bit))
#define      sceSkSsStartSTRADPCM(id,env,ch_bit) sceSkSsSTRADPCM_Control((id),SCESK_START,(env),(ch_bit))
#define  sceSkSsTerminateSTRADPCM(id,env,ch_bit) sceSkSsSTRADPCM_Control((id),SCESK_TERMINATE,(env),(ch_bit))
#define      sceSkSsPauseSTRADPCM(id,env,ch_bit) sceSkSsSTRADPCM_Control((id),SCESK_PAUSE,(env),(ch_bit))
#define    sceSkSsDepauseSTRADPCM(id,env,ch_bit) sceSkSsSTRADPCM_Control((id),SCESK_DEPAUSE,(env),(ch_bit))
#define       sceSkSsMuteSTRADPCM(id,env,ch_bit) sceSkSsSTRADPCM_Control((id),SCESK_MUTE,(env),(ch_bit))
#define     sceSkSsDemuteSTRADPCM(id,env,ch_bit) sceSkSsSTRADPCM_Control((id),SCESK_DEMUTE,(env),(ch_bit))
#define sceSkSsChangeAttrSTRADPCM(id,env,ch_bit) sceSkSsSTRADPCM_Control((id),SCESK_PLAY_CONTROL,(env),(ch_bit))
#define     sceSkSsUpdateSTRADPCM(id,env)        sceSkSsSTRADPCM_Control((id),SCESK_UPDATE,(env))
#define       sceSkSsSyncSTRADPCM(id,env)        sceSkSsSTRADPCM_Control((id),SCESK_SYNC,(env))

#define sceSkSsCheckPreloadSTRADPCM(id,env,ch_bit) sceSkSsSTRADPCM_Status((id),SCESK_START,(env),(ch_bit))

/* stream: universal macros: */
#define sceSkSsStreamInit(f)			 sceSkSs##f##_Init()
#define sceSkSsStreamQuit(f)			 sceSkSs##f##_Quit()
#define sceSkSsStreamInitEnvironment(f,id,env)	 sceSkSs##f##_Control((id),SCESK_INITENV,(env))

#define sceSkSsStreamSetup(f,id,env,ch_bit)      sceSkSs##f##_Control((id),SCESK_SETENV,(env),(ch_bit))
#define sceSkSsStreamUnsetup(f,id,env)           sceSkSs##f##_Control((id),SCESK_UNSETENV,(env))
#define sceSkSsStreamPreload(f,id,env,ch_bit)    sceSkSs##f##_Control((id),SCESK_PRELOAD,(env),(ch_bit))
#define sceSkSsStreamStart(f,id,env,ch_bit)      sceSkSs##f##_Control((id),SCESK_START,(env),(ch_bit))
#define sceSkSsStreamTerminate(f,id,env,ch_bit)  sceSkSs##f##_Control((id),SCESK_TERMINATE,(env),(ch_bit))
#define sceSkSsStreamPause(f,id,env,ch_bit)      sceSkSs##f##_Control((id),SCESK_PAUSE,(env),(ch_bit))
#define sceSkSsStreamDepause(f,id,env,ch_bit)    sceSkSs##f##_Control((id),SCESK_DEPAUSE,(env),(ch_bit))
#define sceSkSsStreamMute(f,id,env,ch_bit)       sceSkSs##f##_Control((id),SCESK_MUTE,(env),(ch_bit))
#define sceSkSsStreamDemute(f,id,env,ch_bit)     sceSkSs##f##_Control((id),SCESK_DEMUTE,(env),(ch_bit))
#define sceSkSsStreamChangeAttr(f,id,env,ch_bit) sceSkSs##f##_Control((id),SCESK_PLAY_CONTROL,(env),(ch_bit))
#define sceSkSsStreamUpdate(f,id,env)            sceSkSs##f##_Control((id),SCESK_UPDATE,(env))
#define sceSkSsStreamSync(f,id,env)              sceSkSs##f##_Control((id),SCESK_SYNC,(env))

#define sceSkSsStreamCheckPreload(f,id,env,ch_bit) sceSkSs##f##_Status((id),SCESK_START,(env),(ch_bit))

#endif /* _SCE_STANDARDKIT_SOUND_STREAM_H */
/* This file ends here, DON'T ADD STUFF AFTER THIS */
