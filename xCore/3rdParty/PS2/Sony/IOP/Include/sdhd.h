/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 $Id: sdhd.h,v 1.24 2003/09/12 05:21:50 tokiwa Exp $
 */
/* 
 * Sound Data .HD Library
 *
 * Copyright (C) 2002 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 *      Date            Design      Log
 *  ----------------------------------------------------
 *      2002-09-26      toki        initial
 *      2003-03-27      toki        change unload module
 */

#ifndef _SCE_SDHD_H
#define _SCE_SDHD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <format/sound/hd.h>


/*******************************************************************************
 * define
 ******************************************************************************/

#define SCESDHD_NOT_IGNORE       ( 0 )    /*サンプルセット情報のベロシティを無視
                                            しない*/
#define SCESDHD_IGNORE           ( 1 )    /*サンプルセット情報のベロシティを無視
                                            する*/

/*******************************************************************************
 * struct
 ******************************************************************************/

/* ***** ProgramParam ***** */
typedef struct{
    unsigned int   volume;      /*プログラムボリューム    */
    int            panpot;      /*プログラムパンポット    */
    int            transpose;   /*プログラムトランスポーズ*/
    int            detune;      /*プログラムディチューン  */
} SceSdHdProgramCommon;

/*
 * volume     0~128
 * panpot     0~64~127 = 左~真中~右 マイナスは逆相
 * transpose  単位はノート
 * detune     単位は半音の１２８等分
 */


typedef struct{
    int            pan;        /*１オクターブ毎に変化するパンポットの変化幅*/
    unsigned int   panCenter;  /*panの効果が０になるノートナンバー*/
} SceSdHdProgramKeyFollow;


typedef struct{
    unsigned int   wavePitch;         /*LFO波形（ピッチ）※注２*/
    unsigned int   waveAmp;           /*LFO波形（アンプ）※注２*/
    unsigned int   startPhasePitch;   /*LFOスタートフェイズ（ピッチ）※注３*/
    unsigned int   startPhaseAmp;     /*LFOスタートフェイズ（アンプ）※注３*/
    unsigned int   phaseRandomPitch;  /*LFOスタートフェイズランダム（ピッチ）*/
    unsigned int   phaseRandomAmp;    /*LFOスタートフェイズランダム（アンプ）*/
    unsigned int   cyclePitch;        /*LFO周期（ピッチ）*/
    unsigned int   cycleAmp;          /*LFO周期（アンプ）*/
    int            pitchDepthUp;      /*ピッチモジュレーション振幅＋※注１*/
    int            pitchDepthDown;    /*ピッチモジュレーション振幅−※注１*/
    int            midiPitchDepthUp;  /*MIDIピッチモジュレーション振幅＋※注１*/
    int            midiPitchDepthDown;/*MIDIピッチモジュレーション振幅−※注１*/
    int            ampDepthUp;        /*アンプモジュレーション振幅＋*/
    int            ampDepthDown;      /*アンプモジュレーション振幅−*/
    int            midiAmpDepthUp;    /*MIDIモジュレーション振幅＋*/
    int            midiAmpDepthDown;  /*MIDIモジュレーション振幅−*/
} SceSdHdProgramLFO;

/*
 * ※注１：単位は半音の１２８等分
 * ※注２：hd.h参照の事
 *         SCEFORMAT_SOUND_HD_LFO_WAVEFORM_NON
 *         SCEFORMAT_SOUND_HD_LFO_WAVEFORM_SAWUP
 *         SCEFORMAT_SOUND_HD_LFO_WAVEFORM_SAWDOWN
 *         SCEFORMAT_SOUND_HD_LFO_WAVEFORM_TRIANGLE
 *         SCEFORMAT_SOUND_HD_LFO_WAVEFORM_SQUEARE
 *         SCEFORMAT_SOUND_HD_LFO_WAVEFORM_NOISE
 *         SCEFORMAT_SOUND_HD_LFO_WAVEFORM_SIN
 *         SCEFORMAT_SOUND_HD_LFO_WAVEFORM_USER
 * ※注３：値は０〜２５５で、０〜３５９度と表す
 */


typedef struct{
    unsigned int              nSplit;      /*内包するSplitBlockの個数*/
    unsigned int              progAttr;    /*プログラムアドリビュート*/
    SceSdHdProgramCommon      common;      /*プログラム基本情報*/
    SceSdHdProgramKeyFollow   keyFollow;   /*プログラムkeyFollow情報*/
    SceSdHdProgramLFO         LFO;         /*プログラムLFO情報*/
} SceSdHdProgramParam;

/*
 * progAttr   SCE_FORMAT_SOUND_HD_ROUND_PAN(0x01) : 逆相可
 */


/* ***** SplitBlock ***** */
typedef struct{
    unsigned int    low;        /*下限ノートナンバー*/
    unsigned int    crossFade;  /*クロスフェイド開始ノートナンバー*/
    unsigned int    high;       /*上限ノートナンバー*/
} SceSdHdSplitRange;

typedef struct{
    unsigned int   low;   /*ベンドレンジ− ※*/
    unsigned int   high;  /*ベンドレンジ＋ ※*/
} SceSdHdSplitBendRange;

/*
 * ※単位は半音の１２８等分
 */

typedef struct{
    int             pitch;      /*１オクターブ毎に変化するピッチLFOの変化幅 ※*/
    unsigned int    pitchCenter;/*pitchの効果が０になるノートナンバー*/
    int             amp;        /*１オクターブ毎に変化するアンプLFOの変化幅*/
    unsigned int    ampCenter;  /*ampの効果が０になるノートナンバー*/
    int             pan;        /*１オクターブ毎に変化するパンポットの変化幅*/
    unsigned int    panCenter;  /*panの効果が０になるノートナンバー*/
} SceSdHdSplitKeyFollow;

typedef struct{
    unsigned int    volume;     /*スプリットボリューム*/
    int             panpot;     /*スプリットパンポット*/
    int             transpose;  /*スプリットトランスポーズ*/
    int             detune;     /*スプリットディチューン*/
} SceSdHdSplitCommon;

/*
 * volume     0~128
 * panpot     0~64~127 = 左~真中~右 マイナスは逆相
 * transpose  単位はノート
 * detune     単位は半音の１２８等分
 */

typedef struct{
    unsigned int            sampleSetIndex; /*リンクするサンプルセットナンバー*/
    unsigned int            splitNumber;    /*スプリットナンバー*/
    SceSdHdSplitRange       range;          /*スプリットレンジ情報*/
    SceSdHdSplitBendRange   bendRange;      /*スプリットベンド情報*/
    SceSdHdSplitKeyFollow   keyFollow;      /*スプリットkeyFollow情報*/
    SceSdHdSplitCommon      common;         /*スプリット基本情報*/
} SceSdHdSplitBlock;


/* ***** SampleSetParam ***** */
typedef struct{
    unsigned int   velCurve;      /*ベロシティカーブタイプ*/
    unsigned int   velLimitLow;   /*発音するベロシティの下限値*/
    unsigned int   velLimitHigh;  /*発音するベロシティの上限値*/
    unsigned int   nSample;       /*内包するサンプル情報の個数*/
} SceSdHdSampleSetParam;


/* ***** sample ***** */
typedef struct{
    unsigned int    low;        /*発音下限ベロシティ*/
    unsigned int    crossFade;  /*クロスフェイド開始ベロシティ*/
    unsigned int    high;       /*発音上限ベロシティ*/
} SceSdHdSampleVelRange;

typedef struct{
    int             pitch;           /*ベロシティによるピッチLFOの変化幅 ※*/
    unsigned int    pitchCenter;     /*pitchの効果が０になるベロシティ値*/
    unsigned int    pitchVelCurve;   /*ピッチLFOに対するベロシティカーブタイプ*/
    int             amp;             /*ベロシティによるアンプLFOの変化幅*/
    unsigned int    ampCenter;       /*ampによる効果が０になるベロシティ値*/
    unsigned int    ampVelCurve;     /*アンプLFOに対するベロシティカーブタイプ*/
} SceSdHdSampleVelFollow;

/*
 * ※単位は半音の１２８等分
 */

typedef struct{
    unsigned int    baseNote;  /*ベースとなるノートナンバー*/
    int             detune;    /*サンプルディチューン ※*/
    int             panpot;    /*サンプルパンポット*/
    unsigned int    group;     /*グループ*/
    unsigned int    priority;  /*プライオリティ*/
    unsigned int    volume;    /*サンプルボリューム*/
} SceSdHdSampleCommon;

/*
 * ※単位は半音の１２８等分
 */

typedef struct{
    unsigned int   ADSR1;  /*エンベロープADSR1 ※*/
    unsigned int   ADSR2;  /*エンベロープADSR2 ※*/
} SceSdHdSampleADSR;

/*
 * ※データ形式はSystem Manual[SPU2 Overview]を参照の事
 */

typedef struct{
    int             ar;        /*ARの１オクターブ毎の変化幅*/
    unsigned int    arCenter;  /*arの効果が０になるノートナンバー*/
    int             dr;        /*DRの１オクターブ毎の変化幅*/
    unsigned int    drCenter;  /*drの効果が０になるノートナンバー*/
    int             sr;        /*SRの１オクターブ毎の変化幅*/
    unsigned int    srCenter;  /*srの効果が０になるノートナンバー*/
    int             rr;        /*RRの１オクターブ毎の変化幅*/
    unsigned int    rrCenter;  /*RRの効果が０になるノートナンバー*/
    int             sl;        /*SLの１オクターブ毎の変化幅*/
    unsigned int    slCenter;  /*slの効果が０になるノートナンバー*/
} SceSdHdSampleKeyFollow;

typedef struct{
    unsigned int   pitchLFODelay;  /*PitchLFOディレイタイム*/
    unsigned int   pitchLFOFade;   /*PitchLFOフェイドタイム*/
    unsigned int   ampLFODelay;    /*AmpLFOディレイタイム*/
    unsigned int   ampLFOFade;     /*AmpLFOフェイドタイム*/
} SceSdHdSampleLFO;

typedef struct{
    int                      vagIndex;   /*リンクするVAGInfoナンバー*/
    unsigned int             spuAttr;    /*SPUに関するAttribute ※注１*/
    unsigned int             lfoAttr;    /*LFOに関するAttribute ※注２*/
    SceSdHdSampleVelRange    velRange;   /*ベロシティレンジ情報*/
    SceSdHdSampleVelFollow   velFollow;  /*ベロシティFollow情報*/
    SceSdHdSampleCommon      common;     /*サンプル基本情報*/
    SceSdHdSampleADSR        ADSR;       /*ADSR情報*/
    SceSdHdSampleKeyFollow   keyFollow;  /*KyuFollow情報*/
    SceSdHdSampleLFO         LFO;        /*LFO情報*/
} SceSdHdSampleParam;

/* ※注１ .hd参照の事
 * SCEFORMAT_SOUND_HD_LFO_KEY_ON_PITCH
 * SCEFORMAT_SOUND_HD_LFO_KEY_OFF_PITCH
 * SCEFORMAT_SOUND_HD_LFO_BOTH_PITCH
 * SCEFORMAT_SOUND_HD_LFO_KEY_ON_AMP
 * SCEFORMAT_SOUND_HD_LFO_KEY_OFF_AMP
 * SCEFORMAT_SOUND_HD_LFO_BOTH_AMP
 */

/* ※注２ .hd参照の事
 * SCEFORMAT_SOUND_HD_SPU_DIRECTSEND_L
 * SCEFORMAT_SOUND_HD_SPU_DIRECTSEND_R
 * SCEFORMAT_SOUND_HD_SPU_EFFECTSEND_L
 * SCEFORMAT_SOUND_HD_SPU_EFFECTSEND_R
 * SCEFORMAT_SOUND_HD_SPU_CORE_0
 * SCEFORMAT_SOUND_HD_SPU_CORE_1
 */


/* vaginfo */
typedef struct{
    unsigned int   vagOffsetAddr;  /*波形データのBDファイル内の位置*/
    unsigned int   vagSize;        /*波形データのサイズ*/
    unsigned int   vagSampleRate;  /*波形データのサンプリングレート*/
    unsigned int   vagAttribute;   /*ループフラグ ※*/
} SceSdHdVAGInfoParam;

/* ※.hd参照の事
 * SCE_FORMAT_SOUND_HD_VAG_ATTR_1SHOT = 1 shot
 * SCE_FORMAT_SOUND_HD_VAG_ATTR_LOOP  = Loop
 */

/* Proto Type */
int sceSdHdGetMaxProgramNumber(void *buffer);

int sceSdHdGetMaxSampleSetNumber(void *buffer);

int sceSdHdGetMaxSampleNumber(void *buffer);

int sceSdHdGetMaxVAGInfoNumber(void *buffer);

int sceSdHdGetProgramParamAddr(void *buffer, unsigned int programNumber,
			       sceHardSynthProgramParam **ptr);

int sceSdHdGetProgramParam(void *buffer, unsigned int programNumber,
			   SceSdHdProgramParam *param);

int sceSdHdGetSplitBlockAddr(void *buffer, unsigned int programNumber,
			     unsigned int splitBlockNumber,
			     sceHardSynthSplitBlock **theParamPtr);

int sceSdHdGetSplitBlock(void *buffer, unsigned int programNumber,
			 unsigned int splitBlockNumber,
			 SceSdHdSplitBlock *param);

int sceSdHdGetSampleSetParamAddr(void *buffer, unsigned int sampleSetNumber,
				 sceHardSynthSampleSetParam **ptr);

int sceSdHdGetSampleSetParam(void *buffer, unsigned int sampleSetNumber,
			     SceSdHdSampleSetParam *param);

int sceSdHdGetSampleParamAddr(void *buffer, unsigned int sampleNumber,
			      sceHardSynthSampleParam **ptr);

int sceSdHdGetSampleParam(void *buffer, unsigned int sampleNumber,
			  SceSdHdSampleParam *param);

int sceSdHdGetVAGInfoParamAddr(void *buffer, unsigned int vagInfoNumber,
			       sceHardSynthVagParam **ptr);

int sceSdHdGetVAGInfoParam(void *buffer, unsigned int vagInfoNumber,
			   SceSdHdVAGInfoParam *param);

int sceSdHdCheckProgramNumber(void *buffer, unsigned int programNumber);

int sceSdHdGetSplitBlockCountByNote(void *buffer, unsigned int programNumber,
				    unsigned int noteNumber);

int sceSdHdGetSplitBlockAddrByNote(void *buffer, unsigned int programNumber,
				   unsigned int noteNumber,
				   sceHardSynthSplitBlock **ptr);

int sceSdHdGetSplitBlockByNote(void *buffer, unsigned int programNumber,
			       unsigned int noteNumber,
			       SceSdHdSplitBlock *param);

int sceSdHdGetSampleSetParamCountByNote(void *buffer,
					unsigned int programNumber,
					unsigned int noteNumber);

int sceSdHdGetSampleSetParamAddrByNote(void *buffer, unsigned int programNumber,
				       unsigned int noteNumber, 
				       sceHardSynthSampleSetParam **ptr);

int sceSdHdGetSampleSetParamByNote(void *buffer, unsigned int programNumber, 
				   unsigned int noteNumber, 
				   SceSdHdSampleSetParam *param);

int sceSdHdGetSampleParamCountByNoteVelocity(void *buffer, 
					     unsigned int programNumber, 
					     unsigned int noteNumber, 
					     unsigned int velocity, 
					     unsigned int mode);

int sceSdHdGetSampleParamAddrByNoteVelocity(void *buffer, 
					    unsigned int programNumber, 
					    unsigned int noteNumber, 
					    unsigned int velocity, 
					    unsigned int mode, 
					    sceHardSynthSampleParam **ptr);

int sceSdHdGetSampleParamByNoteVelocity(void *buffer, 
					unsigned int programNumber, 
					unsigned int noteNumber, 
					unsigned int velocity, 
					unsigned int mode, 
					SceSdHdSampleParam *param);

int sceSdHdGetVAGInfoParamCountByNoteVelocity(void *buffer, 
					      unsigned int programNumber, 
					      unsigned int noteNumber, 
					      unsigned int velocity, 
					      unsigned int mode);

int sceSdHdGetVAGInfoParamAddrByNoteVelocity(void *buffer, 
					     unsigned int programNumber, 
					     unsigned int noteNumber, 
					     unsigned int velocity, 
					     unsigned int mode, 
					     sceHardSynthVagParam **ptr);

int sceSdHdGetVAGInfoParamByNoteVelocity(void *buffer, 
					 unsigned int programNumber, 
					 unsigned int noteNumber, 
					 unsigned int velocity, 
					 unsigned int mode, 
					 SceSdHdVAGInfoParam *param);

int sceSdHdGetSampleParamCountByVelocity(void *buffer, 
					 unsigned int sampleSetNumber, 
					 unsigned int velocity, 
					 unsigned int mode);

int sceSdHdGetSampleParamAddrByVelocity(void *buffer, 
					unsigned int sampleSetNumber, 
					unsigned int velocity, 
					unsigned int mode, 
					sceHardSynthSampleParam **ptr);

int sceSdHdGetSampleParamByVelocity(void *buffer, unsigned int sampleSetNumber,
				    unsigned int velocity, unsigned int mode, 
				    SceSdHdSampleParam *param);

int sceSdHdGetVAGInfoParamCountByVelocity(void *buffer, 
					  unsigned int sampleSetNumber, 
					  unsigned int velocity, 
					  unsigned int mode);

int sceSdHdGetVAGInfoParamAddrByVelocity(void *buffer, 
					 unsigned int sampleSetNumber, 
					 unsigned int velocity, 
					 unsigned int mode, 
					 sceHardSynthVagParam **ptr);

int sceSdHdGetVAGInfoParamByVelocity(void *buffer, unsigned int sampleSetNumber,
				     unsigned int velocity, unsigned int mode, 
				     SceSdHdVAGInfoParam *param);

int sceSdHdGetVAGInfoParamAddrBySampleNumber(void *buffer, 
					     unsigned int sampleNumber, 
					     sceHardSynthVagParam **ptr);

int sceSdHdGetVAGInfoParamBySampleNumber(void *buffer, 
					 unsigned int sampleNumber, 
					 SceSdHdVAGInfoParam *param);

int sceSdHdGetSplitBlockNumberBySplitNumber(void *buffer, 
					    unsigned int programNumber, 
					    unsigned int splitNumber);

int sceSdHdGetVAGSize(void *buffer, unsigned int vagInfoNumber);

int sceSdHdGetSplitBlockCount(void *buffer, unsigned int programNumber);

int sceSdHdGetMaxSplitBlockCount(void *buffer);

int sceSdHdGetMaxSampleSetParamCount(void *buffer);

int sceSdHdGetMaxSampleParamCount(void *buffer);

int sceSdHdGetMaxVAGInfoParamCount(void *buffer);

int sceSdHdModifyVelocity(unsigned int curveType, int velocity);

int sceSdHdModifyVelocityLFO
(unsigned int curveType, int velocity, int center);

int sceSdHdGetValidProgramNumberCount(void *buffer);

int sceSdHdGetValidProgramNumber(void *buffer, unsigned int *ptr);

int sceSdHdGetSampleNumberBySampleIndex(void *buffer,
					unsigned int sampleSetNumber,
					unsigned int sampleIndexNumber);

#ifdef __cplusplus
}
#endif

#endif /* !_SCE_SDHD_H */
