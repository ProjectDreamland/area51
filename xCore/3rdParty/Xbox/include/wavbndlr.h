/***************************************************************************
 *
 *  Copyright (C) 11/2/2001 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       wavbndlr.h
 *  Content:    Wave Bundler definitions.
 *
 ****************************************************************************/

#ifndef __WAVBNDLR_H__
#define __WAVBNDLR_H__

#pragma pack(push, 8)

//
// Wave bank entry compressed data format
//

#define WAVEBANKMINIFORMAT_TAG_PCM      0x0 // PCM data
#define WAVEBANKMINIFORMAT_TAG_ADPCM    0x1 // ADPCM data

#define WAVEBANKMINIFORMAT_BITDEPTH_4   0x0 // 4-bit data (ADPCM only)
#define WAVEBANKMINIFORMAT_BITDEPTH_8   0x0 // 8-bit data (PCM only)
#define WAVEBANKMINIFORMAT_BITDEPTH_16  0x1 // 16-bit data (PCM only)

#pragma pack(push, 4)

typedef struct _WAVEBANKMINIWAVEFORMAT
{
    DWORD       wFormatTag      : 1;    // PCM vs. ADPCM
    DWORD       nChannels       : 3;    // Channel count (1 - 6)
    DWORD       nSamplesPerSec  : 27;   // Sampling rate
    DWORD       wBitsPerSample  : 1;    // Bits per sample (8 vs. 16, PCM only)
} WAVEBANKMINIWAVEFORMAT, *LPWAVEBANKMINIWAVEFORMAT;

typedef const WAVEBANKMINIWAVEFORMAT *LPCWAVEBANKMINIWAVEFORMAT;

#pragma pack(pop)

// 
// Wave bank expanded wave format
//

typedef union _WAVEBANKUNIWAVEFORMAT
{
    WAVEFORMATEX        WaveFormatEx;
    XBOXADPCMWAVEFORMAT AdpcmWaveFormat;
} WAVEBANKUNIWAVEFORMAT, *LPWAVEBANKUNIWAVEFORMAT;

typedef const WAVEBANKUNIWAVEFORMAT *LPCWAVEBANKUNIWAVEFORMAT;

//
// Wave bank entry region indices
//

typedef struct _WAVEBANKENTRYREGION
{
    DWORD           dwStart;                // Starting byte offset
    DWORD           dwLength;               // Region length, in bytes
} WAVEBANKENTRYREGION, *LPWAVEBANKENTRYREGION;

typedef const WAVEBANKENTRYREGION *LPCWAVEBANKENTRYREGION;

//
// Wave bank entry meta-data
//

typedef struct _WAVEBANKENTRY
{
    WAVEBANKMINIWAVEFORMAT Format;         // Entry format
    WAVEBANKENTRYREGION    PlayRegion;     // Offsets from the start of the data segment that contains this entry
    WAVEBANKENTRYREGION    LoopRegion;     // Offests relative to the play region that contains the loop region
} WAVEBANKENTRY, *LPWAVEBANKENTRY;

typedef const WAVEBANKENTRY *LPCWAVEBANKENTRY;

//
// Wave bank file header
//

#define WAVEBANKHEADER_SIGNATURE        'DNBW'
#define WAVEBANKHEADER_VERSION          1
#define WAVEBANKHEADER_BANKNAME_LENGTH  64

typedef struct _WAVEBANKHEADER
{
    DWORD           dwSignature;                                // File signature
    DWORD           dwVersion;                                  // Version of the tool that created the file
    DWORD           dwFlags;                                    // Wave bank flags (currently unused)
    DWORD           dwEntryCount;                               // Number of entries in the bank
    CHAR            szBankName[WAVEBANKHEADER_BANKNAME_LENGTH]; // Bank identifier string
} WAVEBANKHEADER, *LPWAVEBANKHEADER;

typedef const WAVEBANKHEADER *LPCWAVEBANKHEADER;

#pragma pack(pop)

#endif // __WAVBNDLR_H__
