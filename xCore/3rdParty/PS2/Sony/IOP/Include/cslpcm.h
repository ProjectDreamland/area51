/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 * Emotion Engine / I/O Processor Common Header
 *
 * Copyright (C) 1998-1999, 2001 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * cslpcm.h
 *	Component Sound Library PCM Stream
 */
#ifndef _cslpcm_h_
#define _cslpcm_h_
typedef struct {
	unsigned int	pcmbuf_size;	/* real pcm buffer size, NOT include header size */
	unsigned int	validsize;
	void		*pcmbuf;
	unsigned int	pad;
} sceCslPcmStream;
#endif /* !_cslpcm_h_ */
