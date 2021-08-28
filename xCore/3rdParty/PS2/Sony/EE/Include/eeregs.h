/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                      Emotion Engine Library
 *                          Version 4.00
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         eeregs.h
 *                         develop library
 *
 *       Version        Date            Design      Log
 *      --------------------------------------------------
 *      1.00            Jan,8,1998      yutaka      for sbl
 *      2.00            Jan,21,1998     yutaka      for new register mapping
 *      3.00            Sep,10,1998     ywashizu    add structure/macro
 *      4.00            Sep,25,2003     hana        change comment
 */
#ifndef _EEREGS_H_
#define _EEREGS_H_

#include <eekernel.h>


/* TIMER */

/*
 * Tn_COUNT
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               |                               |
 * |                               |             COUNT             |
 * |                               |                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define T_COUNT_COUNT_M     (0xffff<<0)

#define T_COUNT_COUNT_O     ( 0)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned COUNT: 16; /* Counter Value */
    unsigned p0   : 16;
} tT_COUNT;



/*
 * Tn_MODE
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                       |O|E|O|C|C|Z| G |G|G| C |
 * |                                       |V|Q|V|M|U|R| A |A|A| L |
 * |                                       |F|U|F|P|E|E| T |T|T| K |
 * |                                       |F|F|E|E| |T| M |S|E| S |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define T_MODE_CLKS_M       (0x03<< 0)
#define T_MODE_GATE_M       (0x01<< 2)
#define T_MODE_GATS_M       (0x01<< 3)
#define T_MODE_GATM_M       (0x03<< 4)
#define T_MODE_ZRET_M       (0x01<< 6)
#define T_MODE_CUE_M        (0x01<< 7)
#define T_MODE_CMPE_M       (0x01<< 8)
#define T_MODE_OVFE_M       (0x01<< 9)
#define T_MODE_EQUF_M       (0x01<<10)
#define T_MODE_OVFF_M       (0x01<<11)

#define T_MODE_CLKS_O       ( 0)
#define T_MODE_GATE_O       ( 2)
#define T_MODE_GATS_O       ( 3)
#define T_MODE_GATM_O       ( 4)
#define T_MODE_ZRET_O       ( 6)
#define T_MODE_CUE_O        ( 7)
#define T_MODE_CMPE_O       ( 8)
#define T_MODE_OVFE_O       ( 9)
#define T_MODE_EQUF_O       (10)
#define T_MODE_OVFF_O       (11)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned CLKS: 2;   /* Clock Select */
    unsigned GATE: 1;   /* Gate Function */
    unsigned GATS: 1;   /* Gate Select */
    unsigned GATM: 2;   /* Gate mode */
    unsigned ZRET: 1;   /* Zero Return */
    unsigned CUE : 1;   /* Count Up Enable */
    unsigned CMPE: 1;   /* Interrupt by compare */
    unsigned OVFE: 1;   /* Interrupt by overflow */
    unsigned EQUF: 1;   /* Equal Flag */
    unsigned OVFF: 1;   /* overflow Flag */
    unsigned p0  :20;
} tT_MODE;



/*
 * Tn_COMP
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               |                               |
 * |                               |             COMP              |
 * |                               |                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define T_COMP_COMP_M       (0xffff<< 0)

#define T_COMP_COMP_O       ( 0)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned COMP:16;   /* Compare Value */
    unsigned p0  :16;
} tT_COMP;



/*
 * Tn_HOLD
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               |                               |
 * |                               |             HOLD              |
 * |                               |                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define T_HOLD_HOLD_M       (0xffff<< 0)

#define T_HOLD_HOLD_O       ( 0)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned HOLD:16;   /* Holded Value */
    unsigned p0  :16;
} tT_HOLD;



/* IPU */

/*
 * IPU_CMD (write)
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       |                                                       |
 * | CODE  |                          OPTION                       |
 * |       |                                                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define IPU_CMD_OPTION_M    (0xfffffff<< 0)
#define IPU_CMD_CODE_M      (0x0f     <<28)

#define IPU_CMD_OPTION_O    ( 0)
#define IPU_CMD_CODE_O      (28)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned OPTION:28; /* Command optin */
    unsigned CODE  : 4; /* Command code */
} tIPU_CMD_write;



/*
 * IPU_CMD (read)
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |B|                                                             |
 * |U|                                                             |
 * |S|                                                             |
 * |Y|                                                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                              DATA                             |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define IPU_CMD_DATA_M      (0xffffffff<< 0)
#define IPU_CMD_BUSY_M      (0x01L     <<63)

#define IPU_CMD_DATA_O      ( 0)
#define IPU_CMD_BUSY_O      (63)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned DATA:32;   /* VDEC decoded value */
    unsigned p0  :31;
    unsigned BUSY: 1;   /* VDEC command busy */
} tIPU_CMD_read;



/*
 * IPU_TOP
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |B|                                                             |
 * |U|                                                             |
 * |S|                                                             |
 * |Y|                                                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                             BSTOP                             |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define IPU_TOP_BSTOP_M     (0xffffffffL<< 0)
#define IPU_TOP_BUSY_M      (0x1L       <<63)

#define IPU_TOP_BSTOP_O     ( 0)
#define IPU_TOP_BUSY_O      (63)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned BSTOP:32;   /* top data of the bit stream (32 bit) */
    unsigned p0   :31;
    unsigned BUSY : 1;   /* busy flag */
} tIPU_TOP;



/*
 * IPU_CTRL
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |B|R|     |     |M|Q|I|A|   |   |S|E|           |       |       |
 * |U|S|     | PCT |P|S|V|S|   |IDP|C|C|    CBP    |  OFC  |  IFC  |
 * |S|T|     |     |1|T|F| |   |   |D|D|           |       |       |
 * |Y| |     |     | | | | |   |   | | |           |       |       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define IPU_CTRL_IFC_M      (0x0f<< 0)
#define IPU_CTRL_OFC_M      (0x0f<< 4)
#define IPU_CTRL_CBP_M      (0x3f<< 8)
#define IPU_CTRL_ECD_M      (0x01<<14)
#define IPU_CTRL_SCD_M      (0x01<<15)
#define IPU_CTRL_IDP_M      (0x03<<16)
#define IPU_CTRL_AS_M       (0x01<<20)
#define IPU_CTRL_IVF_M      (0x01<<21)
#define IPU_CTRL_QST_M      (0x01<<22)
#define IPU_CTRL_MP1_M      (0x01<<23)
#define IPU_CTRL_PCT_M      (0x07<<24)
#define IPU_CTRL_RST_M      (0x01<<30)
#define IPU_CTRL_BUSY_M     (0x01<<31)

#define IPU_CTRL_IFC_O      ( 0)
#define IPU_CTRL_OFC_O      ( 4)
#define IPU_CTRL_CBP_O      ( 8)
#define IPU_CTRL_ECD_O      (14)
#define IPU_CTRL_SCD_O      (15)
#define IPU_CTRL_IDP_O      (16)
#define IPU_CTRL_AS_O       (20)
#define IPU_CTRL_IVF_O      (21)
#define IPU_CTRL_QST_O      (22)
#define IPU_CTRL_MP1_O      (23)
#define IPU_CTRL_PCT_O      (24)
#define IPU_CTRL_RST_O      (30)
#define IPU_CTRL_BUSY_O     (31)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned IFC : 4;   /* Input FIFO counter */
    unsigned OFC : 4;   /* Output FIFO counter */
    unsigned CBP : 6;   /* Coded block pattern */
    unsigned ECD : 1;   /* Error code pattern */
    unsigned SCD : 1;   /* Start code detected */
    unsigned IDP : 2;   /* Intra DC precision */
    unsigned p0  : 2;
    unsigned AS  : 1;   /* Alternate scan */
    unsigned IVF : 1;   /* Intra VLC format */
    unsigned QST : 1;   /* Q scale step */
    unsigned MP1 : 1;   /* MPEG1 bit strea */
    unsigned PCT : 3;   /* Picture Type */
    unsigned p1  : 3;
    unsigned RST : 1;   /* Reset */
    unsigned BUSY: 1;   /* Busy */
} tIPU_CTRL;



/*
 * IPU_BP
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                           |   |       |       | |             |
 * |                           |FP |       |  IFC  | |      BP     |
 * |                           |   |       |       | |             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define IPU_BP_BP_M     (0x7f<< 0)
#define IPU_BP_IFC_M    (0x0f<< 8)
#define IPU_BP_FP_M     (0x03<<16)

#define IPU_BP_BP_O     ( 0)
#define IPU_BP_IFC_O    ( 8)
#define IPU_BP_FP_O     (16)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned BP : 7;    /* Bit stream point */
    unsigned p0 : 1;
    unsigned IFC: 4;    /* Input FIFO counter */
    unsigned p1 : 4;
    unsigned FP : 2;    /* FIFO point */
    unsigned p2 :14;
} tIPU_BP;



/* GIF */

/*
 * GIF_CTRL
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                       |P|   |R|
 * |                                                       |S|   |S|
 * |                                                       |E|   |T|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GIF_CTRL_RST_M  (0x01<< 0)
#define GIF_CTRL_PSE_M  (0x01<< 3)

#define GIF_CTRL_RST_O  ( 0)
#define GIF_CTRL_PSE_O  ( 3)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned  RST: 1;   /* GIF Reset */
    unsigned  p0 : 2;
    unsigned  PSE: 1;   /* Pause transfar */
    unsigned  p1 :28;
} tGIF_CTRL;



/*
 * GIF_MODE
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                         |I| |M|
 * |                                                         |M| |3|
 * |                                                         |T| |R|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GIF_MODE_M3R_M      (0x01<< 0)
#define GIF_MODE_IMT_M      (0x01<< 2)

#define GIF_MODE_M3R_O      ( 0)
#define GIF_MODE_IMT_O      ( 2)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned  M3R: 1;   /* Mask PATH3 by Register */
    unsigned  p0 : 1;
    unsigned  IMT: 1;   /* PATH3 IMAGE mode termination */
    unsigned  p1 :29;
} tGIF_MODE;



/*
 * GIF_STAT
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     |         |                     |D|APA|O|P|P|P|I| |P|I|M|M|
 * |     |   FQC   |                     |I| TH|P|1|2|3|P| |S|M|3|3|
 * |     |         |                     |R|   |H|Q|Q|Q|3| |E|T|P|R|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GIF_STAT_M3R_M      (0x01<< 0)
#define GIF_STAT_M3P_M      (0x01<< 1)
#define GIF_STAT_IMT_M      (0x01<< 2)
#define GIF_STAT_PSE_M      (0x01<< 3)
#define GIF_STAT_IP3_M      (0x01<< 5)
#define GIF_STAT_P3Q_M      (0x01<< 6)
#define GIF_STAT_P2Q_M      (0x01<< 7)
#define GIF_STAT_P1Q_M      (0x01<< 8)
#define GIF_STAT_OPH_M      (0x01<< 9)
#define GIF_STAT_APATH_M    (0x03<<10)
#define GIF_STAT_DIR_M      (0x01<<12)
#define GIF_STAT_FQC_M      (0x1f<<24)

#define GIF_STAT_M3R_O      ( 0)
#define GIF_STAT_M3P_O      ( 1)
#define GIF_STAT_IMT_O      ( 2)
#define GIF_STAT_PSE_O      ( 3)
#define GIF_STAT_IP3_O      ( 5)
#define GIF_STAT_P3Q_O      ( 6)
#define GIF_STAT_P2Q_O      ( 7)
#define GIF_STAT_P1Q_O      ( 8)
#define GIF_STAT_OPH_O      ( 9)
#define GIF_STAT_APATH_O    (10)
#define GIF_STAT_DIR_O      (12)
#define GIF_STAT_FQC_O      (24)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned  M3R  : 1; /* Mask PATH3 by Register */
    unsigned  M3P  : 1; /* Mask PATH3 by VIF */
    unsigned  IMT  : 1; /* PATH3 IMAGE mode termination status */
    unsigned  PSE  : 1; /* pause */
    unsigned  p0   : 1;
    unsigned  IP3  : 1; /* Interruputed Path3 */
    unsigned  P3Q  : 1; /* Path3 in queue */
    unsigned  P2Q  : 1; /* Path2 in queue */
    unsigned  P1Q  : 1; /* Path1 in queue */
    unsigned  OPH  : 1; /* Output Path */
    unsigned  APATH: 2; /* Data Path */
    unsigned  DIR  : 1; /* Direction */
    unsigned  p1   :11;
    unsigned  FQC  : 5; /* GIF-FIFO Valid data counter */
    unsigned  p2   : 3;
} tGIF_STAT;



/*
 * GIF_TAG0
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               |E|                             |
 * |            tag[31:16]         |O|           NLOOP             |
 * |                               |P|                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GIF_TAG0_NLOOP_M    (0x7fff<< 0)
#define GIF_TAG0_EOP_M      (0x01  <<15)
#define GIF_TAG0_tag_M      (0xffff<<16)

#define GIF_TAG0_NLOOP_O    ( 0)
#define GIF_TAG0_EOP_O      (15)
#define GIF_TAG0_tag_O      (16)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned  NLOOP:15;
    unsigned  EOP  : 1;
    unsigned  tag  :16; /* GIFtag[31:0] */
} tGIF_TAG0;



/*
 * GIF_TAG1
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       |   |                     |P|                           |
 * |  NREG |FLG|         PRIM        |R|         tag[45:32]        |
 * |       |   |                     |E|                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GIF_TAG1_tag_M      (0x3fff<< 0)
#define GIF_TAG1_PRE_M      (0x01  <<14)
#define GIF_TAG1_PRIM_M     (0x7ff <<15)
#define GIF_TAG1_FLG_M      (0x3   <<26)
#define GIF_TAG1_NREG_M     (0x0f  <<28)

#define GIF_TAG1_tag_O      ( 0)
#define GIF_TAG1_PRE_O      (14)
#define GIF_TAG1_PRIM_O     (15)
#define GIF_TAG1_FLG_O      (26)
#define GIF_TAG1_NREG_O     (28)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned  tag :14;  /* GIFtag[63:32] */
    unsigned  PRE : 1;
    unsigned  PRIM:11;
    unsigned  FLG : 2;
    unsigned  NREG: 4;
} tGIF_TAG1;



/*
 * GIF_TAG2
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                           tag[95:64]                          |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GIF_TAG2_tag_M      (0xffffffff<< 0)

#define GIF_TAG2_tag_O      ( 0)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned  tag :32;  /* GIFtag[95:64] */
} tGIF_TAG2;



/*
 * GIF_TAG3
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                          tag[127:96]                          |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GIF_TAG3_tag_M      (0xffffffff<< 0)

#define GIF_TAG3_tag_O      ( 0)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned  tag :32;  /* GIFtag[127:96] */
} tGIF_TAG3;



/*
 * GIF_CNT
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   |                   |       | |                             |
 * |   |       VUADDR      | REGCNT| |          LOOPCNT            |
 * |   |                   |       | |                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GIF_CNT_LOOPCNT_M   (0x7fff<< 0)
#define GIF_CNT_REGCNT_M    (0x0f  <<16)
#define GIF_CNT_VUADDR_M    (0x3ff <<20)

#define GIF_CNT_LOOPCNT_O   ( 0)
#define GIF_CNT_REGCNT_O    (16)
#define GIF_CNT_VUADDR_O    (20)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned LOOPCNT:15;    /* loop counter */
    unsigned p0     : 1;
    unsigned REGCNT : 4;    /* Registar number */
    unsigned VUADDR :10;    /* VU memory address */
    unsigned p1     : 2;
} tGIF_CNT;



/*
 * GIF_P3CNT
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                 |                             |
 * |                                 |            P3CNT            |
 * |                                 |                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GIF_P3CNT_P3CNT_M   (0x7fff<< 0)

#define GIF_P3CNT_P3CNT_O   ( 0)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned P3CNT:15;  /* loop counter */
    unsigned p0   :17;
} tGIF_P3CNT;



/*
 * GIF_P3TAG
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               |E|                             |
 * |                               |O|          LOOPCNT            |
 * |                               |P|                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GIF_P3TAG_LOOPCNT_M (0x7fff<< 0)
#define GIF_P3TAG_EOP_M     (0x01  <<15)

#define GIF_P3TAG_LOOPCNT_O ( 0)
#define GIF_P3TAG_EOP_O     (15)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned LOOPCNT:15;    /* the value of GIFtag LOOPCNT by PATH3 */
    unsigned EOP    : 1;    /* the value of GIFtag EOPbit by PATH3 */
    unsigned p0     :16;
} tGIF_P3TAG;



/* VIF */

/*
 * VIF0_STAT
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       |   F   |                   |E|E|I|V|V|V| |M|     |V| V |
 * |       |   Q   |                   |R|R|N|I|F|S| |R|     |E| P |
 * |       |   C   |                   |1|0|T|S|S|S| |K|     |W| S |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF0_STAT_VPS_M     (0x03<< 0)
#define VIF0_STAT_VEW_M     (0x01<< 2)
#define VIF0_STAT_MRK_M     (0x01<< 6)
#define VIF0_STAT_VSS_M     (0x01<< 8)
#define VIF0_STAT_VFS_M     (0x01<< 9)
#define VIF0_STAT_VIS_M     (0x01<<10)
#define VIF0_STAT_INT_M     (0x01<<11)
#define VIF0_STAT_ER0_M     (0x01<<12)
#define VIF0_STAT_ER1_M     (0x01<<13)
#define VIF0_STAT_FQC_M     (0x0f<<24)

#define VIF0_STAT_VPS_O     ( 0)
#define VIF0_STAT_VEW_O     ( 2)
#define VIF0_STAT_MRK_O     ( 6)
#define VIF0_STAT_VSS_O     ( 8)
#define VIF0_STAT_VFS_O     ( 9)
#define VIF0_STAT_VIS_O     (10)
#define VIF0_STAT_INT_O     (11)
#define VIF0_STAT_ER0_O     (12)
#define VIF0_STAT_ER1_O     (13)
#define VIF0_STAT_FQC_O     (24)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned VPS: 2;    /* VIF0 pipeline status */
    unsigned VEW: 1;    /* VIF0 E-bit wait */
    unsigned p0 : 3;
    unsigned MRK: 1;    /* VIF0 MARK detected flag */
    unsigned p1 : 1;
    unsigned VSS: 1;    /* VIF0 stop stall */
    unsigned VFS: 1;    /* VIF0 ForceBreak stall */
    unsigned VIS: 1;    /* VIF0 interrupt stall */
    unsigned INT: 1;    /* Interrupt bit detected flag */
    unsigned ERO: 1;    /* Mismatch Error detected flag */
    unsigned ER1: 1;    /* Reserved Instruction Error detected flag */
    unsigned p2 :10;
    unsigned FQC: 4;    /* VIF0-FIFO valid data counter */
    unsigned p3 : 4;

} tVIF0_STAT;



/*
 * VIF0_FBRST
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                       |S|S|F|R|
 * |                                                       |T|T|B|S|
 * |                                                       |C|P|K|T|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF0_FBRST_RST_M    (0x01<<0)
#define VIF0_FBRST_FBK_M    (0x01<<1)
#define VIF0_FBRST_STP_M    (0x01<<2)
#define VIF0_FBRST_STC_M    (0x01<<3)

#define VIF0_FBRST_RST_O    (0)
#define VIF0_FBRST_FBK_O    (1)
#define VIF0_FBRST_STP_O    (2)
#define VIF0_FBRST_STC_O    (3)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned RST: 1;    /* Reset */
    unsigned FBK: 1;    /* Force Break */
    unsigned STP: 1;    /* STOP */
    unsigned STC: 1;    /* Stall Cancel */
    unsigned p0 :28;
} tVIF0_FBRST;



/*
 * VIF0_ERR
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                         |M|M|M|
 * |                                                         |E|E|I|
 * |                                                         |1|0|I|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF0_ERR_MII_M      (0x01<<0)
#define VIF0_ERR_ME0_M      (0x01<<1)
#define VIF0_ERR_ME1_M      (0x01<<2)

#define VIF0_ERR_MII_O      (0)
#define VIF0_ERR_ME0_O      (1)
#define VIF0_ERR_ME1_O      (2)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned MII: 1;    /* Mask information */
    unsigned ME0: 1;    /* Mask information by Missmatch Error */
    unsigned ME1: 1;    /* Mask information by Rerved Instruction Error */
    unsigned p0 :29;
} tVIF0_ERR;



/*
 * VIF0_MARK / VIF1_MARK
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               |                               |
 * |                               |              MARK             |
 * |                               |                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_MARK_MARK_M     (0xffff<<0)

#define VIF_MARK_MARK_O     (0)



/*
 * Bitfield Structure
 */
typedef struct {
    unsigned MARK:16;   /* MARK Value */
    unsigned p0  :16;
} tVIF_MARK;



/*
 * VIF0_CYCLE / VIF1_CYCLE
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               |               |               |
 * |                               |       WL      |       CL      |
 * |                               |               |               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_CYCLE_CL_M      (0xff<<0)
#define VIF_CYCLE_WL_M      (0xff<<8)

#define VIF_CYCLE_CL_O      (0)
#define VIF_CYCLE_WL_O      (8)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned CL: 8;     /* Cycle Length */
    unsigned WL: 8;     /* Write Cycle Length */
    unsigned p0:16;
} tVIF_CYCLE;



/*
 * VIF0_MODE / VIF1_MODE
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                           |   |
 * |                                                           |MOD|
 * |                                                           |   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_MODE_MOD_M      (0x03<<0)

#define VIF_MODE_MOD_O      (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned MOD: 2;    /* Calculation mode */
    unsigned p0 :30;
} tVIF_MODE;



/*
 * VIF0_NUM
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                               |               |
 * |                                               |      num      |
 * |                                               |               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF0_NUM_num_M      (0xff<<0)

#define VIF0_NUM_num_O      (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned num: 8;    /* NUM Value */
    unsigned p0 :24;
} tVIF0_NUM;



/*
 * VIF0_MASK / VIF1_MASK
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
 * |m15|m14|m13|m12|m11|m10|m9 |m8 |m7 |m6 |m5 |m4 |m3 |m2 |m1 |m0 |
 * |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_MASK_m0_M       (0x03<< 0)
#define VIF_MASK_m1_M       (0x03<< 2)
#define VIF_MASK_m2_M       (0x03<< 4)
#define VIF_MASK_m3_M       (0x03<< 6)
#define VIF_MASK_m4_M       (0x03<< 8)
#define VIF_MASK_m5_M       (0x03<<10)
#define VIF_MASK_m6_M       (0x03<<12)
#define VIF_MASK_m7_M       (0x03<<14)
#define VIF_MASK_m8_M       (0x03<<16)
#define VIF_MASK_m9_M       (0x03<<18)
#define VIF_MASK_m10_M      (0x03<<20)
#define VIF_MASK_m11_M      (0x03<<22)
#define VIF_MASK_m12_M      (0x03<<24)
#define VIF_MASK_m13_M      (0x03<<26)
#define VIF_MASK_m14_M      (0x03<<28)
#define VIF_MASK_m15_M      (0x03<<30)

#define VIF_MASK_m0_O       ( 0)
#define VIF_MASK_m1_O       ( 2)
#define VIF_MASK_m2_O       ( 4)
#define VIF_MASK_m3_O       ( 6)
#define VIF_MASK_m4_O       ( 8)
#define VIF_MASK_m5_O       (10)
#define VIF_MASK_m6_O       (12)
#define VIF_MASK_m7_O       (14)
#define VIF_MASK_m8_O       (16)
#define VIF_MASK_m9_O       (18)
#define VIF_MASK_m10_O      (20)
#define VIF_MASK_m11_O      (22)
#define VIF_MASK_m12_O      (24)
#define VIF_MASK_m13_O      (26)
#define VIF_MASK_m14_O      (28)
#define VIF_MASK_m15_O      (30)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned m0 :2;     /* Mask pattern */
    unsigned m1 :2;     /* Mask pattern */
    unsigned m2 :2;     /* Mask pattern */
    unsigned m3 :2;     /* Mask pattern */
    unsigned m4 :2;     /* Mask pattern */
    unsigned m5 :2;     /* Mask pattern */
    unsigned m6 :2;     /* Mask pattern */
    unsigned m7 :2;     /* Mask pattern */
    unsigned m8 :2;     /* Mask pattern */
    unsigned m9 :2;     /* Mask pattern */
    unsigned m10:2;     /* Mask pattern */
    unsigned m11:2;     /* Mask pattern */
    unsigned m12:2;     /* Mask pattern */
    unsigned m13:2;     /* Mask pattern */
    unsigned m14:2;     /* Mask pattern */
    unsigned m15:2;     /* Mask pattern */
} tVIF_MASK;



/*
 * VIF0_CODE / VIF1_CODE
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |               |               |                               |
 * |      CMD      |      num      |            immediate          |
 * |               |               |                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_CODE_immediate_M    (0xffff<< 0)
#define VIF_CODE_num_M          (0xff  <<16)
#define VIF_CODE_CMD_M          (0xff  <<24)

#define VIF_CODE_immediate_O    ( 0)
#define VIF_CODE_num_O          (16)
#define VIF_CODE_CMD_O          (24)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned immediate :16; /* VIF code */
    unsigned num       : 8; /* VIF code */
    unsigned CMD       : 8; /* VIF code */
} tVIF_CODE;



/*
 * VIF0_ITOPS / VIF1_ITOPS
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                           |                   |
 * |                                           |        ITOPS      |
 * |                                           |                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_ITOPS_ITOPS_M   (0x1ff<<0)

#define VIF_ITOPS_ITOPS_O   (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned ITOPS :10; /* ITOPS Value */
    unsigned p0    :22;
} tVIF_ITOPS;



/*
 * VIF0_ITOP / VIF1_ITOP
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                           |                   |
 * |                                           |        ITOP       |
 * |                                           |                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_ITOP_ITOP_M     (0x1ff<<0)

#define VIF_ITOP_ITOP_O     (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned ITOP :10;  /* ITOP Value */
    unsigned p0   :22;
} tVIF_ITOP;



/*
 * VIF0_R0 / VIF1_R0
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                              R0                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_R0_R0_M     (0xffffffff<<0)

#define VIF_R0_R0_O     (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned R0 :32;    /* Row Value */
} tVIF_R0;



/*
 * VIF0_R1 / VIF1_R1
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                              R1                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_R1_R1_M     (0xffffffff<<0)

#define VIF_R1_R1_O     (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned R1 :32;    /* Row Value */
} tVIF_R1;



/*
 * VIF0_R2 / VIF1_R2
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                              R2                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_R2_R2_M     (0xffffffff<<0)

#define VIF_R2_R2_O     (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned R2 :32;    /* Row Value */
} tVIF_R2;



/*
 * VIF0_R3 / VIF1_R3
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                              R3                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_R3_R3_M     (0xffffffff<<0)

#define VIF_R3_R3_O     (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned R3 :32;    /* Row Value */
} tVIF_R3;



/*
 * VIF0_C0 / VIF1_C0
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                              C0                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_C0_C0_M     (0xffffffff<<0)

#define VIF_C0_C0_O     (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned C0 :32;    /* Colum Value */
} tVIF_C0;



/*
 * VIF0_C1 / VIF1_C1
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                              C1                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_C1_C1_M     (0xffffffff<<0)

#define VIF_C1_C1_O     (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned C1 :32;    /* Column Value */
} tVIF_C1;



/*
 * VIF0_C2 / VIF1_C2
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                              C2                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_C2_C2_M     (0xffffffff<<0)

#define VIF_C2_C2_O     (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned C2 :32;    /* Column Value */
} tVIF_C2;



/*
 * VIF0_C3 / VIF1_C3
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                              C3                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF_C3_C3_M     (0xffffffff<<0)

#define VIF_C3_C3_O     (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned C3 :32;    /* Column Value */
} tVIF_C3;



/*
 * VIF1_STAT
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     |     F   |F|                 |E|E|I|V|V|V|D|M|   |V|V| V |
 * |     |     Q   |D|                 |R|R|N|I|F|S|B|R|   |G|E| P |
 * |     |     C   |R|                 |1|0|T|S|S|S|F|K|   |W|W| S |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF1_STAT_VPS_M     (0x03<< 0)
#define VIF1_STAT_VEW_M     (0x01<< 2)
#define VIF1_STAT_VGW_M     (0x01<< 3)
#define VIF1_STAT_MRK_M     (0x01<< 6)
#define VIF1_STAT_DBF_M     (0x01<< 7)
#define VIF1_STAT_VSS_M     (0x01<< 8)
#define VIF1_STAT_VFS_M     (0x01<< 9)
#define VIF1_STAT_VIS_M     (0x01<<10)
#define VIF1_STAT_INT_M     (0x01<<11)
#define VIF1_STAT_ER0_M     (0x01<<12)
#define VIF1_STAT_ER1_M     (0x01<<13)
#define VIF1_STAT_FDR_M     (0x01<<23)
#define VIF1_STAT_FQC_M     (0x1f<<24)

#define VIF1_STAT_VPS_O     ( 0)
#define VIF1_STAT_VEW_O     ( 2)
#define VIF1_STAT_VGW_O     ( 3)
#define VIF1_STAT_MRK_O     ( 6)
#define VIF1_STAT_DBF_O     ( 7)
#define VIF1_STAT_VSS_O     ( 8)
#define VIF1_STAT_VFS_O     ( 9)
#define VIF1_STAT_VIS_O     (10)
#define VIF1_STAT_INT_O     (11)
#define VIF1_STAT_ER0_O     (12)
#define VIF1_STAT_ER1_O     (13)
#define VIF1_STAT_FDR_O     (23)
#define VIF1_STAT_FQC_O     (24)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned VPS: 2;    /* VIF1 pipeline status */
    unsigned VEW: 1;    /* VIF1 E-bit wait */
    unsigned VGW: 1;    /* VIF1 GIF wait */
    unsigned p0 : 2;
    unsigned MRK: 1;    /* VIF1 MARK detected flag */
    unsigned DBF: 1;    /* Duble Buffer Flag */
    unsigned VSS: 1;    /* VIF1 stop stall */
    unsigned VFS: 1;    /* VIF1 ForceBreak stall */
    unsigned VIS: 1;    /* VIF1 interrupt stall */
    unsigned INT: 1;    /* Interrupt bit detected flag */
    unsigned ERO: 1;    /* Mismatch Error detected flag */
    unsigned ER1: 1;    /* Reserved Instruction Error detected flag */
    unsigned p1 : 9;
    unsigned FDR: 1;    /* VIF1-FIFO direction */
    unsigned FQC: 5;    /* VIF1-FIFO valid data counter */
    unsigned p2 : 3;

} tVIF1_STAT;



/*
 * VIF1_FBRST
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                       |S|S|F|R|
 * |                                                       |T|T|B|S|
 * |                                                       |C|P|K|T|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF1_FBRST_RST_M    (0x01<<0)
#define VIF1_FBRST_FBK_M    (0x01<<1)
#define VIF1_FBRST_STP_M    (0x01<<2)
#define VIF1_FBRST_STC_M    (0x01<<3)

#define VIF1_FBRST_RST_O    (0)
#define VIF1_FBRST_FBK_O    (1)
#define VIF1_FBRST_STP_O    (2)
#define VIF1_FBRST_STC_O    (3)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned RST: 1;    /* Reset */
    unsigned FBK: 1;    /* Force Break */
    unsigned STP: 1;    /* Stop */
    unsigned STC: 1;    /* Stall Cancel */
    unsigned p0 :28;
} tVIF1_FBRST;


/*
 * VIF1_ERR
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                         |M|M|M|
 * |                                                         |E|E|I|
 * |                                                         |1|0|I|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF1_ERR_MII_M      (0x01<<0)
#define VIF1_ERR_ME0_M      (0x01<<1)
#define VIF1_ERR_ME1_M      (0x01<<2)

#define VIF1_ERR_MII_O      (0)
#define VIF1_ERR_ME0_O      (1)
#define VIF1_ERR_ME1_O      (2)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned MII: 1;    /* Mask information */
    unsigned ME0: 1;    /* Mask information by Missmatch Error */
    unsigned ME1: 1;    /* Mask infor by Reserved Instruction Error */
    unsigned p0 :29;
} tVIF1_ERR;



/*
 * VIF1_NUM
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                               |               |
 * |                                               |      num      |
 * |                                               |               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF1_NUM_num_M      (0xff<<0)

#define VIF1_NUM_num_O      (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned num: 8;    /* NUM Value */
    unsigned p0 :24;
} tVIF1_NUM;



/*
 * VIF1_BASE
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                           |                   |
 * |                                           |        BASE       |
 * |                                           |                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF1_BASE_BASE_M    (0x3ff<<0)

#define VIF1_BASE_BASE_O    (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned BASE:10;   /* BASE Value */
    unsigned p0  :22;
} tVIF1_BASE;



/*
 * VIF1_OFST
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                           |                   |
 * |                                           |       OFFSET      |
 * |                                           |                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF1_OFST_OFFSET_M  (0x3ff<<0)

#define VIF1_OFST_OFFSET_O  (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned OFFSET:10; /* OFFSET Value */
    unsigned p0    :22;
} tVIF1_OFST;



/*
 * VIF1_TOPS
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                           |                   |
 * |                                           |       TOPS        |
 * |                                           |                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF1_TOPS_TOPS_M    (0x3ff<<0)

#define VIF1_TOPS_TOPS_O    (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned TOPS:10;   /* TOPS Value */
    unsigned p0  :22;
} tVIF1_TOPS;



/*
 * VIF1_TOP
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                           |                   |
 * |                                           |        TOP        |
 * |                                           |                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define VIF1_TOP_TOP_M      (0x3ff<<0)

#define VIF1_TOP_TOP_O      (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned TOP:10;    /* TOP Value */
    unsigned p0 :22;
} tVIF1_TOP;


/* DMAC */

/*
 * Dn_CHCR
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               |             |S|T|T|   |   | |D|
 * |            DMA-tag            |             |T|I|T|ASP|MOD| |I|
 * |                               |             |R|E|E|   |   | |R|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_CHCR_DIR_M        (0x01  << 0)
#define D_CHCR_MOD_M        (0x03  << 2)
#define D_CHCR_ASP_M        (0x03  << 4)
#define D_CHCR_TTE_M        (0x01  << 6)
#define D_CHCR_TIE_M        (0x01  << 7)
#define D_CHCR_STR_M        (0x01  << 8)
#define D_CHCR_TAG_M        (0xffff<<16)

#define D_CHCR_DIR_O        ( 0)
#define D_CHCR_MOD_O        ( 2)
#define D_CHCR_ASP_O        ( 4)
#define D_CHCR_TTE_O        ( 6)
#define D_CHCR_TIE_O        ( 7)
#define D_CHCR_STR_O        ( 8)
#define D_CHCR_TAG_O        (16)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned DIR: 1;    /* Direction */
    unsigned p0 : 1;
    unsigned MOD: 2;    /* Mode */
    unsigned ASP: 2;    /* Address stack pointer */
    unsigned TTE: 1;    /* Tag trasfer enable */
    unsigned TIE: 1;    /* Tag interrupt enable */
    unsigned STR: 1;    /* start */
    unsigned p1 : 7;
    unsigned TAG:16;    /* DMAtag */
} tD_CHCR;



/*
 * Dn_MADR
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |S|                                                             |
 * |P|                           ADDR                              |
 * |R|                                                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_MADR_ADDR_M       (0x7fffffff<< 0)
#define D_MADR_SPR_M        (0x01      <<31)

#define D_MADR_ADDR_O       ( 0)
#define D_MADR_SPR_O        (31)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned ADDR:31;   /* Memory address */
    unsigned SPR : 1;   /* Memory/SPR Select */
} tD_MADR;



/*
 * Dn_QWC
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               |                               |
 * |                               |              QWC              |
 * |                               |                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_QWC_QWC_M     (0xffff<<0)

#define D_QWC_QWC_O     (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned QWC:16;    /* QuadWordCounter */
    unsigned p0 :16;
} tD_QWC;



/*
 * Dn_TADR
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |S|                                                             |
 * |P|                        ADDR                                 |
 * |R|                                                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_TADR_ADDR_M       (0x7fffffff<< 0)
#define D_TADR_SPR_M        (0x01      <<31)

#define D_TADR_ADDR_O       ( 0)
#define D_TADR_SPR_O        (31)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned ADDR:31;   /* Next tag memory address */
    unsigned SPR : 1;   /* Next memory/SPR Select */
} tD_TADR;



/*
 * Dn_ASR0
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |S|                                                             |
 * |P|                        ADDR                                 |
 * |R|                                                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_ASR0_ADDR_M       (0x7fffffff<< 0)
#define D_ASR0_SPR_M        (0x01      <<31)

#define D_ASR0_ADDR_O       ( 0)
#define D_ASR0_SPR_O        (31)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned ADDR:31;   /* Tag memory address */
    unsigned SPR : 1;   /* Memory/SPR Select */
} tD_ASR0;



/*
 * Dn_ASR1
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |S|                                                             |
 * |P|                        ADDR                                 |
 * |R|                                                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_ASR1_ADDR_M       (0x7fffffff<< 0)
#define D_ASR1_SPR_M        (0x01      <<31)

#define D_ASR1_ADDR_O       ( 0)
#define D_ASR1_SPR_O        (31)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned ADDR:31;   /* Tag memory address */
    unsigned SPR : 1;   /* Memory/SPR Select */
} tD_ASR1;



/*
 * Dn_SADR
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                   |                           |
 * |                                   |        ADDR               |
 * |                                   |                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_SADR_ADDR_M       (0x3fff<< 0)

#define D_SADR_ADDR_O       ( 0)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned ADDR:14;   /* SPR address */
    unsigned p0  :18;
} tD_SADR;



/*
 * D_CTRL
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                         |     |   |   |   |R|D|
 * |                                         |RCYC |STD|STS|MFD|E|M|
 * |                                         |     |   |   |   |L|A|
 * |                                         |     |   |   |   |E|E|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_CTRL_DMAE_M       (0x01<<0)
#define D_CTRL_RELE_M       (0x01<<1)
#define D_CTRL_MFD_M        (0x03<<2)
#define D_CTRL_STS_M        (0x03<<4)
#define D_CTRL_STD_M        (0x03<<6)
#define D_CTRL_RCYC_M       (0x07<<8)

#define D_CTRL_DMAE_O       (0)
#define D_CTRL_RELE_O       (1)
#define D_CTRL_MFD_O        (2)
#define D_CTRL_STS_O        (4)
#define D_CTRL_STD_O        (6)
#define D_CTRL_RCYC_O       (8)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned DMAE: 1;   /* DMA enable */
    unsigned RELE: 1;   /* release signal enable */
    unsigned MFD : 2;   /* Memory FIFO drain */
    unsigned STS : 2;   /* Stall source channel */
    unsigned STD : 2;   /* Stall drain channel */
    unsigned RCYC: 3;   /* Release Cycle */
    unsigned p0  :21;
} tD_CTRL;



/*
 * D_STAT
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | |M|S|     |C|C|C|C|C|C|C|C|C|C|B|M|S|     |C|C|C|C|C|C|C|C|C|C|
 * | |E|I|     |I|I|I|I|I|I|I|I|I|I|E|E|I|     |I|I|I|I|I|I|I|I|I|I|
 * | |I|M|     |M|M|M|M|M|M|M|M|M|M|I|I|S|     |S|S|S|S|S|S|S|S|S|S|
 * | |M| |     |9|8|7|6|5|4|3|2|1|0|S|S| |     |9|8|7|6|5|4|3|2|1|0|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_STAT_CIS0_M       (0x01<< 0)
#define D_STAT_CIS1_M       (0x01<< 1)
#define D_STAT_CIS2_M       (0x01<< 2)
#define D_STAT_CIS3_M       (0x01<< 3)
#define D_STAT_CIS4_M       (0x01<< 4)
#define D_STAT_CIS5_M       (0x01<< 5)
#define D_STAT_CIS6_M       (0x01<< 6)
#define D_STAT_CIS7_M       (0x01<< 7)
#define D_STAT_CIS8_M       (0x01<< 8)
#define D_STAT_CIS9_M       (0x01<< 9)
#define D_STAT_SIS_M        (0x01<<13)
#define D_STAT_MEIS_M       (0x01<<14)
#define D_STAT_BEIS_M       (0x01<<15)
#define D_STAT_CIM0_M       (0x01<<16)
#define D_STAT_CIM1_M       (0x01<<17)
#define D_STAT_CIM2_M       (0x01<<18)
#define D_STAT_CIM3_M       (0x01<<19)
#define D_STAT_CIM4_M       (0x01<<20)
#define D_STAT_CIM5_M       (0x01<<21)
#define D_STAT_CIM6_M       (0x01<<22)
#define D_STAT_CIM7_M       (0x01<<23)
#define D_STAT_CIM8_M       (0x01<<24)
#define D_STAT_CIM9_M       (0x01<<25)
#define D_STAT_SIM_M        (0x01<<29)
#define D_STAT_MEIM_M       (0x01<<30)

#define D_STAT_CIS0_O       ( 0)
#define D_STAT_CIS1_O       ( 1)
#define D_STAT_CIS2_O       ( 2)
#define D_STAT_CIS3_O       ( 3)
#define D_STAT_CIS4_O       ( 4)
#define D_STAT_CIS5_O       ( 5)
#define D_STAT_CIS6_O       ( 6)
#define D_STAT_CIS7_O       ( 7)
#define D_STAT_CIS8_O       ( 8)
#define D_STAT_CIS9_O       ( 9)
#define D_STAT_SIS_O        (13)
#define D_STAT_MEIS_O       (14)
#define D_STAT_BEIS_O       (15)
#define D_STAT_CIM0_O       (16)
#define D_STAT_CIM1_O       (17)
#define D_STAT_CIM2_O       (18)
#define D_STAT_CIM3_O       (19)
#define D_STAT_CIM4_O       (20)
#define D_STAT_CIM5_O       (21)
#define D_STAT_CIM6_O       (22)
#define D_STAT_CIM7_O       (23)
#define D_STAT_CIM8_O       (24)
#define D_STAT_CIM9_O       (25)
#define D_STAT_SIM_O        (29)
#define D_STAT_MEIM_O       (30)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned CIS0: 1;   /* Channel interrupt status */
    unsigned CIS1: 1;   /* Channel interrupt status */
    unsigned CIS2: 1;   /* Channel interrupt status */
    unsigned CIS3: 1;   /* Channel interrupt status */
    unsigned CIS4: 1;   /* Channel interrupt status */
    unsigned CIS5: 1;   /* Channel interrupt status */
    unsigned CIS6: 1;   /* Channel interrupt status */
    unsigned CIS7: 1;   /* Channel interrupt status */
    unsigned CIS8: 1;   /* Channel interrupt status */
    unsigned CIS9: 1;   /* Channel interrupt status */
    unsigned p0  : 3;
    unsigned SIS : 1;   /* DAM Stall interrupt status */
    unsigned MEIS: 1;   /* Memory FIFO empty interrupt status */
    unsigned BEIS: 1;   /* BUSERR interrupt status */
    unsigned CIM0: 1;   /* Channel interrupt mask */
    unsigned CIM1: 1;   /* Channel interrupt mask */
    unsigned CIM2: 1;   /* Channel interrupt mask */
    unsigned CIM3: 1;   /* Channel interrupt mask */
    unsigned CIM4: 1;   /* Channel interrupt mask */
    unsigned CIM5: 1;   /* Channel interrupt mask */
    unsigned CIM6: 1;   /* Channel interrupt mask */
    unsigned CIM7: 1;   /* Channel interrupt mask */
    unsigned CIM8: 1;   /* Channel interrupt mask */
    unsigned CIM9: 1;   /* Channel interrupt mask */
    unsigned p1  : 3;
    unsigned SIM : 1;   /* DMA Stall interrupt mask */
    unsigned MEIM: 1;   /* Memory FIFO empty interrupt mask */
    unsigned p2  : 1;
} tD_STAT;



/*
 * D_PCR
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |P|         |C|C|C|C|C|C|C|C|C|C|           |C|C|C|C|C|C|C|C|C|C|
 * |C|         |D|D|D|D|D|D|D|D|D|D|           |P|P|P|P|P|P|P|P|P|P|
 * |E|         |E|E|E|E|E|E|E|E|E|E|           |C|C|C|C|C|C|C|C|C|C|
 * | |         |9|8|7|6|5|4|3|2|1|0|           |9|8|7|6|5|4|3|2|1|0|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_PCR_CPC0_M        (0x01<< 0)
#define D_PCR_CPC1_M        (0x01<< 1)
#define D_PCR_CPC2_M        (0x01<< 2)
#define D_PCR_CPC3_M        (0x01<< 3)
#define D_PCR_CPC4_M        (0x01<< 4)
#define D_PCR_CPC5_M        (0x01<< 5)
#define D_PCR_CPC6_M        (0x01<< 6)
#define D_PCR_CPC7_M        (0x01<< 7)
#define D_PCR_CPC8_M        (0x01<< 8)
#define D_PCR_CPC9_M        (0x01<< 9)
#define D_PCR_CDE0_M        (0x01<<16)
#define D_PCR_CDE1_M        (0x01<<17)
#define D_PCR_CDE2_M        (0x01<<18)
#define D_PCR_CDE3_M        (0x01<<19)
#define D_PCR_CDE4_M        (0x01<<20)
#define D_PCR_CDE5_M        (0x01<<21)
#define D_PCR_CDE6_M        (0x01<<22)
#define D_PCR_CDE7_M        (0x01<<23)
#define D_PCR_CDE8_M        (0x01<<24)
#define D_PCR_CDE9_M        (0x01<<25)
#define D_PCR_PCE_M         (0x01<<31)

#define D_PCR_CPC0_O        ( 0)
#define D_PCR_CPC1_O        ( 1)
#define D_PCR_CPC2_O        ( 2)
#define D_PCR_CPC3_O        ( 3)
#define D_PCR_CPC4_O        ( 4)
#define D_PCR_CPC5_O        ( 5)
#define D_PCR_CPC6_O        ( 6)
#define D_PCR_CPC7_O        ( 7)
#define D_PCR_CPC8_O        ( 8)
#define D_PCR_CPC9_O        ( 9)
#define D_PCR_CDE0_O        (16)
#define D_PCR_CDE1_O        (17)
#define D_PCR_CDE2_O        (18)
#define D_PCR_CDE3_O        (19)
#define D_PCR_CDE4_O        (20)
#define D_PCR_CDE5_O        (21)
#define D_PCR_CDE6_O        (22)
#define D_PCR_CDE7_O        (23)
#define D_PCR_CDE8_O        (24)
#define D_PCR_CDE9_O        (25)
#define D_PCR_PCE_O         (31)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned CPC0: 1;   /* COP control */
    unsigned CPC1: 1;   /* COP control */
    unsigned CPC2: 1;   /* COP control */
    unsigned CPC3: 1;   /* COP control */
    unsigned CPC4: 1;   /* COP control */
    unsigned CPC5: 1;   /* COP control */
    unsigned CPC6: 1;   /* COP control */
    unsigned CPC7: 1;   /* COP control */
    unsigned CPC8: 1;   /* COP control */
    unsigned CPC9: 1;   /* COP control */
    unsigned p0  : 6;
    unsigned CDE0: 1;   /* Channel DMA enable */
    unsigned CDE1: 1;   /* Channel DMA enable */
    unsigned CDE2: 1;   /* Channel DMA enable */
    unsigned CDE3: 1;   /* Channel DMA enable */
    unsigned CDE4: 1;   /* Channel DMA enable */
    unsigned CDE5: 1;   /* Channel DMA enable */
    unsigned CDE6: 1;   /* Channel DMA enable */
    unsigned CDE7: 1;   /* Channel DMA enable */
    unsigned CDE8: 1;   /* Channel DMA enable */
    unsigned CDE9: 1;   /* Channel DMA enable */
    unsigned p1  : 5;
    unsigned PCE : 1;   /* Priority control enable */
} tD_PCR;



/*
 * D_SQWC
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |               |               |               |               |
 * |               |      TQWC     |               |      SQWC     |
 * |               |               |               |               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_SQWC_SQWC_M       (0xff<< 0)
#define D_SQWC_TQWC_M       (0xff<<16)

#define D_SQWC_SQWC_O       ( 0)
#define D_SQWC_TQWC_O       (16)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned SQWC: 8;   /* Skip quadword counter */
    unsigned p0  : 8;
    unsigned TQWC: 8;   /* Transfer quadword counter */
    unsigned p1  : 8;
} tD_SQWC;



/*
 * D_RBSR
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | |                                                             |
 * | |                          RMSK                               |
 * | |                                                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_RBSR_RMSK_M       (0x7fffffff<<0)

#define D_RBSR_RMSK_O       (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned RMSK:31;   /* Ring buffer size mask */
    unsigned p0  : 1;
} tD_RBSR;



/*
 * D_RBOR
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | |                                                             |
 * | |                          ADDR                               |
 * | |                                                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_RBOR_ADDR_M       (0x7fffffff<<0)

#define D_RBOR_ADDR_O       (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned ADDR:31;   /* Ring buffer offset address */
    unsigned p0  : 1;
} tD_RBOR;



/*
 * D_STADR
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | |                                                             |
 * | |                          ADDR                               |
 * | |                                                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_STADR_ADDR_M      (0x7fffffff<<0)

#define D_STADR_ADDR_O      (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned ADDR:31;   /* Stall address */
    unsigned p0  : 1;
} tD_STADR;



/*
 * D_ENABLER
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             |C|                               |
 * |                             |P|                               |
 * |                             |N|                               |
 * |                             |D|                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_ENABLER_CPND_M    (0x1 << 16)

#define D_ENABLER_CPND_O    (16)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned p0  :16;
    unsigned CPND: 1;
    unsigned p1  :15;
} tD_ENABLER;



/*
 * D_ENABLEW
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             |C|                               |
 * |                             |P|                               |
 * |                             |N|                               |
 * |                             |D|                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define D_ENABLEW_CPND_M    (0x1 << 16)

#define D_ENABLEW_CPND_O    (16)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned p0  :16;
    unsigned CPND: 1;
    unsigned p1  :15;
} tD_ENABLEW;



/* GS Special */

/*
 * GS_PMODE
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                               |               |S|A|M|     |E|E|
 * |                               |       ALP     |L|M|M|CRTMD|N|N|
 * |                               |               |B|O|O|     |2|1|
 * |                               |               |G|D|D|     | | |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_PMODE_EN1_M      (0x01 << 0)
#define GS_PMODE_EN2_M      (0x01 << 1)
#define GS_PMODE_CRTMD_M    (0x07 << 2)
#define GS_PMODE_MMOD_M     (0x01 << 5)
#define GS_PMODE_AMOD_M     (0x01 << 6)
#define GS_PMODE_SLBG_M     (0x01 << 7)
#define GS_PMODE_ALP_M      (0xff << 8)

#define GS_PMODE_EN1_O      ( 0)
#define GS_PMODE_EN2_O      ( 1)
#define GS_PMODE_CRTMD_O    ( 2)
#define GS_PMODE_MMOD_O     ( 5)
#define GS_PMODE_AMOD_O     ( 6)
#define GS_PMODE_SLBG_O     ( 7)
#define GS_PMODE_ALP_O      ( 8)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned EN1     : 1;   /* */
    unsigned EN2     : 1;   /* */
    unsigned CRTMD   : 3;   /* CRT mode */
    unsigned MMOD    : 1;   /* */
    unsigned AMOD    : 1;   /* */
    unsigned SLBG    : 1;   /* */
    unsigned ALP     : 8;   /* */
    unsigned p0      :16;   /* */
    unsigned int p1;    
} tGS_PMODE;



/*
 * GS_SMODE2
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                       | D |F|I|
 * |                                                       | P |F|N|
 * |                                                       | M |M|T|
 * |                                                       | S |D| |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_SMODE2_INT_M     (0x01 << 0)
#define GS_SMODE2_FFMD_M    (0x01 << 1)


#define GS_SMODE2_DPMS_M    (0x03 << 2)

#define GS_SMODE2_INT_O     ( 0)
#define GS_SMODE2_FFMD_O    ( 1)
#define GS_SMODE2_DPMS_O    ( 2)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned INT  : 1;  /* Interlace mode */
    unsigned FFMD : 1;  /* */
    unsigned DPMS : 2;  /* VESA DPMS mode */
    unsigned p0   :28;  /* */
    unsigned int p1;
} tGS_SMODE2;



/*
 * GS_DISPFB1
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   |                     |                     |
 * |                   |        DBY          |         DBX         |
 * |                   |                     |                     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       |         |           |                 |
 * |                       |   PSM   |     FBW   |      FBP        |
 * |                       |         |           |                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_DISPFB1_FBP_M    (0x1ff << 0)
#define GS_DISPFB1_FBW_M    (0x3f  << 9)
#define GS_DISPFB1_PSM_M    (0x1f  <<15)
#define GS_DISPFB1_DBX_M    (0x7ffL<<32)
#define GS_DISPFB1_DBY_M    (0x7ffL<<43)

#define GS_DISPFB1_FBP_O    ( 0)
#define GS_DISPFB1_FBW_O    ( 9)
#define GS_DISPFB1_PSM_O    (15)
#define GS_DISPFB1_DBX_O    (32)
#define GS_DISPFB1_DBY_O    (43)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned FBP: 9;    /* Base pointer */
    unsigned FBW: 6;    /* Buffer width */
    unsigned PSM: 5;    /* Pixel store mode */
    unsigned p0 :12;
    unsigned DBX:11;    /* */
    unsigned DBY:11;    /* */
    unsigned p1 :10;
} tGS_DISPFB1;



/*
 * GS_DISPLAY1
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                 |                     |                       |
 * |                 |        DH           |         DW            |
 * |                 |                     |                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     |MA |       |                     |                       |
 * |     | GV|  MAGH |        DY           |          DX           |
 * |     |   |       |                     |                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_DISPLAY1_DX_M    (0xfff << 0)
#define GS_DISPLAY1_DY_M    (0x7ff <<12)
#define GS_DISPLAY1_MAGH_M  (0x0f  <<23)
#define GS_DISPLAY1_MAGV_M  (0x03  <<27)
#define GS_DISPLAY1_DW_M    (0xfffL<<32)
#define GS_DISPLAY1_DH_M    (0x7ffL<<44)

#define GS_DISPLAY1_DX_O    ( 0)
#define GS_DISPLAY1_DY_O    (12)
#define GS_DISPLAY1_MAGH_O  (23)
#define GS_DISPLAY1_MAGV_O  (27)
#define GS_DISPLAY1_DW_O    (32)
#define GS_DISPLAY1_DH_O    (44)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned DX  :12;   /* */
    unsigned DY  :11;   /* */
    unsigned MAGH: 4;   /* */
    unsigned MAGV: 2;   /* */
    unsigned p0  : 3;
    unsigned DW  :12;   /* */
    unsigned DH  :11;   /* */
    unsigned p1  : 9;
} tGS_DISPLAY1;



/*
 * GS_DISPFB2
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   |                     |                     |
 * |                   |        DBY          |         DBX         |
 * |                   |                     |                     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       |         |           |                 |
 * |                       |   PSM   |     FBW   |      FBP        |
 * |                       |         |           |                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_DISPFB2_FBP_M    (0x1ff << 0)
#define GS_DISPFB2_FBW_M    (0x3f  << 9)
#define GS_DISPFB2_PSM_M    (0x1f  <<15)
#define GS_DISPFB2_DBX_M    (0x7ffL<<32)
#define GS_DISPFB2_DBY_M    (0x7ffL<<43)

#define GS_DISPFB2_FBP_O    ( 0)
#define GS_DISPFB2_FBW_O    ( 9)
#define GS_DISPFB2_PSM_O    (15)
#define GS_DISPFB2_DBX_O    (32)
#define GS_DISPFB2_DBY_O    (43)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned FBP: 9;    /* Base pointer */
    unsigned FBW: 6;    /* Buffer width */
    unsigned PSM: 5;    /* Pixel store mode */
    unsigned p0 :12;
    unsigned DBX:11;    /* */
    unsigned DBY:11;    /* */
    unsigned p1 :10;
} tGS_DISPFB2;



/*
 * GS_DISPLAY2
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                 |                     |                       |
 * |                 |        DH           |         DW            |
 * |                 |                     |                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     |MA |       |                     |                       |
 * |     | GV|  MAGH |        DY           |          DX           |
 * |     |   |       |                     |                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_DISPLAY2_DX_M    (0xfff << 0)
#define GS_DISPLAY2_DY_M    (0x7ff <<12)
#define GS_DISPLAY2_MAGH_M  (0x0f  <<23)
#define GS_DISPLAY2_MAGV_M  (0x03  <<27)
#define GS_DISPLAY2_DW_M    (0xfffL<<32)
#define GS_DISPLAY2_DH_M    (0x7ffL<<44)

#define GS_DISPLAY2_DX_O    ( 0)
#define GS_DISPLAY2_DY_O    (12)
#define GS_DISPLAY2_MAGH_O  (23)
#define GS_DISPLAY2_MAGV_O  (27)
#define GS_DISPLAY2_DW_O    (32)
#define GS_DISPLAY2_DH_O    (44)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned DX  :12;   /* */
    unsigned DY  :11;   /* */
    unsigned MAGH: 4;   /* */
    unsigned MAGV: 2;   /* */
    unsigned p0  : 3;
    unsigned DW  :12;   /* */
    unsigned DH  :11;   /* */
    unsigned p1  : 9;
} tGS_DISPLAY2;



/*
 * GS_EXTBUF
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   |                     |                     |
 * |                   |                     |                     |
 * |                   |         WDY         |         WDX         |
 * |                   |                     |                     |
 * |                   |                     |                     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |         | E | E |W| F |           |                           |
 * |         | M | M |F| B |           |                           |
 * |         | O | O |F| I |   EXBW    |           EXBP            |
 * |         | D | D |M| N |           |                           |
 * |         | C | A |D|   |           |                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * bit definition
 */
#define GS_EXTBUF_EXBP_M        (0x3fff<< 0)
#define GS_EXTBUF_EXBW_M        (0x003f<<14)
#define GS_EXTBUF_FBIN_M        (0x0003<<20)
#define GS_EXTBUF_WFFMD_M       (0x0001<<22)
#define GS_EXTBUF_EMODA_M       (0x0003<<23)
#define GS_EXTBUF_EMODC_M       (0x0003<<25)
#define GS_EXTBUF_WDX_M         (0x7ffL<<32)
#define GS_EXTBUF_WDY_M         (0x7ffL<<43)

#define GS_EXTBUF_EXBP_O        ( 0)
#define GS_EXTBUF_EXBW_O        (14)
#define GS_EXTBUF_FBIN_O        (20)
#define GS_EXTBUF_WFFMD_O       (22)
#define GS_EXTBUF_EMODA_O       (23)
#define GS_EXTBUF_EMODC_O       (25)
#define GS_EXTBUF_WDX_O         (32)
#define GS_EXTBUF_WDY_O         (43)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned EXBP    :14;
    unsigned EXBW    : 6;
    unsigned FBIN    : 2;
    unsigned WFFMD   : 1;
    unsigned EMODA   : 2;
    unsigned EMODC   : 2;
    unsigned p0      : 5;
    unsigned WDX     :11;
    unsigned WDY     :11;
    unsigned p1      :10;
} tGS_EXTBUF;



/*
 * GS_EXTDATA
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                 |                     |                       |
 * |                 |         WH          |          WW           |
 * |                 |                     |                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     |SM |       |                     |                       |
 * |     | PV|  SMPH |         SY          |          SX           |
 * |     |   |       |                     |                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_EXTDATA_SX_M     (0xfff << 0)
#define GS_EXTDATA_SY_M     (0x7ff <<12)
#define GS_EXTDATA_SMPH_M   (0x0f  <<23)
#define GS_EXTDATA_SMPV_M   (0x03  <<27)
#define GS_EXTDATA_WW_M     (0xfffL<<32)
#define GS_EXTDATA_WH_M     (0x7ffL<<44)

#define GS_EXTDATA_SX_O     ( 0)
#define GS_EXTDATA_SY_O     (12)
#define GS_EXTDATA_SMPH_O   (23)
#define GS_EXTDATA_SMPV_O   (27)
#define GS_EXTDATA_WW_O     (32)
#define GS_EXTDATA_WH_O     (44)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned SX  :12;   /* */
    unsigned SY  :11;   /* */
    unsigned SMPH: 4;   /* */
    unsigned SMPV: 2;   /* */
    unsigned p0  : 3;   /* */
    unsigned WW  :12;   /* */
    unsigned WH  :11;   /* */
    unsigned p1  : 9;   /* */
} tGS_EXTDATA;



/*
 * GS_EXTWRITE
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                             |W|
 * |                                                             |R|
 * |                                                             |I|
 * |                                                             |T|
 * |                                                             |E|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_EXTWRITE_WRITE_M (0x01<<0)

#define GS_EXTWRITE_WRITE_O (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned WRITE: 1;  /* */
    unsigned p0   :31;
    unsigned int p1;
} tGS_EXTWRITE;



/*
 * GS_BGCOLOR
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |               |               |               |               |
 * |               |       B       |       G       |       R       |
 * |               |               |               |               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_BGCOLOR_R_M      (0xff<< 0)
#define GS_BGCOLOR_G_M      (0xff<< 8)
#define GS_BGCOLOR_B_M      (0xff<<16)

#define GS_BGCOLOR_R_O      ( 0)
#define GS_BGCOLOR_G_O      ( 8)
#define GS_BGCOLOR_B_O      (16)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned R : 8;     /* Background color Bulue */
    unsigned G : 8;     /* Background color Green */
    unsigned B : 8;     /* Background color Red */
    unsigned p0: 8;
    unsigned int p1;
} tGS_BGCOLOR;



/*
 * GS_CSR
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |               |               | F |F|N|   |R|F| | | |E|V|H|F|S|
 * |               |               | I |I|F|   |E|L| | | |D|S|S|I|I|
 * |     ID        |     REV       | F |E|I|   |S|U| |0|0|W|I|I|N|G|
 * |               |               | O |L|E|   |E|S| | | |I|N|N|I|N|
 * |               |               |   |D|L|   |T|H| | | |N|T|T|S|A|
 * |               |               |   | |D|   | | | | | |T| | |H|L|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_CSR_SIGNAL_M     (0x01<< 0)
#define GS_CSR_FINISH_M     (0x01<< 1)
#define GS_CSR_HSINT_M      (0x01<< 2)
#define GS_CSR_VSINT_M      (0x01<< 3)
#define GS_CSR_EDWINT_M     (0x01<< 4)
#define GS_CSR_FLUSH_M      (0x01<< 8)
#define GS_CSR_RESET_M      (0x01<< 9)
#define GS_CSR_NFIELD_M     (0x01<<12)
#define GS_CSR_FIELD_M      (0x01<<13)
#define GS_CSR_FIFO_M       (0x03<<14)
#define GS_CSR_REV_M        (0xff<<16)
#define GS_CSR_ID_M         (0xff<<24)

#define GS_CSR_SIGNAL_O     ( 0)
#define GS_CSR_FINISH_O     ( 1)
#define GS_CSR_HSINT_O      ( 2)
#define GS_CSR_VSINT_O      ( 3)
#define GS_CSR_EDWINT_O     ( 4)
#define GS_CSR_FLUSH_O      ( 8)
#define GS_CSR_RESET_O      ( 9)
#define GS_CSR_NFIELD_O     (12)
#define GS_CSR_FIELD_O      (13)
#define GS_CSR_FIFO_O       (14)
#define GS_CSR_REV_O        (16)
#define GS_CSR_ID_O         (24)

/*
 * Bitfield Structure
 */
typedef struct {
    unsigned SIGNAL : 1;    /* SIGNAL event */
    unsigned FINISH : 1;    /* FINISH event */
    unsigned HSINT  : 1;    /* HSync interrupt */
    unsigned VSINT  : 1;    /* VSync interrupt */
    unsigned EDWINT : 1;    /* */
    unsigned p0     : 3;
    unsigned FLUSH  : 1;    /* */
    unsigned RESET  : 1;    /* GS system reset */
    unsigned p1     : 2;
    unsigned NFIELD : 1;    /* NFIELD output */
    unsigned FIELD  : 1;    /* */
    unsigned FIFO   : 2;    /* Host interface FIFO status */
    unsigned REV    : 8;    /* GS revision number */
    unsigned ID     : 8;    /* GS ID */
    unsigned int p2;
} tGS_CSR;



/*
 * GS_IMR
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                 | | |E|V|H|F|S|               |
 * |                                 | | |D|S|S|I|I|               |
 * |                                 | | |W|M|M|N|G|               |
 * |                                 |*|*|M|S|S|I|M|               |
 * |                                 | | |S|K|K|S|S|               |
 * |                                 | | |K| | |H|K|               |
 * |                                 | | | | | |M| |               |
 * |                                 | | | | | |S| |               |
 * |                                 | | | | | |K| |               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_IMR_SIGMSK_M     (0x01<< 8)
#define GS_IMR_FINISHMSK_M  (0x01<< 9)
#define GS_IMR_HSMSK_M      (0x01<<10)
#define GS_IMR_VSMSK_M      (0x01<<11)
#define GS_IMR_EDWMSK_M     (0x01<<12)

#define GS_IMR_SIGMSK_O     ( 8)
#define GS_IMR_FINISHMSK_O  ( 9)
#define GS_IMR_HSMSK_O      (10)
#define GS_IMR_VSMSK_O      (11)
#define GS_IMR_EDWMSK_O     (12)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned p0       : 8;
    unsigned SIGMSK   : 1;  /* SIGNAL event interrupt mask */
    unsigned FINISHMSK: 1;  /* FINISH event interrupt mask */
    unsigned HSMSK    : 1;  /* HSync interrupt mask */
    unsigned VSMSK    : 1;  /* VSync interrupt mask */
    unsigned EDWMSK   : 1;  /* */
    unsigned p1       :19;
    unsigned int p2;
} tGS_IMR;



/*
 * GS_BUSDIR
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                             |D|
 * |                                                             |I|
 * |                                                             |R|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_BUSDIR_DIR_M     (0x01<<0)

#define GS_BUSDIR_DIR_O     (0)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned DIR: 1;    /* */
    unsigned p0 :31;
    unsigned int p1;
} tGS_BUSDIR;



/*
 * GS_SIGLBLID
 *  63            56              48              40              32
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                              LBLID                            |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  31            24              16               8               0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                              SIGID                            |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * bit definition
 */
#define GS_SIGLBLID_SIGID_M (0xffffffff << 0)
#define GS_SIGLBLID_LBLID_M (0xffffffffL<<32)

#define GS_SIGLBLID_SIGID_O ( 0)
#define GS_SIGLBLID_LBLID_O (32)


/*
 * Bitfield Structure
 */
typedef struct {
    unsigned SIGID:32;
    unsigned LBLID:32;
} tGS_SIGLBLID;




/* TIMER */
#define T0_COUNT        ((volatile u_int *)(0x10000000))
#define T0_MODE         ((volatile u_int *)(0x10000010))
#define T0_COMP         ((volatile u_int *)(0x10000020))
#define T0_HOLD         ((volatile u_int *)(0x10000030))
#define T1_COUNT        ((volatile u_int *)(0x10000800))
#define T1_MODE         ((volatile u_int *)(0x10000810))
#define T1_COMP         ((volatile u_int *)(0x10000820))
#define T1_HOLD         ((volatile u_int *)(0x10000830))
#define T2_COUNT        ((volatile u_int *)(0x10001000))
#define T2_MODE         ((volatile u_int *)(0x10001010))
#define T2_COMP         ((volatile u_int *)(0x10001020))
#define T3_COUNT        ((volatile u_int *)(0x10001800))
#define T3_MODE         ((volatile u_int *)(0x10001810))
#define T3_COMP         ((volatile u_int *)(0x10001820))

/* IPU */
#define IPU_CMD         ((volatile u_int *)(0x10002000))
#define IPU_CTRL        ((volatile u_int *)(0x10002010))
#define IPU_BP          ((volatile u_int *)(0x10002020))
#define IPU_TOP         ((volatile u_int *)(0x10002030))

/* GIF */
#define GIF_CTRL        ((volatile u_int *)(0x10003000))
#define GIF_MODE        ((volatile u_int *)(0x10003010))
#define GIF_STAT        ((volatile u_int *)(0x10003020))
#define GIF_TAG0        ((volatile u_int *)(0x10003040))
#define GIF_TAG1        ((volatile u_int *)(0x10003050))
#define GIF_TAG2        ((volatile u_int *)(0x10003060))
#define GIF_TAG3        ((volatile u_int *)(0x10003070))
#define GIF_CNT         ((volatile u_int *)(0x10003080))
#define GIF_P3CNT       ((volatile u_int *)(0x10003090))
#define GIF_P3TAG       ((volatile u_int *)(0x100030a0))

/* VIF0 */
#define VIF0_STAT       ((volatile u_int *)(0x10003800))
#define VIF0_FBRST      ((volatile u_int *)(0x10003810))
#define VIF0_ERR        ((volatile u_int *)(0x10003820))
#define VIF0_MARK       ((volatile u_int *)(0x10003830))
#define VIF0_CYCLE      ((volatile u_int *)(0x10003840))
#define VIF0_MODE       ((volatile u_int *)(0x10003850))
#define VIF0_NUM        ((volatile u_int *)(0x10003860))
#define VIF0_MASK       ((volatile u_int *)(0x10003870))
#define VIF0_CODE       ((volatile u_int *)(0x10003880))
#define VIF0_ITOPS      ((volatile u_int *)(0x10003890))
#define VIF0_ITOP       ((volatile u_int *)(0x100038d0))
#define VIF0_R0         ((volatile u_int *)(0x10003900))
#define VIF0_R1         ((volatile u_int *)(0x10003910))
#define VIF0_R2         ((volatile u_int *)(0x10003920))
#define VIF0_R3         ((volatile u_int *)(0x10003930))
#define VIF0_C0         ((volatile u_int *)(0x10003940))
#define VIF0_C1         ((volatile u_int *)(0x10003950))
#define VIF0_C2         ((volatile u_int *)(0x10003960))
#define VIF0_C3         ((volatile u_int *)(0x10003970))

/* VIF1 */
#define VIF1_STAT       ((volatile u_int *)(0x10003c00))
#define VIF1_FBRST      ((volatile u_int *)(0x10003c10))
#define VIF1_ERR        ((volatile u_int *)(0x10003c20))
#define VIF1_MARK       ((volatile u_int *)(0x10003c30))
#define VIF1_CYCLE      ((volatile u_int *)(0x10003c40))
#define VIF1_MODE       ((volatile u_int *)(0x10003c50))
#define VIF1_NUM        ((volatile u_int *)(0x10003c60))
#define VIF1_MASK       ((volatile u_int *)(0x10003c70))
#define VIF1_CODE       ((volatile u_int *)(0x10003c80))
#define VIF1_ITOPS      ((volatile u_int *)(0x10003c90))
#define VIF1_BASE       ((volatile u_int *)(0x10003ca0))
#define VIF1_OFST       ((volatile u_int *)(0x10003cb0))
#define VIF1_TOPS       ((volatile u_int *)(0x10003cc0))
#define VIF1_ITOP       ((volatile u_int *)(0x10003cd0))
#define VIF1_TOP        ((volatile u_int *)(0x10003ce0))

#define VIF1_R0         ((volatile u_int *)(0x10003d00))
#define VIF1_R1         ((volatile u_int *)(0x10003d10))
#define VIF1_R2         ((volatile u_int *)(0x10003d20))
#define VIF1_R3         ((volatile u_int *)(0x10003d30))
#define VIF1_C0         ((volatile u_int *)(0x10003d40))
#define VIF1_C1         ((volatile u_int *)(0x10003d50))
#define VIF1_C2         ((volatile u_int *)(0x10003d60))
#define VIF1_C3         ((volatile u_int *)(0x10003d70))

/* FIFO */
#define VIF0_FIFO       ((volatile u_long128 *)(0x10004000))
#define VIF1_FIFO       ((volatile u_long128 *)(0x10005000))
#define GIF_FIFO        ((volatile u_long128 *)(0x10006000))
#define IPU_out_FIFO    ((volatile u_long128 *)(0x10007000))
#define IPU_in_FIFO     ((volatile u_long128 *)(0x10007010))

/* DMAC */
#define D0_CHCR         ((volatile u_int *)(0x10008000))
#define D0_MADR         ((volatile u_int *)(0x10008010))
#define D0_QWC          ((volatile u_int *)(0x10008020))
#define D0_TADR         ((volatile u_int *)(0x10008030))
#define D0_ASR0         ((volatile u_int *)(0x10008040))
#define D0_ASR1         ((volatile u_int *)(0x10008050))

#define D1_CHCR         ((volatile u_int *)(0x10009000))
#define D1_MADR         ((volatile u_int *)(0x10009010))
#define D1_QWC          ((volatile u_int *)(0x10009020))
#define D1_TADR         ((volatile u_int *)(0x10009030))
#define D1_ASR0         ((volatile u_int *)(0x10009040))
#define D1_ASR1         ((volatile u_int *)(0x10009050))

#define D2_CHCR         ((volatile u_int *)(0x1000a000))
#define D2_MADR         ((volatile u_int *)(0x1000a010))
#define D2_QWC          ((volatile u_int *)(0x1000a020))
#define D2_TADR         ((volatile u_int *)(0x1000a030))
#define D2_ASR0         ((volatile u_int *)(0x1000a040))
#define D2_ASR1         ((volatile u_int *)(0x1000a050))

#define D3_CHCR         ((volatile u_int *)(0x1000b000))
#define D3_MADR         ((volatile u_int *)(0x1000b010))
#define D3_QWC          ((volatile u_int *)(0x1000b020))

#define D4_CHCR         ((volatile u_int *)(0x1000b400))
#define D4_MADR         ((volatile u_int *)(0x1000b410))
#define D4_QWC          ((volatile u_int *)(0x1000b420))
#define D4_TADR         ((volatile u_int *)(0x1000b430))

#define D5_CHCR         ((volatile u_int *)(0x1000c000))
#define D5_MADR         ((volatile u_int *)(0x1000c010))
#define D5_QWC          ((volatile u_int *)(0x1000c020))

#define D6_CHCR         ((volatile u_int *)(0x1000c400))
#define D6_MADR         ((volatile u_int *)(0x1000c410))
#define D6_QWC          ((volatile u_int *)(0x1000c420))
#define D6_TADR         ((volatile u_int *)(0x1000c430))

#define D7_CHCR         ((volatile u_int *)(0x1000c800))
#define D7_MADR         ((volatile u_int *)(0x1000c810))
#define D7_QWC          ((volatile u_int *)(0x1000c820))

#define D8_CHCR         ((volatile u_int *)(0x1000d000))
#define D8_MADR         ((volatile u_int *)(0x1000d010))
#define D8_QWC          ((volatile u_int *)(0x1000d020))
#define D8_SADR         ((volatile u_int *)(0x1000d080))

#define D9_CHCR         ((volatile u_int *)(0x1000d400))
#define D9_MADR         ((volatile u_int *)(0x1000d410))
#define D9_QWC          ((volatile u_int *)(0x1000d420))
#define D9_TADR         ((volatile u_int *)(0x1000d430))
#define D9_SADR         ((volatile u_int *)(0x1000d480))

#define D_CTRL          ((volatile u_int *)(0x1000e000))
#define D_STAT          ((volatile u_int *)(0x1000e010))
#define D_PCR           ((volatile u_int *)(0x1000e020))
#define D_SQWC          ((volatile u_int *)(0x1000e030))
#define D_RBSR          ((volatile u_int *)(0x1000e040))
#define D_RBOR          ((volatile u_int *)(0x1000e050))
#define D_STADR         ((volatile u_int *)(0x1000e060))
#define D_ENABLER       ((volatile u_int *)(0x1000f520))
#define D_ENABLEW       ((volatile u_int *)(0x1000f590))


/* VU0 */
#define VU0_MICRO       ((volatile u_long *)(0x11000000))
#define VU0_MEM         ((volatile u_long128 *)(0x11004000))

/* VU1 */
#define VU1_MICRO       ((volatile u_long *)(0x11008000))
#define VU1_MEM         ((volatile u_long128 *)(0x1100c000))

/* GS Special */
#define GS_PMODE        ((volatile u_long *)(0x12000000))
#define GS_SMODE2       ((volatile u_long *)(0x12000020))
#define GS_DISPFB1      ((volatile u_long *)(0x12000070))
#define GS_DISPLAY1     ((volatile u_long *)(0x12000080))
#define GS_DISPFB2      ((volatile u_long *)(0x12000090))
#define GS_DISPLAY2     ((volatile u_long *)(0x120000a0))
#define GS_EXTBUF       ((volatile u_long *)(0x120000b0))
#define GS_EXTDATA      ((volatile u_long *)(0x120000c0))
#define GS_EXTWRITE     ((volatile u_long *)(0x120000d0))
#define GS_BGCOLOR      ((volatile u_long *)(0x120000e0))
#define GS_CSR          ((volatile u_long *)(0x12001000))
#define GS_IMR          ((volatile u_long *)(0x12001010))
#define GS_BUSDIR       ((volatile u_long *)(0x12001040))
#define GS_SIGLBLID     ((volatile u_long *)(0x12001080))

/*
 * register access macro
 */

/* TIMER */
#define DGET_T0_MODE()          (*T0_MODE)
#define DPUT_T0_MODE(x)         (*T0_MODE = (x))
#define DGET_T0_COUNT()         (*T0_COUNT)
#define DPUT_T0_COUNT(x)        (*T0_COUNT = (x))
#define DGET_T0_COMP()          (*T0_COMP)
#define DPUT_T0_COMP(x)         (*T0_COMP = (x))
#define DGET_T0_HOLD()          (*T0_HOLD)
#define DPUT_T0_HOLD(x)         (*T0_HOLD = (x))
#define DGET_T1_MODE()          (*T1_MODE)
#define DPUT_T1_MODE(x)         (*T1_MODE = (x))
#define DGET_T1_COUNT()         (*T1_COUNT)
#define DPUT_T1_COUNT(x)        (*T1_COUNT = (x))
#define DGET_T1_COMP()          (*T1_COMP)
#define DPUT_T1_COMP(x)         (*T1_COMP = (x))
#define DGET_T1_HOLD()          (*T1_HOLD)
#define DPUT_T1_HOLD(x)         (*T1_HOLD = (x))
#define DGET_T2_MODE()          (*T2_MODE)
#define DPUT_T2_MODE(x)         (*T2_MODE = (x))
#define DGET_T2_COUNT()         (*T2_COUNT)
#define DPUT_T2_COUNT(x)        (*T2_COUNT = (x))
#define DGET_T2_COMP()          (*T2_COMP)
#define DPUT_T2_COMP(x)         (*T2_COMP = (x))
#define DGET_T3_MODE()          (*T3_MODE)
#define DPUT_T3_MODE(x)         (*T3_MODE = (x))
#define DGET_T3_COUNT()         (*T3_COUNT)
#define DPUT_T3_COUNT(x)        (*T3_COUNT = (x))
#define DGET_T3_COMP()          (*T3_COMP)
#define DPUT_T3_COMP(x)         (*T3_COMP = (x))

/* IPU */
#define DGET_IPU_CMD()          (*IPU_CMD)
#define DGET_IPU_CTRL()         (*IPU_CTRL)
#define DGET_IPU_BP()           (*IPU_BP)
#define DPUT_IPU_CMD(x)         (*IPU_CMD = (x))
#define DPUT_IPU_CTRL(x)        (*IPU_CTRL = (x))
#define DGET_IPU_out_FIFO()     (*IPU_out_FIFO)
#define DPUT_IPU_in_FIFO(x)     (*IPU_in_FIFO = (x))

/* GIF */
#define DPUT_GIF_CTRL(x)        (*GIF_CTRL = (x))
#define DPUT_GIF_MODE(x)        (*GIF_MODE = (x))
#define DPUT_GIF_FIFO(x)        (*GIF_FIFO = (x))
#define DGET_GIF_STAT()         (*GIF_STAT)
#define DGET_GIF_TAG0()         (*GIF_TAG0)
#define DGET_GIF_TAG1()         (*GIF_TAG1)
#define DGET_GIF_TAG2()         (*GIF_TAG2)
#define DGET_GIF_TAG3()         (*GIF_TAG3)
#define DGET_GIF_CNT()          (*GIF_CNT)
#define DGET_GIF_P3CNT()        (*GIF_P3CNT)
#define DGET_GIF_P3TAG()        (*GIF_P3TAG)

/* VIF0 */
#define DGET_VIF0_R0()          (*VIF0_R0)
#define DGET_VIF0_R1()          (*VIF0_R1)
#define DGET_VIF0_R2()          (*VIF0_R2)
#define DGET_VIF0_R3()          (*VIF0_R3)
#define DGET_VIF0_C0()          (*VIF0_C0)
#define DGET_VIF0_C1()          (*VIF0_C1)
#define DGET_VIF0_C2()          (*VIF0_C2)
#define DGET_VIF0_C3()          (*VIF0_C3)
#define DGET_VIF0_CYCLE()       (*VIF0_CYCLE)
#define DGET_VIF0_MASK()        (*VIF0_MASK)
#define DGET_VIF0_MODE()        (*VIF0_MODE)
#define DGET_VIF0_ITOP()        (*VIF0_ITOP)
#define DGET_VIF0_ITOPS()       (*VIF0_ITOPS)
#define DGET_VIF0_MARK()        (*VIF0_MARK)
#define DGET_VIF0_NUM()         (*VIF0_NUM)
#define DGET_VIF0_CODE()        (*VIF0_CODE)
#define DGET_VIF0_STAT()        (*VIF0_STAT)
#define DGET_VIF0_ERR()         (*VIF0_ERR)
#define DPUT_VIF0_FBRST(x)      (*VIF0_FBRST = (x))
#define DPUT_VIF0_MARK(x)       (*VIF0_MARK = (x))
#define DPUT_VIF0_ERR(x)        (*VIF0_ERR = (x))
#define DPUT_VIF0_FIFO(x)       (*VIF0_FIFO = (x))

/* VIF1 */
#define DGET_VIF1_R0()          (*VIF1_R0)
#define DGET_VIF1_R1()          (*VIF1_R1)
#define DGET_VIF1_R2()          (*VIF1_R2)
#define DGET_VIF1_R3()          (*VIF1_R3)
#define DGET_VIF1_C0()          (*VIF1_C0)
#define DGET_VIF1_C1()          (*VIF1_C1)
#define DGET_VIF1_C2()          (*VIF1_C2)
#define DGET_VIF1_C3()          (*VIF1_C3)
#define DGET_VIF1_CYCLE()       (*VIF1_CYCLE)
#define DGET_VIF1_MASK()        (*VIF1_MASK)
#define DGET_VIF1_MODE()        (*VIF1_MODE)
#define DGET_VIF1_ITOP()        (*VIF1_ITOP)
#define DGET_VIF1_ITOPS()       (*VIF1_ITOPS)
#define DGET_VIF1_BASE()        (*VIF1_BASE)
#define DGET_VIF1_OFST()        (*VIF1_OFST)
#define DGET_VIF1_TOP()         (*VIF1_TOP)
#define DGET_VIF1_TOPS()        (*VIF1_TOPS)
#define DGET_VIF1_MARK()        (*VIF1_MARK)
#define DGET_VIF1_NUM()         (*VIF1_NUM)
#define DGET_VIF1_CODE()        (*VIF1_CODE)
#define DGET_VIF1_STAT()        (*VIF1_STAT)
#define DGET_VIF1_ERR()         (*VIF1_ERR)
#define DGET_VIF1_FIFO()        (*VIF1_FIFO)
#define DPUT_VIF1_FBRST(x)      (*VIF1_FBRST = (x))
#define DPUT_VIF1_MARK(x)       (*VIF1_MARK = (x))
#define DPUT_VIF1_ERR(x)        (*VIF1_ERR = (x))
#define DPUT_VIF1_FIFO(x)       (*VIF1_FIFO = (x))

/* DMAC */
#define DGET_D_CTRL()           (*D_CTRL)
#define DGET_D_STAT()           (*D_STAT)
#define DGET_D_PCR()            (*D_PCR)
#define DGET_D_SQWC()           (*D_SQWC)
#define DGET_D_RBOR()           (*D_RBOR)
#define DGET_D_RBSR()           (*D_RBSR)
#define DGET_D_STADR()          (*D_STADR)
#define DGET_D_ENABLER()        (*D_ENABLER)
#define DGET_D_ENABLEW()        (*D_ENABLEW)

#define DPUT_D_CTRL(x)          (*D_CTRL = (x))
#define DPUT_D_STAT(x)          (*D_STAT = (x))
#define DPUT_D_PCR(x)           (*D_PCR = (x))
#define DPUT_D_SQWC(x)          (*D_SQWC = (x))
#define DPUT_D_RBOR(x)          (*D_RBOR = (x))
#define DPUT_D_RBSR(x)          (*D_RBSR = (x))
#define DPUT_D_STADR(x)         (*D_STADR = (x))
#define DPUT_D_ENABLER(x)       (*D_ENABLER = (x))
#define DPUT_D_ENABLEW(x)       (*D_ENABLEW = (x))

#define DGET_D0_CHCR()          (*D0_CHCR)
#define DGET_D0_MADR()          (*D0_MADR)
#define DGET_D0_QWC()           (*D0_QWC)
#define DGET_D0_TADR()          (*D0_TADR)
#define DGET_D0_ASR0()          (*D0_ASR0)
#define DGET_D0_ASR1()          (*D0_ASR1)
#define DGET_D1_CHCR()          (*D1_CHCR)
#define DGET_D1_MADR()          (*D1_MADR)
#define DGET_D1_QWC()           (*D1_QWC)
#define DGET_D1_TADR()          (*D1_TADR)
#define DGET_D1_ASR0()          (*D1_ASR0)
#define DGET_D1_ASR1()          (*D1_ASR1)
#define DGET_D2_CHCR()          (*D2_CHCR)
#define DGET_D2_MADR()          (*D2_MADR)
#define DGET_D2_QWC()           (*D2_QWC)
#define DGET_D2_TADR()          (*D2_TADR)
#define DGET_D2_ASR0()          (*D2_ASR0)
#define DGET_D2_ASR1()          (*D2_ASR1)
#define DGET_D3_CHCR()          (*D3_CHCR)
#define DGET_D3_MADR()          (*D3_MADR)
#define DGET_D3_QWC()           (*D3_QWC)
#define DGET_D4_CHCR()          (*D4_CHCR)
#define DGET_D4_MADR()          (*D4_MADR)
#define DGET_D4_QWC()           (*D4_QWC)
#define DGET_D4_TADR()          (*D4_TADR)
#define DGET_D5_CHCR()          (*D5_CHCR)
#define DGET_D5_MADR()          (*D5_MADR)
#define DGET_D5_QWC()           (*D5_QWC)
#define DGET_D6_CHCR()          (*D6_CHCR)
#define DGET_D6_MADR()          (*D6_MADR)
#define DGET_D6_QWC()           (*D6_QWC)
#define DGET_D6_TADR()          (*D6_TADR)
#define DGET_D7_CHCR()          (*D7_CHCR)
#define DGET_D7_MADR()          (*D7_MADR)
#define DGET_D7_QWC()           (*D7_QWC)
#define DGET_D8_CHCR()          (*D8_CHCR)
#define DGET_D8_MADR()          (*D8_MADR)
#define DGET_D8_QWC()           (*D8_QWC)
#define DGET_D8_SADR()          (*D8_SADR)
#define DGET_D9_CHCR()          (*D9_CHCR)
#define DGET_D9_MADR()          (*D9_MADR)
#define DGET_D9_QWC()           (*D9_QWC)
#define DGET_D9_TADR()          (*D9_TADR)
#define DGET_D9_SADR()          (*D9_SADR)

#define DPUT_D0_CHCR(x)         (*D0_CHCR = (x))
#define DPUT_D0_MADR(x)         (*D0_MADR = (x))
#define DPUT_D0_QWC(x)          (*D0_QWC = (x))
#define DPUT_D0_TADR(x)         (*D0_TADR = (x))
#define DPUT_D0_ASR0(x)         (*D0_ASR0 = (x))
#define DPUT_D0_ASR1(x)         (*D0_ASR1 = (x))
#define DPUT_D1_CHCR(x)         (*D1_CHCR = (x))
#define DPUT_D1_MADR(x)         (*D1_MADR = (x))
#define DPUT_D1_QWC(x)          (*D1_QWC = (x))
#define DPUT_D1_TADR(x)         (*D1_TADR = (x))
#define DPUT_D1_ASR0(x)         (*D1_ASR0 = (x))
#define DPUT_D1_ASR1(x)         (*D1_ASR1 = (x))
#define DPUT_D2_CHCR(x)         (*D2_CHCR = (x))
#define DPUT_D2_MADR(x)         (*D2_MADR = (x))
#define DPUT_D2_QWC(x)          (*D2_QWC = (x))
#define DPUT_D2_TADR(x)         (*D2_TADR = (x))
#define DPUT_D2_ASR0(x)         (*D2_ASR0 = (x))
#define DPUT_D2_ASR1(x)         (*D2_ASR1 = (x))
#define DPUT_D3_CHCR(x)         (*D3_CHCR = (x))
#define DPUT_D3_MADR(x)         (*D3_MADR = (x))
#define DPUT_D3_QWC(x)          (*D3_QWC = (x))
#define DPUT_D4_CHCR(x)         (*D4_CHCR = (x))
#define DPUT_D4_MADR(x)         (*D4_MADR = (x))
#define DPUT_D4_QWC(x)          (*D4_QWC = (x))
#define DPUT_D4_TADR(x)         (*D4_TADR = (x))
#define DPUT_D5_CHCR(x)         (*D5_CHCR = (x))
#define DPUT_D5_MADR(x)         (*D5_MADR = (x))
#define DPUT_D5_QWC(x)          (*D5_QWC = (x))
#define DPUT_D6_CHCR(x)         (*D6_CHCR = (x))
#define DPUT_D6_MADR(x)         (*D6_MADR = (x))
#define DPUT_D6_QWC(x)          (*D6_QWC = (x))
#define DPUT_D6_TADR(x)         (*D6_TADR = (x))
#define DPUT_D7_CHCR(x)         (*D7_CHCR = (x))
#define DPUT_D7_MADR(x)         (*D7_MADR = (x))
#define DPUT_D7_QWC(x)          (*D7_QWC = (x))
#define DPUT_D8_CHCR(x)         (*D8_CHCR = (x))
#define DPUT_D8_MADR(x)         (*D8_MADR = (x))
#define DPUT_D8_QWC(x)          (*D8_QWC = (x))
#define DPUT_D8_SADR(x)         (*D8_SADR = (x))
#define DPUT_D9_CHCR(x)         (*D9_CHCR = (x))
#define DPUT_D9_MADR(x)         (*D9_MADR = (x))
#define DPUT_D9_QWC(x)          (*D9_QWC = (x))
#define DPUT_D9_TADR(x)         (*D9_TADR = (x))
#define DPUT_D9_SADR(x)         (*D9_SADR = (x))


/* GS Special */
#define DGET_GS_CSR()           (*GS_CSR)
#define DGET_GS_SIGLBLID()      (*GS_SIGLBLID)
#define DPUT_GS_PMODE(x)        (*GS_PMODE = (x))
#define DPUT_GS_SMODE2(x)       (*GS_SMODE2 = (x))
#define DPUT_GS_DISPFB1(x)      (*GS_DISPFB1 = (x))
#define DPUT_GS_DISPLAY1(x)     (*GS_DISPLAY1 = (x))
#define DPUT_GS_DISPFB2(x)      (*GS_DISPFB2 = (x))
#define DPUT_GS_DISPLAY2(x)     (*GS_DISPLAY2 = (x))
#define DPUT_GS_EXTBUF(x)       (*GS_EXTBUF = (x))
#define DPUT_GS_EXTDATA(x)      (*GS_EXTDATA = (x))
#define DPUT_GS_EXTWRITE(x)     (*GS_EXTWRITE = (x))
#define DPUT_GS_BGCOLOR(x)      (*GS_BGCOLOR = (x))
#define DPUT_GS_CSR(x)          (*GS_CSR = (x))
#define DPUT_GS_IMR(x)          (*GS_IMR = (x))
#define DPUT_GS_BUSDIR(x)       (*GS_BUSDIR = (x))


#endif /* _EEREGS_H_ */

