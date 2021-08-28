/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    xperf.h

Abstract:

    Include file for the performance measurement APIs

--*/

#ifndef _XPERF_INCLUDED
#define _XPERF_INCLUDED

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// Flag definitions for profiling method used in XPerfBeginCapture
//

#define XPERF_USECPX            0x00000001

//
// Definitions of structure and sampling method of XPERFCPXSTATE used in
// XPerfSetCPXState
//

#define XPERF_SAMPLING_TIMER    0x00000000
#define XPERF_SAMPLING_L2MISSES 0x00000001
#define XPERF_SAMPLING_TYPEMAX  0x00000001

typedef struct _XPERFCPXSTATE {
    DWORD SamplingMethod;
    DWORD Threshold;
} XPERFCPXSTATE, *PXPERFCPXSTATE;



HRESULT
WINAPI
XPerfBeginCapture(
    IN CONST CHAR* FileName,
    IN DWORD Flags
    );
/*++

Routine Description:

    Begins capture data.  The Flags argument specifies what profiling methods
    to use.  The data will be saved to the filename specified by the FileName
    argument.  Data will be captured until XPerfEndCapture is called.

Arguments:

    FileName - Location of the file to dump captured data to.

    Flags - Specifies which profiling methods will be used to capture data.

Return Value:

    HRESULT

--*/

HRESULT
WINAPI
XPerfEndCapture(
    VOID
    );
/*++

Routine Description:

    Stops capturing of data.

Arguments:

    None

Return Value:

    HRESULT

--*/

HRESULT
WINAPI
XPerfSetCPXState(
    IN CONST XPERFCPXSTATE* pState OPTIONAL
    );
/*++

Routine Description:

    Before calling XPerfBeginCapture() with XPERF_USECPX flag set.  This function
    can be used to set the options on the sampling method.  Passing NULL means
    that the default settings should be used.  The defaults are to use the timer
    event with a threshold of 2 million.

Arguments:

    pState - Optional pointer to XPERFCPXSTATE structure containing the options
        for the sampling profiling method.

Return Value:

    HRESULT

--*/

#ifdef __cplusplus
}
#endif

#endif // _XPERF_INCLUDED
