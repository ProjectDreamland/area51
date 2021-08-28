/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.1
 $Id: sceerrno.h,v 1.31 2004/02/26 04:50:20 kaol Exp $
 */
/* 
 * Emotion Engine Library / I/O Processor Library
 *
 * Copyright (C) 2004 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * sceerrno.h - common error number definition and utilities
 *
 *      Date            Design      Log
 *  ----------------------------------------------------
 *      2002-09-11      style       initial draft
 *      2002-09-13      style       revised
 *      2002-09-17      style       official first version
 *      2002-10-15      style       solve EE/IOP diffs
 *      2002-10-18      style       lib. prefix BNNETCNF added
 *      2002-10-20      style       `SCE_ETIMER' is added
 *      2002-10-22      style       `SCE_' prefixed standard errno in sceerr.h
 *      2002-10-23      style       0x9004-9020 are added
 *      2003-04-03      style       `SCE_EHANDLER' is added
 *      2003-04-24      style       lib. prefix SDSQ, ERX TIMER added
 *      2003-04-24      style       `SCE_E{INT,DMA}_HANDLER' are added
 *      2003-04-25      style       0x9024-9039 are added
 *      2003-05-01      style       `SCE_E{ILLEGAL_CONTEXT, CPUDI}' are added
 *      2003-10-02      style       lib. prefix STDIO, MC are added
 *      2003-10-02      style       `SCE_EVERSION' is added
 *      2003-10-14      style       `SCE_ERPC' is added
 *      2003-12-01      style       `SCE_EUSB' is added
 *      2003-12-01      style       0x903a-903c are added
 *      2004-02-24      style       `SCE_EREGISTER' is added
 *      2004-02-24      style       `SCE_ERROR_PREFIX_SESQ2' is added
 *      2004-02-26      style       `SCE_E{CAN_NOT_PLAY,NOT_PLAYED}' are added
 */

#ifndef _SCE_ERRNO_H
#define _SCE_ERRNO_H

#include <errno.h>
#ifdef R3000
#include <kerror.h>		/* IOP kernel original error no. */
#endif

/*
 * 未定義エラー番号の定義:
 *     errno.h の EE/IOP における差異を吸収
 */
#ifndef EFORMAT
#define EFORMAT		47
#endif
#ifndef EUNSUP
#define EUNSUP		48
#endif
#ifndef ENOSHARE
#define ENOSHARE	136
#endif

/*
 * `SCE_' プレフィックス付き標準エラー番号
 */
#include <sceerr.h>

/*
 * SCE 独自定義エラー番号:
 * errno.h/kerror.h 未定義エラー番号 (下位 16 ビット: SCE_EBASE..0xffff)
 *     errno.h:         0-1xx
 *     kerror.h(IOP): 100-4xx
 */

#define SCE_EBASE		0x8000		/* 32768 (decimal) */

/* 対象が汎用なエラー */
#define SCE_EINIT		0x8001		/* 初期化処理が行われていない	*/
#define SCE_EID			0x8002		/* ID が存在しない		*/
#define SCE_ESEMAPHORE		0x8003		/* セマフォ処理が行えない	*/
#define SCE_ESEMA		SCE_ESEMAPHORE
#define SCE_ETHREAD		0x8004		/* スレッド処理が行えない	*/
#define SCE_ETIMER		0x8005		/* タイマー/アラーム処理が行えない */
#define SCE_EHANDLER		0x8006		/* ハンドラ登録処理が行えない	*/
#define SCE_EILLEGAL_CONTEXT	0x8007		/* 例外・割り込みハンドラからの呼び出し */
#define SCE_ECPUDI		0x8008		/* 既に割り込み禁止だった	*/
#define	SCE_EVERSION		0x8009		/* モジュールのバージョン不整合 */
#define	SCE_ERPC		0x800a		/* SIF RPCの発行が行えない */
#define SCE_EUSB		0x800b		/* USB 通信が異常		*/
#define SCE_EREGISTER		0x800c		/* 登録処理が行われていない	*/

/* ライブラリ依存なエラー */
#define SCE_EBASE_LIB		0x9000		/* 36864 (decimal) */

#define SCE_EDEVICE_BROKEN	0x9001		/* デバイス破損の可能性		*/
#define SCE_EFILE_BROKEN	0x9002		/* ファイルまたはディレクトリ破損の可能性 */
#define SCE_ENEW_DEVICE		0x9003		/* 新規デバイス検出		*/
#define SCE_EMDEPTH		0x9004		/* ディレクトリが深過ぎる	*/

#define SCE_ENO_PROGRAM		0x9005		/* プログラムチャンクが無い	*/
#define SCE_ENO_SAMPLESET	0x9006		/* サンプルセットチャンクが無い	*/
#define SCE_ENO_SAMPLE		0x9007		/* サンプルチャンクが無い	*/
#define SCE_ENO_VAGINFO		0x9008		/* VAGInfoチャンクが無い	*/
#define SCE_ENO_SBADDR		0x9009		/* スプリットブロックのアドレス情報が無い */
#define SCE_EBAD_PNUM		0x9010		/* プログラムナンバーは範囲外	*/
#define SCE_ENO_PNUM		0x9011		/* プログラムナンバーは未使用	*/
#define SCE_EBAD_SSNUM		0x9012  	/* サンプルセットナンバーは範囲外 */
#define SCE_ENO_SSNUM		0x9013  	/* サンプルセットナンバーは未使用 */
#define SCE_EBAD_SPNUM		0x9014  	/* サンプルナンバーは範囲外	*/
#define SCE_ENO_SPNUM		0x9015  	/* サンプルナンバーは未使用	*/
#define SCE_EBAD_VAGNUM		0x9016  	/* VAGInfoナンバーは範囲外	*/
#define SCE_ENO_VAGNUM		0x9017  	/* VAGInfoナンバーは未使用	*/
#define SCE_EBAD_SBNUM		0x9018  	/* スプリットブロックナンバーは範囲外 */
#define SCE_EVAGINFO_NOISE	0x9019  	/* VAGInfoが指し示すVAGはノイズである */
#define SCE_ENO_SPLITNUM	0x9020  	/* スプリットナンバーは未使用	*/

#define SCE_EINT_HANDLER	0x9021  	/* 割り込みハンドラ処理が不正	*/
#define SCE_EDMA_HANDLER	0x9022  	/* DMA 割り込みハンドラ処理が不正 */
/*				0x9023	*/ 	/* システム予約			*/

#define SCE_ENO_MIDI            0x9024		/* Midiチャンクが無い		*/
#define SCE_ENO_SONG            0x9025		/* Songチャンクが無い		*/
#define SCE_ENO_MIDINUM         0x9026		/* Midiデータブロックナンバーは未使用 */
#define SCE_ENO_SONGNUM         0x9027		/* Songナンバーは未使用		*/
#define SCE_ENO_COMPTABLE       0x9028		/* 圧縮テーブルが無い		*/
#define SCE_EBAD_COMPTABLEINDEX 0x9029		/* 圧縮テーブルインデックスは範囲外 */
#define SCE_EBAD_POLYKEYDATA    0x902a		/* ポリフォニックキープレッシャーは不正 */

#define SCE_ELINKERROR		0x902b		/* ロードしたモジュールが必要とする常駐ライブラリが存在しない */
#define SCE_ELINKERR		SCE_ELINKERROR
#define SCE_EILLEGAL_OBJECT	0x902c		/* オブジェクトファイルの形式が正しくない */
#define SCE_EUNKNOWN_MODULE 	0x902d		/* 指定したモジュールが見つからない */
#define SCE_EMEMINUSE 		0x902e		/* 指定したアドレスは既に使用中	*/
#define SCE_EALREADY_STARTED	0x902f		/* 指定したモジュールは既にスタートしている */
#define SCE_ENOT_STARTED	0x9030		/* 指定したモジュールはスタートしていない */
#define SCE_EALREADY_STOPPING	0x9031		/* 指定したモジュールはストップ処理中 */
#define SCE_EALREADY_STOPED	0x9032		/* 指定したモジュールは既にストップしている */
#define SCE_ENOT_STOPPED 	0x9033		/* 指定したモジュールはストップしていない */
#define SCE_ECAN_NOT_STOP	0x9034		/* モジュールの停止は出来なかった */
#define SCE_ENOT_REMOVABLE	0x9035		/* 指定したモジュールは削除可能ではない */
#define SCE_ELIBRARY_FOUND 	0x9036		/* ライブラリは既に登録されている */
#define SCE_ELIBRARY_NOTFOUND 	0x9037		/* ライブラリは登録されていない	*/
#define SCE_ELIBRARY_INUSE	0x9038		/* ライブラリは使用中		*/
#define SCE_EILLEGAL_LIBRARY 	0x9039		/* ライブラリヘッダが異常	*/

#define SCE_EBAD_FRAME		0x903a		/* 現在のフレームは不正		*/
#define SCE_ENO_STREAM		0x903b		/* カメラがストリーム状態でない	*/
#define SCE_ENO_CAMERA		0x903c		/* カメラが接続されていない	*/

#define SCE_ECAN_NOT_PLAY	0x903d		/* 演奏が行えない		*/
#define SCE_ECANT_PLAY		SCE_ECAN_NOT_PLAY
#define SCE_ENOT_PLAYED		0x903e		/* 演奏されていない		*/

/* 0xa000-0xa0ff */ /* DNAS予約 */


/*
 * 汎用正常終了マクロ
 */
#define SCE_OK	0

/*
 * ライブラリ識別プレフィックス
 */

/* エラー識別ビット */
#define SCE_ERROR_PREFIX_ERROR		0x80000000

/* プレフィックス・シフト量 */
#define SCE_ERROR_PREFIX_SHIFT		16

/* エラー・ビットマスク */
#define SCE_ERROR_MASK_LIBRARY_PREFIX	0x7fff0000
#define SCE_ERROR_MASK_ERRNO		0x0000ffff

/* ライブラリ識別プレフィックス値 */
#define SCE_ERROR_PREFIX_MC2	  0x01010000 /* メモリーカード        libmc2  */
#define SCE_ERROR_PREFIX_BNNETCNF 0x01020000 /* PS BB Navi. Net conf. bnnetcf */
#define SCE_ERROR_PREFIX_SDHD	  0x01030000 /* Sound Data .HD        libsdhd */
#define SCE_ERROR_PREFIX_SDSQ	  0x01040000 /* Sound Data .SQ        sdsq    */
#define SCE_ERROR_PREFIX_ERX	  0x01050000 /* ERX		      liberx  */
#define SCE_ERROR_PREFIX_TIMER	  0x01060000 /* Timer                 libtimer */
#define SCE_ERROR_PREFIX_STDIO	  0x01070000 /* 標準入出力            libkernl */
#define SCE_ERROR_PREFIX_MC	  0x01080000 /* メモリーカード        libmc   */
#define SCE_ERROR_PREFIX_SESQ2	  0x01090000 /* SESQ2                 modsesq2 */

/*
 * ユーティリティ
 */ 

/* for library author */
#define SCE_ERROR_ENCODE(prefix,err) (SCE_ERROR_PREFIX_ERROR | (prefix) | (err))

/* for user */
#define SCE_ERROR_ERRNO(err)      ((err) & SCE_ERROR_MASK_ERRNO)
#define SCE_ERROR_LIB_PREFIX(err) ((err) & SCE_ERROR_MASK_LIBRARY_PREFIX)

#endif /* _SCE_ERRNO_H */
