/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 * Emotion Engine / I/O Processor Common Header
 *
 * Copyright (C) 2000,2001,2002,2003 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * cslse.h
 *	Component Sound Library SE Stream
 */
#ifndef _cslse_h_
#define _cslse_h_
typedef struct {
    unsigned int buffsize;	/* include Header size		*/
    unsigned int validsize;	/* valid data size		*/
    unsigned char data[0];	/* data max is data[buffsize]	*/
} sceCslSeStream;

#define sceSEMsg_PREFIX  0xfe
#define sceSEMsg_VERSION 0x01
#define sceSEMsg_PREFIX_LENGTH 2
#define sceSEMsg_ID_LENGTH 4
#define sceSEMsg_HEADER_LENGTH \
	(sceSEMsg_PREFIX_LENGTH + sceSEMsg_ID_LENGTH)

#define sceSEMsg_TIMBRESET_NUMBER_MAX	0xf

#define sceSEMsg_STATUS_NOTE		0xa0
#define sceSEMsg_STATUS_NOTE0		0x90
#define sceSEMsg_STATUS_NOTE_ONZERO	sceSEMsg_STATUS_NOTE0
#define sceSEMsg_STATUS_VCTRL		0xb0
#define sceSEMsg_STATUS_VGCTRL		0xc0

/* Voice Control */
#define sceSEMsg_VCTRL_TIME_VOLUME		0x07

#define sceSEMsg_VCTRL_TIME_PITCHLFO_CYCLE	0x0a
#define sceSEMsg_VCTRL_TIME_AMPLFO_CYCLE	0x0b

#define sceSEMsg_VCTRL_TIME_PANPOT_CW		0x0c
#define sceSEMsg_VCTRL_TIME_PANPOT_CCW		0x0d

#define sceSEMsg_VCTRL_TIME_PITCH_P		0x0e
#define sceSEMsg_VCTRL_TIME_PITCH_M		0x0f

#define sceSEMsg_VCTRL_PITCHLFO_DEPTH_P		0x10
#define sceSEMsg_VCTRL_PITCHLFO_DEPTH_M		0x11
#define sceSEMsg_VCTRL_PITCHLFO_CYCLE		0x12

#define sceSEMsg_VCTRL_AMPLFO_DEPTH_P		0x20
#define sceSEMsg_VCTRL_AMPLFO_DEPTH_M		0x21
#define sceSEMsg_VCTRL_AMPLFO_CYCLE		0x22

/* Voice Group Control */
#define sceSEMsg_VGCTRL_REPEAT_DELTATIME	0x00
#define sceSEMsg_VGCTRL_REPEAT_COMMAND		0x01
#define sceSEMsg_VGCTRL_ALL_NOTE_OFF_MASK	0x1e
#define sceSEMsg_VGCTRL_ALL_NOTE_OFF		0x1f

#endif /* !_cslse_h_ */



