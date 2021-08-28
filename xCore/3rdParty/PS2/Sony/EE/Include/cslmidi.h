/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 * Emotion Engine / I/O Processor Common Header
 *
 * Copyright (C) 1998-1999,2001 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * cslmidi.h
 *	Component Sound Library MIDI Stream
 */
#ifndef _cslmidi_h_
#define _cslmidi_h_
typedef struct {
	unsigned int	buffsize;	/* include Header size	*/
	unsigned int	validsize;	/* valid data size	*/
	unsigned char	data[0];	/* data max is data[buffsize] */
} sceCslMidiStream;
/**** MIDI Special Messages ****/
#define sceCslMidiHsStatusEx1		0xf9
#define sceCslMidiHsStatusEx2		0xfd
#define sceCslMidiHsCmdExpression	0x00
#define sceCslMidiHsCmdPanpot		0x01
#define sceCslMidiHsCmdPitchBend	0x02
#define sceCslMidiHsCmdNoteCtrl		0x10
#define sceCslMidiHsCtrlAllKey		(1<<4)
#define sceCslMidiHsKIdAll			0x7f
#endif /* !_cslmidi_h_ */
