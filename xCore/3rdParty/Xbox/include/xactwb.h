/***************************************************************************
 *
 *  Copyright (C) 02/27/2002 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       xactwb.h
 *  Content:    XACT Wave Bank definitions.
 *
 ****************************************************************************/

#ifndef __XACTWB_H__
#define __XACTWB_H__

#pragma pack(push, 1)

#define WAVEBANK_HEADER_SIGNATURE   'DNBW'      // Wave bank file signature
#define WAVEBANK_HEADER_VERSION     3           // Current tool version

#define WAVEBANK_BANKNAME_LENGTH    16          // Wave bank friendly name length, in characters
#define WAVEBANK_ENTRYNAME_LENGTH   64          // Wave bank entry friendly name length, in characters

//
// Bank flags
//

#define WAVEBANK_TYPE_BUFFER        0x00000000  // In-memory buffer
#define WAVEBANK_TYPE_STREAMING     0x00000001  // Streaming
#define WAVEBANK_TYPE_MASK          0x00000001

#define WAVEBANK_FLAGS_ENTRYNAMES   0x00010000  // Bank includes entry names
#define WAVEBANK_FLAGS_COMPACT      0x00020000  // Bank uses compact format
#define WAVEBANK_FLAGS_MASK         0x00030000

//
// Entry flags
//

#define WAVEBANKENTRY_FLAGS_READAHEAD       0x00000001  // Enable stream read-ahead
#define WAVEBANKENTRY_FLAGS_LOOPCACHE       0x00000002  // One or more looping sounds use this wave
#define WAVEBANKENTRY_FLAGS_REMOVELOOPTAIL  0x00000004  // Remove data after the end of the loop region
#define WAVEBANKENTRY_FLAGS_IGNORELOOP      0x00000008  // Used internally when the loop region can't be used
#define WAVEBANKENTRY_FLAGS_MASK            0x0000000F

#define WAVEBANKENTRY_FILTERS_ADPCM         0x00010000  // ADPCM conversion
#define WAVEBANKENTRY_FILTERS_8BITPCM       0x00020000  // 8-bit PCM conversion
#define WAVEBANKENTRY_FILTERS_MASK          0x00030000

//
// Entry wave format identifiers
//

#define WAVEBANKMINIFORMAT_TAG_PCM      0x0     // PCM data
#define WAVEBANKMINIFORMAT_TAG_ADPCM    0x1     // ADPCM data
#define WAVEBANKMINIFORMAT_TAG_WMA      0x2     // WMA data

#define WAVEBANKMINIFORMAT_BITDEPTH_8   0x0     // 8-bit data (PCM only)
#define WAVEBANKMINIFORMAT_BITDEPTH_16  0x1     // 16-bit data (PCM only)

//
// Bank alignment presets
//

#define WAVEBANK_ALIGNMENT_MIN          4       // Minimum alignment
#define WAVEBANK_ALIGNMENT_HARDDRIVE    512     // Hard-drive-optimized alignment
#define WAVEBANK_ALIGNMENT_DVD          2048    // DVD-optimized alignment

//
// Wave bank segment identifiers
//

typedef enum _WAVEBANKSEGIDX
{
    WAVEBANK_SEGIDX_BANKDATA = 0,       // Bank data
    WAVEBANK_SEGIDX_ENTRYMETADATA,      // Entry meta-data
    WAVEBANK_SEGIDX_ENTRYNAMES,         // Entry friendly names
    WAVEBANK_SEGIDX_ENTRYWAVEDATA,      // Entry wave data
    WAVEBANK_SEGIDX_COUNT
} WAVEBANKSEGIDX, *LPWAVEBANKSEGIDX;

typedef const WAVEBANKSEGIDX *LPCWAVEBANKSEGIDX;

//
// Wave bank region
//

typedef struct _WAVEBANKREGION
{
    DWORD       dwOffset;               // Region offset, in bytes
    DWORD       dwLength;               // Region length, in bytes
} WAVEBANKREGION, *LPWAVEBANKREGION;

typedef const WAVEBANKREGION *LPCWAVEBANKREGION;

//
// Wave bank file header
//

typedef struct _WAVEBANKHEADER
{
    DWORD           dwSignature;                        // File signature
    DWORD           dwVersion;                          // Version of the tool that created the file
    WAVEBANKREGION  Segments[WAVEBANK_SEGIDX_COUNT];    // Segment lookup table
} WAVEBANKHEADER, *LPWAVEBANKHEADER;

typedef const WAVEBANKHEADER *LPCWAVEBANKHEADER;

//
// Entry compressed data format
//

typedef union _WAVEBANKMINIWAVEFORMAT
{
    struct
    {
        DWORD       wFormatTag      : 2;        // PCM vs. ADPCM
        DWORD       nChannels       : 3;        // Channel count (1 - 6)
        DWORD       nSamplesPerSec  : 26;       // Sampling rate
        DWORD       wBitsPerSample  : 1;        // Bits per sample (8 vs. 16, PCM only)
    };

    DWORD           dwValue;
} WAVEBANKMINIWAVEFORMAT, *LPWAVEBANKMINIWAVEFORMAT;

typedef const WAVEBANKMINIWAVEFORMAT *LPCWAVEBANKMINIWAVEFORMAT;

//
// Entry meta-data
//

typedef struct _WAVEBANKENTRY
{
    DWORD                   dwFlags;        // Entry flags
    WAVEBANKMINIWAVEFORMAT  Format;         // Entry format
    WAVEBANKREGION          PlayRegion;     // Region within the wave data segment that contains this entry
    WAVEBANKREGION          LoopRegion;     // Region within the wave data that should loop
} WAVEBANKENTRY, *LPWAVEBANKENTRY;

typedef const WAVEBANKENTRY *LPCWAVEBANKENTRY;

//
// Compact entry meta-data
//

typedef struct _WAVEBANKENTRYCOMPACT
{
    DWORD       dwOffset : 21;               // Data offset, in sectors
    DWORD       dwLengthDeviation : 11;     // Data length deviation, in bytes
} WAVEBANKENTRYCOMPACT, *LPWAVEBANKENTRYCOMPACT;

typedef const WAVEBANKENTRYCOMPACT *LPCWAVEBANKENTRYCOMPACT;

//
// Bank data segment
//

typedef struct _WAVEBANKDATA
{
    DWORD                   dwFlags;                                // Wave bank flags
    DWORD                   dwEntryCount;                           // Number of entries in the bank
    CHAR                    szBankName[WAVEBANK_BANKNAME_LENGTH];   // Bank friendly name
    DWORD                   dwEntryMetaDataElementSize;             // Size of each entry meta-data element, in bytes
    DWORD                   dwEntryNameElementSize;                 // Size of each entry name element, in bytes
    DWORD                   dwAlignment;                            // Entry alignment, in bytes
    WAVEBANKMINIWAVEFORMAT  CompactFormat;                          // Format data for compact bank
} WAVEBANKDATA, *LPWAVEBANKDATA;

typedef const WAVEBANKDATA *LPCWAVEBANKDATA;

#pragma pack(pop)

#endif // __XACTWB_H__
