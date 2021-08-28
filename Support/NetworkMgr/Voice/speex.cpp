#include "x_memory.hpp"
#include "x_plus.hpp"
#include "x_log.hpp"
#include "x_debug.hpp"
#include "x_math.hpp"

#include "Speex.hpp"
#include "Speex/Speex.h"

//------------------------------------------------------------------------------
// VARIABLES
//------------------------------------------------------------------------------

static SPEEX8 *g_SPEEX = NULL;

//------------------------------------------------------------------------------
// FORWARD DECLARATIONS
//------------------------------------------------------------------------------

int speex8_frame_encode(short* in, unsigned char* out, void* enc_param);
int speex8_frame_decode(unsigned char* in, short* out, void* dec_param);

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------
extern  void    speex_bits_init(SpeexBits* bits);
extern  int     speex_decoder_ctl(void* state, int request, void* ptr);
extern  int     speex_encoder_ctl(void* state, int request, void* ptr);

xbool SpeexInit( void )
{
    SPEEX8* pcodec = (SPEEX8*)x_malloc(sizeof(SPEEX8));
    int val;

    x_memset(pcodec, 0, sizeof(SPEEX8));
    g_SPEEX = pcodec;

    // codec will allocate enc/dec states
    pcodec->encode_state = speex_encoder_init(&speex_nb_mode);
    pcodec->decode_state = speex_decoder_init(&speex_nb_mode);

    // codec will allocate a bit array
    speex_bits_init(&pcodec->speex_bits);

    // set some control values
    val=0;
    speex_decoder_ctl(pcodec->decode_state, SPEEX_SET_ENH, &val);
    val=0;
    speex_encoder_ctl(pcodec->encode_state, SPEEX_SET_VBR, &val);
    val=1;
    speex_encoder_ctl(pcodec->encode_state, SPEEX_SET_QUALITY, &val);
    val=1;
    speex_encoder_ctl(pcodec->encode_state, SPEEX_SET_COMPLEXITY, &val);
    return TRUE;
}

//------------------------------------------------------------------------------

xbool SpeexKill( void )
{
    ASSERT( g_SPEEX != NULL );

    speex_bits_destroy(&g_SPEEX->speex_bits);
    x_free( g_SPEEX->encode_state );
    x_free( g_SPEEX->decode_state );
    x_free( g_SPEEX);
        
    return TRUE;
}

//------------------------------------------------------------------------------

xbool SpeexEncode( const s16 *src, const u32 src_size, u8 *dest, s32 *dest_size )
{
    const s16 * next_sample_frame   = src;
    u8 *        next_enc_frame      = dest;

    s32 consume_samples             = 0;
    s32 samples_to_encode           = 0;
    s32 max_frames_to_decode        = 0;
    s32 bytes_encoded               = 0;

    ASSERT( src != NULL );
    ASSERT( dest != NULL );
    ASSERT( (src_size % sizeof(s16)) == 0 );
    ASSERT( src_size > 0 );
    ASSERT( dest_size != NULL );
    ASSERT( *dest_size > 0 );

    // quick check to make sure destination has enough space
    max_frames_to_decode = (src_size/2)/SPEEX8_SAMPLES_PER_FRAME;
    
    // if there is residue in the encode_in buffer AND the dest_size is not a multiple of the bytes_per_enc_frame the caller is probably using an arbitrary buffer. be safe and require an extra frame
    if (g_SPEEX->encode_in_samples && (*dest_size % SPEEX8_BYTES_PER_EFRAME))
    {
        max_frames_to_decode++;
    }
    
    if (*dest_size < (max_frames_to_decode*SPEEX8_BYTES_PER_EFRAME))
    {
        LOG_ERROR( "SPEEXEncode", "ERROR: bytes provided = %d. Need=%d\n", *dest_size, max_frames_to_decode*SPEEX8_BYTES_PER_EFRAME );
        *dest_size = 0;
        return FALSE;
    }

    samples_to_encode = (src_size/2);

    // clear output fields
    x_memset( dest, 0, (u32)*dest_size );        
    *dest_size = 0;

    while (1)
    {
        // fill up the buffer (there may be samples already in the encoding buffer)
        consume_samples = MIN( SPEEX8_SAMPLES_PER_FRAME - g_SPEEX->encode_in_samples, samples_to_encode );
        x_memcpy( g_SPEEX->encode_in + g_SPEEX->encode_in_samples, next_sample_frame, (u32)consume_samples*2 );
        g_SPEEX->encode_in_samples += consume_samples;
        next_sample_frame += consume_samples;
        
        // encode it
        if (g_SPEEX->encode_in_samples == SPEEX8_SAMPLES_PER_FRAME)
        {
            bytes_encoded += speex8_frame_encode( g_SPEEX->encode_in, next_enc_frame, g_SPEEX );

            samples_to_encode -= consume_samples;
            g_SPEEX->encode_in_samples = 0;
            x_memset( g_SPEEX->encode_in, 0, (u32)sizeof(g_SPEEX->encode_in) );
            next_enc_frame += SPEEX8_BYTES_PER_EFRAME;
        }
        else
        {
            // copy whatever is in the output buffer to the destination
            *dest_size = bytes_encoded;
            break;            
        }

    }

    return TRUE;
}

//------------------------------------------------------------------------------

xbool SpeexDecode( const u8 *src, const u32 src_size, s16 *dest, s32 *dest_size )
{
    const u8 *  next_enc_frame      = src;
    s16 *       next_sample_frame   = dest;

    s32         frames_to_decode    = 0;
    s32         consume_bytes       = 0;
    s32         samples_decoded     = 0;
    s32         bytes_to_encode     = 0;
    u32         dest_bytes_left     = 0;

    ASSERT( src != NULL );
    ASSERT( dest != NULL );
    ASSERT( dest_size != NULL );
    ASSERT( (*dest_size % sizeof(s16)) == 0 );
    ASSERT( src_size > 0 );
    ASSERT( *dest_size > 0 );

    bytes_to_encode = src_size;
    dest_bytes_left = *dest_size;

    // we must have at least bytes_per_enc_frame bytes
    frames_to_decode = src_size / SPEEX8_BYTES_PER_EFRAME;
    if (frames_to_decode <= 0)
    {
        *dest_size = 0;
        return TRUE;
    }

    frames_to_decode = MIN( *dest_size / (SPEEX8_SAMPLES_PER_FRAME * 2),     frames_to_decode );
    bytes_to_encode  = MIN( frames_to_decode * SPEEX8_SAMPLES_PER_FRAME * 2, bytes_to_encode  );

    // clear output fields
    x_memset( dest, 0, (unsigned int)*dest_size );        
    *dest_size = 0;

    // decode the data
    while (1)
    {

        // fill up the buffer (there may be samples already in the decoding buffer)
        consume_bytes = MIN( SPEEX8_BYTES_PER_EFRAME - g_SPEEX->decode_in_bytes, bytes_to_encode );
        x_memcpy( g_SPEEX->decode_in + g_SPEEX->decode_in_bytes, next_enc_frame, (u32)consume_bytes );
        next_enc_frame += consume_bytes;
        g_SPEEX->decode_in_bytes += consume_bytes;

        if (g_SPEEX->decode_in_bytes == SPEEX8_BYTES_PER_EFRAME && (dest_bytes_left >= (SPEEX8_SAMPLES_PER_FRAME*sizeof(s16))))
        {
            speex8_frame_decode( g_SPEEX->decode_in, next_sample_frame, g_SPEEX );

            samples_decoded += SPEEX8_SAMPLES_PER_FRAME;
            bytes_to_encode -= SPEEX8_BYTES_PER_EFRAME;
            next_sample_frame += SPEEX8_SAMPLES_PER_FRAME;
            g_SPEEX->decode_in_bytes = 0;
            dest_bytes_left -= (samples_decoded * sizeof(short));
        }
        else
        {
            // mark the total bytes decoded
            *dest_size = (samples_decoded * 2);
            break;
        }
    }

    return TRUE;

}


int speex8_frame_encode(short* in, unsigned char* out, void* enc_param)
{
    SPEEX8* codec = (SPEEX8*)enc_param;
    int enc_bytes = 0;

    // reset the bits array
    speex_bits_reset(&codec->speex_bits);

    // encode it
    speex_encode(codec->encode_state, in, &codec->speex_bits);

    // copy the speex bits into the out buffer
    enc_bytes = speex_bits_write(&codec->speex_bits, (char*)out, SPEEX8_BYTES_PER_EFRAME);

    return SPEEX8_BYTES_PER_EFRAME;
}

int speex8_frame_decode(unsigned char* in, short* out, void* dec_param)
{
    SPEEX8* codec = (SPEEX8*)dec_param;

    // copy the encoded buffer to the speex bits
    speex_bits_reset(&codec->speex_bits);
    speex_bits_read_from(&codec->speex_bits, (char*)in, SPEEX8_BYTES_PER_EFRAME);

    // decode the float buffer
    speex_decode(codec->decode_state, &codec->speex_bits, out);

    return SPEEX8_SAMPLES_PER_FRAME;
}

extern "C"
{
    void *speex_alloc (int size)
    {
        void* ptr;
       ptr = x_malloc(size);
       if( ptr )
       {
        x_memset(ptr,0,size);
       }
       return ptr;
    }

    void *speex_realloc (void *ptr, int size)
    {
       return x_realloc(ptr, size);
    }

    void speex_free (void *ptr)
    {
       x_free(ptr);
    }

    void *speex_move (void *dest, void *src, int n)
    {
       return x_memmove(dest,src,n);
    }
}
