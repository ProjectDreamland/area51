/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                     I/O Processor Library
 *                          Version 0.60
 *                           Shift-JIS
 *
 *      Copyright (C) 1999, 2001, 2002 Sony Computer Entertainment Inc.
 *                       All Rights Reserved.
 *
 *                       modmidi - modmidi.h
 *                         IOP Sequencer
 *
 *     Version   Date          Design     Log
 *  --------------------------------------------------------------------
 *     0.60      Oct.12.1999   katayama   first checked in.
 */
#ifndef _modmidi_h_
#define _modmidi_h_

#ifndef Bool
#define Bool int
#define True 1
#define False 0
#endif

#define sceMidiNoError		0
#define sceMidiError			-1

#define sceMidiNumMidiCh		16
#define sceMidiEnvSize		472

#define sceMidiEnv_NoSongNum (-1)
#define sceMidiEnv_NoMidiNum (-1)

typedef struct {
#define sceMidiLoopInfoType_Midi	0
#define sceMidiLoopInfoType_Song	1
	unsigned char	type;
	unsigned char	loopTimes;
	unsigned char	loopCount;
	unsigned int	loopId;
} sceMidiLoopInfo;

#define sceMidiStat_ready		(1<<0)
#define sceMidiStat_inPlay		(1<<1)
#define sceMidiStat_dataEnd		(1<<2)
#define	sceMidiStat_noLoop		(1<<3)
#define sceMidiSongStat_inPlay	(1<<8)
#define sceMidiSongStat_waitMidi	(1<<9)
#define sceMidiSongStat_dataEnd	(1<<10)
typedef struct {
	unsigned int	songNum;
	unsigned int	midiNum;
	unsigned int	position;
	unsigned int	status;
	unsigned short	outPort[sceMidiNumMidiCh];
	unsigned short	excOutPort;
#define sceMidi_ChMsgNoData 0
	unsigned int	(*chMsgCallBack)(unsigned int,unsigned int);
	unsigned int	chMsgCallBackPrivateData;
	Bool			(*metaMsgCallBack)(unsigned char,unsigned char*,unsigned int,unsigned int);
	unsigned int	metaMsgCallBackPrivateData;
	Bool			(*excMsgCallBack)(unsigned char*,unsigned int,unsigned int);
	unsigned int	excMsgCallBackPrivateData;
	Bool			(*repeatCallBack)(sceMidiLoopInfo*,unsigned int);
	unsigned int	repeatCallBackPrivateData;
	Bool			(*markCallBack)(unsigned int, unsigned int, unsigned char *, unsigned int);
	unsigned int	markCallBackPrivateData;
	unsigned char	system[sceMidiEnvSize];
} sceMidiEnv;

#define sceMidi_RelativeTempoNoEffect 0x100

#define sceMidi_SongPlayStop		0
#define sceMidi_SongPlayPause	1
#define sceMidi_SongPlayStart	2
#define sceMidi_SongPlayContinue	sceMidi_SongPlayStart

#define sceMidi_MidiPlayStop		0
#define sceMidi_MidiPlayStart	1

#define sceMidi_GetEnv(x,y) ((sceMidiEnv*)(x)->buffGrp[0].buffCtx[((y)*2)+1].buff)
#define sceMidi_isInPlay(x,y) ((sceMidi_GetEnv(x,y)->status)&sceMidiStat_inPlay)
#define sceMidi_isDataEnd(x,y) ((sceMidi_GetEnv(x,y)->status)&sceMidiStat_dataEnd)

#define sceMidi_Volume0db	128
#define sceMidi_MidiMinTempo 20
#define sceMidi_MidiMaxTempo 255

int sceMidi_Init(sceCslCtx*,unsigned int);
int sceMidi_ATick(sceCslCtx*);
int sceMidi_Load(sceCslCtx*,unsigned int);
int sceMidi_Unload(sceCslCtx*,unsigned int);

/*** for Song Chunk ***/
int sceMidi_SelectSong(sceCslCtx*,unsigned int,unsigned int);
int sceMidi_SongPlaySwitch(sceCslCtx*,unsigned int,int);
int sceMidi_SongSetVolume(sceCslCtx*,unsigned int,unsigned char);
int sceMidi_SongVolumeChange(sceCslCtx*,unsigned int,unsigned char);
int sceMidi_SongSetAbsoluteTempo(sceCslCtx*,unsigned int,unsigned char);
int sceMidi_SongSetRelativeTempo(sceCslCtx*,unsigned int,unsigned short);
#define sceMidi_SSL_Now					(0<<0)
#define sceMidi_SSL_Delay				(1<<0)
#define sceMidi_SSL_WithPreCommand		(0<<1)
#define sceMidi_SSL_WithoutPreCommand	(1<<1)
int sceMidi_SongSetLocation(sceCslCtx*,unsigned int,unsigned int, unsigned int);

/*** for Midi Chunk ***/
int sceMidi_SelectMidi(sceCslCtx*,unsigned int,unsigned int);
int sceMidi_MidiPlaySwitch(sceCslCtx*,unsigned int,int);
int sceMidi_MidiSetLocation(sceCslCtx*,unsigned int,unsigned int);
#define sceMidi_MidiSetVolume_MasterVol 0xff
int sceMidi_MidiSetVolume(sceCslCtx*,unsigned int,unsigned char,unsigned char);
int sceMidi_MidiVolumeChange(sceCslCtx*,unsigned int,unsigned char,unsigned char);
#define sceMidi_MidiVolumeChange_AllMIDIChannel 0xff
int sceMidi_MidiSetAbsoluteTempo(sceCslCtx*,unsigned int,unsigned char);
unsigned char sceMidi_MidiGetAbsoluteTempo(sceCslCtx*,unsigned int);
int sceMidi_MidiSetRelativeTempo(sceCslCtx*,unsigned int,unsigned short);
unsigned short sceMidi_MidiGetRelativeTempo(sceCslCtx*,unsigned int);
#define sceMidi_GetTempo(x,y) \
	((((unsigned int)(x))*(unsigned int)(y))/sceMidi_RelativeTempoNoEffect)
extern int sceMidi_MidiSetUSecTempo (sceCslCtx *, unsigned int, unsigned int);
extern unsigned int sceMidi_MidiGetUSecTempo (sceCslCtx *, unsigned int);

#endif /*!_modmidi_h_*/
/* $Id: modmidi.h,v 1.12 2003/09/24 04:35:20 kaol Exp $ */

