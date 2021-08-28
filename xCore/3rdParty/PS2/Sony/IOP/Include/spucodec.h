/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* $Id: spucodec.h,v 1.3 2002/03/27 02:01:47 xokano Exp $ */

/*
 *                     I/O Processor System Services
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         spucodec.h
 *                         --- Codec Handler for SPU2
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 */

#ifndef _SPUCODEC_H
#define _SPUCODEC_H

#define SPUCODEC_ENCODE_ERROR    (-1)
#define SPUCODEC_ENCODE_WHOLE     0
#define SPUCODEC_ENCODE_START    (1<<0)
#define SPUCODEC_ENCODE_CONTINUE (1<<1)
#define SPUCODEC_ENCODE_END      (1<<2)

#define SPUCODEC_ENCODE_LOOP    1
#define SPUCODEC_ENCODE_NO_LOOP 0

#define SPUCODEC_ENCODE_ENDIAN_LITTLE 0
#define SPUCODEC_ENCODE_ENDIAN_BIG    1

#define SPUCODEC_ENCODE_MIDDLE_QULITY 0
#define SPUCODEC_ENCODE_HIGH_QULITY   1

typedef struct {
    short *src;			/* 16-bit strait PCM */
    short *dest;		/* PlayStation original waveform data */
    short *work;		/* scratch pad or NULL */
    int    size;		/* size (unit: byte) of source data */
    int    loop_start;		/* loop start point (unit: byte) of source data */
    int    loop;		/* whether loop or not */
    int    byte_swap;		/* source data is 16-bit big endian (1) / little endian (0) */
    int    proceed;		/* proceeding ? whole (0) / start (1) / cont. (2) / end (4) */
    int    quality;		/* quality ? middle (0) / high (1) */ 
} sceSpuEncodeEnv;

#ifdef __cplusplus
extern "C" {
#endif
extern int sceSpuCodecEncode (sceSpuEncodeEnv *);
#ifdef __cplusplus
}
#endif
#endif /* _SPUCODEC_H */
