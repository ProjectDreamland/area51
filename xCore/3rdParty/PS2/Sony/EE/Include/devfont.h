/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *                      Emotion Engine Library
 *                          Version 0.03
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                        libdev - devfont.h
 *                         develop library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.01           Mar,29,1999     shibuya
 *       0.02           Jun,11,1999     shibuya
 *       0.03           Sep,25,2003     hana        change comment
 */

#ifndef __devfont__
#define __devfont__

#include <eekernel.h>
#include <eetypes.h>
#include <eestruct.h>

#include <libdma.h>
#include <libgraph.h>
#include <libgifpk.h>

#ifdef __cplusplus
extern "C" {
#endif

/* コンソールライブラリを初期化 */
void sceDevConsInit(void);

/* コンソールをオープン。（１つしかない。サイズの最大は80 x 64キャラクタ） */
int sceDevConsOpen(u_int gs_x, u_int gs_y, u_int chr_w, u_int chr_h);

/* コンソールをクローズ */
void sceDevConsClose(int cd);


/* コンソールのイメージをパケットにつける。 */
void sceDevConsRef(int cd, sceGifPacket* pPacket);

/* コンソールのイメージを描画 */
void sceDevConsDraw(int cd);

/* コンソールのイメージを描画（スクラッチパッド使用） */
void sceDevConsDrawS(int cd);

/* コンソールクリア */
void sceDevConsClear(int cd);

/* コンソールへ文字表示 */
u_int sceDevConsPrintf(int cd, const char* str, ...);

/* フォントのカラーテーブルの変更 */
void sceDevConsSetColor(int cd, u_char c, u_char r, u_char g, u_char b);


/* カーソル位置を変更 */
void sceDevConsLocate(int cd, u_int lx, u_int ly);

/* カーソル位置に１文字出力 */
void sceDevConsPut(int cd, u_char c, u_char a);

/* カーソル位置から１文字取得(上位８ビットにはアトリビュート) */
u_short sceDevConsGet(int cd);

/* アトリビュート(色)変更 */
void sceDevConsAttribute(int cd, u_char col);


/* 矩形領域クリア */
void sceDevConsClearBox(int cd, int x, int y, u_int w, u_int h);

/* 矩形領域移動 */
void sceDevConsMove(int cd, int dx, int dy, int sx, int sy, u_int w, u_int h);

/* ロールアップ */
void sceDevConsRollup(int cd, u_int line);


/* 枠つきメッセージ表示 */
void sceDevConsMessage(int cd, int x, int y, char const* str);

/* フレーム */
void sceDevConsFrame(int cd, int x, int y, u_int w, u_int h);

/* 旧ライブラリサポート */
u_long128* sceDevFont(u_long128*, int, int, int, char *, int);
  /* packet, x, y, z(無効), str, ctxt */

#ifdef __cplusplus
}
#endif

#endif /* __devfont__ */
