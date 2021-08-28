/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 1.0
 *                           Shift-JIS
 *
 *      Copyright (C) 2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libatok - atok_top.h
 *                           ATOK Library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      1.00            Nov,26,2001      kataoka     first ci
 */

#ifndef _LIBATOK_H_
#define _LIBATOK_H_

#if defined(__LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif	/* defined(__LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus) */

/* ATOKのためのコンテキストを定義 */
#define SCE_ATOK_CONTEXT_SIZE (4*1024)
typedef unsigned char sceAtokContext[ SCE_ATOK_CONTEXT_SIZE ] __attribute__((aligned(64)));

/* ATOKの各種設定 */
typedef struct {
	int Size;     /* sizeof(sceAtokConfig)を設定します */
	/* -----------------------------------------------------------------------*/
	int KanjiInput;     /* 漢字入力モード */
	                    /* 1= ローマ字漢字 */
	                    /* 2= かな入力漢字 */
	                    /* 3= 半角スルー   */

	int KanDspType;     /* 入力文字種の設定 */
	                    /* 1= ひらがな     */
	                    /* 2= 全角カタカナ */
	                    /* 3= 全角無変換   */
	                    /* 4= 半角カタカナ */
	                    /* 5= 半角無変換   */

	int FixMode;        /* 固定入力モード */
	                    /* 0= OFF              */
	                    /* 1= 固定ひらがな     */
	                    /* 2= 固定全角カタカナ */
	                    /* 3= 固定全角無変換   */
	                    /* 4= 固定半角カタカナ */
	                    /* 5= 固定半角無変換   */

	int HZCnvMode;      /* 半角全角変換する/しない */
	                    /* bit 3: 後変換単語への適用       */
	                    /* bit 2: ユーザー登録単語への適用 */
	                    /* bit 1: 入力中文字列への適用     */
	                    /* bit 0: 半角全角変換             */

	int HZCnvDetail;    /* 半角全角変換詳細 */
	                    /* bit 6: 数字    →全角 */
	                    /* bit 5: 英字    →全角 */
	                    /* bit 4: カタカナ→全角 */
	                    /* bit 2: 数字    →半角 */
	                    /* bit 1: 英字    →半角 */
	                    /* bit 0: カタカナ→半角 */

	char *HankakuCnvStr; /* 半角変換する記号文字列へのポインタ */

	char *ZenkakuCnvStr; /* 全角変換する記号文字列へのポインタ */

	int FuriganaMode;   /* ふりがな入力モード */
	                    /* 0= 終了 */
	                    /* 1= 開始 */

	int MaxInputChars;  /* 入力文字数 */

	int PutCursorMode;  /* 未確定文字列末尾にカーソル文字を追加する/しない */
	                    /* 0= しない */
	                    /* 1= する   */

	int PreAnalyze;     /* 入力時の辞書先読みする/しない */
	                    /* 0= しない */
	                    /* 1= する   */

	/* -----------------------------------------------------------------------*/
	int ConvMode;       /* 変換モード */
	                    /* 0= 連文節変換 */
	                    /* 1= 単文節変換 */
	                    /* 2= 自動変換   */

	int LearningMode;   /* 学習モード */
	                    /* 0= 学習しない */
	                    /* 1= 学習する   */

	int KutouCharSet;   /* 句読点組み合わせパターン */
	                    /* 0= ",.[]/ｰ"	(カンマ/ ピリオド/ 中括弧/   スラッシュ/ 音引き) */
	                    /* 1= "､｡｢｣･ｰ"	(読点  / 句点    / かぎ括弧/ 中黒      / 音引き) */

	int OkuriMode;      /* 送りがなモード */
	                    /* 0= 本則(正しい日本語) */
	                    /* 1= 省く               */
	                    /* 2= 送る               */

	int AutoRegKind;    /* 自動登録種別 */
	                    /* bit 3: 未登録語   */
	                    /* bit 2: 後変換     */
	                    /* bit 1: 複合語     */
	                    /* bit 0: 文節区切り */

	int LearningKind;   /* 学習種別 */
	                    /* bit 3: 未登録語         */
	                    /* bit 2: 後変換語         */
	                    /* bit 1: 複合語学習語     */
	                    /* bit 0: 区切り直し学習語 */

	int RomanRecover;   /* ローマ字立ち直りする/しない */
	                    /* 0= しない */
	                    /* 1= する   */

	int AutoRecMode;    /* 入力支援する/しない */
	                    /* 0= しない */
	                    /* 1= する   */

	int AutoRecRoman;   /* 入力支援(ローマ字入力時)の詳細設定 */
	                    /* bit 2: [N]の過不足  */
	                    /* bit 1: 子音の超過   */
	                    /* bit 0: 母音の過不足 */

	int AutoRecKana;    /* 入力支援(かな入力時)の詳細設定 */
	                    /* bit 2: 「わ」→「を」                 */
	                    /* bit 1: 「つ」→「っ」(「つ」を拗音に) */
	                    /* bit 0: 「゜」→「゛」(半濁点を濁点に) */

	int AutoRecRep;     /* 入力支援(置換)の詳細設定 */
	                    /* bit 3: 「。」→「．」(句点をピリオドに)   */
	                    /* bit 2: 「、」→「，」(読点をカンマに)     */
	                    /* bit 1: 「・」→「／」(中黒をスラッシュに) */
	                    /* bit 0: 「ー」→「－」(音引きをハイフンに) */

	int AppendAtoHen;   /* 後変換候補の候補リストへの追加をする/しない */
	                    /* bit 4: 半角無変換候補   */
	                    /* bit 3: 全角無変換候補   */
	                    /* bit 2: 半角カタカナ候補 */
	                    /* bit 1: 全角カタカナ候補 */
	                    /* bit 0: ひらがな候補     */

	int KutouCnvMode;   /* 句読点変換する/しない */
	                    /* 0= しない */
	                    /* 1= する   */

	int KutouCnvDetail; /* 句読点変換の詳細設定 */
	                    /* bit 3: [！](感嘆符)で変換 */
	                    /* bit 2: [？](疑問符)で変換 */
	                    /* bit 1: [、](読点)で変換   */
	                    /* bit 0: [。](句点)で変換   */

	int RuledZenCnv;    /* 全角変換例外規則の設定 */
	                    /* bit 0: [~]→[～]に変換 */
} sceAtokConfig;



/*
 * sceAtokSetConfig()関数、sceAtokGetConfig()関数の
 * sceAtokConfig構造体内の有効メンバ指定用マクロ定義
 */
#define SCE_ATOK_CONFIG_KANJIINPUT      (0x00000001UL)	/* 漢字入力モード */
#define SCE_ATOK_CONFIG_KANDSPTYPE      (0x00000002UL)	/* 入力文字種 */
#define SCE_ATOK_CONFIG_FIXMODE         (0x00000004UL)	/* 固定入力モード */
#define SCE_ATOK_CONFIG_HZCNVMODE       (0x00000008UL)	/* 半角全角変換する/しない */
#define SCE_ATOK_CONFIG_HZCNVDETAIL     (0x00000010UL)	/* 半角全角変換詳細 */
#define SCE_ATOK_CONFIG_HANKAKUCNVSTR   (0x00000020UL)	/* 半角変換する記号文字列へのポインタ */
#define SCE_ATOK_CONFIG_ZENKAKUCNVSTR   (0x00000040UL)	/* 全角変換する記号文字列へのポインタ */
#define SCE_ATOK_CONFIG_FURIGANAMODE    (0x00000080UL)	/* ふりがな入力モード */
#define SCE_ATOK_CONFIG_MAXINPUTCHARS   (0x00000100UL)	/* 入力文字数 */
#define SCE_ATOK_CONFIG_PUTCURSORMODE   (0x00000200UL)	/* 未確定文字列末尾にカーソル文字を追加する/しない */
#define SCE_ATOK_CONFIG_PREANALYZE      (0x00000400UL)	/* 入力時の辞書先読みする/しない */
#define SCE_ATOK_CONFIG_CONVMODE        (0x00010000UL)	/* 変換モード */
#define SCE_ATOK_CONFIG_LEARNINGMODE    (0x00020000UL)	/* 学習モード */
#define SCE_ATOK_CONFIG_KUTOUCHARSET    (0x00040000UL)	/* 句読点組み合わせパターン */
#define SCE_ATOK_CONFIG_OKURIMODE       (0x00080000UL)	/* 送りがなモード */
#define SCE_ATOK_CONFIG_AUTOREGKIND     (0x00100000UL)	/* 自動登録種別 */
#define SCE_ATOK_CONFIG_LEARNINGKIND    (0x00200000UL)	/* 学習種別 */
#define SCE_ATOK_CONFIG_ROMANRECOVER    (0x00400000UL)	/* ローマ字立ち直りする/しない */
#define SCE_ATOK_CONFIG_AUTORECMODE     (0x00800000UL)	/* 入力支援する/しない */
#define SCE_ATOK_CONFIG_AUTORECROMAN    (0x01000000UL)	/* 入力支援(ローマ字入力時)の詳細設定 */
#define SCE_ATOK_CONFIG_AUTORECKANA     (0x02000000UL)	/* 入力支援(かな入力時)の詳細設定 */
#define SCE_ATOK_CONFIG_AUTORECREP      (0x04000000UL)	/* 入力支援(置換)の詳細設定 */
#define SCE_ATOK_CONFIG_APPENDATOHEN    (0x08000000UL)	/* 後変換候補の候補リストへの追加をする/しない */
#define SCE_ATOK_CONFIG_KUTOUCNVMODE    (0x10000000UL)	/* 句読点変換する/しない */
#define SCE_ATOK_CONFIG_KUTOUCNVDETAIL  (0x20000000UL)	/* 句読点変換の詳細設定 */
#define SCE_ATOK_CONFIG_RULEDZENCNV     (0x40000000UL)	/* 全角変換例外規則の設定 */


/* 全ての設定をを一括して指定する場合に使用 */
#define SCE_ATOK_CONFIG_ALL  ( \
					SCE_ATOK_CONFIG_KANJIINPUT      | \
					SCE_ATOK_CONFIG_KANDSPTYPE      | \
					SCE_ATOK_CONFIG_FIXMODE         | \
					SCE_ATOK_CONFIG_HZCNVMODE       | \
					SCE_ATOK_CONFIG_HZCNVDETAIL     | \
					SCE_ATOK_CONFIG_HANKAKUCNVSTR   | \
					SCE_ATOK_CONFIG_ZENKAKUCNVSTR   | \
					SCE_ATOK_CONFIG_FURIGANAMODE    | \
					SCE_ATOK_CONFIG_MAXINPUTCHARS   | \
					SCE_ATOK_CONFIG_PUTCURSORMODE   | \
					SCE_ATOK_CONFIG_PREANALYZE      | \
					                                  \
					SCE_ATOK_CONFIG_CONVMODE        | \
					SCE_ATOK_CONFIG_LEARNINGMODE    | \
					SCE_ATOK_CONFIG_KUTOUCHARSET    | \
					SCE_ATOK_CONFIG_OKURIMODE       | \
					SCE_ATOK_CONFIG_AUTOREGKIND     | \
					SCE_ATOK_CONFIG_LEARNINGKIND    | \
					SCE_ATOK_CONFIG_ROMANRECOVER    | \
					SCE_ATOK_CONFIG_AUTORECMODE     | \
					SCE_ATOK_CONFIG_AUTORECROMAN    | \
					SCE_ATOK_CONFIG_AUTORECKANA     | \
					SCE_ATOK_CONFIG_AUTORECREP      | \
					SCE_ATOK_CONFIG_APPENDATOHEN    | \
					SCE_ATOK_CONFIG_KUTOUCNVMODE    | \
					SCE_ATOK_CONFIG_KUTOUCNVDETAIL  | \
					SCE_ATOK_CONFIG_RULEDZENCNV       \
					)



/* sceAtokConfig構造体内のメンバ設定用マクロ定義
*/
/* SCE_ATOK_CONFIG_KANJIINPUT : 漢字入力モード */
#define SCE_ATOK_KANJIINPUT_ROMAN      (1)		/* ローマ字漢字 */
#define SCE_ATOK_KANJIINPUT_KANA       (2)		/* かな入力漢字 */
#define SCE_ATOK_KANJIINPUT_HANTHROUGH (3)		/* 半角スルー   */
/* SCE_ATOK_CONFIG_KANDSPTYPE : 入力文字種*/
#define SCE_ATOK_KANDSPTYPE_HIRA      (1)		/* ひらがな入力     */
#define SCE_ATOK_KANDSPTYPE_ZENKANA   (2)		/* 全角カタカナ入力 */
#define SCE_ATOK_KANDSPTYPE_ZENRAW    (3)		/* 全角無変換入力   */
#define SCE_ATOK_KANDSPTYPE_HANKANA   (4)		/* 半角カタカナ入力 */
#define SCE_ATOK_KANDSPTYPE_HANRAW    (5)		/* 半角無変換入力   */
/* SCE_ATOK_CONFIG_CONVMODE : 変換モード*/
#define SCE_ATOK_CONVMODE_REN         (0)		/* 連文節変換 */
#define SCE_ATOK_CONVMODE_TAN         (1)		/* 単文節変換 */
#define SCE_ATOK_CONVMODE_AUTO        (2)		/* 自動変換   */
/* SCE_ATOK_CONFIG_LEARNINGMODE : 学習モード*/
#define SCE_ATOK_LEARNINGMODE_OFF     (0)		/* 学習しない */
#define SCE_ATOK_LEARNINGMODE_ON      (1)		/* 学習する   */
/* SCE_ATOK_CONFIG_FIXMODE : 固定入力モード*/
#define SCE_ATOK_FIXMODE_OFF          (0)		/* 固定解除     */
#define SCE_ATOK_FIXMODE_HIRAGANA     (1)		/* ひらがな     */
#define SCE_ATOK_FIXMODE_ZENKANA      (2)		/* 全角カタカナ */
#define SCE_ATOK_FIXMODE_ZENRAW       (3)		/* 全角無変換   */
#define SCE_ATOK_FIXMODE_HANKANA      (4)		/* 半角カタカナ */
#define SCE_ATOK_FIXMODE_HANRAW       (5)		/* 半角無変換   */
/* SCE_ATOK_CONFIG_KUTOUCHARSET : 句読点組み合わせパターン*/
#define SCE_ATOK_KUTOUCHARSET_SET0    (0)		/* カンマ/ ピリオド/ 中括弧/ スラッシュ/ 音引き*/
#define SCE_ATOK_KUTOUCHARSET_SET1    (1)		/* 読点/ 句点/ かぎ括弧/ 中黒/ 音引き */
/* SCE_ATOK_CONFIG_AUTOREGKIND : 自動登録種別*/
#define SCE_ATOK_AUTOREGKIND_CLAUSEGAP    (0x00000001)	/* 文節区切り*/
#define SCE_ATOK_AUTOREGKIND_MULTI        (0x00000002)	/* 複合語*/
#define SCE_ATOK_AUTOREGKIND_CNVRAW       (0x00000004)	/* 後変換*/
#define SCE_ATOK_AUTOREGKIND_NONREG       (0x00000008)	/* 未登録語*/
/* SCE_ATOK_CONFIG_LEARNINGKIND : 学習種別*/
#define SCE_ATOK_LEARNINGKIND_CLAUSEGAP   (0x00000001)	/* 区切りなおし学習語*/
#define SCE_ATOK_LEARNINGKIND_MULTI       (0x00000002)	/* 複合語学習語*/
#define SCE_ATOK_LEARNINGKIND_CNVRAW      (0x00000004)	/* 後変換後*/
#define SCE_ATOK_LEARNINGKIND_NONREG      (0x00000008)	/* 未登録語*/
/* SCE_ATOK_CONFIG_ROMANRECOVER : ローマ字立ち直りする/しない */
#define SCE_ATOK_ROMANRECOVER_OFF         (0)			/* しない*/
#define SCE_ATOK_ROMANRECOVER_ON          (1)			/* する*/
/* SCE_ATOK_CONFIG_HZCNVMODE : 半角全角変換する/しない */
#define SCE_ATOK_HZCNVMODE_CNVHANZEN      (1<<0)		/* 半角全角変換*/
#define SCE_ATOK_HZCNVMODE_INPUTCHARS     (1<<1)		/* 入力中文字列への適用*/
#define SCE_ATOK_HZCNVMODE_USERREG        (1<<2)		/* ユーザ登録単語への適用*/
#define SCE_ATOK_HZCNVMODE_RAWCNV         (1<<3)		/* 後変換単語への適用*/
/* SCE_ATOK_CONFIG_HZCNVDETAIL : 半角全角変換詳細 */
#define SCE_ATOK_HZCNVDETAIL_KANA2HAN     (1<<0)		/* カタカナ → 半角 */
#define SCE_ATOK_HZCNVDETAIL_ALPH2HAN     (1<<1)		/* 英字     → 半角 */
#define SCE_ATOK_HZCNVDETAIL_NUM2HAN      (1<<2)		/* 数字     → 半角 */
#define SCE_ATOK_HZCNVDETAIL_KANA2ZEN     (1<<4)		/* カタカナ → 全角 */
#define SCE_ATOK_HZCNVDETAIL_ALPH2ZEN     (1<<5)		/* 英字     → 全角 */
#define SCE_ATOK_HZCNVDETAIL_NUM2ZEN      (1<<6)		/* 数字     → 全角 */
/* SCE_ATOK_CONFIG_AUTORECMODE : 入力支援する/しない */
#define SCE_ATOK_AUTORECMODE_OFF            (0) 		/* しない */
#define SCE_ATOK_AUTORECMODE_ON             (1) 		/* する   */
/* SCE_ATOK_CONFIG_AUTORECROMAN : 入力支援（ローマ字入力時）の詳細設定/読み出し*/
#define SCE_ATOK_AUTORECROMAN_BOIN          (1<<0)		/* 母音の過不足 */
#define SCE_ATOK_AUTORECROMAN_SHIIN         (1<<1)		/* 子音の超過   */
#define SCE_ATOK_AUTORECROMAN_N             (1<<2)		/* [N]の過不足  */
/* SCE_ATOK_CONFIG_AUTORECKANA : 入力支援(かな入力時)の詳細設定 */
#define SCE_ATOK_AUTORECKANA_HANDAKU2DAKU   (1<<0)		/* 「ﾟ」 →「゛」(半濁点を濁点)  */
#define SCE_ATOK_AUTORECKANA_TSU            (1<<1)		/* 「つ」→「っ」(「つ」を拗音に)*/
#define SCE_ATOK_AUTORECKANA_WA2WO          (1<<2)		/* 「わ」→「を」*/
/* SCE_ATOK_CONFIG_AUTORECREP : 入力支援(置換)の詳細設定 */
#define SCE_ATOK_AUTORECREP_ONBIKI2HYPHEN   (1<<0)		/* 「ー」→「-」(音引きをハイフンに)*/
#define SCE_ATOK_AUTORECREP_NAKAGURO2SLASH  (1<<1)		/* 「・」→「/」(中黒をスラッシュに)*/
#define SCE_ATOK_AUTORECREP_DOKUTEN2COMMA   (1<<2)		/* 「、」→「,」(読点をカンマに)    */
#define SCE_ATOK_AUTORECREP_KUTEN2PERIOD    (1<<3)		/* 「。」→「.」(句点をピリオドに)  */
/* SCE_ATOK_CONFIG_APPENDATOHEN : 後変換候補の候補リストへの追加をする/しない */
#define SCE_ATOK_APPENDATOHEN_HIRAGANACAND    (1<<0)	/* ひらがな候補     */
#define SCE_ATOK_APPENDATOHEN_ZENKATAKANACAND (1<<1)	/* 全角カタカナ候補 */
#define SCE_ATOK_APPENDATOHEN_HANKATAKANACAND (1<<2)	/* 半角カタカナ候補 */
#define SCE_ATOK_APPENDATOHEN_ZENCNVRAWCAND   (1<<3)	/* 全角無変換候補   */
#define SCE_ATOK_APPENDATOHEN_HANCNVRAWCAND   (1<<4)	/* 半角無変換候補   */
/* SCE_ATOK_CONFIG_KUTOUCNVMODE : 句読点変換する/しない */
#define SCE_ATOK_KUTOUCNVMOD_OFF            (0)  		/* しない */
#define SCE_ATOK_KUTOUCNVMOD_ON             (1)  		/* する   */
/* SCE_ATOK_CONFIG_KUTOUCNVDETAIL : 句読点変換の詳細設定 */
#define SCE_ATOK_KUTOUCNVDETAIL_KUTEN       (1<<0)		/* 。(句点)で変換   */
#define SCE_ATOK_KUTOUCNVDETAIL_DOKUTEN     (1<<1)		/* 、(読点)で変換   */
#define SCE_ATOK_KUTOUCNVDETAIL_GIMONFU     (1<<2)		/* ？(疑問符)で変換 */
#define SCE_ATOK_KUTOUCNVDETAIL_KANTANFU    (1<<3)		/* ！(感嘆符)で変換 */
/* SCE_ATOK_CONFIG_FURIGANAMODE : ふりがな入力モード */
#define SCE_ATOK_FURIGANAMODE_FINISH        (0)  		/* 終了 */
#define SCE_ATOK_FURIGANAMODE_START         (1)  		/* 開始 */
/* SCE_ATOK_CONFIG_PUTCURSORMODE : 未確定文字列末尾にカーソル文字を追加する/しない */
#define SCE_ATOK_PUTCURSORMODE_OFF          (0)  		/* しない */
#define SCE_ATOK_PUTCURSORMODE_ON           (1)  		/* する   */
/* SCE_ATOK_CONFIG_PREANALYZE : 入力時の辞書先読みする/しない */
#define SCE_ATOK_PREANALYZE_OFF             (0)
#define SCE_ATOK_PREANALYZE_ON              (1)
/* SCE_ATOK_CONFIG_OKURIMODE : 送りがなモード */
#define SCE_ATOK_OKURIMODE_HONSOKU          (0)  		/* 0= 本則(正しい日本語) */
#define SCE_ATOK_OKURIMODE_HABUKU           (1)  		/* 1= 省く */
#define SCE_ATOK_OKURIMODE_OKURU            (2)  		/* 2= 送る */
/* SCE_ATOK_CONFIG_RULEDZENCNV : 全角変換例外規則の設定 */
#define SCE_ATOK_RULEDZENCNV_TILDE          (1<<0)		/* 「~」→「～」*/



/* 編集用キーコードの値を定義
 * sceAtokEditConv()関数の引数として使用します
 */
#define SCE_ATOK_EDIT_BACKSPACE     (0x0100)	/* 左文字1文字削除(BACKSPACE) */
#define SCE_ATOK_EDIT_DELETE        (0x0101)	/* カーソル位置の1文字削除(DELETE) */
#define SCE_ATOK_EDIT_CURRIGHT      (0x0102)	/* カーソル移動(右) */
#define SCE_ATOK_EDIT_CURLEFT       (0x0103)	/* カーソル移動(左) */
#define SCE_ATOK_EDIT_CUREND        (0x0104)	/* カーソル移動(文末) */
#define SCE_ATOK_EDIT_CURTOP        (0x0105)	/* カーソル移動(文頭) */
#define SCE_ATOK_EDIT_CONVFORWARD   (0x0107)	/* 変換、次候補順次取得 */
#define SCE_ATOK_EDIT_CONVBACKWARD  (0x0108)	/* 変換、次候補順次取得 */
#define SCE_ATOK_EDIT_1ONKAKUTEI    (0x010A)	/* 1音確定 */
#define SCE_ATOK_EDIT_POSTCONVHIRA  (0x010B)	/* 後変換:ひらがな */
#define SCE_ATOK_EDIT_POSTCONVKANA  (0x010C)	/* 後変換:カタカナ */
#define SCE_ATOK_EDIT_POSTCONVHALF  (0x010D)	/* 後変換:半角 */
#define SCE_ATOK_EDIT_POSTCONVRAW   (0x010E)	/* 後変換:無変換 */
#define SCE_ATOK_EDIT_DICCONV       (0x010F)	/* 辞書指定変換 */
#define SCE_ATOK_EDIT_KAKUTEIPART   (0x0110)	/* 部分確定 */
#define SCE_ATOK_EDIT_KAKUTEIALL    (0x0111)	/* 全文確定 */
#define SCE_ATOK_EDIT_DELETEALL     (0x0112)	/* 全文字削除 */
#define SCE_ATOK_EDIT_KAKUTEIREP    (0x0113)	/* 確定リピート */
#define SCE_ATOK_EDIT_KAKUTEIUNDO   (0x0114)	/* 確定アンドゥ */
#define SCE_ATOK_EDIT_KAKUTEIHEAD   (0x0115)	/* 文頭文字確定 */
#define SCE_ATOK_EDIT_KAKUTEITAIL   (0x0116)	/* 文末文字確定 */
#define SCE_ATOK_EDIT_CONVCANCEL    (0x0117)	/* 変換キャンセル(注目文節以降) */
#define SCE_ATOK_EDIT_CANCELALL     (0x0118)	/* 全変換取り消し */
#define SCE_ATOK_EDIT_FOCUSRIGHT    (0x0119)	/* 注目文節区切りを1文字のばす */
#define SCE_ATOK_EDIT_FOCUSLEFT     (0x011A)	/* 注目文節区切りを1文字縮める */
#define SCE_ATOK_EDIT_MOVFOCUSCLAUS (0x011B)	/* 注目文節移動 文節区切り直し中には使えません */
#define SCE_ATOK_EDIT_SELECTCAND    (0x011E)	/* 候補選択移動 */


/* ATOK キー入力状態の定義
 * sceAtokGetInputState()関数の戻り値
 */
#define SCE_ATOK_ISTATE_BEFOREINPUT     (0)		/* 未入力 */
#define SCE_ATOK_ISTATE_BEFORECONVERT   (1)		/* 変換前 */
#define SCE_ATOK_ISTATE_CONVERTING      (2)		/* 変換中 */
#define SCE_ATOK_ISTATE_CANDEMPTY       (3)		/* 候補切れ(候補なし) */
#define SCE_ATOK_ISTATE_POSTCONVKANA    (4)		/* 後変換(カタカナ) */
#define SCE_ATOK_ISTATE_POSTCONVHALF    (5)		/* 後変換(半角) */
#define SCE_ATOK_ISTATE_POSTCONVRAW     (6)		/* 後変換(無変換) */
#define SCE_ATOK_ISTATE_CANDIDATES      (7)		/* 候補一覧 */
#define SCE_ATOK_ISTATE_MOVECLAUSEGAP   (8)		/* 文節区切り直し */




/* プロトタイプ宣言 */
/*erx化用*/
void *sceAtokGetErxEntries(void);

/* ATOKの初期化 */
int sceAtokInit(sceAtokContext *context, const char *sysdic, const char *userdic ,const char *sysconf, const void *loadaddr);
/* ATOKのリセット */
int sceAtokReset(sceAtokContext *context);
/* ATOKの解放 */
int sceAtokExit(sceAtokContext *context);

/* ATOKの各種コンフィギュレーションの読み出しおよび設定 */
int sceAtokGetConfig(sceAtokContext *context, sceAtokConfig *config, unsigned long bits);
int sceAtokSetConfig(sceAtokContext *context, const sceAtokConfig *config, unsigned long bits);


/* ATOKにコマンドを発行する関数 */
int sceAtokEditConv(sceAtokContext *context, int cmd, int param);  /* 編集,変換処理 */
int sceAtokKanjiOn(sceAtokContext *context);                       /* 漢字モードON */
int sceAtokKanjiOff(sceAtokContext *context);                      /* 漢字モードOFF */
int sceAtokUndoClear(sceAtokContext *context);                     /* 確定アンドゥの無効化 */
int sceAtokConvAll(sceAtokContext *context, const char *str, int mode); /* 一括読み変換処理 */


/* ATOKコンテキスト内のワーク情報を取得する関数 */
int sceAtokGetKanjiState(sceAtokContext *context);     /* ATOK主状態を取得 */
int sceAtokGetInputState(sceAtokContext *context);     /* 入力状態を取得   */

int sceAtokGetFocusClauseTop(sceAtokContext *context);    /* フォーカス(カーソル)先頭位置の取得 */
int sceAtokGetFocusClauseLen(sceAtokContext *context);    /* フォーカス選択バイト数の取得(0:なし) */
int sceAtokGetFocusClauseIndex(sceAtokContext *context);  /* フォーカスがある文節番号を得る */

int sceAtokGetCurrentCandidateIndex(sceAtokContext *context); /* 現在の選択候補番号       */
int sceAtokGetCandidateCount(sceAtokContext *context);        /* 候補リストを取得         */
int sceAtokGetCandidateListSize(sceAtokContext *context);     /* 候補リストのサイズを取得 */
int sceAtokGetCandidateList(sceAtokContext *context, char *buf, int size); /* 候補リストの取得 */


/* 確定文字列と確定読み文字列をフラッシュする */
int sceAtokFlushConverted(sceAtokContext *context);
/* 確定文字列の文をバッファに受け取る */
int sceAtokGetConvertedStr(sceAtokContext *context, char *buf, unsigned int size);
/* 確定読み文字列の文をバッファに受け取る */
int sceAtokGetConvertedReadStr(sceAtokContext *context, char *buf, unsigned int size);

/* 未確定文字列の文をバッファに受け取る */
int sceAtokGetConvertingStr(sceAtokContext *context, char *buf, unsigned int size);
/* 未定読み文字列の文をバッファに受け取る */
int sceAtokGetConvertingReadStr(sceAtokContext *context, char *buf, unsigned int size);

/* 未確定文字列の文節数を取得 */
int sceAtokGetConvertingClauseCount(sceAtokContext *context);
/* 未確定文字列を文節ごとに取得 */
int sceAtokGetConvertingClause(sceAtokContext *context, char *buf, unsigned int size, int index );

int sceAtokDicDelWord(sceAtokContext *p, int p1, int p2, const char *pszWord, const char *pszReading, int iMode);
int sceAtokFlushDic(sceAtokContext *context);
int sceAtokDicExpand(sceAtokContext *p, void *work, size_t size);
int sceAtokDicRegWord(sceAtokContext *p, int iHinshi, const char *pszWord, const char *pszReading);
int sceAtokDicRegWordAndExpand(sceAtokContext *p, int iHinshi, const char *pszWord, const char *pszReading, void *work, size_t size);

#if defined(__LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
}
#endif	/* defined(__LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus) */

#endif	/* _LIBATOK_H_ */

