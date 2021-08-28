//==============================================================================
//==============================================================================
// DspTool.hpp
//==============================================================================
//==============================================================================

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include "windows.h"

#include "x_files.hpp"

//==============================================================================
//  Types
//==============================================================================

struct gcn_adpcminfo
{
    // start context
    s16 coef[16];
    u16 gain;
    u16 pred_scale;
    s16 yn1;
    s16 yn2;

    // loop context
    u16 loop_pred_scale;
    s16 loop_yn1;
    s16 loop_yn2;
};

typedef u32     (*lpFunc1)(u32);
typedef u32     (*lpFunc2)(void);
typedef void    (*lpFunc3)(s16*, u8*, gcn_adpcminfo*, u32);
typedef void    (*lpFunc4)(u8*, s16*, gcn_adpcminfo*, u32);
typedef void    (*lpFunc5)(u8*, gcn_adpcminfo*, u32);

//==============================================================================
//  Function Prototypes
//==============================================================================

xbool   dsptool_Load( void );
void    dsptool_Unload( void );

extern lpFunc1 dsptool_getBytesForAdpcmBuffer;
extern lpFunc1 dsptool_getBytesForAdpcmSamples;
extern lpFunc1 dsptool_getBytesForPcmBuffer;
extern lpFunc1 dsptool_getBytesForPcmSamples;
extern lpFunc1 dsptool_getSampleForAdpcmNibble;
extern lpFunc1 dsptool_getNibblesForNSamples;
extern lpFunc2 dsptool_getBytesForAdpcmInfo;
extern lpFunc3 dsptool_encode;
extern lpFunc4 dsptool_decode;
extern lpFunc5 dsptool_getLoopContext;

//==============================================================================
