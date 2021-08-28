
#ifndef _LIBLGCODEC_H_
#define _LIBLGCODEC_H_

#include <sys/types.h>

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" 
{
#endif

typedef struct lgCodecDesc
{
    int id;
    const char* name;
    int bits_per_second;
    int bytes_per_pcm_frame; // in
    int bytes_per_enc_frame; // out
}lgCodecDesc;

typedef struct lgCodecHeader
{
    lgCodecDesc desc;
    

}lgCodecHeader;


int lgCodecInit(void);
int lgCodecEncode(int codec_handle, const short* src, int src_size, u_char* dest, int* dest_size);
int lgCodecDecode(int codec_handle, const u_char* src, int src_size, short* dest, int* dest_size);
int lgCodecOpen(int codec_id, int* codec_handle);
int lgCodecClose(int codec_handle);
lgCodecDesc* lgCodecEnumerate(int index);


//************************************************************************
// Error codes
//************************************************************************

#define LGCODEC_SUCCESS                          (0x00000000)
#define LGCODEC_ERROR                            (0x80000000)
#define LGCODEC_ERR_INVALID_PARAMETER             (0x80000004)
#define LGCODEC_ERR_ALREADY_OPENED                (0x80000005)
#define LGCODEC_ERR_DEVICE_LOST                   (0x80000006)
#define LGCODEC_ERR_OUT_OF_MEMORY                 (0x80000007)

// quick test against error condition
#define LGCODEC_SUCCEEDED(x)                      (0 == ((x) & LGCODEC_ERROR))
#define LGCODEC_FAILED(x)                         (0 != ((x) & LGCODEC_ERROR))

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif
