/***************************************************************************
 *
 *  Copyright (C) 2000 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       dsfxparm.h
 *  Content:    DirectSound effect parameters.
 *
 ****************************************************************************/

#ifndef __DSFXPARM_H__
#define __DSFXPARM_H__

//
// data structs describing effect state parameters
// The generated header from xgpimage will contain these for each
// effect in the image and all of their parameters
//

#define DSFX_PARAM_TYPE_INTEGER    0x1
#define DSFX_PARAM_TYPE_FLOAT      0x2

typedef struct _FX_PARAM_DESCRIPTION 
{
    BYTE    ParameterName[255];
    BYTE    ParameterDesc[255];
    BYTE    UnitsName[64];
    DWORD   dwOffset;
    DWORD   dwDefaultValue;
    DWORD   dwParameterType;
    DWORD   dwParameterMin;
    DWORD   dwParameterMax;
    BOOL    bIsAtomic;
} FX_PARAM_DESCRIPTION, *PFX_PARAM_DESCRIPTION;


typedef struct _FX_STATE_PARAMETERS {

    BYTE  EffectName[255];
    DWORD dwNumParams;
    FX_PARAM_DESCRIPTION aParamDescriptions[1];

} FX_STATE_PARAMETERS, *PFX_STATE_PARAMETERS;

//
// State flags
//

#define DSFX_STATE_INITIALIZED  0x00000001
#define DSFX_STATE_GLOBAL       0x00000002
#define DSFX_STATE_UPDATE       0x00000004

//
// Amplitude modulation (mono)
//

typedef struct _DSFX_AMPMOD_MONO_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[2];
    DWORD       dwOutMixbinPtrs[1];
} DSFX_AMPMOD_MONO_STATE, *LPDSFX_AMPMOD_MONO_STATE;

typedef const DSFX_AMPMOD_MONO_STATE *LPCDSFX_AMPMOD_MONO_STATE;

typedef struct _DSFX_AMPMOD_MONO_PARAMS
{
    DSFX_AMPMOD_MONO_STATE  State;
} DSFX_AMPMOD_MONO_PARAMS, *LPDSFX_AMPMOD_MONO_PARAMS;

typedef const DSFX_AMPMOD_MONO_PARAMS *LPCDSFX_AMPMOD_MONO_PARAMS;

//
// Amplitude modulation (stereo)
//

typedef struct _DSFX_AMPMOD_STEREO_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[3];
    DWORD       dwOutMixbinPtrs[2];
} DSFX_AMPMOD_STEREO_STATE, *LPDSFX_AMPMOD_STEREO_STATE;

typedef const DSFX_AMPMOD_STEREO_STATE *LPCDSFX_AMPMOD_STEREO_STATE;

typedef struct _DSFX_AMPMOD_STEREO_PARAMS
{
    DSFX_AMPMOD_STEREO_STATE    State;
} DSFX_AMPMOD_STEREO_PARAMS, *LPDSFX_AMPMOD_STEREO_PARAMS;

typedef const DSFX_AMPMOD_STEREO_PARAMS *LPCDSFX_AMPMOD_STEREO_PARAMS;

//
// Chorus (mono)
//

typedef struct _DSFX_CHORUS_MONO_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[2];
    DWORD       dwOutMixbinPtrs[1];
} DSFX_CHORUS_MONO_STATE, *LPDSFX_CHORUS_MONO_STATE;

typedef const DSFX_CHORUS_MONO_STATE *LPCDSFX_CHORUS_MONO_STATE;

typedef struct _DSFX_CHORUS_MONO_PARAMS
{
    DSFX_CHORUS_MONO_STATE  State;
    DWORD                   dwGain;
    DWORD                   dwModScale;
} DSFX_CHORUS_MONO_PARAMS, *LPDSFX_CHORUS_MONO_PARAMS;

typedef const DSFX_CHORUS_MONO_PARAMS *LPCDSFX_CHORUS_MONO_PARAMS;

//
// Chorus (stereo)
//

typedef struct _DSFX_CHORUS_STEREO_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[3];
    DWORD       dwOutMixbinPtrs[2];
} DSFX_CHORUS_STEREO_STATE, *LPDSFX_CHORUS_STEREO_STATE;

typedef const DSFX_CHORUS_STEREO_STATE *LPCDSFX_CHORUS_STEREO_STATE;

typedef struct _DSFX_CHORUS_STEREO_PARAMS
{
    DSFX_CHORUS_STEREO_STATE    State;
    DWORD                       dwGain;
    DWORD                       dwModScale;
} DSFX_CHORUS_STEREO_PARAMS, *LPDSFX_CHORUS_STEREO_PARAMS;

typedef const DSFX_CHORUS_STEREO_PARAMS *LPCDSFX_CHORUS_STEREO_PARAMS;

//
// Distortion
//

typedef struct _DSFX_DISTORTION_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[1];
    DWORD       dwOutMixbinPtrs[1];
} DSFX_DISTORTION_STATE, *LPDSFX_DISTORTION_STATE;

typedef const DSFX_DISTORTION_STATE *LPCDSFX_DISTORTION_STATE;

typedef struct _DSFX_DISTORTION_PARAMS
{
    DSFX_DISTORTION_STATE   State;
    DWORD                   dwGain;
    DWORD                   dwPreFilterB0;
    DWORD                   dwPreFilterB1;
    DWORD                   dwPreFilterB2;
    DWORD                   dwPreFilterA1;
    DWORD                   dwPreFilterA2;
    DWORD                   dwPostFilterB0;
    DWORD                   dwPostFilterB1;
    DWORD                   dwPostFilterB2;
    DWORD                   dwPostFilterA1;
    DWORD                   dwPostFilterA2;
} DSFX_DISTORTION_PARAMS, *LPDSFX_DISTORTION_PARAMS;

typedef const DSFX_DISTORTION_PARAMS *LPCDSFX_DISTORTION_PARAMS;

//
// Echo (mono)
//

typedef struct _DSFX_ECHO_MONO_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[1];
    DWORD       dwOutMixbinPtrs[1];
} DSFX_ECHO_MONO_STATE, *LPDSFX_ECHO_MONO_STATE;

typedef const DSFX_ECHO_MONO_STATE *LPCDSFX_ECHO_MONO_STATE;

typedef struct _DSFX_ECHO_MONO_PARAMS
{
    DSFX_ECHO_MONO_STATE    State;
    DWORD                   dwGain;
} DSFX_ECHO_MONO_PARAMS, *LPDSFX_ECHO_MONO_PARAMS;

typedef const DSFX_ECHO_MONO_PARAMS *LPCDSFX_ECHO_MONO_PARAMS;

//
// Echo (stereo)
//

typedef struct _DSFX_ECHO_STEREO_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[2];
    DWORD       dwOutMixbinPtrs[2];
} DSFX_ECHO_STEREO_STATE, *LPDSFX_ECHO_STEREO_STATE;

typedef const DSFX_ECHO_STEREO_STATE *LPCDSFX_ECHO_STEREO_STATE;

typedef struct _DSFX_ECHO_STEREO_PARAMS
{
    DSFX_ECHO_STEREO_STATE  State;
    DWORD                   dwGain;
} DSFX_ECHO_STEREO_PARAMS, *LPDSFX_ECHO_STEREO_PARAMS;

typedef const DSFX_ECHO_STEREO_PARAMS *LPCDSFX_ECHO_STEREO_PARAMS;

//
// Flange (mono)
//

typedef struct _DSFX_FLANGE_MONO_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[2];
    DWORD       dwOutMixbinPtrs[1];
} DSFX_FLANGE_MONO_STATE, *LPDSFX_FLANGE_MONO_STATE;

typedef const DSFX_FLANGE_MONO_STATE *LPCDSFX_FLANGE_MONO_STATE;

typedef struct _DSFX_FLANGE_MONO_PARAMS
{
    DSFX_FLANGE_MONO_STATE  State;
    DWORD                   dwFeedback;
    DWORD                   dwModScale;
} DSFX_FLANGE_MONO_PARAMS, *LPDSFX_FLANGE_MONO_PARAMS;

typedef const DSFX_FLANGE_MONO_PARAMS *LPCDSFX_FLANGE_MONO_PARAMS;

//
// Flange (stereo)
//

typedef struct _DSFX_FLANGE_STEREO_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[3];
    DWORD       dwOutMixbinPtrs[2];
} DSFX_FLANGE_STEREO_STATE, *LPDSFX_FLANGE_STEREO_STATE;

typedef const DSFX_FLANGE_STEREO_STATE *LPCDSFX_FLANGE_STEREO_STATE;

typedef struct _DSFX_FLANGE_STEREO_PARAMS
{
    DSFX_FLANGE_STEREO_STATE    State;
    DWORD                       dwFeedback;
    DWORD                       dwModScale;
} DSFX_FLANGE_STEREO_PARAMS, *LPDSFX_FLANGE_STEREO_PARAMS;

typedef const DSFX_FLANGE_STEREO_PARAMS *LPCDSFX_FLANGE_STEREO_PARAMS;

//
// IIR
//

typedef struct _DSFX_IIR_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[1];
    DWORD       dwOutMixbinPtrs[1];
} DSFX_IIR_STATE, *LPDSFX_IIR_STATE;

typedef const DSFX_IIR_STATE *LPCDSFX_IIR_STATE;

typedef struct _DSFX_IIR_PARAMS
{
    DSFX_IIR_STATE  State;
    DWORD           dwDelayLength;
    DWORD           dwGain;
    DWORD           dwType;
} DSFX_IIR_PARAMS, *LPDSFX_IIR_PARAMS;

typedef const DSFX_IIR_PARAMS *LPCDSFX_IIR_PARAMS;

//
// Second-order IIR
//

typedef struct _DSFX_IIR2_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[1];
    DWORD       dwOutMixbinPtrs[1];
} DSFX_IIR2_STATE, *LPDSFX_IIR2_STATE;

typedef const DSFX_IIR2_STATE *LPCDSFX_IIR2_STATE;

typedef struct _DSFX_IIR2_PARAMS
{
    DSFX_IIR2_STATE State;
    DWORD           dwFilterB0;
    DWORD           dwFilterB1;
    DWORD           dwFilterB2;
    DWORD           dwFilterA1;
    DWORD           dwFilterA2;    
} DSFX_IIR2_PARAMS, *LPDSFX_IIR2_PARAMS;

typedef const DSFX_IIR2_PARAMS *LPCDSFX_IIR2_PARAMS;

//
// Oscillator
//

typedef struct _DSFX_OSCILLATOR_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwOutMixbinPtrs[4];
} DSFX_OSCILLATOR_STATE, *LPDSFX_OSCILLATOR_STATE;

typedef const DSFX_OSCILLATOR_STATE *LPCDSFX_OSCILLATOR_STATE;


typedef struct _DSFX_OSCILLATOR_PARAMS
{
    DSFX_OSCILLATOR_STATE   State;
    DWORD                   dwNumOutputs;
    DWORD                   adwFrequency[4];
} DSFX_OSCILLATOR_PARAMS, *LPDSFX_OSCILLATOR_PARAMS;

typedef const DSFX_OSCILLATOR_PARAMS *LPCDSFX_OSCILLATOR_PARAMS;

//
// I3DL2 reverb
//

typedef struct _DSFX_I3DL2REVERB_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[2];
    DWORD       dwOutMixbinPtrs[35];
} DSFX_I3DL2REVERB_STATE, *LPDSFX_I3DL2REVERB_STATE;

typedef const DSFX_I3DL2REVERB_STATE *LPCDSFX_I3DL2REVERB_STATE;

typedef struct _DSFX_I3DL2REVERB_DELAYLINE
{
    DWORD       dwBase;
    DWORD       dwLength;
} DSFX_I3DL2REVERB_DELAYLINE, *LPDSFX_I3DL2REVERB_DELAYLINE;

typedef const DSFX_I3DL2REVERB_DELAYLINE *LPCDSFX_I3DL2REVERB_DELAYLINE;

typedef struct _DSFX_I3DL2REVERB_IIR
{
    DWORD       dwDelay;
    DWORD       dwCoefficients[2];
} DSFX_I3DL2REVERB_IIR, *LPDSFX_I3DL2REVERB_IIR;

typedef const DSFX_I3DL2REVERB_IIR *LPCDSFX_I3DL2REVERB_IIR;

typedef struct _DSFX_I3DL2REVERB_PARAMS
{
    DSFX_I3DL2REVERB_STATE      State;
    DSFX_I3DL2REVERB_DELAYLINE  DelayLines[14];
    DWORD                       dwReflectionsInputDelay[5];
    DWORD                       dwShortReverbInputDelay;
    DWORD                       dwLongReverbInputDelay[8];
    DWORD                       dwReflectionsFeedbackDelay[4];
    DWORD                       dwLongReverbFeedbackDelay;
    DWORD                       dwShortReverbInputGain[8];
    DWORD                       dwLongReverbInputGain;
    DWORD                       dwLongReverbCrossfeedGain;
    DWORD                       dwReflectionsOutputGain[4];
    DWORD                       dwShortReverbOutputGain;
    DWORD                       dwLongReverbOutputGain;
    DWORD                       dwChannelCount;
    DSFX_I3DL2REVERB_IIR        IIR[10];
} DSFX_I3DL2REVERB_PARAMS, *LPDSFX_I3DL2REVERB_PARAMS;

typedef const DSFX_I3DL2REVERB_PARAMS *LPCDSFX_I3DL2REVERB_PARAMS;

//
// Simple reverb
//

typedef struct _DSFX_MINIREVERB_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[1];
    DWORD       dwOutMixbinPtrs[12];
} DSFX_MINIREVERB_STATE, *LPDSFX_MINIREVERB_STATE;

typedef const DSFX_MINIREVERB_STATE *LPCDSFX_MINIREVERB_STATE;

typedef struct _DSFX_MINIREVERB_PARAMS
{
    DSFX_MINIREVERB_STATE   State;
    DWORD                   dwDelayLineLengths[8];
    DWORD                   dwReflectionTaps[8];
    DWORD                   dwReflectionGains[8];
    DWORD                   dwInputIIRCoefficients[2];
    DWORD                   dwInputIIRDelay;
    DWORD                   dwOutputIIRCoefficients[2];
    DWORD                   dwOutputIIRDelay;
} DSFX_MINIREVERB_PARAMS, *LPDSFX_MINIREVERB_PARAMS;

typedef const DSFX_MINIREVERB_PARAMS *LPCDSFX_MINIREVERB_PARAMS;

//
// RMS peak meter
//

typedef struct _DSFX_RMS_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[6];
} DSFX_RMS_STATE, *LPDSFX_RMS_STATE;

typedef const DSFX_RMS_STATE *LPCDSFX_RMS_STATE;

typedef struct _DSFX_RMS_PARAMS
{
    DSFX_RMS_STATE  State;
    DWORD           dwNumMixBins;
    DWORD           dwRMSValues[6];
    DWORD           dwPeakValues[6];
} DSFX_RMS_PARAMS, *LPDSFX_RMS_PARAMS;

typedef const DSFX_RMS_PARAMS *LPCDSFX_RMS_PARAMS;

//
// Splitter
//

typedef struct _DSFX_SPLITTER_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[1];
    DWORD       dwOutMixbinPtrs[8];
} DSFX_SPLITTER_STATE, *LPDSFX_SPLITTER_STATE;

typedef const DSFX_SPLITTER_STATE *LPCDSFX_SPLITTER_STATE;

typedef struct _DSFX_SPLITTER_PARAMS
{
    DSFX_SPLITTER_STATE State;
    DWORD               dwNumOutputs;
    DWORD               dwGains[8];
} DSFX_SPLITTER_PARAMS, *LPDSFX_SPLITTER_PARAMS;

typedef const DSFX_SPLITTER_PARAMS *LPCDSFX_SPLITTER_PARAMS;

//
// 2x1 mixer
//
                            
typedef struct _DSFX_MIXER_2x1_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[2];
    DWORD       dwOutMixbinPtrs[1];
} DSFX_MIXER_2x1_STATE, *LPDSFX_MIXER_2x1_STATE;

typedef const DSFX_MIXER_2x1_STATE *LPCDSFX_MIXER_2x1_STATE;

typedef struct _DSFX_MIXER_2x1_PARAMS
{
    DSFX_MIXER_2x1_STATE State;
    DWORD                dwGains[2];
} DSFX_MIXER_2x1_PARAMS, *LPDSFX_MIXER_2x1_PARAMS;

typedef const DSFX_MIXER_2x1_PARAMS *LPCDSFX_MIXER_2x1_PARAMS;

//
// Sample Rate Converter
//

typedef struct _DSFX_SAMPLE_RATE_CONVERTER_STATE
{
    DWORD       dwScratchOffset;
    DWORD       dwScratchLength;
    DWORD       dwYMemoryOffset;
    DWORD       dwYMemoryLength;
    DWORD       dwFlags;
    DWORD       dwInMixbinPtrs[1];
    DWORD       dwOutMixbinPtrs[1];
} DSFX_SAMPLE_RATE_CONVERTER_STATE, *LPDSFX_SAMPLE_RATE_CONVERTER_STATE;

typedef const DSFX_SAMPLE_RATE_CONVERTER_STATE *LPCDSFX_SAMPLE_RATE_CONVERTER_STATE;

typedef struct _DSFX_SAMPLE_RATE_CONVERTER_PARAMS
{
    DSFX_SAMPLE_RATE_CONVERTER_STATE State;
    DWORD               dwConversionRatio;
    DWORD               dwAmplificationFactor;
    DWORD               dwReserved[4];
    DWORD               dwScratchSampleOffset;
} DSFX_SAMPLE_RATE_CONVERTER_PARAMS, *LPDSFX_SAMPLE_RATE_CONVERTER_PARAMS;

typedef const DSFX_SAMPLE_RATE_CONVERTER_PARAMS *LPCDSFX_SAMPLE_RATE_CONVERTER_PARAMS;


#endif // __DSFXPARM_H__
