/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 $Id: hd.h,v 1.21 2003/09/12 05:23:13 tokiwa Exp $
 */
/*
 * Copyright (C) 1999, 2000, 2002, 2003 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * format/sound/hd.h - .hd file format
 *
 *	See "sformat.pdf" or converted HTML file for more details.
 */

#ifndef _SCE_FORMAT_SOUND_HD_H
#define _SCE_FORMAT_SOUND_HD_H

/*
 * typedef char			S8;	  8bit   signed
 * typedef unsigned char	U8;	  8bit unsigned
 * typedef short		S16;	 16bit   signed
 * typedef unsigned short	U16;	 16bit unsigned
 * typedef int			S32;	 32bit   signed
 * typedef unsigned int		U32;	 32bit unsigned
 *
 *  ファイルオフセット(ファイルの先頭からのバイト数)
 * typedef U32 FOFST;
 * #define NO_FOFST ((FOFST)-1)
 *  チャンクオフセット(チャンクの先頭からのバイト数)
 * typedef U32 COFST;
 * #define NO_COFST ((COFST)-1)
 *  チャンク内のオフセット(該当チャンクの先頭からのbyte数)
 * typedef U32 SOFST;
 * #define NO_SOFST ((SOFST)-1)
 *  ポジションオフセット(チャンク内の現在の位置からのバイト数)
 * typedef U32 POFST;
 * #define NO_POFST ((POFST)-1)
 *  チャンク内の現在の位置からのオフセット
 * typedef U32 TOFST;
 * #define NO_TOFST ((TOFST)-1)
 *  チャンクインデックス(チャンク内の構造体へのインデックス番号)
 * typedef U16 CINDEX;
 * #define NO_CINDEX ((CINDEX)-1)
 *  チャンク内の構造体へのインデックスナンバー
 * typedef U16 SINDEX;
 * #define NO_SINDEX ((SINDEX)-1)
 */

/*
 * 以下の構造体メンバで利用:
 *	sceHardSynthProgramParam.progAttr
 *	sceHardSynthSeTimbreNoteBlock.seTimbreNoteAttr
 */
#define SCEHD_ROUND_PAN			0x01	/* パンポットの相対変化において逆相も可	*/
#define SCEHD_REVERSE_PHASE_VOL		0x02	/* 逆相ボリューム指定			*/

/*
 * 以下の構造体メンバで利用:
 *	sceHardSynthProgramParam.progLfoWave
 *	sceHardSynthProgramParam.progLfoWave2
 *	sceHardSynthSeTimbreNoteBlock.seTimbreNotePitchLfoWave
 *	sceHardSynthSeTimbreNoteBlock.seTimbreNoteAmpLfoWave
 */
#define SCEHD_LFO_NON			0	/* LFO なし			*/
#define SCEHD_LFO_SAWUP			1	/* 右上がり鋸波			*/
#define SCEHD_LFO_SAWDOWN		2	/* 右下がり鋸波			*/
#define SCEHD_LFO_TRIANGLE		3	/* 三角波			*/
#define SCEHD_LFO_SQUEARE		4	/* 矩形波			*/
#define SCEHD_LFO_NOISE			5	/* ノイズ			*/
#define SCEHD_LFO_SIN			6	/* 正弦波			*/
#define SCEHD_LFO_USER			0x80	/* ユーザ定義			*/

/*
 * 以下の構造体メンバで利用:
 *	sceHardSynthSampleParam.sampleLfoAttr
 *	sceHardSynthSeTimbreNoteBlock.seTimbreNoteLfoAttr
 */
/* PITCH LFO */
#define SCEHD_LFO_PITCH_KEYON		0x01	/* トリガーがキーオン時			*/
#define SCEHD_LFO_PITCH_KEYOFF		0x02	/* トリガーがキーオフ時			*/
#define SCEHD_LFO_PITCH_BOTH		0x04	/* トリガーがキーオンとキーオフ時	*/
/* Amp. LFO */
#define SCEHD_LFO_AMP_KEYON		0x10	/* トリガーがキーオン時			*/
#define SCEHD_LFO_AMP_KEYOFF		0x20	/* トリガーがキーオフ時			*/
#define SCEHD_LFO_AMP_BOTH		0x40	/* トリガーがキーオンとキーオフ時	*/

/*
 * 以下の構造体メンバで利用:
 *	sceHardSynthVagParam.vagAttribute
 */
#define SCEHD_VAG_1SHOT			0x00	/* No loop = 1 shot	*/
#define SCEHD_VAG_LOOP			0x01	/* Loop			*/

/*
 * 以下の構造体メンバで利用:
 *	sceHardSynthSplitBlock.splitRangeLow
 *	sceHardSynthSplitBlock.splitRangeHigh
 *	sceHardSynthSampleParam.velRangeLow
 *	sceHardSynthSampleParam.velRangeHigh
 */
#define SCEHD_CROSSFADE			0x80	/* クロスフェードの指定 */

/*
 * 以下の構造体メンバで利用:
 *	sceHardSynthSampleParam.sampleSpuAttr
 *	sceHardSynthSeTimbreNoteBlock.seTimbreNoteSpuAttr
 */
#define SCEHD_SPU_DIRECTSEND_L		0x01	/* SPU2 [VMIXL0/1] レジスタ	*/
#define SCEHD_SPU_DIRECTSEND_R		0x02	/* SPU2 [VMIXR0/1] レジスタ	*/
#define SCEHD_SPU_EFFECTSEND_L		0x04	/* SPU2 [VMIXEL0/1] レジスタ	*/
#define SCEHD_SPU_EFFECTSEND_R		0x08	/* SPU2 [VMIXER0/1] レジスタ	*/
#define SCEHD_SPU_CORE_0		0x10	/* SPU2 CORE0選択		*/
#define SCEHD_SPU_CORE_1		0x20	/* SPU2 CORE1選択		*/

/*
 * 以下の構造体メンバで利用:
 *	sceHardSynthSampleSetParam.velCurve
 *	sceHardSynthSeTimbreNoteBlock.seTimbreNoteVelCurve
 */
#define SCEHD_VEL_CURVE_B		0	/* B カーブ	*/
#define SCEHD_VEL_CURVE_B_REV		1
#define SCEHD_VEL_CURVE_A		2	/* A カーブ	*/
#define SCEHD_VEL_CURVE_A_REV		3
#define SCEHD_VEL_CURVE_C		4	/* C カーブ	*/
#define SCEHD_VEL_CURVE_C_REV		5

/*
 * 以下の構造体メンバで利用:
 *	sceHardSynthSampleParam.velFollowPitchVelCurve
 *	sceHardSynthSampleParam.velFollowAmpVelCurve
 *	sceHardSynthSeTimbreNoteBlock.seTimbreNoteVelFollowPitchCurve
 *	sceHardSynthSeTimbreNoteBlock.seTimbreNoteVelFollowAmpCurve
 */
#define SCEHD_VELFOLLOW_CURVE_B		0	/* B カーブ	*/
#define SCEHD_VELFOLLOW_CURVE_B_REV	1
#define SCEHD_VELFOLLOW_CURVE_A		2	/* A カーブ	*/
#define SCEHD_VELFOLLOW_CURVE_A_REV	3
#define SCEHD_VELFOLLOW_CURVE_C		4	/* C カーブ	*/
#define SCEHD_VELFOLLOW_CURVE_C_REV	5
#define SCEHD_VELFOLLOW_CURVE_BB	6	/* 折れ線	*/
#define SCEHD_VELFOLLOW_CURVE_BB_REV	7
#define SCEHD_VELFOLLOW_CURVE_CA	8	/* C+A カーブ	*/
#define SCEHD_VELFOLLOW_CURVE_AC	9	/* A+C カーブ	*/

/****************************************************************
 * VAGInfo Chunk
 ****************************************************************/
typedef struct {
    unsigned int   vagOffsetAddr;	/*FOFST 波形データの位置		*/
    unsigned short vagSampleRate;	/*U16 サンプリングレート		*/
    unsigned char  vagAttribute;	/*U8 ループフラグ			*/
    unsigned char  dmy;			/*U8 予約領域				*/
} sceHardSynthVagParam;

typedef struct {
    unsigned int   Creator;		/*U32 作成者コード:   'SCEI'		*/
    unsigned int   Type;		/*U32 チャンクタイプ: 'Vagi'		*/
    unsigned int   chunkSize;		/*U32 チャンクサイズ			*/
    unsigned int   maxVagInfoNumber;	/*U32 VagInfoナンバーの最大値		*/

    unsigned int   vagInfoOffsetAddr[0];/*COFST VagInfoテーブルへのオフセット	*/
    /* sceHardSynthVagParam vagInfoParam[] */
} sceHardSynthVagInfoChunk;

/****************************************************************
 * Sample Chunk
 ****************************************************************/
typedef struct {
    unsigned short VagIndex;			/*CINDEX VagInfoChunkでのインデックスナンバー(NO_CINDEX:SPUノイズ) */

    unsigned char  velRangeLow;			/*U8 発音下限ベロシティ(0-127)				*/
    unsigned char  velCrossFade;		/*U8 クロスフェイド開始ベロシティ(0-127)		*/
    unsigned char  velRangeHigh;		/*U8 発音上限ベロシティ(0-127)				*/

    char	   velFollowPitch;		/*S8 ベロシティによるピッチLFOの変化幅(注1)(0-127)	*/
    unsigned char  velFollowPitchCenter;	/*U8 velFollowPitchの効果が0になるベロシティ(0-127)	*/
    unsigned char  velFollowPitchVelCurve;	/*U8 ピッチLFOに対するベロシティカーブ			*/

    char	   velFollowAmp;		/*S8 ベロシティによるアンプLFOの変化幅			*/
    unsigned char  velFollowAmpCenter;		/*U8 velFollowAmpの効果が0になるベロシティ(0-127)	*/
    unsigned char  velFollowAmpVelCurve;	/*U8 アンプLFOに対するベロシティカーブ			*/

    unsigned char  sampleBaseNote;		/*U8 ベースとなるノートナンバー(0-127)			*/
    char	   sampleDetune;		/*S8 ディチューン(注1)(0-127)				*/
    char	   samplePanpot;		/*S8 パンポット(0-64-127=左-中央-右;負は逆相)		*/
    unsigned char  sampleGroup;			/*U8 グループ(0-127;0はグループなし)			*/
    unsigned char  samplePriority;		/*U8 プライオリティ(127でプライオリティ最大)		*/
    unsigned char  sampleVolume;		/*U8 ボリューム(0-128)					*/
    unsigned char  dmy;				/*U8 予約領域						*/

    unsigned short sampleAdsr1;			/*U16 エンベロープ ADSR1				*/
    unsigned short sampleAdsr2;			/*U16 エンベロープ ADSR2				*/

    char	   keyFollowAr;			/*S8 ARの1オクターブ毎の変化幅				*/
    unsigned char  keyFollowArCenter;		/*U8 keyFollowArの効果が0になるノート(0-127)		*/
    char	   keyFollowDr;			/*S8 DRの1オクターブ毎の変化幅				*/
    unsigned char  keyFollowDrCenter;		/*U8 keyFollowDrの効果が0になるノート(0-127)		*/
    char	   keyFollowSr;			/*S8 SRの1オクターブ毎Rの変化幅				*/
    unsigned char  keyFollowSrCenter;		/*U8 keyFollowSrの効果が0になるノート(0-127)		*/
    char	   keyFollowRr;			/*S8 RRの1オクターブ毎の変化幅				*/
    unsigned char  keyFollowRrCenter;		/*U8 keyFollowRrの効果が0になるノート(0-127)		*/
    char	   keyFollowSl;			/*S8 SLの1オクターブ毎の変化幅				*/
    unsigned char  keyFollowSlCenter;		/*U8 keyFollowSlの効果が0になるノート(0-127)		*/

    unsigned short samplePitchLfoDelay;		/*U16 Pitch LFOディレイタイム(単位:msec)		*/
    unsigned short samplePitchLfoFade;		/*U16 Pitch LFOフェイドタイム(単位:msec)		*/
    unsigned short sampleAmpLfoDelay;		/*U16 Amp LFOディレイタイム(単位:msec)			*/
    unsigned short sampleAmpLfoFade;		/*U16 Amp LFOフェイドタイム(単位:msec)			*/

    unsigned char  sampleLfoAttr;		/*U8 LFOに関するAttribute				*/
    unsigned char  sampleSpuAttr;		/*U8 SPUに関するAttribute				*/
} sceHardSynthSampleParam;
/* 注1: 単位: 半音を128等分 */

typedef struct {
    unsigned int   Creator;			/*U32 作成者コード:   'SCEI'				*/
    unsigned int   Type;			/*U32 チャンクタイプ: 'Smpl'				*/
    unsigned int   chunkSize;			/*U32 チャンクサイズ					*/
    unsigned int   maxSampleNumber;		/*U32 サンプルナンバーの最大値				*/

    unsigned int   sampleOffsetAddr[0];		/*COFST サンプルテーブルへのオフセット			*/
    /* sceHardSynthSampleRaram sampleParam[] */
} sceHardSynthSampleChunk;

/****************************************************************
 * Sample Set Chunk
 ****************************************************************/
typedef struct {
    unsigned char  velCurve;			/*U8 ベロシティカーブタイプ(for AMP LEVEL)		*/
    unsigned char  velLimitLow;			/*U8 発音上限ベロシティ					*/
    unsigned char  velLimitHigh;		/*U8 発音下限ベロシティ					*/
    unsigned char  nSample;			/*U8 このサンプルセットに属するサンプルの個数		*/

    unsigned short sampleIndex[0];		/*CINDEX サンプルチャンクでのインデックスナンバー	*/
} sceHardSynthSampleSetParam;

typedef struct {
    unsigned int   Creator;			/*U32 作成者コード:   'SCEI'				*/
    unsigned int   Type;			/*U32 チャンクタイプ: 'Sset'				*/
    unsigned int   chunkSize;			/*U32 チャンクサイズ					*/
    unsigned int   maxSampleSetNumber;		/*U32 サンプルセットナンバーの最大値			*/

    unsigned int   sampleSetOffsetAddr[0];	/*COFST サンプルセットテーブルへのオフセット		*/
    /* sceHardSynthSampleSetParam sampleSetParam[] */
} sceHardSynthSampleSetChunk;

/****************************************************************
 * Program Chunk
 ****************************************************************/
typedef struct {
    unsigned short sampleSetIndex;		/*CINDEX サンプルセットチャンクでのインデックスナンバー	*/

    unsigned char  splitRangeLow;		/*U8 発音下限ノートナンバー(0-127)			*/
    unsigned char  splitCrossFade;		/*U8 クロスフェイド開始ノートナンバー(0-127)		*/
    unsigned char  splitRangeHigh;		/*U8 発音上限ノートナンバー(0-127)			*/
    unsigned char  splitNumber;			/*U8 スプリットナンバー(0-127)				*/

    unsigned short splitBendRangeLow;		/*U16 ベンドレンジ-(注1)				*/
    unsigned short splitBendRangeHigh;		/*U16 ベンドレンジ+(注1)				*/

    char	   keyFollowPitch;		/*S8 1オクターブ毎に変化するピッチLFOの変化幅(注1)	*/
    unsigned char  keyFollowPitchCenter;	/*U8 keyFollowPitch による効果が0になるノート(0-127)	*/
    char	   keyFollowAmp;		/*S8 1オクターブ毎に変化するアンプLFOの変化幅		*/
    unsigned char  keyFollowAmpCenter;		/*U8 keyFollowAmp による効果が0になるノート(0-127)	*/
    char	   keyFollowPan;		/*S8 1オクターブ毎に変化するパンポットの変化幅		*/
    unsigned char  keyFollowPanCenter;		/*U8 keyFollowPan による効果が0になるノート(0-127)	*/

    unsigned char  splitVolume;			/*U8 スプリットボリューム(0-128)			*/
    char	   splitPanpot;			/*S8 スプリットパンポット(0-64-127=左-中央-右;負は逆相)	*/
    char	   splitTranspose;		/*S8 トランスポーズ(単位:ノート)			*/
    char	   splitDetune;			/*S8 ディチューン(注1)					*/
} sceHardSynthSplitBlock;
/* 注1: 単位: 半音を128等分 */

typedef struct {
    unsigned int   splitBlockAddr;		/*POFST スプリットブロックへのオフセット		*/

    unsigned char  nSplit;			/*U8 このプログラムに属する splitBlock の個数		*/
    unsigned char  sizeSplitBlock;		/*U8 ひとつの splitBlock のサイズ			*/

    unsigned char  progVolume;			/*U8 プログラムボリューム(0-128)			*/
    char	   progPanpot;			/*S8 プログラムパンポット(0-64-127=左-中央-右;負は逆相)	*/
    char	   progTranspose;		/*S8 トランスポーズ(単位:ノート)			*/
    char	   progDetune;			/*S8 ディチューン(注1)					*/
    char	   keyFollowPan;		/*S8 1オクターブ毎に変化するパンポットの変化幅		*/
    unsigned char  keyFollowPanCenter;		/*U8 keyFollowPan による効果が0になるノート(0-127)	*/
    unsigned char  progAttr;			/*U8 プログラムアトリビュート				*/
    unsigned char  dmy;				/*U8 予約領域						*/

    unsigned char  progLfoWave;			/*U8 LFO波形(ピッチ)					*/
    unsigned char  progLfoWave2;		/*U8 LFO波形(アンプ)					*/
    unsigned char  progLfoStartPhase;		/*U8 LFOスタート位相(ピッチ)(0-255:0-359°)		*/
    unsigned char  progLfoStartPhase2;		/*U8 LFOスタート位相(アンプ)(0-255:0-359°)		*/
    unsigned char  progLfoPhaseRandom;		/*U8 LFOランダムスタート位相(ピッチ)(0-255:0-359°)	*/
    unsigned char  progLfoPhaseRandom2;		/*U8 LFOランダムスタート位相(アンプ)(0-255:0-359°)	*/
    unsigned short progLfoFreq;			/*U16 LFO周期(ピッチ)(単位:msec)			*/
    unsigned short progLfoFreq2;		/*U16 LFO周期(アンプ)(単位:msec)			*/
    short	   progLfoPitchDepth;		/*S16 ピッチモジュレーション振幅+(注1)			*/
    short	   progLfoPitchDepth2;		/*S16 ピッチモジュレーション振幅-(注1)			*/
    short	   progLfoMidiPitchDepth;	/*S16 MIDIピッチモジュレーション最大振幅+(注1)		*/
    short	   progLfoMidiPitchDepth2;	/*S16 MIDIピッチモジュレーション最大振幅-(注1)		*/
    char	   progLfoAmpDepth;		/*S8 アンプモジュレーション振幅+			*/
    char	   progLfoAmpDepth2;		/*S8 アンプモジュレーション振幅-			*/
    char	   progLfoMidiAmpDepth;		/*S8 MIDIモジュレーション最大振幅+			*/
    char	   progLfoMidiAmpDepth2;	/*S8 MIDIモジュレーション最大振幅-			*/

    sceHardSynthSplitBlock splitBlock[0];	/* スプリットブロック (個数分)				*/
} sceHardSynthProgramParam;
/* 注1: 単位: 半音を128等分 */

typedef struct {
    unsigned int   Creator;			/*U32 作成者コード:   'SCEI'		*/
    unsigned int   Type;			/*U32 チャンクタイプ: 'Prog'		*/
    unsigned int   chunkSize;			/*U32 チャンクサイズ			*/
    unsigned int   maxProgramNumber;		/*U32 プログラムナンバーの最大値	*/

    unsigned int   programOffsetAddr[0];	/*COFST プログラムへのオフセットアドレス */
} sceHardSynthProgramChunk;

/****************************************************************
 * SE Timbre Chunk
 ****************************************************************/
typedef struct {
    unsigned short VagIndex;				/*CINDEX VAGInfo チャンクのインデックス		*/

    unsigned char  seTimbreNoteVelCurve;		/*U8 ノートベロシティカーブタイプ(0-5)		*/
    unsigned char  seTimbreNoteVolume;			/*U8 ノートボリューム(0-128)			*/
    char	   seTimbreNotePanpot;			/*S8 ノートパンポット(0-64-127=左-中央-右;負は逆相) */
    char	   seTimbreNoteTranspose;		/*S8 ノートトランスポーズ(単位ノート)		*/
    char	   seTimbreNoteDetune;			/*S8 ノートディチューン(注1)			*/
    unsigned char  seTimbreNoteGroup;			/*U8 ノートグループ(0-127;0はグループなし)	*/
    unsigned char  seTimbreNoteGroupLimit;		/*U8 ノートグループ内発音数リミット(0-48;0はリミットなし) */
    unsigned char  seTimbreNotePriority;		/*U8 ノートプライオリティ			*/
    unsigned char  dmy0;				/*U8 予約領域					*/
    unsigned char  dmy1;				/*U8 予約領域					*/
    unsigned char  seTimbreNoteAttr;			/*U8 ノートアトリビュート			*/
    unsigned char  seTimbreNoteSpuAttr;			/*U8 SPU Attribute				*/
    unsigned short seTimbreNoteAdsr1;			/*U16 ADSR1					*/
    unsigned short seTimbreNoteAdsr2;			/*U16 ADSR2					*/

    unsigned char  seTimbreNotePitchLfoWave;		/*U8  LFO 波形(ピッチ)				*/
    unsigned char  seTimbreNoteAmpLfoWave;		/*U8  LFO 波形(アンプ)				*/
    unsigned char  seTimbreNotePitchLfoStartPhase;	/*U8  LFO スタート位相(ピッチ)(0-255:0-359°)	*/
    unsigned char  seTimbreNoteAmpLfoStartPhase;	/*U8  LFO スタート位相(アンプ)(0-255:0-359°)	*/
    unsigned char  seTimbreNotePitchLfoRandomPhase;	/*U8  LFO ランダムスタート位相(ピッチ)(0-255:0-359°) */
    unsigned char  seTimbreNoteAmpLfoRandomPhase;	/*U8  LFO ランダムスタート位相(アンプ)(0-255:0-359°) */
    unsigned short seTimbreNotePitchLfoCycle;		/*U16 LFO 周期(ピッチ)(単位:msec)		*/
    unsigned short seTimbreNoteAmpLfoCycle;		/*U16 LFO 周期(アンプ)(単位:msec)		*/
    short	   seTimbreNotePitchLfoDepth;		/*S16 LFO 振幅+(ピッチ)(注1)			*/
    short	   seTimbreNotePitchLfoDepth2;		/*S16 LFO 振幅-(ピッチ)(注1)			*/
    char	   seTimbreNoteAmpLfoDepth;		/*S8  LFO 振幅+(アンプ)				*/
    char	   seTimbreNoteAmpLfoDepth2;		/*S8  LFO 振幅-(アンプ)				*/

    unsigned short seTimbreNotePitchLfoDelay;		/*U16 LFO ディレイタイム(単位:msec)(ピッチ)	*/
    unsigned short seTimbreNoteAmpLfoDelay;		/*U16 LFO ディレイタイム(単位:msec)(アンプ)	*/
    unsigned short seTimbreNotePitchLfoFade;		/*U16 LFO フェイドタイム(単位:msec)(ピッチ)	*/
    unsigned short seTimbreNoteAmpLfoFade;		/*U16 LFO フェイドタイム(単位:msec)(アンプ)	*/
    char	   seTimbreNoteVelFollowPitch;		/*S8 ベロシティによるピッチLFOの変化幅(-127..0..127)(注1) */
    char	   seTimbreNoteVelFollowAmp;		/*S8 ベロシティによるアンプLFOの変化幅(-127..0..127)	*/
    unsigned char  seTimbreNoteVelFollowPitchCenter;	/*U8 seTimbreNoteVelFollowPitchの効果が0になるベロシティ(1-127)	*/
    unsigned char  seTimbreNoteVelFollowAmpCenter;	/*U8 seTimbreNoteVelFollowAmpの効果が0になるベロシティ(1-127)	*/
    unsigned char  seTimbreNoteVelFollowPitchCurve;	/*U8 seTimbreNoteVelFollowPitchの効果量を定めるためのベロシティカーブ(0-9) */
    unsigned char  seTimbreNoteVelFollowAmpCurve;	/*U8 seTimbreNoteVelFollowAmpの効果量を定めるためのベロシティカーブ(0-9) */
    unsigned char  dmy;					/*U8 予約領域					*/
    unsigned char  seTimbreNoteLfoAttr;			/*U8 LFO Attribute				*/
} sceHardSynthSeTimbreNoteBlock;
/* 注1: 単位:半音を128等分 */

typedef struct {
    unsigned int   seTimbreNoteBlockAddr;	/*POFST 効果音ティンバーノートの先頭へのオフセット	*/
    unsigned char  seTimbreNoteBlockSize;	/*U8 ひとつの効果音ドラムノートブロックのサイズ		*/
    unsigned char  seTimbreNoteStart;		/*U8 効果音ティンバースタートノートナンバー		*/
    unsigned char  seTimbreNoteEnd;		/*U8 効果音ティンバーエンドノートナンバー		*/
    unsigned char  dmy;				/*U8 予約領域	*/

    sceHardSynthSeTimbreNoteBlock seTimbreNoteBlock[0];	/* 効果音ティンバーノートブロック(個数分)	*/
} sceHardSynthSeTimbreBlock;

typedef struct {
    unsigned int   maxSeTimbreNumber;	/*U32 効果音ティンバーセットに属する
					  効果音ティンバーナンバーの最大値	*/
    unsigned int   seTimbreOffsetAddr[0];/*COFST 効果音ティンバーへのオフセット	*/
} sceHardSynthSeTimbreSetBlock;

typedef struct {
    unsigned int   Creator;		/*U32 作成者コード:   'SCEI'	*/
    unsigned int   Type;		/*U32 チャンクタイプ: 'Setb'	*/
    unsigned int   chunkSize;		/*U32 チャンクサイズ		*/
    unsigned int   maxSeTimbreSetNumber;/*U32 効果音ティンバーセットナンバーの最大値 */

    unsigned int   seTimbreSetOffsetAddr[0];/*COFST 効果音ティンバーセットへのオフセット */
} sceHardSynthSeTimbreChunk;

/****************************************************************
 * Header Chunk
 ****************************************************************/
typedef struct {
    unsigned int   Creator;		/*U32 作成者コード:   'SCEI'	*/
    unsigned int   Type;		/*U32 チャンクタイプ: 'Head'	*/
    unsigned int   chunkSize;		/*U32 チャンクサイズ		*/
    unsigned int   headerSize;		/*U32 ヘッダファイルサイズ	*/
    unsigned int   bodySize;		/*U32 ボディファイルサイズ	*/

    unsigned int   programChunkAddr;	/*FOFST オフセットアドレス: プログラムチャンク		*/
    unsigned int   sampleSetChunkAddr;	/*FOFST オフセットアドレス: サンプルセットチャンク	*/
    unsigned int   sampleChunkAddr;	/*FOFST オフセットアドレス: サンプルチャンク		*/
    unsigned int   vagInfoChunkAddr;	/*FOFST オフセットアドレス: VagInfoチャンク		*/
    unsigned int   seTimbreChunkAddr;	/*FOFST オフセットアドレス: 効果音ティンバーチャンク	*/
} sceHardSynthHeaderChunk;

/****************************************************************
 * Version Chunk
 ****************************************************************/
typedef struct {
    unsigned int   Creator;		/*U32 作成者コード:   'SCEI'	*/
    unsigned int   Type;		/*U32 チャンクタイプ: 'Vers'	*/
    unsigned int   chunkSize;		/*U32 チャンクサイズ		*/
    unsigned short reserved;		/*U16 予約領域			*/
    unsigned char  versionMajor;	/*U8 メジャーバージョン		*/
    unsigned char  versionMinor;	/*U8 マイナーバージョン		*/
} sceHardSynthVersionChunk;

#endif /* _SCE_FORMAT_SOUND_HD_H */
