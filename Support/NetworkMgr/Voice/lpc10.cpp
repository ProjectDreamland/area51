#include "x_memory.hpp"
#include "x_plus.hpp"
#include "x_log.hpp"
#include "x_debug.hpp"
#include "x_math.hpp"

#include "lpc10.hpp"

//------------------------------------------------------------------------------
// VARIABLES
//------------------------------------------------------------------------------

static LPC10 *g_LPC10 = NULL;
static s32 c10 = 10;

//------------------------------------------------------------------------------
// FORWARD DECLARATIONS
//------------------------------------------------------------------------------

void init_lpc10_encoder_state( LPC10_ENCODER_STATE *st );
void init_lpc10_decoder_state( LPC10_DECODER_STATE *st );

s32 LPC10FrameEncode( s16 *in, u8 * out );
s32 LPC10FrameDecode( u8 * in, s16 *out );

// encoding functions
void hp100( f32 *speech, s32 end, LPC10_ENCODER_STATE *st );
void analys( f32 *, s32 *, s32 *, f32 *, f32 *, LPC10_ENCODER_STATE * );
s32  chanwr( s32 *, s32 *, s32 *, s32 *, s32 *, LPC10_ENCODER_STATE * );
void encode( s32 *voice, s32 pitch, f32 rms, f32 *rc, s32 *ipitch, s32 *irms, s32 *irc, LPC10_ENCODER_STATE *st );

// decoding function
s32  chanrd( s32 *, s32 *, s32 *, s32 *, s32 * );
void synths( s32 *voice, s32 pitch, f32 rms, f32 *rc, f32 *speech, LPC10_DECODER_STATE *st );
void decode( s32 ipitv, s32 irms, s32 *irc, s32 *voice, s32 *pitch, f32 *rms, f32 *rc, LPC10_DECODER_STATE *st );

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------

xbool LPC10Init( void )
{
    ASSERT( g_LPC10 == NULL );

    g_LPC10 = (LPC10 *)x_malloc( sizeof(LPC10) );
    x_memset( g_LPC10, 0, sizeof(LPC10) );

    g_LPC10->encode_state = (LPC10_ENCODER_STATE *)x_malloc( sizeof(LPC10_ENCODER_STATE) );
    init_lpc10_encoder_state( g_LPC10->encode_state );

    g_LPC10->decode_state = (LPC10_DECODER_STATE *)x_malloc( sizeof(LPC10_DECODER_STATE) );
    init_lpc10_decoder_state( g_LPC10->decode_state );

    g_LPC10->name = "LPC10";
    g_LPC10->bytes_per_enc_frame = LPC10_BYTES_PER_EFRAME;
    g_LPC10->bytes_per_pcm_frame = LPC10_SAMPLES_PER_FRAME * sizeof(s16);
    g_LPC10->bits_per_second = 2400;
        
    return TRUE;
}

//------------------------------------------------------------------------------

xbool LPC10Kill( void )
{
    ASSERT( g_LPC10 != NULL );

    x_free( g_LPC10->encode_state );
    x_free( g_LPC10->decode_state );
    x_free( g_LPC10 );
    g_LPC10 = NULL;
        
    return TRUE;
}

//------------------------------------------------------------------------------

xbool LPC10Encode( const s16 *src, const u32 src_size, u8 *dest, s32 *dest_size )
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
    max_frames_to_decode = (src_size/2)/LPC10_SAMPLES_PER_FRAME;
    
    // if there is residue in the encode_in buffer AND the dest_size is not a multiple of the bytes_per_enc_frame the caller is probably using an arbitrary buffer. be safe and require an extra frame
    if (g_LPC10->encode_in_samples && (*dest_size % LPC10_BYTES_PER_EFRAME))
    {
        max_frames_to_decode++;
    }
    
    if (*dest_size < (max_frames_to_decode*LPC10_BYTES_PER_EFRAME))
    {
        LOG_ERROR( "LPC10Encode", "ERROR: bytes provided = %d. Need=%d\n", *dest_size, max_frames_to_decode*LPC10_BYTES_PER_EFRAME );
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
        consume_samples = MIN( LPC10_SAMPLES_PER_FRAME - g_LPC10->encode_in_samples, samples_to_encode );
        x_memcpy( g_LPC10->encode_in + g_LPC10->encode_in_samples, next_sample_frame, (u32)consume_samples*2 );
        g_LPC10->encode_in_samples += consume_samples;
        next_sample_frame += consume_samples;
        
        // encode it
        if (g_LPC10->encode_in_samples == LPC10_SAMPLES_PER_FRAME)
        {
            bytes_encoded += LPC10FrameEncode( g_LPC10->encode_in, next_enc_frame );

            samples_to_encode -= consume_samples;
            g_LPC10->encode_in_samples = 0;
            x_memset( g_LPC10->encode_in, 0, (u32)sizeof(g_LPC10->encode_in) );
            next_enc_frame += LPC10_BYTES_PER_EFRAME;
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

xbool LPC10Decode( const u8 *src, const u32 src_size, s16 *dest, s32 *dest_size )
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
    frames_to_decode = src_size / LPC10_BYTES_PER_EFRAME;
    if (frames_to_decode <= 0)
    {
        *dest_size = 0;
        return TRUE;
    }

    frames_to_decode = MIN( *dest_size / (LPC10_SAMPLES_PER_FRAME * 2),     frames_to_decode );
    bytes_to_encode  = MIN( frames_to_decode * LPC10_SAMPLES_PER_FRAME * 2, bytes_to_encode  );

    // clear output fields
    x_memset( dest, 0, (unsigned int)*dest_size );        
    *dest_size = 0;

    // decode the data
    while (1)
    {

        // fill up the buffer (there may be samples already in the decoding buffer)
        consume_bytes = MIN( LPC10_BYTES_PER_EFRAME - g_LPC10->decode_in_bytes, bytes_to_encode );
        x_memcpy( g_LPC10->decode_in + g_LPC10->decode_in_bytes, next_enc_frame, (u32)consume_bytes );
        next_enc_frame += consume_bytes;
        g_LPC10->decode_in_bytes += consume_bytes;

        if (g_LPC10->decode_in_bytes == LPC10_BYTES_PER_EFRAME && (dest_bytes_left >= (LPC10_SAMPLES_PER_FRAME*sizeof(s16))))
        {
            LPC10FrameDecode( g_LPC10->decode_in, next_sample_frame );

            samples_decoded += LPC10_SAMPLES_PER_FRAME;
            bytes_to_encode -= LPC10_BYTES_PER_EFRAME;
            next_sample_frame += LPC10_SAMPLES_PER_FRAME;
            g_LPC10->decode_in_bytes = 0;
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

/*
    return lgFrameDecode(src, src_size, dest, dest_size,
                         g_LPC10->decode_in, sizeof(lpc10->decode_in), &g_LPC10->decode_in_bytes, 
                         LPC10_SAMPLES_PER_FRAME, LPC10_BYTES_PER_EFRAME, 
                         lpc10_frame_decode, lpc10);
*/
}

//------------------------------------------------------------------------------

void init_lpc10_encoder_state( LPC10_ENCODER_STATE *st )
{
    s32 i;
    st->z11 = 0.0f;
    st->z21 = 0.0f;
    st->z12 = 0.0f;
    st->z22 = 0.0f;
    for (i = 0; i < 540; i++) 
    {
	    st->inbuf[i] = 0.0f;
	    st->pebuf[i] = 0.0f;
    }
    for (i = 0; i < 696; i++) st->lpbuf[i] = 0.0f;
    for (i = 0; i < 312; i++) st->ivbuf[i] = 0.0f;
    st->bias = 0.0f;
    st->osptr = 1;
    for (i = 0; i < 3; i++) st->obound[i] = 0;
    st->vwin[4] = 307;
    st->vwin[5] = 462;
    st->awin[4] = 307;
    st->awin[5] = 462;
    for (i = 0; i < 8; i++) st->voibuf[i] = 0;
    for (i = 0; i < 3; i++) st->rmsbuf[i] = 0.0f;
    for (i = 0; i < 30; i++) st->rcbuf[i] = 0.0f;
    st->zpre = 0.0f;
    st->n = 0.0f;
    st->d = 1.0f;
    for (i = 0; i < 16; i++) st->l2buf[i] = 0.0f;
    st->l2sum1 = 0.0f;
    st->l2ptr1 = 1;
    st->l2ptr2 = 9;
    st->hyst = FALSE;
    st->dither = 20.0f;
    st->maxmin = 0.0f;
    for (i = 0; i < 6; i++) st->voice[i] = 0.0f;
    st->lbve = 3000;
    st->fbve = 3000;
    st->fbue = 187;
    st->ofbue = 187;
    st->sfbue = 187;
    st->lbue = 93;
    st->olbue = 93;
    st->slbue = 93;
    st->snr = (f32)(st->fbve / st->fbue << 6);
    for (i = 0; i < 60; i++) st->s[i] = 0.0f;
    for (i = 0; i < 120; i++) st->p[i] = 0;
    st->ipoint = 0;
    st->alphax = 0.0f;
    st->isync = 0;
    st->lframe = 180;
    st->order = 10;
}

//------------------------------------------------------------------------------

void init_lpc10_decoder_state( LPC10_DECODER_STATE *st )
{
    s32 i;
    st->iptold = 60;
    st->first = TRUE;
    st->ivp2h = 0;
    st->iovoic = 0;
    st->iavgp = 60;
    st->erate = 0;
    for (i = 0; i < 30; i++) st->drc[i] = 0;
    for (i = 0; i < 3; i++) 
    {
	    st->dpit[i] = 0;
	    st->drms[i] = 0;
    }
    for (i = 0; i < 360; i++) st->buf[i] = 0.0f;
    st->buflen = 180;
    st->rmso = 1.0f;
    st->first_pitsyn = TRUE;
    st->ipo = 0;
    for (i = 0; i < 166; i++)
    {
	    st->exc[i] = 0.0f;
    	st->exc2[i] = 0.0f;
    }
    st->lpi1 = 0.0f;
    st->lpi2 = 0.0f;
    st->lpi3 = 0.0f;
    st->hpi1 = 0.0f;
    st->hpi2 = 0.0f;
    st->hpi3 = 0.0f;
    st->rmso_bsynz = 0.0f;
    st->dei1 = 0.0f;
    st->dei2 = 0.0f;
    st->deo1 = 0.0f;
    st->deo2 = 0.0f;
    st->deo3 = 0.0f;
    st->lframe = 180;
    st->order = 10;
}

//------------------------------------------------------------------------------

int LPC10FrameEncode( s16 *in, u8 *out )
{
    f32     speech[LPC10_SAMPLES_PER_FRAME];
    f32 *   sp = speech;
    s32     bits[LPC10_BITS_IN_COMPRESSED_FRAME];
    s32     i;
    s32     irms, voice[2], pitch, ipitv;
    f32     rc[10];
    s32     irc[10];
    f32     rms;

    /* convert sound from short to float */
    for (i = LPC10_SAMPLES_PER_FRAME; i--; )
    {
        *sp++ = (float)(*in++ / 32768.0f);
    }

    /* encode it */
    x_memset( bits, 0, sizeof(bits) );
    {
        hp100 ( speech, g_LPC10->encode_state->lframe, g_LPC10->encode_state );
        analys( speech, voice, &pitch, &rms, rc, g_LPC10->encode_state );
        encode( voice, pitch, rms, rc, &ipitv, &irms, irc, g_LPC10->encode_state );
        chanwr( &c10, &ipitv, &irms, irc, bits, g_LPC10->encode_state );
    }

    /* pack the bits */
    x_memset( out, 0, LPC10_BYTES_PER_EFRAME );
	for (i = 0; i < LPC10_BITS_IN_COMPRESSED_FRAME; i++)
    {
        out[i >> 3] |= ((bits[i] != 0)? 1 : 0) << (i & 7);
	}

    /* return 7 bytes encoded*/
    return LPC10_BYTES_PER_EFRAME;
}

//------------------------------------------------------------------------------

int LPC10FrameDecode( u8 *in, s16 *out )
{
    f32     speech[LPC10_SAMPLES_PER_FRAME];
    s32     bits[LPC10_BITS_IN_COMPRESSED_FRAME];
    s32     i;
    s32     irms, voice[2], pitch, ipitv;
    f32     rc[10];
    s32     irc[10];
    f32     rms;

    /* unpack bits into array */
	for (i = 0; i < LPC10_BITS_IN_COMPRESSED_FRAME; i++)
    {
        bits[i] = (in[i >> 3] & (1 << (i & 7))) != 0 ? 1 : 0;
	}

    /* decode speech */
    {
        chanrd( &c10, &ipitv, &irms, irc, &bits[0] );
        decode( ipitv, irms, irc, voice, &pitch, &rms, rc, g_LPC10->decode_state );
        synths( voice, pitch, rms, rc, &speech[0], g_LPC10->decode_state );
    }

    /* convert from float to short */
    for (i = 0; i < LPC10_SAMPLES_PER_FRAME; i++)
    {
        out[i] = (s16)MINMAX( -32768, (s16)( 32768.0f * speech[i] + 0.5f ), 32767 );
    }

    return LPC10_SAMPLES_PER_FRAME;
}

//------------------------------------------------------------------------------

void hp100( f32 *speech, s32 end, LPC10_ENCODER_STATE *st )
{
    f32 z11;
    f32 z21;
    f32 z12;
    f32 z22;
    s32 i;
    f32 si, err;

    z11 = st->z11;
    z21 = st->z21;
    z12 = st->z12;
    z22 = st->z22;

    for (i = 0; i < end; i++) 
    {
	    err = *speech + z11 * 1.859076f - z21 * .8648249f;
	    si = err - z11 * 2.f + z21;
	    z21 = z11;
	    z11 = err;
	    err = si + z12 * 1.935715f - z22 * .9417004f;
	    si = err - z12 * 2.f + z22;
	    z22 = z12;
	    z12 = err;
	    *speech++ = si * .902428f;
    }

    st->z11 = z11;
    st->z21 = z21;
    st->z12 = z12;
    st->z22 = z22;
} 

//------------------------------------------------------------------------------

static s32 c3 = 3;
static s32 c156 = 156;

inline
void mload( s32 awins, s32 awinf, f32 *speech, f32 *phi, f32 *psi )
{
    /* Local variables */
    s32 c, i, r, start;
    
    /*       Arguments */
    /*       Local variables that need not be saved */
    /*   Load first column of triangular covariance matrix PHI */
    /* Parameter adjustments */
    phi -= 10;
    speech--;
    
    /* Function Body */
    start = awins + 10;
    for (r = 1; r <= 10; ++r) 
    {
        phi[r + 10 - 1] = 0.f;
        for (i = start; i <= awinf; ++i) 
        {
            phi[r + 10 - 1] += speech[i - 1] * speech[i - r];
        }
    }
    /*   Load last element of vector PSI */
    psi[10 - 1] = 0.f;
    for (i = start; i <= awinf; ++i) 
    {
        psi[10 - 1] += speech[i] * speech[i - 10];
    }
    /*   End correct to get additional columns of PHI */
    for (r = 2; r <= 10; ++r) 
    {
        for (c = 2; c <= r; ++c) 
        {
            phi[r + c * 10 - 1] = phi[r - 1 + (c - 1) * 10 - 1] - 
                speech[awinf + 1 - r] * speech[awinf + 1 - c] + 
                speech[start - r] * speech[start - c];
        }
    }
    /*   End correct to get additional elements of PSI */
    for (c = 1; c < 10; ++c) 
    {
        psi[c - 1] = phi[c + 1 + 10 - 1] - speech[start]
                * speech[start - 1 - c] + speech[awinf] * speech[awinf - c];
    }
} 

inline
void rcchk( f32 *rc1f, f32 *rc2f )
{
    /* Local variables */
    s32 i;
    
    /* Function Body */
    for (i = 0; i < 10; ++i) 
    {
        if ((fabsf(rc2f[i])) > .99f) 
        {
            for (i = 0; i < 10; ++i) 
            {
                rc2f[i] = rc1f[i];
            }
            return;
        }
    }
} 

inline
void dcbias( s32 len, f32 *speech, f32 *sigout )
{
    /* Local variables */
    f32 bias;
    s32 i;

    /* Function Body */
    bias = 0.f;
    for (i = 0; i < len; ++i) {
        bias += speech[i];
    }
    bias /= len;
    for (i = 0; i < len; ++i) {
        *sigout++ = *speech++ - bias;
    }
}

inline
void preemp( f32 *inbuf, f32 *pebuf, s32 nsamp, f32 coef, f32 *z )
{
    /* Local variables */
    f32 temp;
    s32 i;

    /* Function Body */
    for (i = 0; i< nsamp; ++i) {
	    temp = *inbuf - coef * *z;
	    *z = *inbuf++;
	    *pebuf++ = temp;
    }
} 

inline
void lpfilt( f32 *inbuf, f32 *lpbuf, s32 len, s32 nsamp )
{
    /* Local variables */
    s32 j;
    
    /* Function Body */
    lpbuf = &lpbuf[len - nsamp];
    for (j = len - nsamp; j < len; ++j) 
    {
        *lpbuf++ = (inbuf[j] + inbuf[j - 30]) * -.0097201988f
                    + (inbuf[j - 1] + inbuf[j - 29]) * -.0105179986f
                    + (inbuf[j - 2] + inbuf[j - 28]) * -.0083479648f
                    + (inbuf[j - 3] + inbuf[j - 27]) * 5.860774e-4f
                    + (inbuf[j - 4] + inbuf[j - 26]) * .0130892089f
                    + (inbuf[j - 5] + inbuf[j - 25]) * .0217052232f
                    + (inbuf[j - 6] + inbuf[j - 24]) * .0184161253f
                    + (inbuf[j - 7] + inbuf[j - 23]) * 3.39723e-4f
                    + (inbuf[j - 8] + inbuf[j - 22]) * -.0260797087f
                    + (inbuf[j - 9] + inbuf[j - 21]) * -.0455563702f
                    + (inbuf[j - 10] + inbuf[j - 20]) * -.040306855f
                    + (inbuf[j - 11] + inbuf[j - 19]) * 5.029835e-4f
                    + (inbuf[j - 12] + inbuf[j - 18]) * .0729262903f
                    + (inbuf[j - 13] + inbuf[j - 17]) * .1572008878f
                    + (inbuf[j - 14] + inbuf[j - 16]) * .2247288674f
                    + inbuf[j - 15] * .250535965f;
    }
} 

inline
void ivfilt( f32 *lpbuf, f32 *ivbuf, s32 len, s32 nsamp, f32 *ivrc )
{
    /* Local variables */
    s32 i, j, k;
    f32 r[3], pc1, pc2;

    /* Parameter adjustments */
    --ivbuf;
    --lpbuf;
    --ivrc;
    
    /* Function Body */
    for (i = 1; i <= 3; ++i) 
    {
        r[i - 1] = 0.f;
        k = (i - 1) << 2;
        for (j = (i << 2) + len - nsamp; j <= len; j += 2) 
        {
            r[i - 1] += lpbuf[j] * lpbuf[j - k];
        }
    }

    /*  Calculate predictor coefficients */
    pc1 = 0.f;
    pc2 = 0.f;
    ivrc[1] = 0.f;
    ivrc[2] = 0.f;
    if (r[0] > 1e-10f) 
    {
        ivrc[1] = r[1] / r[0];
        ivrc[2] = (r[2] - ivrc[1] * r[1]) / (r[0] - ivrc[1] * r[1]);
        pc1 = ivrc[1] - ivrc[1] * ivrc[2];
        pc2 = ivrc[2];
    }
    /*  Inverse filter LPBUF into IVBUF */
    for (i = len + 1 - nsamp; i <= len; ++i) 
    {
        ivbuf[i] = lpbuf[i] - pc1 * lpbuf[i - 4] - pc2 * lpbuf[i - 8];
    }
} 

inline
void invert( f32 *phi, f32 *psi, f32 *rc )
{
    /* Local variables */
    f32 save;
    s32 i, j, k;
    f32 v[100]	/* was [10][10] */;
    
    /* Function Body */
    for (j = 0; j < 10; ++j) 
    {
        for (i = j; i < 10; ++i) 
        {
            v[i + j * 10] = phi[i + j * 10];
        }
        for (k = 0; k < j; ++k) 
        {
            save = v[j + k * 10] * v[k + k * 10];
            for (i = j; i < 10; ++i) 
            {
                v[i + j * 10] -= v[i + k * 10] * save;
            }
        }
        /*  Compute intermediate results, which are similar to RC's */
        if ((fabsf(v[j + j * 10])) < 1e-10f) 
        {
            /*  Zero out higher order RC's if algorithm terminated early */
            for (i = j; i < 10; ++i) 
            {
                rc[i] = 0.f;
            }
            return;
        }
        rc[j] = psi[j];
        for (k = 0; k < j; ++k) {
            rc[j] -= rc[k] * v[j + k * 10];
        }
        v[j + j * 10] = 1.f / v[j + j * 10];
        rc[j] *= v[j + j * 10];

        rc[j] = MINMAX( -0.999f, rc[j], 0.999f );
    }
} 

inline
void energy( s32 len, f32 *speech, f32 *rms )
{
    /* Local variables */
    s32 i;

    /* Parameter adjustments */
    --speech;

    /* Function Body */
    *rms = 0.f;
    for (i = 1; i <= len; ++i) 
    {
	    *rms += speech[i] * speech[i];
    }

    *rms = (f32)sqrtf(*rms / len);
} 

inline
void dyptrk( f32 *amdf, s32 ltau, s32 *minptr, s32 *voice, s32 *pitch, s32 *midx, LPC10_ENCODER_STATE *st )
{
    f32 *s;
    s32 *p;
    s32 *ipoint;
    f32 *alphax;
    s32 pbar;
    f32 sbar;
    s32 path[2], iptr, i, j;
    f32 alpha, minsc, maxsc;
    
    s = &(st->s[0]);
    p = &(st->p[0]);
    ipoint = &(st->ipoint);
    alphax = &(st->alphax);
    
    /* Parameter adjustments */
    if (amdf) --amdf;
    
    /* Function Body */
    if (*voice == 1) 
        *alphax = *alphax * .75f + amdf[*minptr] / 2.f;
    else 
        *alphax *= .984375f;

    alpha = *alphax / 16;
    if (*voice == 0 && *alphax < 128.f) 
        alpha = 8.f;

    /* SEESAW: Construct a pitch pointer array and intermediate winner function - Left to right pass: */
    iptr = *ipoint + 1;
    p[iptr * 60 - 60] = 1;
    i = 1;
    pbar = 1;
    sbar = s[0];
    for (i = 1; i <= ltau; ++i) 
    {
        sbar += alpha;
        if (sbar < s[i - 1]) 
        {
            s[i - 1] = sbar;
            p[i + iptr * 60 - 61] = pbar;
        } 
        else 
        {
            sbar = s[i - 1];
            p[i + iptr * 60 - 61] = i;
            pbar = i;
        }
    }

    /*   Right to left pass: */
    i = pbar - 1;
    sbar = s[i];
    while(i >= 1) 
    {
        sbar += alpha;
        if (sbar < s[i - 1]) 
        {
            s[i - 1] = sbar;
            p[i + iptr * 60 - 61] = pbar;
        } 
        else 
        {
            pbar = p[i + iptr * 60 - 61];
            i = pbar;
            sbar = s[i - 1];
        }
        --i;
    }

    /*   Update S using AMDF - Find maximum, minimum, and location of minimum */
    s[0] += amdf[1] / 2;
    minsc = s[0];
    maxsc = minsc;
    *midx = 1;
    for (i = 2; i <= ltau; ++i) 
    {
        s[i - 1] += amdf[i] / 2;
        if (s[i - 1] > maxsc) 
        {
            maxsc = s[i - 1];
        }
        if (s[i - 1] < minsc) 
        {
            *midx = i;
            minsc = s[i - 1];
        }
    }
    /*   Subtract MINSC from S to prevent overflow */
    for (i = 1; i <= ltau; ++i) 
    {
        s[i - 1] -= minsc;
    }
    maxsc -= minsc;

    /*   Use higher octave pitch if significant null there */
    j = 0;
    for (i = 20; i <= 40; i += 10) 
    {
        if (*midx > i) 
        {
            if (s[*midx - i - 1] < maxsc / 4)
            {
                j = i;
            }
        }
    }

    *midx -= j;

    /*   TRACE: look back two frames to find minimum cost pitch estimate */
    j = *ipoint;
    *pitch = *midx;
    for (i = 1; i <= 2; ++i) 
    {
        j = j % 2 + 1;
        *pitch = p[*pitch + j * 60 - 61];
        path[i - 1] = *pitch;
    }
    
    *ipoint = (*ipoint + 1) % 2;
}

inline
void onset( f32 *pebuf, s32 *osbuf, s32 *osptr, s32 oslen, s32 sbufl, s32 sbufh, s32 lframe, LPC10_ENCODER_STATE *st )
{
    /* Initialized data */
    f32 n;
    f32 d;
    f32 *l2buf;
    f32 *l2sum1;
    s32 *l2ptr1;
    s32 *l2ptr2;
    s32 *hyst;
    s32 pebuf_offset;
    f32 temp;
    s32 i;
    s32 *lasti;
    f32 l2sum2;
    f32 *fpc;

    n = st->n;
    d = st->d;
    fpc = &(st->fpc);
    l2buf = &(st->l2buf[0]);
    l2sum1 = &(st->l2sum1);
    l2ptr1 = &(st->l2ptr1);
    l2ptr2 = &(st->l2ptr2);
    lasti = &(st->lasti);
    hyst = &(st->hyst);
    
    /* Parameter adjustments */
    if (osbuf) --osbuf;
    if (pebuf) 
    {
        pebuf_offset = sbufl;
        pebuf -= pebuf_offset;
    }
    
    /* Function Body */
    if (*hyst) *lasti -= lframe;

    for (i = sbufh - lframe + 1; i <= sbufh; ++i) 
    {
    /*   Compute FPC; Use old FPC on divide by zero; Clamp FPC to +/- 1. */
        n = (pebuf[i] * pebuf[i - 1] + n * 63.f) * 0.015625f;
        /* Computing 2nd power */
        temp = pebuf[i - 1];
        d = (temp * temp + d * 63.f) * 0.015625f;
        if (d != 0.f) 
        {
            if(n > d || n < -(d))
            {
                *fpc = (n<0.0f)?-1.0f:1.0f;
            } 
            else 
            {
                *fpc = n / d;
            }
        }
        
        l2sum2 = l2buf[*l2ptr1 - 1];
        *l2sum1 = *l2sum1 - l2buf[*l2ptr2 - 1] + *fpc;
        l2buf[*l2ptr2 - 1] = *l2sum1;
        l2buf[*l2ptr1 - 1] = *fpc;
        *l2ptr1 = *l2ptr1 % 16 + 1;
        *l2ptr2 = *l2ptr2 % 16 + 1;
        temp = *l2sum1 - l2sum2;
        if (temp > 1.7f || temp < -1.7f) 
        {
            if (! (*hyst)) 
            {
                /*   Ignore if buffer full */
                if (*osptr <= oslen) 
                {
                    osbuf[*osptr] = i - 9;
                    ++(*osptr);
                }
                *hyst = TRUE;
            }
            *lasti = i;
            /*       After one onset detection, at least OSHYST sample times must go */
            /*       by before another is allowed to occur. */
        } 
        else if ((*hyst) && i - *lasti >= 10) 
        {
            *hyst = FALSE;
        }
    }
    st->n = n;
    st->d = d;
} 

inline
void placev( s32 *osbuf, s32 *osptr, s32 *obound, s32 *vwin, s32 af, s32 *lframe, s32 minwin, s32 maxwin, s32 dvwinl )
{
    /* Local variables */
    s32 crit;
    s32 i, q, osptr1, hrange, lrange;
    
    /* Parameter adjustments */
    --osbuf;
    vwin -= 3;
    
    /* Function Body */
    lrange = MAX(vwin[((af - 1) << 1) + 2] + 1, (af - 2) * *lframe + 1);
    hrange = af * *lframe;
    /* Compute OSPTR1, so the following code only looks at relevant onsets. */
    for (osptr1 = *osptr - 1; osptr1 >= 1; --osptr1) 
    {
        if (osbuf[osptr1] <= hrange) 
        {
            break;
        }
    }

    ++osptr1;
    /* Check for case 1 first (fast case): */
    if (osptr1 <= 1 || osbuf[osptr1 - 1] < lrange) 
    {
        /* Computing MAX */
        vwin[(af << 1) + 1] = MAX(vwin[((af - 1) << 1) + 2] + 1,dvwinl);
        vwin[(af << 1) + 2] = vwin[(af << 1) + 1] + maxwin - 1;
        *obound = 0;
    } 
    else 
    {
        /* Search backward in OSBUF for first onset in range. */
        /* This code relies on the above check being performed first. */
        for (q = osptr1 - 1; q >= 1; --q) 
        {
            if (osbuf[q] < lrange) 
            {
                break;
            }
        }
        ++q;
        /* Check for case 2 (placement before onset): */
        /* Check for critical region exception: */
        crit = FALSE;
        for (i = q + 1; i <= (osptr1 - 1); ++i) 
        {
            if (osbuf[i] - osbuf[q] >= minwin) 
            {
                crit = TRUE;
                break;
            }
        }
        /* Computing MAX */
        if (! crit && osbuf[q] > MAX((af - 1) * *lframe, lrange + minwin - 1)) 
        {
            vwin[(af << 1) + 2] = osbuf[q] - 1;
            /* Computing MAX */
            vwin[(af << 1) + 1] = MAX(lrange, vwin[(af << 1) + 2] - maxwin + 1);
            *obound = 2;
            /* Case 3 (placement after onset) */
        } 
        else 
        {
            vwin[(af << 1) + 1] = osbuf[q];
L110:
            ++q;
            if (q >= osptr1) 
            {
                goto L120;
            }
            if (osbuf[q] > vwin[(af << 1) + 1] + maxwin) 
            {
                goto L120;
            }
            if (osbuf[q] < vwin[(af << 1) + 1] + minwin) 
            {
                goto L110;
            }
            vwin[(af << 1) + 2] = osbuf[q] - 1;
            *obound = 3;
            return;
L120:
            /* Computing MIN */
            vwin[(af << 1) + 2] = MIN(vwin[(af << 1) + 1] + maxwin - 1,hrange);
            *obound = 1;
        }
    }
} 

inline
void difmag( f32 *speech, s32 lpita, s32 *tau, s32 ltau, s32 maxlag, f32 *amdf,s32 *minptr, s32 *maxptr )
{
    /* Local variables */
    s32 i, j, n1, n2;
    s32 lmin, lmax;
    f32 sum;
    
    /* Parameter adjustments */
    --amdf;
    --tau;
    --speech;
    
    /* Function Body */
    lmin = 1;
    lmax = 1;

    for (i = 1; i <= ltau; ++i) 
    {
        s32 t = tau[i];

        n1 = (maxlag - t) / 2 + 1;
        n2 = n1 + lpita - 1;
        sum = 0.f;
        t += n1;
        for (j = n1; j <= n2; j += 4, t += 4) 
        {
            f32 temp = speech[j] - speech[t];

            if(temp < 0.0f)
            {
                sum -= temp;
            }
            else
            {
                sum += temp;
            }
        }
        if (sum < amdf[lmin]) 
        {
            lmin = i;
        } 
        else if (sum > amdf[lmax]) 
        {
            lmax = i;
        }
        amdf[i] = sum;
    }
    *minptr = lmin;
    *maxptr = lmax;
} 

inline
void tbdm( f32 *speech, s32 lpita, s32 *tau, s32 ltau, f32 *amdf, s32 *minptr, s32 *maxptr, s32 *mintau )
{
    /* Local variables */
    f32 amdf2[6];
    s32 minp2, ltau2, maxp2, i, j;
    s32 minamd, ptr, tau2[6];
    
    /*   Compute full AMDF using log spaced lags, find coarse minimum */
    /* Parameter adjustments */
    --speech;
    --amdf;
    --tau;
    
    /* Function Body */
    difmag(&speech[1], lpita, &tau[1], ltau, tau[ltau], &amdf[1], minptr, maxptr);
    *mintau = tau[*minptr];
    minamd = (s32)amdf[*minptr];
    /*   Build table containing all lags within +/- 3 of the AMDF minimum */
    /*    excluding all that have already been computed */
    ltau2 = 0;
    ptr = *minptr - 2;
    /* Computing MIN */
    j = MIN(*mintau + 3, tau[ltau] - 1);
    /* Computing MAX */
    i = MAX(*mintau - 3, 41);
    for (; i <= j; ++i) {
        while(tau[ptr] < i) 
        {
            ++ptr;
        }
        if (tau[ptr] != i) 
        {
            ++ltau2;
            tau2[ltau2 - 1] = i;
        }
    }
    /*   Compute AMDF of the new lags, if there are any, and choose one */
    /*    if it is better than the coarse minimum */
    if (ltau2 > 0) 
    {
        difmag(&speech[1], lpita, tau2, ltau2, tau[ltau], amdf2, &minp2, &maxp2);
        if (amdf2[minp2 - 1] < (f32) minamd) 
        {
            *mintau = tau2[minp2 - 1];
            minamd = (s32)amdf2[minp2 - 1];
        }
    }
    /*   Check one octave up, if there are any lags not yet computed */
    if (*mintau >= 80) 
    {
        i = *mintau / 2;
        if ((i & 1) == 0) 
        {
            ltau2 = 2;
            tau2[0] = i - 1;
            tau2[1] = i + 1;
        } 
        else 
        {
            ltau2 = 1;
            tau2[0] = i;
        }
        difmag(&speech[1], lpita, tau2, ltau2, tau[ltau], amdf2, &minp2, &maxp2);
        if (amdf2[minp2 - 1] < (f32) minamd) 
        {
            *mintau = tau2[minp2 - 1];
            minamd = (s32)amdf2[minp2 - 1];
            *minptr += -20;
        }
    }

    /*   Force minimum of the AMDF array to the high resolution minimum */

    amdf[*minptr] = (f32) minamd;

    /*   Find maximum of AMDF within 1/2 octave of minimum */
    /* Computing MAX */
    *maxptr = MAX(*minptr - 5,1);
    /* Computing MIN */
    j = MIN(*minptr + 5,ltau);
    for (i = *maxptr + 1; i <= j; ++i) 
    {
        if (amdf[i] > amdf[*maxptr]) 
        {
            *maxptr = i;
        }
    }
} 

inline
void vparms(s32 *vwin, f32 *inbuf, f32 *lpbuf, s32 *buflim, s32 *half, f32 *dither, s32 *mintau, s32 *zc, s32 *lbe, s32 *fbe, f32 *qs, f32 *rc1, f32 *ar_b__, f32 *ar_f__ )
{
    /* System generated locals */
    s32 inbuf_offset, lpbuf_offset;
    f32 r__1;
    
    /* Local variables */
    s32 vlen, stop, i;
    f32 e_pre__;
    s32 start;
    f32 ap_rms__, e_0__, oldsgn, lp_rms__, e_b__, e_f__, r_b__, r_f__, e0ap;
    
    --vwin;
    --buflim;
    lpbuf_offset = buflim[3];
    lpbuf -= lpbuf_offset;
    inbuf_offset = buflim[1];
    inbuf -= inbuf_offset;
    
    /* Function Body */
    lp_rms__ = 0.f;
    ap_rms__ = 0.f;
    e_pre__ = 0.f;
    e0ap = 0.f;
    *rc1 = 0.f;
    e_0__ = 0.f;
    e_b__ = 0.f;
    e_f__ = 0.f;
    r_f__ = 0.f;
    r_b__ = 0.f;
    *zc = 0;
    vlen = vwin[2] - vwin[1] + 1;
    start = vwin[1] + (*half - 1) * vlen / 2 + 1;
    stop = start + vlen / 2 - 1;
    
    r__1 = inbuf[start - 1] - *dither;
    oldsgn = (r__1<0.0f)?-1.0f:1.0f;
    for (i = start; i <= stop; ++i) 
    {
        if(lpbuf[i] < 0.0f)
            lp_rms__ -= lpbuf[i];
        else
            lp_rms__ += lpbuf[i];
        if(inbuf[i] < 0.0f)
            ap_rms__ -= inbuf[i];
        else
            ap_rms__ += inbuf[i];
        r__1 = inbuf[i] - inbuf[i - 1];
        if(r__1 < 0.0f)
            e_pre__ -= r__1;
        else
            e_pre__ += r__1;
        /* Computing 2nd power */
        r__1 = inbuf[i];
        e0ap += r__1 * r__1;
        *rc1 += inbuf[i] * inbuf[i - 1];
        /* Computing 2nd power */
        r__1 = lpbuf[i];
        e_0__ += r__1 * r__1;
        /* Computing 2nd power */
        r__1 = lpbuf[i - *mintau];
        e_b__ += r__1 * r__1;
        /* Computing 2nd power */
        r__1 = lpbuf[i + *mintau];
        e_f__ += r__1 * r__1;
        r_f__ += lpbuf[i] * lpbuf[i + *mintau];
        r_b__ += lpbuf[i] * lpbuf[i - *mintau];
        r__1 = inbuf[i] + *dither;
        if (((r__1<0.0f)?-1.0f:1.0f) != oldsgn) 
        {
            ++(*zc);
            oldsgn = -oldsgn;
        }
        *dither = -(*dither);
    }

    *rc1 /= MAX(e0ap,1.f);
    *qs = e_pre__ / MAX(ap_rms__ * 2.f, 1.f);
    *ar_b__ = r_b__ / MAX(e_b__,1.f) * (r_b__ / MAX(e_0__,1.f));
    *ar_f__ = r_f__ / MAX(e_f__,1.f) * (r_f__ / MAX(e_0__,1.f));
    *zc = (s32)x_round((f32) (*zc << 1) * (90.f / vlen), 1.0f );
    /* Computing MIN */
    *lbe = MIN((s32)x_round(lp_rms__ * 0.25f * (90.f / vlen),1.0f),32767);
    /* Computing MIN */
    *fbe = MIN((s32)x_round(ap_rms__ * 0.25f * (90.f / vlen),1.0f),32767);
} /* vparms_ */

static f32 vdc[100]	= { 0.f,1714.f,-110.f, 334.f,-4096.f,-654.f,3752.f,3769.f,0.f,1181.f,0.f,874.f,-97.f, 300.f,-4096.f,-1021.f,2451.f,2527.f,0.f,-500.f,0.f,510.f,-70.f, 250.f,-4096.f,-1270.f,2194.f,2491.f,0.f,-1500.f,0.f,500.f,-10.f, 200.f,-4096.f,-1300.f,2e3f,2e3f,0.f,-2e3f,0.f,500.f,0.f,0.f, -4096.f,-1300.f,2e3f,2e3f,0.f,-2500.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f, 0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f, 0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f, 0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f };
static s32 nvdcl = 5;
static f32 vdcl[10] = { 600.f,450.f,300.f,200.f,0.f,0.f,0.f,0.f,0.f,0.f };

inline
void voicin( s32 *vwin, f32 *inbuf, f32 *lpbuf, s32 *buflim, s32 *half, f32 *minamd, f32 *maxamd, s32 *mintau, f32 *ivrc, s32 *obound, s32 *voibuf, LPC10_ENCODER_STATE *st )
{
    /* Initialized data */
    f32 *dither;
    
    /* System generated locals */
    s32 inbuf_offset, lpbuf_offset;
    
    /* Local variables */
    f32 ar_b__, ar_f__;
    s32 *lbve, *lbue, *fbve, *fbue;
    s32 snrl, i;
    s32 *ofbue, *sfbue;
    f32 *voice;
    s32 *olbue, *slbue;
    f32 value[9];
    s32 zc;
    s32 ot;
    f32 qs;
    f32 *maxmin;
    s32 vstate;
    f32 rc1;
    s32 fbe, lbe;
    f32 *snr;
    f32 snr2;
    
    
    /*   Declare and initialize filters: */
    dither = (&st->dither);
    snr = (&st->snr);
    maxmin = (&st->maxmin);
    voice = (&st->voice[0]);
    lbve = (&st->lbve);
    lbue = (&st->lbue);
    fbve = (&st->fbve);
    fbue = (&st->fbue);
    ofbue = (&st->ofbue);
    olbue = (&st->olbue);
    sfbue = (&st->sfbue);
    slbue = (&st->slbue);
    
    /* Parameter adjustments */
    if (vwin) --vwin;
    if (buflim) --buflim;
    if (ivrc) --ivrc;
    if (obound) --obound;
    if (voibuf) --voibuf;
    
    inbuf_offset = buflim[1];
    inbuf -= inbuf_offset;
    lpbuf_offset = buflim[3];
    lpbuf -= lpbuf_offset;
  
    /* Function Body */
   
    /*   Update linear discriminant function history each frame: */
    if (*half == 1) 
    {
        voice[0] = voice[2];
        voice[1] = voice[3];
        voice[2] = voice[4];
        voice[3] = voice[5];
        *maxmin = *maxamd / MAX(*minamd,1.f);
    }
    /*   Calculate voicing parameters twice per frame: */
    vparms( &vwin[1], &inbuf[inbuf_offset], &lpbuf[lpbuf_offset], &buflim[1], half, dither, mintau, &zc, &lbe, &fbe, &qs, &rc1, &ar_b__, &ar_f__ );
    *snr = x_round((*snr + *fbve / (f32) MAX(*fbue,1)) * 63 / 64.f, 1.0f);
    snr2 = *snr * *fbue / MAX(*lbue,1);

    /*   Quantize SNR to SNRL according to VDCL thresholds. */
    for (snrl = 1; snrl <= nvdcl - 1; ++snrl) 
    {
        if (snr2 > vdcl[snrl - 1]) break;
    }

    /*   Linear discriminant voicing parameters: */
    value[0] = *maxmin;
    value[1] = (f32) lbe / MAX(*lbve,1);
    value[2] = (f32) zc;
    value[3] = rc1;
    value[4] = qs;
    value[5] = ivrc[2];
    value[6] = ar_b__;
    value[7] = ar_f__;
    /*   Evaluation of linear discriminant function: */
    voice[*half + 3] = vdc[snrl * 10 - 1];
    for (i = 1; i <= 8; ++i) 
    {
        voice[*half + 3] += vdc[i + snrl * 10 - 11] * value[i - 1];
    }
    /*   Classify as voiced if discriminant > 0, otherwise unvoiced */
    /*   Voicing decision for current half-frame:  1 = Voiced; 0 = Unvoiced */
    if (voice[*half + 3] > 0.f) 
    {
        voibuf[*half + 6] = 1;
    } 
    else 
    {
        voibuf[*half + 6] = 0;
    }
    /*   Skip voicing decision smoothing in first half-frame: */
    vstate = -1;
    if (*half != 1) 
    {
        /*   Determine if there is an onset transition between P and 1F. */
        /*   OT (Onset Transition) is true if there is an onset between */
        /*   P and 1F but not after 1F. */
        ot = ((obound[1] & 2) != 0 || obound[2] == 1) && (obound[3] & 1) == 0;
        /*   Multi-way dispatch on voicing decision history: */
        vstate = (voibuf[3] << 3) + (voibuf[4] << 2) + (voibuf[5] << 1) + voibuf[6];
        
        switch (vstate + 1) 
        {
        case 1:
            break;
        case 2:
            if (ot && voibuf[7] == 1) 
            {
                voibuf[5] = 1;
            }
            break;
        case 3:
            if (voibuf[7] == 0 || voice[2] < -voice[3]) 
            {
                voibuf[5] = 0;
            } 
            else 
            {
                voibuf[6] = 1;
            }
            break;
        case 4:
            break;
        case 5:
            voibuf[4] = 0;
            break;
        case 6:
            if (voice[1] < -voice[2]) 
            {
                voibuf[4] = 0;
            } 
            else 
            {
                voibuf[5] = 1;
            }
            break;
        case 7:
            if (voibuf[1] == 1 || voibuf[7] == 1 || voice[3] > voice[0]) 
            {
                voibuf[6] = 1;
            } 
            else 
            {
                voibuf[3] = 1;
            }
            break;
        case 8:
            if (ot) 
            {
                voibuf[4] = 0;
            }
            break;
        case 9:
            if (ot) 
            {
                voibuf[4] = 1;
            }
            break;
        case 10:
            break;
        case 11:
            if (voice[2] < -voice[1])
            {
                voibuf[5] = 0;
            } 
            else 
            {
                voibuf[4] = 1;
            }
            break;
        case 12:
            voibuf[4] = 1;
            break;
        case 13:
            break;
        case 14:
            if (voibuf[7] == 0 && voice[3] < -voice[2]) 
            {
                voibuf[6] = 0;
            } 
            else 
            {
                voibuf[5] = 1;
            }
            break;
        case 15:
            if (ot && voibuf[7] == 0) 
            {
                voibuf[5] = 0;
            }
            break;
        case 16:
            break;
        }
    }
    /*   Now update parameters: */
    if (voibuf[*half + 6] == 0) 
    {
        /* Computing MIN */
        *sfbue = (s32)x_round((*sfbue * 63 + (MIN(fbe, *ofbue * 3) << 3)) / 64.f, 1.0f);
        *fbue = *sfbue / 8;
        *ofbue = fbe;
        /* Computing MIN */
        *slbue = (s32)x_round((*slbue * 63 + (MIN(lbe, *olbue * 3) << 3)) / 64.f,1.0f);
        *lbue = *slbue / 8;
        *olbue = lbe;
    } 
    else 
    {
        *lbve = (s32)x_round((*lbve * 63 + lbe) / 64.f,1.0f);
        *fbve = (s32)x_round((*fbve * 63 + fbe) / 64.f,1.0f);
    }
    /*   Set dither threshold to yield proper zero crossing rates in the */
    /*   presence of low frequency noise and low level signal input. */
    *dither = MINMAX( 1.0f, x_sqrt( (*lbue * *lbve) ) * 64.0f / 3000.0f, 20.0f );
    /*   Voicing decisions are returned in VOIBUF. */
} /* voicin_ */

inline
void placea( s32 *ipitch, s32 *voibuf, s32 *obound, s32 *af, s32 *vwin, s32 *awin, s32 *ewin, s32 *lframe, s32 *maxwin )
{
    /* System generated locals */
    f32 r__1;

    /* Local variables */
    s32 allv, winv;
    s32 i__, j, k, l, hrange;
    s32 ephase;
    s32 lrange;

    ewin -= 3;
    awin -= 3;
    vwin -= 3;
    --voibuf;

    lrange = (*af - 2) * *lframe + 1;
    hrange = *af * *lframe;

    allv = voibuf[((*af - 2) << 1) + 2] == 1;
    allv = allv && voibuf[((*af - 1) << 1) + 1] == 1;
    allv = allv && voibuf[((*af - 1) << 1) + 2] == 1;
    allv = allv && voibuf[(*af << 1) + 1] == 1;
    allv = allv && voibuf[(*af << 1) + 2] == 1;
    winv = voibuf[(*af << 1) + 1] == 1 || voibuf[(*af << 1) + 2] == 1;
    if ((allv || winv) && (*obound == 0)) 
    {
	    i__ = (lrange + *ipitch - 1 - awin[((*af - 1) << 1) + 1]) / *ipitch;
	    i__ *= *ipitch;
	    i__ += awin[((*af - 1) << 1) + 1];
    	l = *maxwin;
    	k = (vwin[(*af << 1) + 1] + vwin[(*af << 1) + 2] + 1 - l) / 2;
	    r__1 = (f32) (k - i__) / *ipitch;
	    awin[(*af << 1) + 1] = i__ + (s32)x_round(r__1,1.0f) * *ipitch;
	    awin[(*af << 1) + 2] = awin[(*af << 1) + 1] + l - 1;
	    if (*obound >= 2 && awin[(*af << 1) + 2] > vwin[(*af << 1) + 2]) 
        {
	        awin[(*af << 1) + 1] -= *ipitch;
	        awin[(*af << 1) + 2] -= *ipitch;
	    }
	    if ((*obound == 1 || *obound == 3) && awin[(*af << 1) + 1] < vwin[(*af << 1) + 1]) 
        {
	        awin[(*af << 1) + 1] += *ipitch;
	        awin[(*af << 1) + 2] += *ipitch;
	    }
	    while(awin[(*af << 1) + 2] > hrange) 
        {
	        awin[(*af << 1) + 1] -= *ipitch;
	        awin[(*af << 1) + 2] -= *ipitch;
	    }
	    while(awin[(*af << 1) + 1] < lrange) 
        {
	        awin[(*af << 1) + 1] += *ipitch;
	        awin[(*af << 1) + 2] += *ipitch;
	    }
	    ephase = TRUE;
    } 
    else 
    {
	    awin[(*af << 1) + 1] = vwin[(*af << 1) + 1];
	    awin[(*af << 1) + 2] = vwin[(*af << 1) + 2];
	    ephase = FALSE;
    }
    j = (awin[(*af << 1) + 2] - awin[(*af << 1) + 1] + 1) / *ipitch * *ipitch;
    if (j == 0 || ! winv) 
    {
	    ewin[(*af << 1) + 1] = vwin[(*af << 1) + 1];
	    ewin[(*af << 1) + 2] = vwin[(*af << 1) + 2];
    } 
    else if (! ephase && *obound == 2) 
    {
	    ewin[(*af << 1) + 1] = awin[(*af << 1) + 2] - j + 1;
	    ewin[(*af << 1) + 2] = awin[(*af << 1) + 2];
    } 
    else 
    {
	    ewin[(*af << 1) + 1] = awin[(*af << 1) + 1];
	    ewin[(*af << 1) + 2] = awin[(*af << 1) + 1] + j - 1;
    }
} 

static s32 tau[60] = { 20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156 };
static s32 buflim[4] = { 181,720,25,720 };
static f32 precoef = 0.9375f;

inline
void analys( f32 *speech, s32 *voice, s32 *pitch, f32 *rms, f32 *rc, LPC10_ENCODER_STATE *st )
{
    /* Initialized data */
    
    /* System generated locals */
    s32 i1;
    f32 amdf[60];
    s32 half;
    f32 abuf[156];
    f32 *bias;
    s32 *awin;
    s32 midx, ewin[6]	/* was [2][3] */;
    f32 ivrc[2], temp;
    f32 *zpre;
    s32 *vwin;
    s32 i, j, lanal;
    f32 *inbuf, *pebuf;
    f32 *lpbuf, *ivbuf;
    f32 *rcbuf;
    s32 *osptr;
    s32 *osbuf;
    s32 ipitch;
    s32 *obound;
    s32 *voibuf;
    s32 mintau;
    f32 *rmsbuf;
    s32 minptr, maxptr;
    f32 phi[100]	/* was [10][10] */, psi[10];
    s32 lframe;
    
    /* Parameter adjustments */
    if (speech) --speech;
    if (voice)  --voice;
    if (rc)     --rc;
    
    /* Function Body */
    inbuf = &(st->inbuf[0]);
    pebuf = &(st->pebuf[0]);
    lpbuf = &(st->lpbuf[0]);
    ivbuf = &(st->ivbuf[0]);
    bias = &(st->bias);
    osbuf = &(st->osbuf[0]);
    osptr = &(st->osptr);
    obound = &(st->obound[0]);
    vwin = &(st->vwin[0]);
    awin = &(st->awin[0]);
    voibuf = &(st->voibuf[0]);
    rmsbuf = &(st->rmsbuf[0]);
    rcbuf = &(st->rcbuf[0]);
    zpre = &(st->zpre);
    lframe = st->lframe;
    
    i1 = 720 - lframe;
    for (i = lframe; i < i1; ++i) 
    {
        inbuf[i - lframe] = inbuf[i];
        pebuf[i - lframe] = pebuf[i];
    }
    i1 = 540 - lframe - 229;
    for (i = 0; i <= i1; ++i) 
    {
        ivbuf[i] = ivbuf[lframe + i];
    }
    i1 = 720 - lframe - 25;
    for (i = 0; i <= i1; ++i) 
    {
        lpbuf[i] = lpbuf[lframe + i];
    }
    j = 1;
    i1 = (*osptr) - 1;
    for (i = 1; i <= i1; ++i) 
    {
        if (osbuf[i - 1] > lframe) 
        {
            osbuf[j - 1] = osbuf[i - 1] - lframe;
            ++j;
        }
    }
    *osptr = j;
    voibuf[0] = voibuf[2];
    voibuf[1] = voibuf[3];
    for (i = 1; i <= 2; ++i) 
    {
        vwin[(i << 1) - 2] = vwin[((i + 1) << 1) - 2] - lframe;
        vwin[(i << 1) - 1] = vwin[((i + 1) << 1) - 1] - lframe;
        awin[(i << 1) - 2] = awin[((i + 1) << 1) - 2] - lframe;
        awin[(i << 1) - 1] = awin[((i + 1) << 1) - 1] - lframe;
        obound[i - 1] = obound[i];
        voibuf[i << 1] = voibuf[(i + 1) << 1];
        voibuf[(i << 1) + 1] = voibuf[((i + 1) << 1) + 1];
        rmsbuf[i - 1] = rmsbuf[i];
        for (j = 1; j <= 10; ++j) 
        {
            rcbuf[j + i * 10 - 11] = rcbuf[j + (i + 1) * 10 - 11];
        }
    }
    temp = 0.f;
    for (i = 1; i <= lframe; ++i) 
    {
        inbuf[720 - lframe + i - 181] = speech[i] * 4096.f - (*bias);
        temp += inbuf[720 - lframe + i - 181];
    }
    if (temp > (f32) lframe) 
    {
        *bias += 1;
    }
    if (temp < (f32) (-lframe)) 
    {
        *bias += -1;
    }
    /*   Place Voicing Window */
    i = 721 - lframe;
    preemp(&inbuf[i - 181], &pebuf[i - 181], lframe, precoef, zpre);
    onset(pebuf, osbuf, osptr, 10, 181, 720, lframe, st);
    
    placev(osbuf, osptr, &obound[2], vwin, 3, &lframe, 90, 156, 307);
    lpfilt(&inbuf[228], &lpbuf[384], 312, lframe);
    ivfilt(&lpbuf[204], ivbuf, 312, lframe, ivrc);
    tbdm(ivbuf, 156, tau, 60, amdf, &minptr, &maxptr, &mintau);

    /*   voicing decisions. */
    for (half = 1; half <= 2; ++half) 
    {
        voicin(&vwin[4], inbuf, lpbuf, buflim, &half, &amdf[minptr - 1], &amdf[maxptr - 1], &mintau, ivrc, obound, voibuf, st);
    }
    /*   Find the minimum cost pitch decision over several frames */
    /*   given the current voicing decision and the AMDF array */
    dyptrk(amdf, 60, &minptr, &voibuf[7], pitch, &midx, st);
    ipitch = tau[midx - 1];
    /*   Place spectrum analysis and energy windows */
    placea(&ipitch, voibuf, &obound[2], &c3, vwin, awin, ewin, &lframe, &c156);
    /*  Remove short term DC bias over the analysis window, Put result in ABUF */
    lanal = awin[5] + 1 - awin[4];
    dcbias(lanal, &pebuf[awin[4] - 181], abuf);
    i1 = ewin[5] - ewin[4] + 1;
    energy(i1, &abuf[ewin[4] - awin[4]], &rmsbuf[2]);
    /*   Matrix load and invert, check RC's for stability */
    mload(1, lanal, abuf, phi, psi);
    invert(phi, psi, &rcbuf[20]);
    rcchk(&rcbuf[10], &rcbuf[20]);
    /*   Set return parameters */
    voice[1] = voibuf[2];
    voice[2] = voibuf[3];
    *rms = rmsbuf[0];
    for (i = 1; i <= 10; ++i) 
    {
        rc[i] = rcbuf[i - 1];
    }
} 

//------------------------------------------------------------------------------

static s32 bit[10] = { 2,4,8,8,8,8,16,16,16,16 };
static s32 iblist[53] = { 13,12,11,1,2,13,12,11,1,2,13,10,11,2,1,10,13,12,11,10,2,13,12,11,10,2,1,12,7,6,1,10,9,8,7,4,6,9,8,7,5,1,9,8,4,6,1,5,9,8,7,5,6 };

inline
s32 chanwr_0_( s32 n__, s32 *order, s32 *ipitv, s32 *irms, s32 *irc, s32 *ibits, LPC10_ENCODER_STATE *st )
{
    /* Initialized data */
    s32 *isync;
    s32 i__1;
    s32 itab[13], i__;
    
    /* Parameter adjustments */
    --irc;
    --ibits;
    
    /* Function Body */
    if (n__ == 0)
    {
        isync = &(st->isync);
    
        /*   Place parameters into ITAB */
        itab[0] = *ipitv;
        itab[1] = *irms;
        itab[2] = 0;
        i__1 = *order;
        for (i__ = 1; i__ <= i__1; ++i__) 
        {
            itab[i__ + 2] = irc[*order + 1 - i__] & 32767;
        }
        /*   Put 54 bits into IBITS array */
        for (i__ = 1; i__ <= 53; ++i__) 
        {
            ibits[i__] = itab[iblist[i__ - 1] - 1] & 1;
            itab[iblist[i__ - 1] - 1] /= 2;
        }
        ibits[54] = *isync & 1;
        *isync = 1 - *isync;
    }
    else
    {
        /*   Reconstruct ITAB */
        for (i__ = 1; i__ <= 13; ++i__) 
        {
            itab[i__ - 1] = 0;
        }
        for (i__ = 1; i__ <= 53; ++i__) 
        {
            itab[iblist[54 - i__ - 1] - 1] = (itab[iblist[54 - i__ - 1] - 1] << 1)
                + ibits[54 - i__];
        }
        /*   Sign extend RC's */
        i__1 = *order;
        for (i__ = 1; i__ <= i__1; ++i__) 
        {
            if ((itab[i__ + 2] & bit[i__ - 1]) != 0) 
            {
                itab[i__ + 2] -= bit[i__ - 1] << 1;
            }
        }
        /*   Restore variables */
        *ipitv = itab[0];
        *irms = itab[1];
        i__1 = *order;
        for (i__ = 1; i__ <= i__1; ++i__) 
        {
            irc[i__] = itab[*order + 4 - i__ - 1];
        }
    }

    return 0;
} 

//------------------------------------------------------------------------------

inline
s32 chanwr( s32 *order, s32 *ipitv, s32 *irms, s32 *irc, s32 *ibits, LPC10_ENCODER_STATE *st )
{
    return chanwr_0_( 0, order, ipitv, irms, irc, ibits, st );
}

//------------------------------------------------------------------------------

static s32 entau[60] = { 19,11,27,25,29,21,23,22,30,14,15,7,39,38,46,42,43,41,45,37,53,49,51,50,54,52,60,56,58,26,90,88,92,84,86,82,83,81,85,69,77,73,75,74,78,70,71,67,99,97,113,112,114,98,106,104,108,100,101,76 };
static s32 enadd[8] = { 1920,-768,2432,1280,3584,1536,2816,-1152 };
static f32 enscl[8] = { .0204f,.0167f,.0145f,.0147f,.0143f,.0135f,.0125f,.0112f };
static s32 enbits[8] = { 6,5,4,4,4,4,3,3 };
static s32 entab6[64] = { 0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,4,4,5,5,5,5,5,6,6,6,6,6,7,7,7,7,7,8,8,8,8,9,9,9,10,10,11,11,12,13,14,15 };
static s32 rmst[64] = { 1024,936,856,784,718,656,600,550,502,460,420,384,352,328,294,270,246,226,206,188,172,158,144,132,120,110,102,92,84,78,70,64,60,54,50,46,42,38,34,32,30,26,24,22,20,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 };

inline
void encode( s32 *voice, s32 pitch, f32 rms, f32 *rc, s32 *ipitch, s32 *irms, s32 *irc, LPC10_ENCODER_STATE *st )
{
    /* Local variables */
    s32 idel, nbit, i, j, i2, i3, mrk, order;
    
    /* Parameter adjustments */
    --irc;
    --rc;
    --voice;
    order = st->order;
    
    /* Function Body */
    *irms = (s32)(rms+0.5f);
    for (i = 1; i <= order; ++i) 
    {
        irc[i] = (s32)x_round(rc[i] * 32768.f,1.0f);
    }
    /*  Encode pitch and voicing */
    if (voice[1] != 0 && voice[2] != 0) 
    {
        *ipitch = entau[pitch - 1];
    } 
    else 
    {
        *ipitch = (voice[1] << 1) + voice[2];
    }
    /*  Encode RMS by binary table search */
    j = 32;
    idel = 16;
    *irms = MIN(*irms,1023);
    while(idel > 0) 
    {
        if (*irms > rmst[j - 1]) 
        {
            j -= idel;
        }
        if (*irms < rmst[j - 1]) 
        {
            j += idel;
        }
        idel /= 2;
    }
    if (*irms > rmst[j - 1]) 
    {
        --j;
    }
    *irms = 31 - j / 2;
    /*  Encode RC(1) and (2) as log-area-ratios */
    for (i = 1; i <= 2; ++i) 
    {
        i2 = irc[i];
        mrk = 0;
        if (i2 < 0) 
        {
            i2 = -i2;
            mrk = 1;
        }
        i2 /= 512;
        i2 = MIN(i2,63);
        i2 = entab6[i2];
        if (mrk != 0) 
        {
            i2 = -i2;
        }
        irc[i] = i2;
    }
    /*  Encode RC(3) - (10) linearly, remove bias then scale */
    for (i = 3; i <= order; ++i) 
    {
        i2 = irc[i] / 2;
        i2 = (s32)x_round((i2 + enadd[order + 1 - i - 1]) * enscl[order + 1 - i - 1],1.0f);
        /* Computing MIN */
        i2 = MINMAX(-127,i2,127);
        nbit = enbits[order + 1 - i - 1];
        i3 = 0;
        if (i2 < 0)
        {
            i3 = -1;
        }
        i2 = i2 / (2 << (nbit-1));
        if (i3 == -1) 
        {
            --i2;
        }
        irc[i] = i2;
    }
} 

//------------------------------------------------------------------------------

inline
s32 chanrd( s32 *order, s32 *ipitv, s32 *irms, s32 *irc, s32 *ibits )
{
    return chanwr_0_( 1, order, ipitv, irms, irc, ibits, 0 );
}

//------------------------------------------------------------------------------

inline
s32 pitsyn( s32 *order, s32 *voice, s32 *pitch, f32 *rms, f32 *rc, s32 *lframe, s32 *ivuv, s32 *ipiti, f32 *rmsi, f32 *rci, s32 *nout, f32 *ratio, LPC10_DECODER_STATE *st )
{
    /* Initialized data */
    f32 *rmso;
    s32 *first;
    s32 rci_dim1 = *order;
    s32 rci_offset;
    f32 alrn, alro, yarc[10], prop;
    s32 i, j, vflag, jused, lsamp;
    s32 *jsamp;
    f32 slope;
    s32 *ipito;
    f32 uvpit;
    s32 ip, nl, ivoice;
    s32 *ivoico;
    s32 istart;
    f32 *rco;
    f32 xxy;
    
    if (rc)    --rc;
    if (voice) --voice;
    if (ivuv)  --ivuv;
    if (ipiti) --ipiti;
    if (rmsi)  --rmsi;
    if (rci) 
    {
        rci_offset = rci_dim1 + 1;
        rci -= rci_offset;
    }
    
    /* Function Body */
    ivoico = &(st->ivoico);
    ipito = &(st->ipito);
    rmso = &(st->rmso);
    rco = &(st->rco[0]);
    jsamp = &(st->jsamp);
    first = &(st->first_pitsyn);
    
    if (*rms < 1.f) 
    {
        *rms = 1.f;
    }
    if (*rmso < 1.f) 
    {
        *rmso = 1.f;
    }
    uvpit = 0.f;
    *ratio = *rms / (*rmso + 8.f);
    if (*first) 
    {
        lsamp = 0;
        ivoice = voice[2];
        if (ivoice == 0) 
        {
            *pitch = *lframe / 4;
        }
        *nout = *lframe / *pitch;
        *jsamp = *lframe - *nout * *pitch;
        
        for (i = 1; i <= *nout; ++i) 
        {
            for (j = 1; j <= *order; ++j) 
            {
                rci[j + i * rci_dim1] = rc[j];
            }
            ivuv[i] = ivoice;
            ipiti[i] = *pitch;
            rmsi[i] = *rms;
        }
        *first = FALSE;
    } 
    else 
    {
        vflag = 0;
        lsamp = *lframe + *jsamp;
        slope = (*pitch - *ipito) / (f32) lsamp;
        *nout = 0;
        jused = 0;
        istart = 1;
        if (voice[1] == *ivoico && voice[2] == voice[1]) 
        {
            if (voice[2] == 0) 
            {
                /* SSUV - -   0  ,  0  ,  0 */
                *pitch = *lframe / 4;
                *ipito = *pitch;
                if (*ratio > 8.f) 
                {
                    *rmso = *rms;
                }
            }
            /* SSVC - -   1  ,  1  ,  1 */
            slope = (*pitch - *ipito) / (f32) lsamp;
            ivoice = voice[2];
        } 
        else 
        {
            if (*ivoico != 1) 
            {
                if (*ivoico == voice[1]) 
                {
                    /* UV2VC2 - -  0  ,  0  ,  1 */
                    nl = lsamp - *lframe / 4;
                }
                else 
                {
                    /* UV2VC1 - -  0  ,  1  ,  1 */
                    nl = lsamp - *lframe * 3 / 4;
                }
                ipiti[1] = nl / 2;
                ipiti[2] = nl - ipiti[1];
                ivuv[1] = 0;
                ivuv[2] = 0;
                rmsi[1] = *rmso;
                rmsi[2] = *rmso;
                for (i = 1; i <= *order; ++i) 
                {
                    rci[i + rci_dim1] = rco[i - 1];
                    rci[i + (rci_dim1 << 1)] = rco[i - 1];
                    rco[i - 1] = rc[i];
                }
                slope = 0.f;
                *nout = 2;
                *ipito = *pitch;
                jused = nl;
                istart = nl + 1;
                ivoice = 1;
            } 
            else 
            {
                if (*ivoico != voice[1])
                {
                    /* VC2UV1 - -   1  ,  0  ,  0 */
                    lsamp = *lframe / 4 + *jsamp;
                } 
                else
                {
                    /* VC2UV2 - -   1  ,  1  ,  0 */
                    lsamp = *lframe * 3 / 4 + *jsamp;
                }
                for (i = 1; i <= *order; ++i) 
                {
                    yarc[i - 1] = rc[i];
                    rc[i] = rco[i - 1];
                }
                ivoice = 1;
                slope = 0.f;
                vflag = 1;
            }
        }
        
        for(;;) 
        {
            for (i = istart; i <= lsamp; ++i) 
            {
                if (uvpit != 0.f) 
                {
                    ip = (s32)(uvpit+0.5f);
                }
                else 
                {
                    ip = (s32)x_round(*ipito + slope * i + .5f,1.0f);
                }
                
                if (ip <= i - jused) 
                {
                    ++(*nout);
                    ipiti[*nout] = ip;
                    *pitch = ip;
                    ivuv[*nout] = ivoice;
                    jused += ip;
                    prop = (jused - ip / 2) / (f32) lsamp;
                    for (j = 1; j <= *order; ++j) 
                    {
                        alro = logf((rco[j - 1] + 1) / (1 - rco[j - 1]));
                        alrn = logf((rc[j] + 1) / (1 - rc[j]));
                        xxy = alro + prop * (alrn - alro);
                        xxy = expf(xxy);
                        rci[j + *nout * rci_dim1] = (xxy - 1) / (xxy + 1);
                    }
                    rmsi[*nout] = (f32)(logf(*rmso) + prop * (logf(*rms) - logf(*rmso)));
                    rmsi[*nout] = (f32)expf(rmsi[*nout]);
                }
            }
            if (vflag != 1) 
            {
                break;
            }
            
            vflag = 0;
            istart = jused + 1;
            lsamp = *lframe + *jsamp;
            slope = 0.f;
            ivoice = 0;
            uvpit = (f32) ((lsamp - istart) / 2);
            if (uvpit > 90.f) 
            {
                uvpit /= 2;
            }
            *rmso = *rms;
            for (i = 1; i <= *order; ++i) 
            {
                rco[i - 1] = rc[i] = yarc[i - 1];
            }
        }
        *jsamp = lsamp - jused;
    }

    if (*nout != 0) 
    {
        *ivoico = voice[2];
        *ipito = *pitch;
        *rmso = *rms;
        for (i = 1; i <= *order; ++i) 
        {
            rco[i - 1] = rc[i];
        }
    }

    return 0;
} 

#define MIDTAP 1
#define MAXTAP 4
static s16 y[MAXTAP+1]={-21161, -8478, 30892,-10216, 16950};
static s32 j=MIDTAP, k=MAXTAP;

inline
s32 random()
{
    int the_random;
    
    /*   The following is a 16 bit 2's complement addition,
    *   with overflow checking disabled	*/
    
    y[k] = (short)(y[k] + y[j]);
    
    the_random = y[k];
    k--;
    if (k < 0) k = MAXTAP;
    j--;
    if (j < 0) j = MAXTAP;
    
    return(the_random);
}

static s32 kexc[25] = { 8,-16,26,-48,86,-162,294,-502,718,-728,184,672,-610,-672,184,728,718,502,294,162,86,48,26,16,8 };

inline
void bsynz( f32 *coef, s32 ip, s32 *iv, f32 *sout, f32 *rms, f32 *ratio, f32 *g2pass, LPC10_DECODER_STATE *st )
{
    /* Initialized data */
    s32 *ipo;
    f32 *rmso;
    f32 *exc;
    f32 *exc2;
    f32 lpi1;
    f32 lpi2;
    f32 hpi1;
    f32 hpi2;
    f32 gain, xssq;
    s32 i, j, k;
    f32 pulse;
    s32 px;
    f32 sscale;
    f32 xy, sum, ssq;
    f32 lpi0, hpi0;
    s32 order;
    
    if (coef) --coef;
    
    /* Function Body */
    ipo = &(st->ipo);
    exc = &(st->exc[0]);
    exc2 = &(st->exc2[0]);
    lpi1 = st->lpi1;
    lpi2 = st->lpi2;
    hpi1 = st->hpi1;
    hpi2 = st->hpi2;
    rmso = &(st->rmso_bsynz);
    order = st->order;
    
    /*  Calculate history scale factor XY and scale filter state */
    /* Computing MIN */
    xy = MIN((*rmso / (*rms + 1e-6f)),8.f);
    *rmso = *rms;
    for (i = 0; i < order; ++i) 
    {
        exc2[i] = exc2[*ipo + i] * xy;
    }
    *ipo = ip;
    if (*iv == 0) 
    {
        /*  Generate white noise for unvoiced */
        for (i = 0; i < ip; ++i) 
        {
            exc[order + i] = (f32) (random() >> 6);
        }
        px = ((random() + 32768) * (ip - 1) >> 16) + order + 1;
        pulse = *ratio * 85.5f;
        if (pulse > 2e3f) 
        {
            pulse = 2e3f;
        }
        exc[px - 1] += pulse;
        exc[px] -= pulse;
        /*  Load voiced excitation */
    } 
    else 
    {
        sscale = sqrtf((ip)) * 0.144341801f;
        for (i = 0; i < ip; ++i) 
        {
            f32 temp;

            if (i > 27) 
            {
                temp = 0.f;
            }
            else if (i < 25) 
            {
                lpi0 = temp = sscale * kexc[i];
                temp = lpi0 * .125f + lpi1 * .75f + lpi2 * .125f;
                lpi2 = lpi1;
                lpi1 = lpi0;
            }
            else
            {
                lpi0 = temp = 0.f;
                temp = lpi1 * .75f + lpi2 * .125f;
                lpi2 = lpi1;
                lpi1 = lpi0;
            }
            hpi0 = (f32)(random() >> 6);
            exc[order + i] = temp + hpi0 * -.125f + hpi1 * .25f + hpi2 * -.125f;
            hpi2 = hpi1;
            hpi1 = hpi0;
        }
    }
    /*   Synthesis filters: */
    /*    Modify the excitation with all-zero filter  1 + G*SUM */
    xssq = 0.f;
    for (i = 0; i < ip; ++i) 
    {
        k = order + i;
        sum = 0.f;
        for (j = 1; j <= order; ++j)
        {
            sum += coef[j] * exc[k - j];
        }
        sum *= *g2pass;
        exc2[k] = sum + exc[k];
    }
    /*   Synthesize using the all pole filter  1 / (1 - SUM) */
    for (i = 0; i < ip; ++i)
    {
        k = order + i;
        sum = 0.f;
        for (j = 1; j <= order; ++j)
        {
            sum += coef[j] * exc2[k - j];
        }
        exc2[k] += sum;
        xssq += exc2[k] * exc2[k];
    }
    /*  Save filter history for next epoch */
    for (i = 0; i < order; ++i)
    {
        exc[i] = exc[ip + i];
        exc2[i] = exc2[ip + i];
    }
    /*  Apply gain to match RMS */
    ssq = *rms * *rms * ip;
    gain = sqrtf(ssq / xssq);
    for (i = 0; i < ip; ++i) 
    {
        sout[i] = gain * exc2[order + i];
    }
    st->lpi1 = lpi1;
    st->lpi2 = lpi2;
    st->hpi1 = hpi1;
    st->hpi2 = hpi2;
} 

inline
void irc2pc( f32 *rc, f32 *pc, f32 gprime, f32 *g2pass )
{
    s32 i2;
    f32 temp[10];
    s32 i, j;

    --pc;
    --rc;
    
    *g2pass = 1.f;
    for (i = 1; i <= 10; ++i) 
    {
        *g2pass *= 1.f - rc[i] * rc[i];
    }

    *g2pass = gprime * sqrtf(*g2pass);
    pc[1] = rc[1];
    for (i = 2; i <= 10; ++i) 
    {
        i2 = i - 1;
        for (j = 1; j <= i2; ++j) 
        {
            temp[j - 1] = pc[j] - rc[i] * pc[i - j];
        }
        i2 = i - 1;
        for (j = 1; j <= i2; ++j) 
        {
            pc[j] = temp[j - 1];
        }
        pc[i] = rc[i];
    }
} 

inline
void deemp( f32 *x, s32 n, LPC10_DECODER_STATE *st )
{
    f32 dei1;
    f32 dei2;
    f32 deo1;
    f32 deo2;
    f32 deo3;
    s32 k;
    f32 dei0;
    
    dei1 = st->dei1;
    dei2 = st->dei2;
    deo1 = st->deo1;
    deo2 = st->deo2;
    deo3 = st->deo3;
    
    for (k = 0; k < n; ++k) 
    {
        dei0 = x[k];
        x[k] = dei0 - dei1 * 1.9998f + dei2 + deo1 * 2.5f - deo2 * 2.0925f + deo3 * .585f;
        dei2 = dei1;
        dei1 = dei0;
        deo3 = deo2;
        deo2 = deo1;
        deo1 = x[k];
    }

    st->dei1 = dei1;
    st->dei2 = dei2;
    st->deo1 = deo1;
    st->deo2 = deo2;
    st->deo3 = deo3;
} 

inline
void synths( s32 *voice, s32 pitch, f32 rms, f32 *rc, f32 *speech, LPC10_DECODER_STATE *st )
{
    f32 *buf;
    s32 *buflen;
    s32 i1;
    f32 r__1, r__2;
    f32 rmsi[16];
    s32 nout, ivuv[16], i, j;
    f32 ratio;
    s32 ipiti[16];
    f32 g2pass;
    f32 pc[10];
    f32 rci[160];

    if (voice)  --voice;
    if (rc)     --rc;
    if (speech) --speech;

    /* Function Body */
    buf = &(st->buf[0]);
    buflen = &(st->buflen);

    i1 = MIN(pitch,156);
    pitch = MAX(i1,20);
    for (i = 1; i <= st->order; ++i) 
    {
	    r__2 = rc[i];
	    r__1 = MIN(r__2,.99f);
	    rc[i] = MAX(r__1,-.99f);
    }
    pitsyn( &st->order, &voice[1], &pitch, &rms, &rc[1], &st->lframe, ivuv, ipiti, rmsi, rci, &nout, &ratio, st );
    if (nout > 0) 
    {
	    i1 = nout;
	    for (j = 1; j <= i1; ++j) 
        {
	        irc2pc( &rci[j * 10 - 10], pc, 0.7f, &g2pass);
	        bsynz( pc, ipiti[j - 1], &ivuv[j - 1], &buf[*buflen], &rmsi[j - 1], &ratio, &g2pass, st );
	        deemp( &buf[*buflen], ipiti[j - 1], st);
	        *buflen += ipiti[j - 1];
	    }

	    for (i = 1; i <= st->lframe; ++i) 
        {
	        speech[i] = buf[i - 1] / 4096.f;
	    }
	    *buflen += -st->lframe;
	    i1 = *buflen;
	    for (i = 1; i <= i1; ++i) 
        {
    	    buf[i - 1] = buf[i + st->lframe - 1];
	    }
    }
}

//------------------------------------------------------------------------------

static s32 detau[128] = { 0,0,0,3,0,3,3,31,0,3,3,21,3,3,29,30,0,3,3,20,3,25,27,26,3,23,58,22,3,24,28,3,0,3,3,3,3,39,33,32,3,37,35,36,3,38,34,3,3,42,46,44,50,40,48,3,54,3,56,3,52,3,3,1,0,3,3,108,3,78,100,104,3,84,92,88,156,80,96,3,3,74,70,72,66,76,68,3,62,3,60,3,64,3,3,1,3,116,132,112,148,152,3,3,140,3,136,3,144,3,3,1,124,120,128,3,3,3,3,1,3,3,3,1,3,1,1,1 };
static s32 detab7[32] = { 4,11,18,25,32,39,46,53,60,66,72,77,82,87,92,96,101,104,108,111,114,115,117,119,121,122,123,124,125,126,127,127 };
static f32 descl[8] = { .6953f,.625f,.5781f,.5469f,.5312f,.5391f,.4688f,.3828f };
static s32 deadd[8] = { 1152,-2816,-1536,-3584,-1280,-2432,768,-1920 };
static s32 qb[8] = { 511,511,1023,1023,1023,1023,2047,4095 };
static s32 nbit[10] = { 8,8,5,5,4,4,4,4,3,2 };

inline
void decode( s32 ipitv, s32 irms, s32 *irc, s32 *voice, s32 *pitch, f32 *rms, f32 *rc, LPC10_DECODER_STATE *st )
{
    /* Initialized data */
    s32 *first;
    s32 *ivp2h;
    s32 *iovoic;
    s32 *iavgp;
    s32 *iptold;
    s32 *erate;
    s32 *drc;
    s32 *dpit;
    s32 *drms;
    s32 i, i1, i2, i4;
    s32 ishift;
    
    /* Parameter adjustments */
    if (irc)  --irc;
    if (voice)    --voice;
    if (rc)    --rc;

    iptold = &(st->iptold);
    first = &(st->first);
    ivp2h = &(st->ivp2h);
    iovoic = &(st->iovoic);
    iavgp = &(st->iavgp);
    erate = &(st->erate);
    drc = &(st->drc[0]);
    dpit = &(st->dpit[0]);
    drms = &(st->drms[0]);

    i4 = detau[ipitv];
    voice[1] = 1;
    voice[2] = 1;
    if (ipitv <= 1) voice[1] = 0;
    if (ipitv == 0 || ipitv == 2) voice[2] = 0;
    *pitch = i4;
    if (*pitch <= 4) *pitch = *iptold;
    if (voice[1] == 1 && voice[2] == 1) *iptold = *pitch;
    if (voice[1] != voice[2]) *pitch = *iptold;

    /*   Decode RMS */
    irms = rmst[(31 - irms) * 2];

    for (i = 1; i <= 2; ++i) 
    {
        i2 = irc[i];
        i1 = 0;
        if (i2 < 0) 
        {
            i1 = 1;
            i2 = -i2;
            if (i2 > 15) i2 = 0;
        }
        i2 = detab7[i2 * 2];
        if (i1 == 1)
        {
            i2 = -i2;
        }
        ishift = 15 - nbit[i - 1];
        irc[i] = i2 * (2 << (ishift-1));
    }
    /*  Decode RC(3)-RC(10) to sign plus 14 bits */
    for (i = 3; i <= st->order; ++i) 
    {
        i2 = irc[i];
        ishift = 15 - nbit[i - 1];
        i2 *= (2 << (ishift-1));
        i2 += qb[i - 3];
        irc[i] = (s32)(i2 * descl[i - 3] + deadd[i - 3]);
    }
    /*  Scale RMS and RC's to f32s */
    *rms = (f32) (irms);
    for (i = 1; i <= st->order; ++i) 
    {
        rc[i] = irc[i] / 16384.f;
    }
}

//------------------------------------------------------------------------------
