#ifndef _SPEEX_HPP_
#define _SPEEX_HPP_

#include "x_types.hpp"

//------------------------------------------------------------------------------
// DEFINES
//------------------------------------------------------------------------------

// Error codes
#define LGCODEC_SUCCESS                          (0x00000000)
#define LGCODEC_ERROR                            (0x80000000)
#define LGCODEC_ERR_INVALID_PARAMETER             (0x80000004)
#define LGCODEC_ERR_ALREADY_OPENED                (0x80000005)
#define LGCODEC_ERR_DEVICE_LOST                   (0x80000006)
#define LGCODEC_ERR_OUT_OF_MEMORY                 (0x80000007)

// quick test against error condition
#define LGCODEC_SUCCEEDED(x)                      (0 == ((x) & LGCODEC_ERROR))
#define LGCODEC_FAILED(x)                         (0 != ((x) & LGCODEC_ERROR))

#include "speex/speex.h"
//#include "liblgcodec.h"

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

#define SPEEX8_SAMPLES_PER_FRAME    160
#define SPEEX8_BYTES_PER_EFRAME     20

typedef struct SPEEX8
{
//    lgCodecHeader header;
    void* encode_state; 
    void* decode_state; 
    short encode_in[SPEEX8_SAMPLES_PER_FRAME];
    int encode_in_samples;
    unsigned char decode_in[SPEEX8_BYTES_PER_EFRAME];
    int decode_in_bytes;
    SpeexBits speex_bits;
}SPEEX8;

xbool   SpeexInit(void);
xbool   SpeexKill(void);

xbool   SpeexEncode(const s16* src, const u32 src_size, u8* dest, s32* dest_size);
xbool   SpeexDecode(const u8* src, const u32 src_size, s16* dest, s32* dest_size);
#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif

