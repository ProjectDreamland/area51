/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/*
 * Emotion Engine / I/O Processor Common Header
 *
 * Copyright (C) 2000, 2001 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * sdmacro.h
 */

#ifndef _SDMACRO_H_
#define _SDMACRO_H_

/* return value */
#define SCESD_OK			0
#define SCESD_ERROR			(-1)

#define SCESD_EINVALID_ARGUMENT		(-100)
#define SCESD_EINVALID_STATUS		(-201)
#define SCESD_EILLEGAL_CONTEXT		(-202)
#define SCESD_EBUSY			(-210)

#define SCESD_ENO_RESOURCES		(-301)

#define SCESD_TRANSFER_FINISHED		1
#define SCESD_TRANSFER_CONTINUE		0

/* コア設定 */
#define SD_CORE_0	0
#define SD_CORE_1	1
#define SD_CORE_MAX	2

/* ボイス設定 */
#define SD_MAX_VOICE   24
#define SD_VOICE_MAX   24

#define SD_VOICE_00    ( 0<<1)
#define SD_VOICE_01    ( 1<<1)
#define SD_VOICE_02    ( 2<<1)
#define SD_VOICE_03    ( 3<<1)
#define SD_VOICE_04    ( 4<<1)
#define SD_VOICE_05    ( 5<<1)
#define SD_VOICE_06    ( 6<<1)
#define SD_VOICE_07    ( 7<<1)
#define SD_VOICE_08    ( 8<<1)
#define SD_VOICE_09    ( 9<<1)
#define SD_VOICE_10    (10<<1)
#define SD_VOICE_11    (11<<1)
#define SD_VOICE_12    (12<<1)
#define SD_VOICE_13    (13<<1)
#define SD_VOICE_14    (14<<1)
#define SD_VOICE_15    (15<<1)
#define SD_VOICE_16    (16<<1)
#define SD_VOICE_17    (17<<1)
#define SD_VOICE_18    (18<<1)
#define SD_VOICE_19    (19<<1)
#define SD_VOICE_20    (20<<1)
#define SD_VOICE_21    (21<<1)
#define SD_VOICE_22    (22<<1)
#define SD_VOICE_23    (23<<1)
#define SD_VOICE_XX    (0x1f<<1)
#define SD_VOICE(no)   ((no)<<1)


/* param */
#define SD_VP_VOLL      (0x00<<8)
#define SD_VP_VOLR      (0x01<<8)
#define SD_VP_PITCH     (0x02<<8)
#define SD_VP_ADSR1     (0x03<<8)
#define SD_VP_ADSR2     (0x04<<8)
#define SD_VP_ENVX      (0x05<<8)
#define SD_VP_VOLXL     (0x06<<8)
#define SD_VP_VOLXR     (0x07<<8)
#define SD_P_MMIX       (0x08<<8)
#define SD_P_MVOLL      ((0x09<<8)+(0x01<<7))
#define SD_P_MVOLR      ((0x0a<<8)+(0x01<<7))
#define SD_P_EVOLL      ((0x0b<<8)+(0x01<<7))
#define SD_P_EVOLR      ((0x0c<<8)+(0x01<<7))
#define SD_P_AVOLL      ((0x0d<<8)+(0x01<<7))
#define SD_P_AVOLR      ((0x0e<<8)+(0x01<<7))
#define SD_P_BVOLL      ((0x0f<<8)+(0x01<<7))
#define SD_P_BVOLR      ((0x10<<8)+(0x01<<7))
#define SD_P_MVOLXL     ((0x11<<8)+(0x01<<7))
#define SD_P_MVOLXR     ((0x12<<8)+(0x01<<7))


/* switch */
#define SD_S_PMON       (0x13<<8)
#define SD_S_NON        (0x14<<8)
#define SD_S_KON        (0x15<<8)
#define SD_S_KOFF       (0x16<<8)
#define SD_S_ENDX       (0x17<<8)
#define SD_S_VMIXL      (0x18<<8)
#define SD_S_VMIXEL     (0x19<<8)
#define SD_S_VMIXR      (0x1a<<8)
#define SD_S_VMIXER     (0x1b<<8)


/* address */
#define SD_A_ESA        (0x1c<<8)
#define SD_A_EEA        (0x1d<<8)
#define SD_A_TSA        (0x1e<<8)
#define SD_A_IRQA       (0x1f<<8)
#define SD_VA_SSA       ((0x20<<8)+(0x01<<6))
#define SD_VA_LSAX      ((0x21<<8)+(0x01<<6))
#define SD_VA_NAX       ((0x22<<8)+(0x01<<6))


/* CoreAttr */
#define SD_C_EFFECT_ENABLE    (1<<1)
#define SD_C_IRQ_ENABLE       (2<<1)
#define SD_C_MUTE_ENABLE      (3<<1)
#define SD_C_NOISE_CLK        (4<<1)
#define SD_C_SPDIF_MODE       (5<<1)

/* SPDIF OUT */
#define SD_SPDIF_OUT_PCM	0
#define SD_SPDIF_OUT_BITSTREAM  1
#define SD_SPDIF_OUT_OFF	2
#define SD_SPDIF_OUT_BYPASS     3

#define SD_SPDIF_COPY_NORMAL    0x00
#define SD_SPDIF_COPY_PROHIBIT  0x80

#define SD_SPDIF_MEDIA_CD       0x000
#define SD_SPDIF_MEDIA_DVD      0x800


/* MIX */
#define SD_MMIX_SINER  (1 <<  0)
#define SD_MMIX_SINEL  (1 <<  1)
#define SD_MMIX_SINR   (1 <<  2)
#define SD_MMIX_SINL   (1 <<  3)
#define SD_MMIX_MINER  (1 <<  4)
#define SD_MMIX_MINEL  (1 <<  5)
#define SD_MMIX_MINR   (1 <<  6)
#define SD_MMIX_MINL   (1 <<  7)
#define SD_MMIX_MSNDER (1 <<  8)
#define SD_MMIX_MSNDEL (1 <<  9)
#define SD_MMIX_MSNDR  (1 << 10)
#define SD_MMIX_MSNDL  (1 << 11)


/* ADSR */
#define SD_ADSR_MASK_AR 0xff00
#define SD_ADSR_MASK_DR 0x00f0
#define SD_ADSR_MASK_SL 0x000f
#define SD_ADSR_MASK_SR 0xffc0
#define SD_ADSR_MASK_RR 0x003f
#define SD_ADSR_MASK_AM 0x8000
#define SD_ADSR_MASK_SM 0xe000

#define SD_ADSR_A_LINEAR        0
#define SD_ADSR_A_EXP           0x8000
#define SD_ADSR_ARATE(x)        (((x)&0x7f)<<8)
#define SD_ADSR_DRATE(x)        (((x)&0x0f)<<4)
#define SD_ADSR_SLEVEL(x)       (((x)&0x0f))

#define SD_ADSR_S_LINEAR_INC    (0<<13)
#define SD_ADSR_S_LINEAR_DEC    (2<<13)
#define SD_ADSR_S_EXP_INC       (4<<13)
#define SD_ADSR_S_EXP_DEC       (6<<13)
#define SD_ADSR_SRATE(x)        (((x)&0x7f)<<6)
#define SD_ADSR_R_LINEAR        0
#define SD_ADSR_R_EXP           0x20
#define SD_ADSR_RRATE(x)        (((x)&0x01f))

#define SD_ADSR1( x, y, z, w )  ((x)|SD_ADSR_ARATE(y)|SD_ADSR_DRATE(z)|SD_ADSR_SLEVEL(w))
#define SD_ADSR2( x, y, z, w )  ((x)|SD_ADSR_SRATE(y)|(z)|SD_ADSR_RRATE(w))


/* バッチコマンド用 */
#define SD_BSET_PARAM	0x01
#define SD_BGET_PARAM	0x11
#define SD_BSET_SWITCH	0x02
#define SD_BGET_SWITCH	0x12
#define SD_BSET_ADDR	0x03
#define SD_BGET_ADDR	0x13
#define SD_BSET_CORE	0x04
#define SD_BGET_CORE	0x14
#define SD_WRITE_IOP	0x05
#define SD_WRITE_EE	0x06
#define SD_RETURN_EE	0x07


/* 転送関係 */
#define SD_TRANS_MODE_WRITE 0
#define SD_TRANS_MODE_READ  1
#define SD_TRANS_MODE_STOP  2
#define SD_TRANS_MODE_WRITE_FROM 3

#define SD_TRANS_BY_DMA     (0x0<<3)
#define SD_TRANS_BY_IO      (0x1<<3)

#define SD_BLOCK_ONESHOT (0<<4) 
#define SD_BLOCK_LOOP (1<<4) 
#define SD_BLOCK_HANDLER (1<<7)

#define SD_BLOCK_C0_VOICE1    ( 0x0<<8 )
#define SD_BLOCK_C0_VOICE3    ( 0x1<<8 )
#define SD_BLOCK_C1_SINL      ( 0x2<<8 )
#define SD_BLOCK_C1_SINR      ( 0x3<<8 )
#define SD_BLOCK_C1_VOICE1    ( 0x4<<8 )
#define SD_BLOCK_C1_VOICE3    ( 0x5<<8 )
#define SD_BLOCK_C0_MEMOUTL   ( 0x6<<8 )
#define SD_BLOCK_C0_MEMOUTR   ( 0x7<<8 )
#define SD_BLOCK_C0_MEMOUTEL  ( 0x8<<8 )
#define SD_BLOCK_C0_MEMOUTER  ( 0x9<<8 )
#define SD_BLOCK_C1_MEMOUTL   ( 0xa<<8 )
#define SD_BLOCK_C1_MEMOUTR   ( 0xb<<8 )
#define SD_BLOCK_C1_MEMOUTEL  ( 0xc<<8 )
#define SD_BLOCK_C1_MEMOUTER  ( 0xd<<8 )

#define SD_BLOCK_COUNT(x)  ( (x)<<12 )

#define SD_TRANS_STATUS_WAIT  1
#define SD_TRANS_STATUS_CHECK 0

/* エフェクト関係 */
#define SD_REV_MODE_OFF  	   0
#define SD_REV_MODE_ROOM	   1
#define SD_REV_MODE_STUDIO_A	   2
#define SD_REV_MODE_STUDIO_B	   3
#define SD_REV_MODE_STUDIO_C	   4
#define SD_REV_MODE_HALL	   5
#define SD_REV_MODE_SPACE	   6
#define SD_REV_MODE_ECHO	   7
#define SD_REV_MODE_DELAY	   8
#define SD_REV_MODE_PIPE	   9
#define SD_REV_MODE_MAX		   10
#define SD_REV_MODE_CLEAR_WA	   0x100
#define SD_REV_MODE_CLEAR_CH0	   0x000
#define SD_REV_MODE_CLEAR_CH1	   0x200
#define SD_REV_MODE_CLEAR_CH_MASK  0x200

#define SD_REV_SIZE_OFF  	   0x80
#define SD_REV_SIZE_ROOM	   0x26c0
#define SD_REV_SIZE_STUDIO_A	   0x1f40
#define SD_REV_SIZE_STUDIO_B	   0x4840
#define SD_REV_SIZE_STUDIO_C	   0x6fe0
#define SD_REV_SIZE_HALL	   0xade0
#define SD_REV_SIZE_SPACE	   0xf6c0
#define SD_REV_SIZE_ECHO	   0x18040
#define SD_REV_SIZE_DELAY	   0x18040
#define SD_REV_SIZE_PIPE	   0x3c00

/* SPU2ローカルメモリアドレス関係 */
#define SD_SOUNDOUT_TOPADDR        0x0
#define SD_SOUNDIN_TOPADDR         0x4000
#define SD_USERAREA_TOPADDR        0x5010

/* 初期化モード */
#define SD_INIT_COLD	0
#define SD_INIT_HOT	1

/* 型定義 */
#define SD_TRANS_CBProc   void*
#define SD_IRQ_CBProc     void*

typedef int (*sceSdTransIntrHandler)(int, void *);
typedef int (*sceSdSpu2IntrHandler)(int, void *);
typedef int (*sceSdBlockTransHandler)(int ch, void *data, void **addr, int *size);

/* 構造体定義 */
typedef struct {
	unsigned short func;
	unsigned short entry;
	unsigned int  value;
} sceSdBatch;

typedef struct {
     int     core;
     int     mode;
     short   depth_L;
     short   depth_R;
     int     delay;
     int     feedback;
} sceSdEffectAttr;

#endif /* _SDMACRO_H_ */
