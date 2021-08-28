//==============================================================================
//==============================================================================
// DspTool.cpp
//==============================================================================
//==============================================================================

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "stdafx.h"
#include "windows.h"
#include "x_files.hpp"

#include "dsptool.hpp"

//==============================================================================
//  Data
//==============================================================================

static HINSTANCE hDll;

lpFunc1 dsptool_getBytesForAdpcmBuffer;
lpFunc1 dsptool_getBytesForAdpcmSamples;
lpFunc1 dsptool_getBytesForPcmBuffer;
lpFunc1 dsptool_getBytesForPcmSamples;
lpFunc1 dsptool_getSampleForAdpcmNibble;
lpFunc1 dsptool_getNibblesForNSamples;
lpFunc2 dsptool_getBytesForAdpcmInfo;
lpFunc3 dsptool_encode;
lpFunc4 dsptool_decode;
lpFunc5 dsptool_getLoopContext;

//==============================================================================
//  Functions
//==============================================================================

void dsptool_Unload( void )
{
    if( hDll )
    {
        FreeLibrary( hDll );
        hDll = NULL;
    }
}

//==============================================================================

xbool dsptool_Load(void)
{
    hDll = LoadLibrary( "dsptool.dll" );
    if( hDll )
    {
        if( !( dsptool_getBytesForAdpcmBuffer = (lpFunc1)GetProcAddress( hDll, "getBytesForAdpcmBuffer" ) ) )
            return FALSE;

        if( !( dsptool_getBytesForAdpcmSamples = (lpFunc1)GetProcAddress( hDll, "getBytesForAdpcmSamples" ) ) )
            return FALSE;

        if( !( dsptool_getBytesForPcmBuffer = (lpFunc1)GetProcAddress( hDll, "getBytesForPcmBuffer" ) ) )
            return FALSE;

        if( !( dsptool_getBytesForPcmSamples = (lpFunc1)GetProcAddress( hDll, "getBytesForPcmSamples" ) ) )
            return FALSE;

        if( !( dsptool_getNibblesForNSamples = (lpFunc1)GetProcAddress( hDll, "getNibbleAddress" ) ) )
            return FALSE;

        if( !( dsptool_getSampleForAdpcmNibble = (lpFunc1)GetProcAddress( hDll, "getSampleForAdpcmNibble" ) ) )
            return FALSE;

        if( !( dsptool_getBytesForAdpcmInfo = (lpFunc2)GetProcAddress( hDll, "getBytesForAdpcmInfo" ) ) )
            return FALSE;

        if( !( dsptool_encode = (lpFunc3)GetProcAddress( hDll, "encode" ) ) )
            return FALSE;

        if( !( dsptool_decode = (lpFunc4)GetProcAddress( hDll, "decode" ) ) )
            return FALSE;

        if( !( dsptool_getLoopContext = (lpFunc5)GetProcAddress( hDll, "getLoopContext" ) ) )
            return FALSE;

        return TRUE;
    }

    return FALSE;
}

//==============================================================================
