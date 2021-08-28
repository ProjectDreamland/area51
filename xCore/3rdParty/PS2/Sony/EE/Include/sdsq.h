/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 $Id: sdsq.h,v 1.38 2003/04/25 09:53:15 tokiwa Exp $
 */
/* 
 * Sound Data .SQ Library
 *
 * Copyright (C) 2003 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 *      Date            Design      Log
 *  ----------------------------------------------------
 *      2003-02-28      toki        initial
 */

#ifndef _SCE_SDSQ_H
#define _SCE_SDSQ_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <format/sound/sq.h>

/* 圧縮モード */
#define SCESDSQ_COMPMODE_NOTCOMP   0  /* SceSdSqMidiData->compMode */
#define SCESDSQ_COMPMODE_COMP      1  /* SceSdSqMidiData->compMode */

/* 状態ステータス */
#define SCESDSQ_READSTATUS_OK      0  /* 構造体使用可能 */
#define SCESDSQ_READSTATUS_END     1  /* リードポジションが末端に到達 */
                                      /*（構造体使用不可） */
#define SCESDSQ_READSTATUS_ERROR   2  /* リードエラー（構造体使用不可）*/

#define SCESDSQ_MAXMIDIMESSAGELENGTH       8
#define SCESDSQ_MAXORIGINALMESSAGELENGTH   12
#define SCESDSQ_MAXSONGMESSAGELENGTH       3

/* MIDIデータ情報構造体 */
typedef struct{
    u_int                 readStatus;
    u_int                 midiNumber;
    sceSeqMidiDataBlock   *midiData;
    u_int                 offset;
    u_int                 nextOffset;
    u_int                 division;
    u_int                 compMode;
    u_int                 compTableSize;
    u_int                 deltaTime;
    u_char                lastStatus;
    u_char                reserve[3];
    u_int                 messageLength;
    u_char                message[SCESDSQ_MAXMIDIMESSAGELENGTH];
    u_int                 originalMessageLength;
    u_char                originalMessage[SCESDSQ_MAXORIGINALMESSAGELENGTH];
} SceSdSqMidiData;

/* J
  readStatus            : 状態ステータス
  midiNumber            : この構造体が使用しているMIDIデータブロックナンバー
  midiData              : midiNumberが指すデータブロック（sceSeqMidiDataBlock）
                          のアドレス
  offset                : 現在のデータへのオフセット
  nextOffset            : 次データへのオフセット
  division              : ４分音符あたりの分解能
  compMode              : 圧縮モード
  compTableSize         : 圧縮テーブルのサイズ（非圧縮モードの場合０）
  deltaTime             : ３２ビットデータに変換後のデルタタイム
  lastStatus            : ランニングステータス
  reserve[3]            : 予約領域
  messageLength         : 補正後のデータ長
  message[8]            : 補正後のデータ
  originalMessageLength : 補正前のデータ長
  originalMessage[12]   : 補正前のデータ（deltaTime + message）
*/
/* E

 */

/* Songデータ構造体 */
typedef struct{
    u_int    readStatus;
    u_int    songNumber;
    void     *topAddr;
    u_int    offset;
    u_int    nextOffset;
    u_char   message[SCESDSQ_MAXSONGMESSAGELENGTH];
    u_char   reserve;
} SceSdSqSongData;

/*
  readStatus : 状態ステータス
  songNumber : この構造体が使用しているMIDIデータブロックナンバー
  topAddr    : songNumberが指すsongコマンドの先頭アドレス
  offset     : 現在のデータへのオフセット
  nextOffset : 次データへのオフセット
  message[3] : Songコマンドデータ
  reserve    : 予約領域
 */

/* 圧縮テーブルデータ構造体 */
/* ポリフォニックキープレッシャー情報構造体 */
typedef struct{
    u_char   status;  /* ステータスバイトデータ */
    u_char   data;    /* データ */
} SceSdSqCompTableData, SceSdSqPolyKeyData;

/* 解凍ノートオンデータ構造体 */
typedef struct{
    u_char   status;    /* ステータスバイトデータ */
    u_char   note;      /* ノート */
    u_char   velocity;  /* ベロシティ */
    u_char   reserve;
} SceSdSqCompTableNoteOnEvent;

int sceSdSqGetMaxMidiNumber(void *addr);
int sceSdSqGetMaxSongNumber(void *addr);
int sceSdSqInitMidiData(void *addr, u_int midiNumber, 
			SceSdSqMidiData *midiData);
int sceSdSqReadMidiData(SceSdSqMidiData *midiData);
int sceSdSqInitSongData(void *addr, u_int songNumber, 
			SceSdSqSongData *songData);
int sceSdSqReadSongData(SceSdSqSongData *songData);
int sceSdSqGetMaxCompTableIndex(void *addr, u_int midiNumber);
int sceSdSqGetCompTableOffset(void *addr, u_int midiNumber, u_int *offset);
int sceSdSqGetCompTableDataByIndex(void *addr, u_int midiNumber,
				   u_int compTableIndex,
				   SceSdSqCompTableData *data);
int sceSdSqGetNoteOnEventByPolyKeyPress(void *addr, u_int midiNumber,
					SceSdSqPolyKeyData *pData,
					SceSdSqCompTableNoteOnEvent *kData);
int sceSdSqCopyMidiData(SceSdSqMidiData *to, const SceSdSqMidiData *from);
int sceSdSqCopySongData(SceSdSqSongData *to, const SceSdSqSongData *from);

#ifdef __cplusplus
}
#endif

#endif /* !_SCE_SDSQ_H */
