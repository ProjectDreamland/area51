#ifndef _LPC10_HPP_
#define _LPC10_HPP_

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

#define LPC10_SAMPLES_PER_FRAME         (180)
#define LPC10_BYTES_PER_EFRAME          (7)
#define LPC10_BITS_IN_COMPRESSED_FRAME  (54)

//------------------------------------------------------------------------------
// STRUCTS
//------------------------------------------------------------------------------

struct LPC10_ENCODER_STATE 
{
    /* State used only by function hp100 */
    f32 z11;
    f32 z21;
    f32 z12;
    f32 z22;
    
    /* State used by function analys */
    f32 inbuf[540], pebuf[540];
    f32 lpbuf[696], ivbuf[312];
    f32 bias;
    s32 osbuf[10];  /* no initial value necessary */
    s32 osptr;     /* initial value 1 */
    s32 obound[3];
    s32 vwin[6]    /* was [2][3] */;   /* initial value vwin[4] = 307; vwin[5] = 462; */
    s32 awin[6]    /* was [2][3] */;   /* initial value awin[4] = 307; awin[5] = 462; */
    s32 voibuf[8]    /* was [2][4] */;
    f32 rmsbuf[3];
    f32 rcbuf[30]    /* was [10][3] */;
    f32 zpre;

    /* State used by function onset */
    f32 n;
    f32 d;   /* initial value 1.f */
    f32 fpc;   /* no initial value necessary */
    f32 l2buf[16];
    f32 l2sum1;
    s32 l2ptr1;   /* initial value 1 */
    s32 l2ptr2;   /* initial value 9 */
    s32 lasti;    /* no initial value necessary */
    s32 hyst;   /* initial value FALSE_ */

    /* State used by function voicin */
    f32 dither;   /* initial value 20.f */
    f32 snr;
    f32 maxmin;
    f32 voice[6]    /* was [2][3] */;   /* initial value is probably unnecessary */
    s32 lbve, lbue, fbve, fbue;
    s32 ofbue, sfbue;
    s32 olbue, slbue;

    /* State used by function dyptrk */
    f32 s[60];
    s32 p[120]    /* was [60][2] */;
    s32 ipoint;
    f32 alphax;

    /* State used by function chanwr */
    s32 isync;

    /* added for lower rates */
    s32 lframe;

    /* misc */
    s32 order;
};

struct LPC10_DECODER_STATE 
{
    /* State used by function decode */
    s32 iptold;   /* initial value 60 */
    s32 first;   /* initial value TRUE_ */
    s32 ivp2h;
    s32 iovoic;
    s32 iavgp;   /* initial value 60 */
    s32 erate;
    s32 drc[30]    /* was [3][10] */;
    s32 dpit[3];
    s32 drms[3];

    /* State used by function synths */
    f32 buf[360];
    s32 buflen;   /* initial value 180 */

    /* State used by function pitsyn */
    s32 ivoico;   /* no initial value necessary as long as first_pitsyn is initially TRUE_ */
    s32 ipito;   /* no initial value necessary as long as first_pitsyn is initially TRUE_ */
    f32 rmso;   /* initial value 1.f */
    f32 rco[10];   /* no initial value necessary as long as first_pitsyn is initially TRUE_ */
    s32 jsamp;   /* no initial value necessary as long as first_pitsyn is initially TRUE_ */
    s32 first_pitsyn;   /* initial value TRUE_ */

    /* State used by function bsynz */
    s32 ipo;
    f32 exc[166];
    f32 exc2[166];
    f32 lpi1;
    f32 lpi2;
    f32 lpi3;
    f32 hpi1;
    f32 hpi2;
    f32 hpi3;
    f32 rmso_bsynz;

    /* State used by function deemp */
    f32 dei1;
    f32 dei2;
    f32 deo1;
    f32 deo2;
    f32 deo3;

    /* added for lower rates */
    s32 lframe;

    /* misc */
    s32 order;
};

struct LPC10
{
    const char *            name;
    s32                     bits_per_second;
    s32                     bytes_per_pcm_frame; // in
    s32                     bytes_per_enc_frame; // out

    LPC10_ENCODER_STATE *   encode_state;
    s16                     encode_in[LPC10_SAMPLES_PER_FRAME];
    s32                     encode_in_samples;

    LPC10_DECODER_STATE *   decode_state;    
    u8                      decode_in[LPC10_BYTES_PER_EFRAME];
    s32                     decode_in_bytes;
};

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

xbool LPC10Init( void );
xbool LPC10Kill( void );

xbool LPC10Encode( const s16 *src, const u32 src_size, u8  *dest, s32 *dest_size );
xbool LPC10Decode( const u8  *src, const u32 src_size, s16 *dest, s32 *dest_size );

#endif //_LPC10_HPP_

