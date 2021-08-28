
#pragma once

typedef enum _DSP_IMAGE_dmusicfx_FX_INDICES {
    GraphZeroTempMixBins_ZeroTempMixbins0 = 0,
    UserStereoFlange_StereoFlange = 1,
    UserStereoEcho_StereoEcho = 2,
    UserStereoAmpMod_StereoAmpMod = 3,
    UserStereoChorus_StereoChorus = 4,
    UserI3DL224KReverb_I3DL224KReverb = 5,
    Useriir2Left_iir2Left = 6,
    Useriir2Right_iir2Right = 7,
    User1xNSplitter_1xNSplitter = 8,
    GraphI3DL2_I3DL2Reverb = 9,
    GraphXTalk_XTalk = 10,
    GraphVoice_Voice_0 = 11,
    GraphVoice_Voice_1 = 12,
    GraphVoice_Voice_2 = 13,
    GraphVoice_Voice_3 = 14
} DSP_IMAGE_dmusicfx_FX_INDICES;

#define DSI3DL2_ENVIRONMENT_UserI3DL224KReverb_I3DL224KReverb -1000, -270, 0.000000, 1.490000, 0.860000, -1204, 0.007000, -4, 0.011000, 100.000000, 100.000000, 5000.000000

#define DSI3DL2_ENVIRONMENT_GraphI3DL2_I3DL2Reverb -1000, -100, 0.000000, 1.490000, 0.830000, -2602, 0.007000, 200, 0.011000, 100.000000, 100.000000, 5000.000000

typedef struct _GraphZeroTempMixBins_FX0_ZeroTempMixbins0_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[8];     // XRAM offsets in DSP WORDS, of output mixbins
} GraphZeroTempMixBins_FX0_ZeroTempMixbins0_STATE, *LPGraphZeroTempMixBins_FX0_ZeroTempMixbins0_STATE;

typedef const GraphZeroTempMixBins_FX0_ZeroTempMixbins0_STATE *LPCGraphZeroTempMixBins_FX0_ZeroTempMixbins0_STATE;

typedef struct _UserStereoFlange_FX0_StereoFlange_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[3];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[2];     // XRAM offsets in DSP WORDS, of output mixbins
} UserStereoFlange_FX0_StereoFlange_STATE, *LPUserStereoFlange_FX0_StereoFlange_STATE;

typedef const UserStereoFlange_FX0_StereoFlange_STATE *LPCUserStereoFlange_FX0_StereoFlange_STATE;

typedef struct _UserStereoEcho_FX0_StereoEcho_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[2];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[2];     // XRAM offsets in DSP WORDS, of output mixbins
} UserStereoEcho_FX0_StereoEcho_STATE, *LPUserStereoEcho_FX0_StereoEcho_STATE;

typedef const UserStereoEcho_FX0_StereoEcho_STATE *LPCUserStereoEcho_FX0_StereoEcho_STATE;

typedef struct _UserStereoAmpMod_FX0_StereoAmpMod_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[3];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[2];     // XRAM offsets in DSP WORDS, of output mixbins
} UserStereoAmpMod_FX0_StereoAmpMod_STATE, *LPUserStereoAmpMod_FX0_StereoAmpMod_STATE;

typedef const UserStereoAmpMod_FX0_StereoAmpMod_STATE *LPCUserStereoAmpMod_FX0_StereoAmpMod_STATE;

typedef struct _UserStereoChorus_FX0_StereoChorus_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[3];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[2];     // XRAM offsets in DSP WORDS, of output mixbins
} UserStereoChorus_FX0_StereoChorus_STATE, *LPUserStereoChorus_FX0_StereoChorus_STATE;

typedef const UserStereoChorus_FX0_StereoChorus_STATE *LPCUserStereoChorus_FX0_StereoChorus_STATE;

typedef struct _UserI3DL224KReverb_FX0_I3DL224KReverb_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[2];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[35];     // XRAM offsets in DSP WORDS, of output mixbins
} UserI3DL224KReverb_FX0_I3DL224KReverb_STATE, *LPUserI3DL224KReverb_FX0_I3DL224KReverb_STATE;

typedef const UserI3DL224KReverb_FX0_I3DL224KReverb_STATE *LPCUserI3DL224KReverb_FX0_I3DL224KReverb_STATE;

typedef struct _Useriir2Left_FX0_iir2Left_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} Useriir2Left_FX0_iir2Left_STATE, *LPUseriir2Left_FX0_iir2Left_STATE;

typedef const Useriir2Left_FX0_iir2Left_STATE *LPCUseriir2Left_FX0_iir2Left_STATE;

typedef struct _Useriir2Right_FX0_iir2Right_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} Useriir2Right_FX0_iir2Right_STATE, *LPUseriir2Right_FX0_iir2Right_STATE;

typedef const Useriir2Right_FX0_iir2Right_STATE *LPCUseriir2Right_FX0_iir2Right_STATE;

typedef struct _User1xNSplitter_FX0_1xNSplitter_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[8];     // XRAM offsets in DSP WORDS, of output mixbins
} User1xNSplitter_FX0_1xNSplitter_STATE, *LPUser1xNSplitter_FX0_1xNSplitter_STATE;

typedef const User1xNSplitter_FX0_1xNSplitter_STATE *LPCUser1xNSplitter_FX0_1xNSplitter_STATE;

typedef struct _GraphI3DL2_FX0_I3DL2Reverb_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[2];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[35];     // XRAM offsets in DSP WORDS, of output mixbins
} GraphI3DL2_FX0_I3DL2Reverb_STATE, *LPGraphI3DL2_FX0_I3DL2Reverb_STATE;

typedef const GraphI3DL2_FX0_I3DL2Reverb_STATE *LPCGraphI3DL2_FX0_I3DL2Reverb_STATE;

typedef struct _GraphXTalk_FX0_XTalk_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[4];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[4];     // XRAM offsets in DSP WORDS, of output mixbins
} GraphXTalk_FX0_XTalk_STATE, *LPGraphXTalk_FX0_XTalk_STATE;

typedef const GraphXTalk_FX0_XTalk_STATE *LPCGraphXTalk_FX0_XTalk_STATE;

typedef struct _GraphVoice_FX0_Voice_0_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} GraphVoice_FX0_Voice_0_STATE, *LPGraphVoice_FX0_Voice_0_STATE;

typedef const GraphVoice_FX0_Voice_0_STATE *LPCGraphVoice_FX0_Voice_0_STATE;

typedef struct _GraphVoice_FX1_Voice_1_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} GraphVoice_FX1_Voice_1_STATE, *LPGraphVoice_FX1_Voice_1_STATE;

typedef const GraphVoice_FX1_Voice_1_STATE *LPCGraphVoice_FX1_Voice_1_STATE;

typedef struct _GraphVoice_FX2_Voice_2_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} GraphVoice_FX2_Voice_2_STATE, *LPGraphVoice_FX2_Voice_2_STATE;

typedef const GraphVoice_FX2_Voice_2_STATE *LPCGraphVoice_FX2_Voice_2_STATE;

typedef struct _GraphVoice_FX3_Voice_3_STATE {
    DWORD dwScratchOffset;        // Offset in bytes, of scratch area for this FX
    DWORD dwScratchLength;        // Length in DWORDS, of scratch area for this FX
    DWORD dwYMemoryOffset;        // Offset in DSP WORDS, of Y memory area for this FX
    DWORD dwYMemoryLength;        // Length in DSP WORDS, of Y memory area for this FX
    DWORD dwFlags;                // FX bitfield for various flags. See xgpimage documentation
    DWORD dwInMixbinPtrs[1];      // XRAM offsets in DSP WORDS, of input mixbins
    DWORD dwOutMixbinPtrs[1];     // XRAM offsets in DSP WORDS, of output mixbins
} GraphVoice_FX3_Voice_3_STATE, *LPGraphVoice_FX3_Voice_3_STATE;

typedef const GraphVoice_FX3_Voice_3_STATE *LPCGraphVoice_FX3_Voice_3_STATE;
