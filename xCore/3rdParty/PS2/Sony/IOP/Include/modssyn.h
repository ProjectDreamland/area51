/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                     I/O Processor Library
 *                          Version 0.60
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                       All Rights Reserved.
 *
 *                       modssyn - modssyn.h
 *               IOP Software Synthesizer Comunication
 *
 *     Version   Date          Design     Log
 *  --------------------------------------------------------------------
 *     0.60      Oct.12.1999   katayama   first checked in.
 */
#ifndef _modssyn_h_
#define _modssyn_h_

#ifndef Bool
#define Bool int
#define True 1
#define False 0
#endif

#define sceSSynNoError		0
#define sceSSynError			-1

typedef struct {
	unsigned int	ee_info_addr;
	unsigned int	ee_buff_addr;
	unsigned int	ee_buff_length;
	unsigned int	atickCount;
	unsigned int	ee_buff_write_index;
	int				pad1[3];
	unsigned int	ee_buff_read_index;
	int				pad2[3];
	unsigned char	alignment_adjust_buff[16];
	sceSifDmaData	dma[4];
} sceSSynEnv;

#define sceSSyn_GetEnv(x,y) \
	((sceSSynEnv*)(x)->buffGrp[0].buffCtx[((y)*2)+1].buff)

int sceSSyn_Init(sceCslCtx*,unsigned int);
int sceSSyn_ATick(sceCslCtx*);
int sceSSyn_Load(sceCslCtx*,unsigned int);
#endif /*!_modssyn_h_*/
/* $Id: modssyn.h,v 1.6 2003/09/12 05:28:29 tokiwa Exp $ */
