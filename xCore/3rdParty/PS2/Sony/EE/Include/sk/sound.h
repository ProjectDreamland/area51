/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.1
 */
/*
 * Emotion Engine Library
 *
 * Copyright (C) 2001, 2002, 2003 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * sk/sound.h
 *     Standard Kit / Sound System --- header file
 */

#ifndef _STANDARDKIT_EE_SOUND_PUBLIC_H_
#define _STANDARDKIT_EE_SOUND_PUBLIC_H_

#include <sdmacro.h>

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

extern int sceSkSsInit (unsigned int);
extern int sceSkSsQuit (unsigned int);

extern int sceSkSsSOUND_Send    (unsigned int, ...);
extern int sceSkSsSOUND_Control (unsigned int, ...);
extern int sceSkSsSOUND_Status  (unsigned int, ...);

extern int sceSkSsHSYNTH_Bind    (unsigned int, ...);
extern int sceSkSsHSYNTH_Unbind  (unsigned int, ...);
extern int sceSkSsHSYNTH_Control (unsigned int, ...);
extern int sceSkSsHSYNTH_Status  (unsigned int, ...);
#define sceSkSsHDBD_Bind    sceSkSsHSYNTH_Bind
#define sceSkSsHDBD_Unbind  sceSkSsHSYNTH_Unbind
#define sceSkSsHDBD_Control sceSkSsHSYNTH_Control
#define sceSkSsHDBD_Status  sceSkSsHSYNTH_Status

extern int sceSkSsMIDI_Bind    (unsigned int, ...);
extern int sceSkSsMIDI_Unbind  (unsigned int, ...);
extern int sceSkSsMIDI_Control (unsigned int, ...);
extern int sceSkSsMIDI_Status  (unsigned int, ...);
#define sceSkSsSONG_Bind    sceSkSsMIDI_Bind
#define sceSkSsSONG_Unbind  sceSkSsMIDI_Unbind
#define sceSkSsSONG_Control sceSkSsMIDI_Control
#define sceSkSsSONG_Status  sceSkSsMIDI_Status

extern int sceSkSsMSIN_Bind    (unsigned int, ...);
extern int sceSkSsMSIN_Unbind  (unsigned int, ...);
extern int sceSkSsMSIN_Control (unsigned int, ...);
extern int sceSkSsMSIN_Status  (unsigned int, ...);

extern int sceSkSsSESQ_Bind    (unsigned int, ...);
extern int sceSkSsSESQ_Unbind  (unsigned int, ...);
extern int sceSkSsSESQ_Control (unsigned int, ...);
extern int sceSkSsSESQ_Status  (unsigned int, ...);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

/*
 * Constant Macros
 */

/* for `sceSkSsSetDigitalOut' */
#define SCESK_DOUT_CD_NORMAL         (SD_SPDIF_MEDIA_CD  | SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_NORMAL)
#define SCESK_DOUT_CD_COPY_PROHIBIT  (SD_SPDIF_MEDIA_CD  | SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_PROHIBIT)
#define SCESK_DOUT_DVD_NORMAL        (SD_SPDIF_MEDIA_DVD | SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_NORMAL)
#define SCESK_DOUT_DVD_COPY_PROHIBIT (SD_SPDIF_MEDIA_DVD | SD_SPDIF_OUT_PCM | SD_SPDIF_COPY_PROHIBIT)

/* for `sceSkSsSend' */
#define SCESK_SEND_IOP2SPU 0

/* for `sceSkSsSetMasterVolume' */
#define SCESK_MASTER_VOLUME_MAX 127
#define SCESK_MASTERVOLUME_MAX SCESK_MASTER_VOLUME_MAX

/* for `sceSkSsSetEffectMode' */
#define SCESK_EFFECT_MODE_OFF		SD_REV_MODE_OFF
#define SCESK_EFFECT_MODE_ROOM		SD_REV_MODE_ROOM
#define SCESK_EFFECT_MODE_STUDIO_A	SD_REV_MODE_STUDIO_A
#define SCESK_EFFECT_MODE_STUDIO_B	SD_REV_MODE_STUDIO_B
#define SCESK_EFFECT_MODE_STUDIO_C	SD_REV_MODE_STUDIO_C
#define SCESK_EFFECT_MODE_HALL		SD_REV_MODE_HALL
#define SCESK_EFFECT_MODE_SPACE		SD_REV_MODE_SPACE
#define SCESK_EFFECT_MODE_ECHO		SD_REV_MODE_ECHO
#define SCESK_EFFECT_MODE_DELAY		SD_REV_MODE_DELAY
#define SCESK_EFFECT_MODE_PIPE		SD_REV_MODE_PIPE
#define SCESK_EFFECT_MODE_MAX		SD_REV_MODE_MAX
#define SCESK_EFFECT_MODE_CLEAR_WA	SD_REV_MODE_CLEAR_WA

/* for `sceSkSsSetEffect' */
#define SCESK_EFFECT_ON 1
#define SCESK_EFFECT_OFF 0

/* for `sceSkSsSetEffectVolume' */
#define SCESK_EFFECT_VOLUME_MAX 127

/* for `sceSkSsSetVoiceOutputMode()' */
#define SCESK_OUTPUT_MODE_MONO 0
#define SCESK_OUTPUT_MODE_STEREO 1

/*
 * Public functions
 */

/* common attribute */
#define sceSkSsSetDigitalOut(v)		sceSkSsSOUND_Control(0,SCESK_DIGITAL_OUT, (v))
#define sceSkSsSetMasterVolume(l,r)	sceSkSsSOUND_Control(3,SCESK_MASTERVOLUME,(l),(r))
#define sceSkSsSetEffectMode(m,d,f)	sceSkSsSOUND_Control(3,SCESK_EFFECT_MODE,(m),(d),(f))
#define sceSkSsSetEffect(b)		sceSkSsSOUND_Control(3,SCESK_EFFECT,(b))
#define sceSkSsSetEffectVolume(l,r)	sceSkSsSOUND_Control(3,SCESK_EFFECT_VOLUME,(l),(r))

/* transfer */
#define sceSkSsSend(f,c,iopa,spu2a,s)	sceSkSsSOUND_Send(c,iopa,spu2a,s)

/* bind/unbind HD, BD data */
#define sceSkSsBindHDBD(id,hda,hds,bda,bds)     sceSkSsHSYNTH_Bind(SCESK_HDBD,(id),(hda),(hds),(bda),(bds))
#define sceSkSsUnbindHDBD(id)                   sceSkSsHSYNTH_Unbind(SCESK_HDBD,(id))
#define sceSkSsBindTrackHDBD(id,hdbdid,bank_no) sceSkSsHSYNTH_Bind(SCESK_TRACK,(id),(hdbdid),(bank_no))
#define sceSkSsUnbindTrackHDBD(id) 	        sceSkSsHSYNTH_Unbind(SCESK_TRACK,(id))

/* bind/unbind SQ data */
#define sceSkSsBindSQ(f,id,sqa,sqs)	sceSkSs##f##_Bind(SCESK_SQ,(id),(sqa),(sqs))
#define sceSkSsBindMIDI(id,sqa,sqs)	sceSkSsMIDI_Bind(SCESK_SQ,(id),(sqa),(sqs))
#define sceSkSsBindSONG(id,sqa,sqs)	sceSkSsMIDI_Bind(SCESK_SQ,(id),(sqa),(sqs))
#define sceSkSsBindSESQ(id,sqa,sqs)	sceSkSsSESQ_Bind(SCESK_SQ,(id),(sqa),(sqs))
#define sceSkSsBindMSIN(id,a,b)		sceSkSsMSIN_Bind(SCESK_SQ,(id))
#define sceSkSsBindVoice(id,a,b)	sceSkSsMSIN_Bind(SCESK_SQ,(id))
#define sceSkSsBindTrackSQ(f,id,sqid)	sceSkSs##f##_Bind(SCESK_TRACK,(id),(sqid))
#define sceSkSsBindTrackMIDI(id,sqid)	sceSkSsMIDI_Bind(SCESK_TRACK,(id),(sqid))
#define sceSkSsBindTrackSONG(id,sqid)	sceSkSsMIDI_Bind(SCESK_TRACK,(id),(sqid))
#define sceSkSsBindTrackSESQ(id,sqid)	sceSkSsSESQ_Bind(SCESK_TRACK,(id),(sqid))
#define sceSkSsBindTrackMSIN(id,sqid)	sceSkSsMSIN_Bind(SCESK_TRACK,(id),(sqid))
#define sceSkSsBindTrackVoice(id,sqid)	sceSkSsMSIN_Bind(SCESK_TRACK,(id),(sqid))

#define sceSkSsUnbindSQ(f,id)		sceSkSs##f##_Unbind(SCESK_SQ,(id))
#define sceSkSsUnbindMIDI(id)		sceSkSsMIDI_Unbind(SCESK_SQ,(id))
#define sceSkSsUnbindSONG(id)		sceSkSsMIDI_Unbind(SCESK_SQ,(id))
#define sceSkSsUnbindSESQ(id)		sceSkSsSESQ_Unbind(SCESK_SQ,(id))
#define sceSkSsUnbindMSIN(id)		sceSkSsMSIN_Unbind(SCESK_SQ,(id))
#define sceSkSsUnbindVoice(id)		sceSkSsMSIN_Unbind(SCESK_SQ,(id))
#define sceSkSsUnbindTrackSQ(f,id)	sceSkSs##f##_Unbind(SCESK_TRACK,(id))
#define sceSkSsUnbindTrackMIDI(id)	sceSkSsMIDI_Unbind(SCESK_TRACK,(id))
#define sceSkSsUnbindTrackSONG(id)	sceSkSsMIDI_Unbind(SCESK_TRACK,(id))
#define sceSkSsUnbindTrackSESQ(id)	sceSkSsSESQ_Unbind(SCESK_TRACK,(id))
#define sceSkSsUnbindTrackMSIN(id)	sceSkSsMSIN_Unbind(SCESK_TRACK,(id))
#define sceSkSsUnbindTrackVoice(id)	sceSkSsMSIN_Unbind(SCESK_TRACK,(id))

/* bind/unbind: universal macros: */
/*   f = HDBD                   ... d = TRACK or HDBD
     f = MIDI, SONG, SESQ, MSIN ... d = TRACK or SQ */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)	/* ISO/IEC C99 */
#define sceSkSsBind(f,d,...)		sceSkSs##f##_Bind(SCESK_##d,__VA_ARGS__)
#define sceSkSsUnbind(f,d,...)		sceSkSs##f##_Unbind(SCESK_##d,__VA_ARGS__)
#elif defined(__GNUC__)		/* GNU Compiler Collection */
#define sceSkSsBind(f,d,args...)	sceSkSs##f##_Bind(SCESK_##d,args)
#define sceSkSsUnbind(f,d,args...)	sceSkSs##f##_Unbind(SCESK_##d,args)
#else
/* CPP has no capability for the macros with variable number of arguments. */
#endif

/* play/pause/stop */
#define sceSkSsPlayMIDI(id,n)	sceSkSsMIDI_Control((id),SCESK_PLAY, SCESK_MIDI,(n))
#define sceSkSsPlaySONG(id,n)	sceSkSsMIDI_Control((id),SCESK_PLAY, SCESK_SONG,(n))
#define sceSkSsPauseMIDI(id)	sceSkSsMIDI_Control((id),SCESK_PAUSE,SCESK_MIDI)
#define sceSkSsPauseSONG(id)	sceSkSsMIDI_Control((id),SCESK_PAUSE,SCESK_SONG)
#define sceSkSsStopMIDI(id)	sceSkSsMIDI_Control((id),SCESK_STOP, SCESK_MIDI)
#define sceSkSsStopSONG(id)	sceSkSsMIDI_Control((id),SCESK_STOP, SCESK_SONG)

/* volume */
#define sceSkSsSetPlayVolumeMIDI(id,v)	sceSkSsMIDI_Control((id),SCESK_VOLUME,SCESK_MIDI,(v))
#define sceSkSsSetPlayVolumeSONG(id,v)	sceSkSsMIDI_Control((id),SCESK_VOLUME,SCESK_SONG,(v))
#define sceSkSsSetPlayVolume(f,id,v)	sceSkSs##f##_Control((id),SCESK_VOLUME,SCESK_##f,(v))

/* tempo */
#define sceSkSsSetPlayRelativeTempoMIDI(id,t)	sceSkSsMIDI_Control((id),SCESK_TEMPO_REL,SCESK_MIDI,(t))
#define sceSkSsSetPlayAbsoluteTempoMIDI(id,t)	sceSkSsMIDI_Control((id),SCESK_TEMPO,    SCESK_MIDI,(t))
#define sceSkSsSetPlayRelativeTempoSONG(id,t)	sceSkSsMIDI_Control((id),SCESK_TEMPO_REL,SCESK_SONG,(t))
#define sceSkSsSetPlayAbsoluteTempoSONG(id,t)	sceSkSsMIDI_Control((id),SCESK_TEMPO,    SCESK_SONG,(t))
#define sceSkSsSetPlayTempoMIDI(f,id,t)		sceSkSsMIDI_Control((id),SCESK_TEMPO_##f,SCESK_MIDI,(t))
#define sceSkSsSetPlayTempoSONG(f,id,t)		sceSkSsMIDI_Control((id),SCESK_TEMPO_##f,SCESK_SONG,(t))
/* f = MIDI or SONG, d = RELATIVE or ABSOLUTE */
#define sceSkSsSetPlayTempo(f,d,id,t)		sceSkSsMIDI_Control((id),SCESK_TEMPO_##d,SCESK_##f,(t))

#define sceSkSsPlaySESQ(id,n1,n2)	sceSkSsSESQ_Control((id),SCESK_PLAY,SCESK_SESQ,(n1),(n2))
#define sceSkSsStopSESQ(id,id2)		sceSkSsSESQ_Control((id),SCESK_STOP,SCESK_SESQ,(id2))

/* play/pause/stop: universal macros: */
/*   f = MIDI, SONG, SESQ(`pause' is not supported)  */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)	/* ISO/IEC C99 */
#define sceSkSsPlay(f,id,...)	 sceSkSs##f##_Control((id),SCESK_PLAY, SCESK_##f,__VA_ARGS__)
#define sceSkSsPause(f,id)	 sceSkSs##f##_Control((id),SCESK_PAUSE,SCESK_##f)
#define _sceSkSsStopMIDI(id)	 sceSkSsMIDI_Control((id),SCESK_STOP, SCESK_MIDI)
#define _sceSkSsStopSONG(id)	 sceSkSsSONG_Control((id),SCESK_STOP, SCESK_SONG)
#define _sceSkSsStopSESQ(id,n)	 sceSkSsSESQ_Control((id),SCESK_STOP, SCESK_SESQ,(n))
#define sceSkSsStop(f,...)	_sceSkSsStop##f(__VA_ARGS__)
#elif defined(__GNUC__)		/* GNU Compiler Collection */
#define sceSkSsPlay(f,id,n...)	 sceSkSs##f##_Control((id),SCESK_PLAY, SCESK_##f,n)
#define sceSkSsPause(f,id)	 sceSkSs##f##_Control((id),SCESK_PAUSE,SCESK_##f)
#define _sceSkSsStopMIDI(id)	 sceSkSsMIDI_Control((id),SCESK_STOP, SCESK_MIDI)
#define _sceSkSsStopSONG(id)	 sceSkSsSONG_Control((id),SCESK_STOP, SCESK_SONG)
#define _sceSkSsStopSESQ(id,n)	 sceSkSsSESQ_Control((id),SCESK_STOP, SCESK_SESQ,(n))
#define sceSkSsStop(f,id...)	_sceSkSsStop##f(id)
#else
/* CPP has no capability for the macros with variable number of arguments. */
#endif

#define sceSkSsAllNoteOff(id)		sceSkSsHSYNTH_Control((id),SCESK_ALLNOTEOFF)
#define sceSkSsAllSoundOff(id)		sceSkSsHSYNTH_Control((id),SCESK_ALLSOUNDOFF)

#define sceSkSsExcludeVoice(c0,c1)	sceSkSsHSYNTH_Control(0,SCESK_EXCLUDED_VOICE,c0,c1)
#define sceSkSsGetExcludedVoice(c)	sceSkSsHSYNTH_Status(0,SCESK_EXCLUDED_VOICE,c)
#define sceSkSsSetExcludedVoice		sceSkSsExcludeVoice

#define sceSkSsSetOutputMode(m)		sceSkSsHSYNTH_Control(0,SCESK_OUTPUT_MODE,m)
#define sceSkSsGetOutputMode()		sceSkSsHSYNTH_Status(0,SCESK_OUTPUT_MODE)

#define sceSkSsVoiceNoteOn(tid,id,b,pg,ch,n,v,p) sceSkSsMSIN_Control(tid, SCESK_NOTEON, id,b,pg,ch,n,v,p)
#define sceSkSsVoiceNoteOff(tid,id,b,pg,ch,n)    sceSkSsMSIN_Control(tid, SCESK_NOTEOFF,id,b,pg,ch,n,0,0)
#define sceSkSsVoiceSetPanpot(tid,id,ch,n,p)     sceSkSsMSIN_Control(tid, SCESK_PANPOT,id,ch,n,p)
#define sceSkSsVoiceSetPitchBend(tid,id,ch,n,p)  sceSkSsMSIN_Control(tid, SCESK_PITCHBEND,id,ch,n,p)
#define sceSkSsVoiceSetPitchbend		 sceSkSsVoiceSetPitchBend
#define sceSkSsVoiceSetExpression(tid,id,ch,n,e) sceSkSsMSIN_Control(tid, SCESK_EXPRESSION,id,ch,n,e)

/* status */
#define sceSkSsStatusMIDI(id,c)   sceSkSsMIDI_Status((id),(c))
#define sceSkSsStatusSONG(id,c)   sceSkSsSONG_Status((id),(c))
#define sceSkSsStatusSESQ(id,c,n) sceSkSsSESQ_Status((id),(c),SCESK_SESQ,(n))
#define sceSkSsStatusMSIN(id,c,vid)   sceSkSsMSIN_Status((id),(c),(vid))
#define sceSkSsStatusHDBD(tid,c,hb_id) sceSkSsHSYNTH_Status((tid),(c),(hb_id))
#define sceSkSsGetStatusMIDI	sceSkSsStatusMIDI
#define sceSkSsGetStatusSONG	sceSkSsStatusSONG
#define sceSkSsGetStatusSESQ	sceSkSsStatusSESQ
#define sceSkSsGetStatusMSIN	sceSkSsStatusMSIN
#define sceSkSsGetStatusHDBD	sceSkSsStatusHDBD
#define sceSkSsGetStatusVoice	sceSkSsStatusMSIN
#define sceSkSsStatusVoice	sceSkSsStatusMSIN

/* status: universal macros: */
/*   f = MIDI, SONG, SESQ, MSIN */
/* #define sceSkSsStatus(f,id,c)   sceSkSsMIDI_Status((id),(c)) */
#define _sceSkSsStatusMIDI(id,c)	 sceSkSsMIDI_Status((id),(c))
#define _sceSkSsStatusSONG(id,c)	 sceSkSsSONG_Status((id),(c))
#define _sceSkSsStatusSESQ(id,c,n)	 sceSkSsSESQ_Status((id),(c),SCESK_SESQ,(n))
#define _sceSkSsStatusMSIN(id,c,n)	 sceSkSsMSIN_Status((id),(c),(n))
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)	/* ISO/IEC C99 */
#define sceSkSsStatus(f,...)		_sceSkSsStatus##f(__VA_ARGS__)
#elif defined(__GNUC__)		/* GNU Compiler Collection */
#define sceSkSsStatus(f,id...)		_sceSkSsStatus##f(id)
#else
/* CPP has no capability for the macros with variable number of arguments. */
#endif
#define sceSkSsGetStatus	sceSkSsStatus

#endif /* _STANDARDKIT_EE_SOUND_PUBLIC_H_ */
/* This file ends here, DON'T ADD STUFF AFTER THIS */
