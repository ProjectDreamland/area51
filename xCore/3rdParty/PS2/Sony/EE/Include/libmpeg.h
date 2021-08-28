/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 0.70
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libmpeg - libmpeg.h
 *                     header file of libmpeg 
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      0.10            Aug.14.1999     umemura     the first version
 *      0.20            Dec.11.1999     umemura     PSS support
 *      0.30            Dec.15.1999     umemura     API is modified
 *      0.40            Jan.19.2000     umemura     flags area added
 *      0.50            Feb.05.2000     umemura     MPEG_BUFFER_SIZE
 *      0.60            Jul.22.2002     ywashizu    SCE_MPEG_FREE_SPR_ADDR
 *                                                  SCE_MPEG_FREE_SPR_SIZE
 *      0.70            Aug.13.2003     hana        add SCE_MPEG_BUFF_TYPE_SPR
 *                                                      SCE_MPEG_BUFF_TYPE_DRAM
 *                                                      sceMpegSetBuffType
 *                                                      sceMpegGetBuffType
 *      0.80            Oct.05.2003     hana        change comment
 */
#ifndef _LIBMPEG_H_
#define _LIBMPEG_H_

#include <eetypes.h>
#include <eeregs.h>
#include <libipu.h>

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************
 * Definitions for MPEG decoder
 **********************************************************************/
#define SCE_MPEG_BUFFER_SIZE(w, h)  ((w)*(h)*9/2 + 512 + 24*64 + 8192)
#define SCE_MPEG_DECODE_ALL     (-1)

#define SCE_MPEG_FREE_SPR_ADDR      (0x70003900)
#define SCE_MPEG_FREE_SPR_SIZE      (0x700)

/**********************************************************************
 *
 * Flags for MPEG decoder
 *
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                             |p|p|t|r|  p|    p|
 * |                                             |s|f|f|f|  s|    i|
 * |                                             |e|r|f|f|  t|    c|
 * |                                             |q|m| | |  r|     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *    <pic>: picture_coding_type
 *
 *        000: Reserved
 *    001: I picture
 *    010: P picture
 *    011: B picture
 *        100: D picture (mpeg1)
 *    101: Reserved
 *    110: Reserved
 *    111: Reserved
 *
 *    <pstr>: picture_structure
 *
 *        00: reserved
 *    01: Top Field
 *    10: Bottom Field
 *    11: Frame Picture
 *
 *    <rff>: repeat_first_field
 *    <tff>: top_field_first
 *    <pfrm>: progressive_frame
 *    <pseq>: progressive_sequence
 */

/* Picture Coding Type */
#define SCE_MPEG_I_PIC      0x1
#define SCE_MPEG_P_PIC      0x2
#define SCE_MPEG_B_PIC      0x3
#define SCE_MPEG_D_PIC      0x4

/* Picture Structure */
#define SCE_MPEG_TOP_FIELD  0x1
#define SCE_MPEG_BOT_FIELD  0x2
#define SCE_MPEG_FRAME      0x3

/**********************************************************************
 * Stream types
 **********************************************************************/
typedef enum {
    sceMpegStrM2V       = 0,    /* MPEG2 video stream */
    sceMpegStrIPU       = 1,    /* IPU stream */
    sceMpegStrPCM       = 2,    /* PCM stream */
    sceMpegStrADPCM     = 3,    /* ADPCM stream */
    sceMpegStrDATA      = 4     /* DATA stream */
} sceMpegStrType;

/**********************************************************************
 * MPEG callback types
 **********************************************************************/
typedef enum {
    sceMpegCbError      = 0,
    sceMpegCbNodata     = 1,
    sceMpegCbStopDMA    = 2,
    sceMpegCbRestartDMA = 3,
    sceMpegCbBackground = 4,
    sceMpegCbTimeStamp  = 5,
    sceMpegCbStr        = 6
} sceMpegCbType;


/**********************************************************************
 * MPEG callback data
 **********************************************************************/

/* data structure for sceMpegCbError callback */
typedef struct {
    sceMpegCbType type;
    char *errMessage;
} sceMpegCbDataError;

/* data structure for sceMpegCbTimeStamp callback */
typedef struct {
    sceMpegCbType type;
    long pts;       /* PTS value; valid only when pts >= 0 */
    long dts;       /* DTS value; valid only when dts >= 0 */
} sceMpegCbDataTimeStamp;

/* data structure for sceMpegCbStr callback */
typedef struct {
    sceMpegCbType type;
    u_char *header; /* the begining of packet header */
    u_char *data;   /* the begining of packet data */
    u_int  len;     /* length of packet data */
    long   pts;     /* PTS value; valid only when pts >= 0 */
    long   dts;     /* DTS value; valid only when dts >= 0 */
} sceMpegCbDataStr;

/* Universal callback data structure */
/* sceMpegCbData is used as the default callback data structure */
typedef union {
    sceMpegCbType type;
    sceMpegCbDataError error;
    sceMpegCbDataTimeStamp ts;
    sceMpegCbDataStr str;
} sceMpegCbData;

/**********************************************************************
 * MPEG decoder
 **********************************************************************/
typedef struct {
    int width;      /*  width of decoded image */
    int height;     /*  height of decoded image */
    int frameCount; /*  frame number in the stream */

    long pts;       /*  PTS(Presentation Time Stamp) value  */
                    /*  pts is valid only when pts >= 0 */

    long dts;       /*  DTS(Decoding Time Stamp) value */
                    /*  dts is valid only when dts >= 0 */

    u_long flags;   /*  flags */

    long pts2nd;    /*  PTS for 2nd field(for future use) */
    long dts2nd;    /*  DTS for 2nd field(for future use) */
    u_long flags2nd;    /*  flags for 2nd field(for future use) */

    void *sys;      /*  system data for decoding */
} sceMpeg;

/**********************************************************************
 * MPEG callback function definition
 **********************************************************************/
typedef int (*sceMpegCallback)(sceMpeg *mp,
            sceMpegCbData *cbData, void *anyData);

/**********************************************************************
 * Functions of MPEG library
 **********************************************************************/

/* for Erx */
void *sceMpegGetErxEntries(void);
    
/* =====================================================================
 * sceMpegInit
 * =====================================================================
 * Initialize MPEG library
 */
int sceMpegInit(void);  

/* =====================================================================
 * sceMpegCreate
 * =====================================================================
 * Create MPEG decoder
 */
int sceMpegCreate(sceMpeg *mp, u_char *work_area, int work_area_size);

/* =====================================================================
 * sceMpegDelete
 * =====================================================================
 * Delete MPEG decoder
*/
int sceMpegDelete(sceMpeg *mp);

/* =====================================================================
 * sceMpegAddBs
 * =====================================================================
 * Set bitstream to MPEG decoder
 */
int sceMpegAddBs(sceMpeg *mp, u_long128 *p, int size);

/* =====================================================================
 * sceMpegGetPicture
 * =====================================================================
 * Decode and extract one picture from input bitsream (RGB32 format)
 */
int sceMpegGetPicture(sceMpeg *mp, sceIpuRGB32 *rgb32, int mbcount);

/* =====================================================================
 * sceMpegGetPictureRAW8
 * =====================================================================
 * Decode and extract one picture from input bitsream (RAW8 format)
 */
int sceMpegGetPictureRAW8(sceMpeg *mp, sceIpuRAW8 *raw8, int mbcount);

/* =====================================================================
 * sceMpegGetPictureAbort
 * =====================================================================
 * Abort sceMpegGetPicture function
 */
void sceMpegGetPictureAbort(sceMpeg *mp);

/* =====================================================================
 * sceMpegReset
 * =====================================================================
 * Re-initialize MPEG decoder
 */
int sceMpegReset(sceMpeg *mp);

/* =====================================================================
 * sceMpegIsEnd
 * =====================================================================
 * Check to see if MPEG decoder encounters sequence_end_code or not
 */
int sceMpegIsEnd(sceMpeg *mp);

/* =====================================================================
 * sceMpegIsRefBuffEmpty
 * =====================================================================
 * Check to see if inner reference image buffers are empty or not
 */
int sceMpegIsRefBuffEmpty(sceMpeg *mp);

/* =====================================================================
 * sceMpegDemuxPss
 * =====================================================================
 * Parse pss stream and call callback function for each 
 * elementary stream. This is used for de-multiplexing PSS data.
 */
int sceMpegDemuxPss(sceMpeg *mp, u_char *pss, int pss_size);

/* =====================================================================
 * sceMpegDemuxPssRing
 * =====================================================================
 * Parse pss stream in a ring shape buffer and call callback
 * function for each elementary stream. This is used for de-multiplexing
 * PSS data.
 */
int sceMpegDemuxPssRing(sceMpeg *mp, u_char *pss, int pss_size,
    u_char *buf_top, int buf_size);

/* =====================================================================
 * sceMpegSetDecodeMode
 * =====================================================================
 * Set decode mode of MPEG decoder
 */
void sceMpegSetDecodeMode(sceMpeg *mp, int ni, int np, int nb);

/* =====================================================================
 * sceMpegGetDecodeMode
 * =====================================================================
 * Get decode mode of MPEG decoder
 */
void sceMpegGetDecodeMode(sceMpeg *mp, int *ni, int *np, int *nb);

/* =====================================================================
 * sceMpegAddCallback
 * =====================================================================
 * Add callback function to MPEG decoder
 */
sceMpegCallback sceMpegAddCallback(
    sceMpeg *mp,
    sceMpegCbType type,
    sceMpegCallback callback,
    void *anyData
);

/* =====================================================================
 * sceMpegAddStrCallback
 * =====================================================================
 * Add callback function for sceMpegCbStr callback
 */
sceMpegCallback sceMpegAddStrCallback(
    sceMpeg *mp,
    sceMpegStrType strType,
    int ch,
    sceMpegCallback callback,
    void *anyData
);


#define SCE_MPEG_BUFF_TYPE_SPR      0
#define SCE_MPEG_BUFF_TYPE_DRAM     1

/* =====================================================================
 * sceMpegSetBuffType
 * =====================================================================
 * Set buffer type of MPEG decorder
 */
int sceMpegSetBuffType(sceMpeg * mp, int buffType);

/* =====================================================================
 * sceMpegGetBuffType
 * =====================================================================
 * Get buffer type of MPEG decorder
 */
int sceMpegGetBuffType(sceMpeg * mp);

#ifdef __cplusplus
}
#endif

#endif /* _LIBMPEG_H_ */

