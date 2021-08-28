/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 0.30
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libipu - libipu.h
 *                     header file of libipu 
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      0.10            Mar.25.1999     umemura     the first version
 *      0.20            Feb.29.2000     umemura     inline -> define
 *      0.30            Sep.26.2003     hana        change comment
 */
#ifndef _LIBIPU_H_
#define _LIBIPU_H_

#include <eetypes.h>
#include <eeregs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************
 *  IPU_CTRL.IDP
 **********************************************************************/
#define SCE_IPU_CTRL_IDP_8BIT 		(0x00<<16)
#define SCE_IPU_CTRL_IDP_9BIT 		(0x01<<16)
#define SCE_IPU_CTRL_IDP_10BIT 		(0x02<<16)

/**********************************************************************
 *  IPU_CTRL.AS
 **********************************************************************/
#define SCE_IPU_CTRL_AS_ZIGZAG 		(0x00<<20)
#define SCE_IPU_CTRL_AS_ALTERNATE 	(0x01<<20)

/**********************************************************************
 *  IPU_CTRL.IVF
 **********************************************************************/
#define SCE_IPU_CTRL_IVF_SAME 		(0x00<<21)
#define SCE_IPU_CTRL_IVF_SWITCH		(0x01<<21)

/**********************************************************************
 *  IPU_CTRL.QST
 **********************************************************************/
#define SCE_IPU_CTRL_QST_LINEAR 	(0x00<<22)
#define SCE_IPU_CTRL_QST_NONLINEAR 	(0x01<<22)

/**********************************************************************
 *  IPU_CTRL.MP1
 **********************************************************************/
#define SCE_IPU_CTRL_MP1_NONCOMPATIBLE 	(0x00<<23)
#define SCE_IPU_CTRL_MP1_COMPATIBLE	(0x01<<23)

/**********************************************************************
 *  IPU_CTRL.PCT
 **********************************************************************/
#define SCE_IPU_CTRL_PCT_IPIC 		(0x01<<24)
#define SCE_IPU_CTRL_PCT_PPIC 		(0x02<<24)
#define SCE_IPU_CTRL_PCT_BPIC 		(0x03<<24)
#define SCE_IPU_CTRL_PCT_DPIC 		(0x04<<24)

/**********************************************************************
 *  IPU commands
 **********************************************************************/
#define SCE_IPU_BCLR	0x00000000
#define SCE_IPU_IDEC	0x10000000
#define SCE_IPU_BDEC	0x20000000
#define SCE_IPU_VDEC	0x30000000
#define SCE_IPU_FDEC	0x40000000
#define SCE_IPU_SETIQ	0x50000000
#define SCE_IPU_SETVQ	0x60000000
#define SCE_IPU_CSC	0x70000000
#define SCE_IPU_PACK	0x80000000
#define SCE_IPU_SETTH	0x90000000

/**********************************************************************
 *  IPU commands' arguments
 **********************************************************************/

/* for IDEC */
#define SCE_IPU_IDEC_NODTDECODE		0
#define SCE_IPU_IDEC_DTDECODE		1
#define SCE_IPU_IDEC_NOOFFSET		0
#define SCE_IPU_IDEC_OFFSET			1
#define SCE_IPU_IDEC_NODITHER		0
#define SCE_IPU_IDEC_DITHER			1
#define SCE_IPU_IDEC_RGB32			0
#define SCE_IPU_IDEC_RGB16			1

/* for BDEC */
#define SCE_IPU_BDEC_FRAMEDCT		0
#define SCE_IPU_BDEC_FIELDDCT		1
#define SCE_IPU_BDEC_NODCRESET		0
#define SCE_IPU_BDEC_DCRESET		1
#define SCE_IPU_BDEC_NONINTRA		0
#define SCE_IPU_BDEC_INTRA			1

/* for VDEC */
#define SCE_IPU_VDEC_MBAI			0
#define SCE_IPU_VDEC_MBTYPE			1
#define SCE_IPU_VDEC_MOTIONCODE		2
#define SCE_IPU_VDEC_DMVECTOR		3

/* for SETIQ */
#define SCE_IPU_SETIQ_INTRA			0
#define SCE_IPU_SETIQ_NONINTRA		1

/* for CSC */
#define SCE_IPU_CSC_NODITHER		0
#define SCE_IPU_CSC_DITHER			1
#define SCE_IPU_CSC_RGB32			0
#define SCE_IPU_CSC_RGB16			1

/* for PACK */
#define SCE_IPU_PACK_NODITHER		0
#define SCE_IPU_PACK_DITHER			1
#define SCE_IPU_PACK_INDX4			0
#define SCE_IPU_PACK_RGB16			1

/**********************************************************************
 *  Structures
 **********************************************************************/
typedef struct {
    u_char y[16*16];
    u_char cb[8*8];
    u_char cr[8*8];
} sceIpuRAW8;

typedef struct {
    short y[16*16];
    short cb[8*8];
    short cr[8*8];
} sceIpuRAW16;

typedef struct {
    u_int pix[16*16];
} sceIpuRGB32;

typedef struct {
    u_short pix[16*16];
} sceIpuRGB16;

typedef struct {
    u_int pix[2*16];
} sceIpuINDX4;

typedef struct {
    u_int d4madr; /* 停止したときの D4_MADR レジスタの値 */
    u_int d4tadr; /* 停止したときの D4_TADR レジスタの値 */
    u_int d4qwc;  /* 停止したときの D4_QWC レジスタの値 */
    u_int d4chcr; /* 停止したときの D4_CHCR レジスタの値 */
    u_int d3madr; /* 停止したときの D3_MADR レジスタの値 */
    u_int d3qwc;  /* 停止したときの D3_QWC レジスタの値 */
    u_int d3chcr; /* 停止したときの D3_CHCR レジスタの値 */
    u_int ipubp;  /* 停止したときの IPU_BP レジスタの値 */
    u_int ipuctrl;/* 停止したときの IPU_CTRL レジスタの値 */
} sceIpuDmaEnv;

/**********************************************************************
 *  Functions
 **********************************************************************/

/* for erx */
void *sceIpuGetErxEntries(void);
	
/* ---------------------------------------------------------------------
 * sceIpuInit
 * ---------------------------------------------------------------------
 * [書式] void sceIpuInit(void)
 * [引数] なし
 * [返値] なし
 * [解説] IPU 自体をリセットし、FIFOをクリアします。
 */
void sceIpuInit(void);

/* ---------------------------------------------------------------------
 * sceIpuBCLR
 * ---------------------------------------------------------------------
 * [書式] void sceIpuBCLR(int bp)
 * [引数] bp   最初の 128 bit のうち復号を開始するビット位置
 * [返値] なし
 * [解説] BCLR コマンドを実行して入力FIFOをクリアします。
 *        この関数を呼び出す前に、入力FIFOへのDMA(toIPU:ch-4)を
 *        停止しておく必要があります。
 */
#define sceIpuBCLR(bp) DPUT_IPU_CMD(SCE_IPU_BCLR | (bp))

/* ---------------------------------------------------------------------
 * sceIpuIDEC
 * ---------------------------------------------------------------------
 * [書式]
 *      void sceIpuIDEC(
 *           int ofm,
 *           int dte,
 *           int sgn,
 *           int dtd,
 *           int qsc,
 *           int fb
 *      )
 * 
 * [引数]
 *      ofm  Output Format
 *           SCE_IPU_IDEC_RGB32(0)      : RGB32
 *           SCE_IPU_IDEC_RGB16(1)      : RGB16
 *      dte  Dither Enable
 *           SCE_IPU_IDEC_NODITHER(0)   : ディザなし
 *           SCE_IPU_IDEC_DITHER(1)     : ディザあり
 *           (ofm = RGB16 のときのみ有効)
 *      sgn  Pseudo Sign Offset
 *           SCE_IPU_IDEC_NOOFFSET(0)   : offset なし
 *           SCE_IPU_IDEC_OFFSET(1)     : offset -128
 *      dtd  DT Decode
 *           SCE_IPU_IDEC_NODTDECODE(0) : Dct Type をデコードしない
 *           SCE_IPU_IDEC_DTDECODE(1)   : Dct Type をデコードする
 *      qsc  Quantizer Step Code
 *      fb   Forward Bit
 * 
 * [返値] なし
 * [解説] IDEC コマンドを実行してイントラ復号を行います。
 */
#define sceIpuIDEC(ofm, dte, sgn, dtd, qsc, fb) \
    DPUT_IPU_CMD(SCE_IPU_IDEC \
    	| ((ofm) << 27) \
    	| ((dte) << 26) \
    	| ((sgn) << 25) \
    	| ((dtd) << 24) \
    	| ((qsc) << 16) \
	| (fb) \
    )

/* ---------------------------------------------------------------------
 * sceIpuBDEC
 * ---------------------------------------------------------------------
 * [書式]
 *      void sceIpuBDEC(
 *           int mbi,
 *           int dcr,
 *           int dt,
 *           int qsc,
 *           int fb
 *      )
 * 
 * [引数]
 *      mbi  Macroblock Intra
 *           SCE_IPU_BDEC_NONINTRA(0) : 非イントラマクロブロック
 *           SCE_IPU_BDEC_INTRA(1)    : イントラマクロブロック
 *      dcr  DC Reset
 *           SCE_IPU_BDEC_NODCRESET(0): DC予測値をリセットしない
 *           SCE_IPU_BDEC_DCRESET(1)  : DC予測値をリセットする
 *      dt   DCT Type
 *           SCE_IPU_BDEC_FRAMEDCT(0) : frame DCT
 *           SCE_IPU_BDEC_FIELDDCT(1) : field DCT
 *      qsc  Quantiser Step Code
 *      fb   Forward Bit
 * 
 * [返値] なし
 * [解説] BDEC コマンドを実行してブロック復号を行います。
 */
#define sceIpuBDEC(mbi, dcr, dt, qsc, fb) \
    DPUT_IPU_CMD(SCE_IPU_BDEC \
    	| ((mbi) << 27) \
    	| ((dcr) << 26) \
    	| ((dt)  << 25) \
    	| ((qsc) << 16) \
	| (fb) \
    )

/* ---------------------------------------------------------------------
 * sceIpuVDEC
 * ---------------------------------------------------------------------
 * [書式] void sceIpuVDEC(int tbl, int fb)
 * [引数]
 *      tbl  VLC table
 *           SCE_IPU_VDEC_MBAI(0)      : Macroblock Address Increment
 *           SCE_IPU_VDEC_MBTYPE(1)    : Macroblock Type
 *           SCE_IPU_VDEC_MOTIONCODE(2): Motion Code
 *           SCE_IPU_VDEC_DMVECTOR(3)  : DMVector
 *      fb   Forward Bit
 * 
 * [返値] なし
 * [解説] VDEC コマンドを実行して、tbl で指定されたシンボルを復号しま
 *        す。復号結果は sceIpuGetVdecResult() で取得することができ
 *        ます。
 */
#define sceIpuVDEC(tbl, fb) \
    DPUT_IPU_CMD(SCE_IPU_VDEC | ((tbl) << 26) | (fb))

/* ---------------------------------------------------------------------
 * sceIpuFDEC
 * ---------------------------------------------------------------------
 * [書式] void sceIpuFDEC(int fb)
 * [引数] fb     Forward Bit
 * [返値] なし
 * [解説] FDEC コマンドを実行して固定長データを復号します。
 *        復号結果は sceIpuGetFdecResult() で取得することができます。
 */
#define sceIpuFDEC(fb) DPUT_IPU_CMD(SCE_IPU_FDEC | (fb))

/* ---------------------------------------------------------------------
 * sceIpuSETIQ
 * ---------------------------------------------------------------------
 * [書式] void sceIpuSETIQ(int iqm, int fb)
 * [引数]
 *      iqm  Intra IQ Matrix
 *           SCE_IPU_SETIQ_INTRA(0)    : イントラ量子化マトリクス
 *           SCE_IPU_SETIQ_NONINTRA(1) : 非イントラ量子化マトリクス
 *      fb   Forward Bit
 * 
 * [返値] なし
 * [解説] SETIQ コマンドを実行して IQ テーブルを設定します。
 */
#define sceIpuSETIQ(iqm,  fb) \
    DPUT_IPU_CMD(SCE_IPU_SETIQ | ((iqm) << 27) | (fb))

/* ---------------------------------------------------------------------
 * sceIpuSETVQ
 * ---------------------------------------------------------------------
 * [書式] void sceIpuSETVQ(void)
 * [引数] なし
 * [返値] なし
 * [解説] SETVQ コマンドを実行して VQCLUT テーブルを設定します。
 */
#define sceIpuSETVQ() DPUT_IPU_CMD(SCE_IPU_SETVQ)

/* ---------------------------------------------------------------------
 * sceIpuCSC
 * ---------------------------------------------------------------------
 * [書式] void sceIpuCSC(int ofm, int dte, int mbc)
 * [引数]
 *      ofm  Output Format
 *           SCE_IPU_CSC_RGB32(0)    : RGB32
 *           SCE_IPU_CSC_RGB16(1)    : RGB16
 *      dte  Dither Enable
 *           SCE_IPU_CSC_NODITHER(0) : ディザなし
 *           SCE_IPU_CSC_DITHER(1)   : ディザあり
 *           (ofm = RGB16 のときのみ有効)
 *      mbc  Macroblock Count 変換するマクロブロック数
 * [返値] なし
 * [解説] CSC コマンドを実行して色空間の変換を行います。
 */
#define sceIpuCSC(ofm, dte, mbc) \
    DPUT_IPU_CMD(SCE_IPU_CSC | ((ofm) << 27) | ((dte) << 26) | (mbc))

/* ---------------------------------------------------------------------
 * sceIpuPACK
 * ---------------------------------------------------------------------
 * [書式] void sceIpuPACK(int ofm, int dte, int mbc)
 * [引数]
 *      ofm  Output Format
 *           SCE_IPU_PACK_INDX4(0)    : INDX4
 *           SCE_IPU_PACK_RGB16(1)    : RGB16
 *      dte  Dither Enable
 *           SCE_IPU_PACK_NODITHER(0) : ディザなし
 *           SCE_IPU_PACK_DITHER(1)   : ディザあり
 *      mbc  Macroblock Count 変換するマクロブロック数
 * [返値] なし
 * [解説] PACK コマンドを実行してフォーマット変換を行います。
 */
#define sceIpuPACK(ofm, dte, mbc) \
    DPUT_IPU_CMD(SCE_IPU_PACK | ((ofm) << 27) | ((dte) << 26) | (mbc))

/* ---------------------------------------------------------------------
 * sceIpuSETTH
 * ---------------------------------------------------------------------
 * [書式] void sceIpuSETTH(int th1, int th0)
 * [引数]
 *      th1  半透明スレショルド
 *      th0  透明スレショルド
 * [返値] なし
 * [解説] SETTH コマンドを実行してスレショルド値を設定します。
 *        このスレショルド値は CSC コマンドで色変換を行う際に使用され
 *        ます。
 */
#define sceIpuSETTH(th1, th0) \
    DPUT_IPU_CMD(SCE_IPU_SETTH | ((th1) << 16) | (th0))

/* ---------------------------------------------------------------------
 * sceIpuGetFVdecResult
 * ---------------------------------------------------------------------
 * [書式] u_int sceIpuGetFVdecResult(void)
 * [引数] なし
 * [返値] 直前の FDEC コマンドまたは VDEC コマンドで復号されたデータ
 * [解説] 直前に実行された FDEC コマンドまたは VDEC コマンドの実行結果
 *        を読み出します。
 */
#define sceIpuGetFVdecResult() DGET_IPU_CMD()

/* ---------------------------------------------------------------------
 * sceIpuReset
 * ---------------------------------------------------------------------
 * [書式] void sceIpuReset(void)
 * [引数] なし
 * [返値] なし
 * [解説] IPU をリセットします。
 */
#define sceIpuReset() DPUT_IPU_CTRL(IPU_CTRL_RST_M)

/* ---------------------------------------------------------------------
 * sceIpuIsBusy
 * ---------------------------------------------------------------------
 * [書式] int sceIpuIsBusy(void)
 * [引数] なし
 * [返値]
 *      0 : 停止中
 *      その他 : 動作中
 * [解説] IPU が動作中かどうかを返します。
 */
#define sceIpuIsBusy() ((int)((int)DGET_IPU_CTRL() < 0))

/* ---------------------------------------------------------------------
 * sceIpuIsError
 * ---------------------------------------------------------------------
 * [書式] int sceIpuIsError(void)
 * [引数] なし
 * [返値]
 *      0 : エラーなし
 *      その他 : エラーあり
 * [解説] IPU の処理途中でエラーが発生したかどうかを返します。
 *        この値は IPU コマンドを実行するたびに自動的にクリアされます。
 */
#define sceIpuIsError() ((int)(DGET_IPU_CTRL() & IPU_CTRL_ECD_M))

/* ---------------------------------------------------------------------
 * sceIpuSync
 * ---------------------------------------------------------------------
 * [書式] int sceIpuSync(int mode, u_short timeout)
 * [引数]
 *     mode:
 * 	0: IPU が動作中である間ブロックします。
 * 	1: 即座に終了し、そのときのステータスを返します。
 *	   これは、sceIpuIsBusy() と同じ動作となります。
 * 
 *     timeout:
 * 	mode = 0 のときのタイムアウト値を指定します。
 * 	指定する値の単位は Horizontal line カウントで、
 *      有効な値の範囲は
 *             0 <= timeout <= 65535 
 *      です。
 * 
 * 	timeout = 0: ライブラリでデフォルトのタイムアウト値を
 * 		     使用します。
 * 	timeout > 0: 指定される値をタイムアウト値として使用します。
 *
 * [返値]
 *    mode = 0 のとき
 *       0 以上の値: 正常終了
 *       負の値:     異常終了(タイムアウト発生)
 *
 *    mode = 1 のとき
 *       0: IPU が動作中でない
 *       正の値: IPU が動作中
 *
 * [解説] IPU が動作中かどうかを判断し、IPU の動作終了を待ったり、
 *        IPU の状態を返したりします。
 */
int sceIpuSync(int mode, u_short timeout);

/* ---------------------------------------------------------------------
 * sceIpuStopDMA
 * ---------------------------------------------------------------------
 * [書式] void sceIpuStopDMA(sceIpuDmaEnv *env)
 * [引数] env  内部状態を保存する構造体変数へのポインタ
 *
 *     typedef struct {
 *         u_int d4madr; // 停止したときの D4_MADR レジスタの値
 *         u_int d4tadr; // 停止したときの D4_TADR レジスタの値
 *         u_int d4qwc;  // 停止したときの D4_QWC レジスタの値
 *         u_int d4chcr; // 停止したときの D4_CHCR レジスタの値
 *         u_int d3madr; // 停止したときの D3_MADR レジスタの値
 *         u_int d3qwc;  // 停止したときの D3_QWC レジスタの値
 *         u_int d3chcr; // 停止したときの D3_CHCR レジスタの値
 *         u_int ipubp;  // 停止したときの IPU_BP レジスタの値
 *         u_int ipuctrl;// 停止したときの IPU_CTRL レジスタの値
 *     } sceIpuDmaEnv;
 *
 * [返値] なし
 * [解説] toIPU(ch-4) および fromIPU(ch-3) の DMA を安全に停止し、
 *        これらの DMAの状態および IPU の内部状態を保存します。
 */
void sceIpuStopDMA(sceIpuDmaEnv *env);

/* ---------------------------------------------------------------------
 * sceIpuRestartDMA
 * ---------------------------------------------------------------------
 * [書式] void sceIpuRestartDMA(sceIpuDmaEnv *env)
 * [引数] env  DMA および IPU の状態を保存した構造体変数のポインタ
 * [返値] なし
 * [解説] 保存された DMA および IPU の状態に従って、toIPU(ch-4) および
 *        fromIPU(ch-3) の DMA を再開する。
 */
void sceIpuRestartDMA(sceIpuDmaEnv *env);

#ifdef __cplusplus
}
#endif

#endif /* _LIBIPU_H_ */

