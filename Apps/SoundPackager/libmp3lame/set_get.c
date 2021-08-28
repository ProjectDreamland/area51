/* -*- mode: C; mode: fold -*- */
/*
 * set/get functions for lame_global_flags
 *
 * Copyright (c) 2001 Alexander Leidinger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* $Id: set_get.c,v 1.42 2002/10/16 18:01:10 bouvigne Exp $ */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include "util.h"
#include "bitstream.h"  /* because of compute_flushbits */

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif



extern int apply_preset(lame_global_flags*  gfp, int preset);




/*
 * input stream description
 */

/* number of samples */
/* it's unlikely for this function to return an error */
int
lame_set_num_samples( lame_global_flags*  gfp,
                      unsigned long       num_samples)
{
    /* default = 2^32-1 */

    gfp->num_samples = num_samples;

    return 0;
}

unsigned long
lame_get_num_samples( const lame_global_flags* gfp )
{
    return gfp->num_samples;
}


/* input samplerate */
int
lame_set_in_samplerate( lame_global_flags*  gfp,
                        int                 in_samplerate )
{
    /* input sample rate in Hz,  default = 44100 Hz */
    gfp->in_samplerate = in_samplerate;

    return 0;
}

int
lame_get_in_samplerate( const lame_global_flags*  gfp )
{
    return gfp->in_samplerate;
}


/* number of channels in input stream */
int
lame_set_num_channels( lame_global_flags*  gfp,
                       int                 num_channels )
{
    /* default = 2 */

    if ( 2 < num_channels || 0 == num_channels )
        return -1;    /* we don't support more than 2 channels */

    gfp->num_channels = num_channels;

    return 0;
}

int
lame_get_num_channels( const lame_global_flags*  gfp )
{
    return gfp->num_channels;
}


/* scale the input by this amount before encoding (not used for decoding) */
int
lame_set_scale( lame_global_flags*  gfp,
                float               scale )
{
    /* default = 0 */
    gfp->scale = scale;

    return 0;
}

float
lame_get_scale( const lame_global_flags*  gfp )
{
    return gfp->scale;
}


/* scale the channel 0 (left) input by this amount before 
   encoding (not used for decoding) */
int
lame_set_scale_left( lame_global_flags*  gfp,
                     float               scale )
{
    /* default = 0 */
    gfp->scale_left = scale;

    return 0;
}

float
lame_get_scale_left( const lame_global_flags*  gfp )
{
    return gfp->scale_left;
}


/* scale the channel 1 (right) input by this amount before 
   encoding (not used for decoding) */
int
lame_set_scale_right( lame_global_flags*  gfp,
                      float               scale )
{
    /* default = 0 */
    gfp->scale_right = scale;

    return 0;
}

float
lame_get_scale_right( const lame_global_flags*  gfp )
{
    return gfp->scale_right;
}


/* output sample rate in Hz */
int
lame_set_out_samplerate( lame_global_flags*  gfp,
                         int                 out_samplerate )
{
    /*
     * default = 0: LAME picks best value based on the amount
     *              of compression
     * MPEG only allows:
     *  MPEG1    32, 44.1,   48khz
     *  MPEG2    16, 22.05,  24
     *  MPEG2.5   8, 11.025, 12
     *
     * (not used by decoding routines)
     */
    gfp->out_samplerate = out_samplerate;

    return 0;
}

int
lame_get_out_samplerate( const lame_global_flags*  gfp )
{
    return gfp->out_samplerate;
}




/*
 * general control parameters
 */

/* collect data for an MP3 frame analzyer */
int
lame_set_analysis( lame_global_flags*  gfp,
                   int                 analysis )
{
    /* default = 0 */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > analysis || 1 < analysis )
        return -1;

    gfp->analysis = analysis;

    return 0;
}

int
lame_get_analysis( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->analysis && 1 >= gfp->analysis );

    return gfp->analysis;
}


/* write a Xing VBR header frame */
int
lame_set_bWriteVbrTag( lame_global_flags*  gfp,
                       int bWriteVbrTag )
{
    /* default = 1 (on) for VBR/ABR modes, 0 (off) for CBR mode */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > bWriteVbrTag || 1 < bWriteVbrTag )
        return -1;

    gfp->bWriteVbrTag = bWriteVbrTag;

    return 0;
}

int
lame_get_bWriteVbrTag( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->bWriteVbrTag && 1 >= gfp->bWriteVbrTag );

    return gfp->bWriteVbrTag;
}



/* decode only, use lame/mpglib to convert mp3/ogg to wav */
int
lame_set_decode_only( lame_global_flags*  gfp,
                      int                 decode_only )
{
    /* default = 0 (disabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > decode_only || 1 < decode_only )
        return -1;

    gfp->decode_only = decode_only;

    return 0;
}

int
lame_get_decode_only( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->decode_only && 1 >= gfp->decode_only );

    return gfp->decode_only;
}


/* encode a Vorbis .ogg file */
int
lame_set_ogg( lame_global_flags*  gfp,
              int                 ogg )
{
    /* default = 0 (disabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > ogg || 1 < ogg )
        return -1;

    gfp->ogg = ogg;

    return 0;
}

int
lame_get_ogg( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->ogg && 1 >= gfp->ogg );

    return gfp->ogg;
}


/*
 * Internal algorithm selection.
 * True quality is determined by the bitrate but this variable will effect
 * quality by selecting expensive or cheap algorithms.
 * quality=0..9.  0=best (very slow).  9=worst.  
 * recommended:  2     near-best quality, not too slow
 *               5     good quality, fast
 *               7     ok quality, really fast
 */
int
lame_set_quality( lame_global_flags*  gfp,
                  int                 quality )
{
    gfp->quality = quality;

    return 0;
}

int
lame_get_quality( const lame_global_flags*  gfp )
{
    return gfp->quality;
}


/* mode = STEREO, JOINT_STEREO, DUAL_CHANNEL (not supported), MONO */
int
lame_set_mode( lame_global_flags*  gfp,
               MPEG_mode           mode )
{
    /* default: lame chooses based on compression ratio and input channels */

    if( 0 > mode || MAX_INDICATOR <= mode )
        return -1;  /* Unknown MPEG mode! */

    gfp->mode = mode;

    return 0;
}

MPEG_mode
lame_get_mode( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->mode && MAX_INDICATOR > gfp->mode );

    return gfp->mode;
}


/* Us a M/S mode with a switching threshold based on compression ratio */
int
lame_set_mode_automs( lame_global_flags*  gfp,
                      int                 mode_automs )
{
    /* default = 0 (disabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > mode_automs || 1 < mode_automs )
        return -1;

    gfp->mode_automs = mode_automs;

    return 0;
}

int
lame_get_mode_automs( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->mode_automs && 1 >= gfp->mode_automs );

    return gfp->mode_automs;
}


/*
 * Force M/S for all frames.  For testing only.
 * Requires mode = 1.
 */
int
lame_set_force_ms( lame_global_flags*  gfp,
                   int                 force_ms )
{
    /* default = 0 (disabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > force_ms || 1 < force_ms )
        return -1;

    gfp->force_ms = force_ms;

    return 0;
}

int
lame_get_force_ms( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->force_ms && 1 >= gfp->force_ms );

    return gfp->force_ms;
}


/* Use free_format. */
int
lame_set_free_format( lame_global_flags*  gfp,
                      int                 free_format )
{
    /* default = 0 (disabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > free_format || 1 < free_format )
        return -1;

    gfp->free_format = free_format;

    return 0;
}

int
lame_get_free_format( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->free_format && 1 >= gfp->free_format );

    return gfp->free_format;
}


/* message handlers */
int
lame_set_errorf( lame_global_flags*  gfp,
                 void                (*func)( const char*, va_list ) )
{
    gfp->report.errorf = func;

    return 0;
}

int
lame_set_debugf( lame_global_flags*  gfp,
                 void                (*func)( const char*, va_list ) )
{
    gfp->report.debugf = func;

    return 0;
}

int
lame_set_msgf( lame_global_flags*  gfp,
               void                (*func)( const char *, va_list ) )
{
    gfp->report.msgf = func;

    return 0;
}


/*
 * Set one of
 *  - brate
 *  - compression ratio.
 *
 * Default is compression ratio of 11.
 */
int
lame_set_brate( lame_global_flags*  gfp,
                int                 brate )
{
    gfp->brate = brate;

    if (brate >= 320) {
        gfp->disable_reservoir = 1;
    }

    return 0;
}

int
lame_get_brate( const lame_global_flags*  gfp )
{
    return gfp->brate;
}

int
lame_set_compression_ratio( lame_global_flags*  gfp,
                            float               compression_ratio )
{
    gfp->compression_ratio = compression_ratio;

    return 0;
}

float
lame_get_compression_ratio( const lame_global_flags*  gfp )
{
    return gfp->compression_ratio;
}




/*
 * frame parameters
 */

/* Mark as copyright protected. */
int
lame_set_copyright( lame_global_flags*  gfp,
                    int                 copyright )
{
    /* default = 0 (disabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > copyright || 1 < copyright )
        return -1;

    gfp->copyright = copyright;

    return 0;
}

int
lame_get_copyright( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->copyright && 1 >= gfp->copyright );

    return gfp->copyright;
}


/* Mark as original. */
int
lame_set_original( lame_global_flags*  gfp,
                   int                 original )
{
    /* default = 1 (enabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > original || 1 < original )
        return -1;

    gfp->original = original;

    return 0;
}

int
lame_get_original( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->original && 1 >= gfp->original );

    return gfp->original;
}


/*
 * error_protection.
 * Use 2 bytes from each frame for CRC checksum.
 */
int
lame_set_error_protection( lame_global_flags*  gfp,
                           int                 error_protection )
{
    /* default = 0 (disabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > error_protection || 1 < error_protection )
        return -1;

    gfp->error_protection = error_protection;

    return 0;
}

int
lame_get_error_protection( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->error_protection && 1 >= gfp->error_protection );

    return gfp->error_protection;
}


/*
 * padding_type.
 *  PAD_NO     = pad no frames
 *  PAD_ALL    = pad all frames
 *  PAD_ADJUST = adjust padding
 */
int
lame_set_padding_type( lame_global_flags*  gfp,
                       Padding_type        padding_type )
{
    /* default = 2 */

    if ( 0 > padding_type || PAD_MAX_INDICATOR < padding_type )
        return -1;  /* Unknown padding type */

    gfp->padding_type = padding_type;

    return 0;
}

Padding_type
lame_get_padding_type( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->padding_type && PAD_MAX_INDICATOR > gfp->padding_type );

    return gfp->padding_type;
}


/* MP3 'private extension' bit. Meaningless. */
int
lame_set_extension( lame_global_flags*  gfp,
                    int                 extension )
{
    /* default = 0 (disabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > extension || 1 < extension )
        return -1;

    gfp->extension = extension;

    return 0;
}

int
lame_get_extension( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->extension && 1 >= gfp->extension );

    return gfp->extension;
}


/* Enforce strict ISO compliance. */
int
lame_set_strict_ISO( lame_global_flags*  gfp,
                     int                 strict_ISO )
{
    /* default = 0 (disabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > strict_ISO || 1 < strict_ISO )
        return -1;

    gfp->strict_ISO = strict_ISO;

    return 0;
}

int
lame_get_strict_ISO( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->strict_ISO && 1 >= gfp->strict_ISO );

    return gfp->strict_ISO;
}
 



/********************************************************************
 * quantization/noise shaping 
 ***********************************************************************/

/* Disable the bit reservoir. For testing only. */
int
lame_set_disable_reservoir( lame_global_flags*  gfp,
                            int                 disable_reservoir )
{
    /* default = 0 (disabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > disable_reservoir || 1 < disable_reservoir )
        return -1;

    gfp->disable_reservoir = disable_reservoir;

    return 0;
}

int
lame_get_disable_reservoir( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->disable_reservoir && 1 >= gfp->disable_reservoir );

    return gfp->disable_reservoir;
}




/* Select a different "best quantization" function. default = 0 */
int
lame_set_experimentalX( lame_global_flags*  gfp,
                        int                 experimentalX )
{
    gfp->experimentalX = experimentalX;

    return 0;
}

int
lame_get_experimentalX( const lame_global_flags*  gfp )
{
    return gfp->experimentalX;
}


/* Another experimental option. For testing only. */
int
lame_set_experimentalY( lame_global_flags*  gfp,
                        int                 experimentalY )
{
    gfp->experimentalY = experimentalY;

    return 0;
}

int
lame_get_experimentalY( const lame_global_flags*  gfp )
{
    return gfp->experimentalY;
}


/* Another experimental option. For testing only. */
int
lame_set_experimentalZ( lame_global_flags*  gfp,
                        int                 experimentalZ )
{
    gfp->experimentalZ += experimentalZ;

    return 0;
}

int
lame_get_experimentalZ( const lame_global_flags*  gfp )
{
    return gfp->experimentalZ;
}


/* Naoki's psycho acoustic model. */
int
lame_set_exp_nspsytune( lame_global_flags*  gfp,
                        int                 exp_nspsytune )
{
    /* default = 0 (disabled) */

    gfp->exp_nspsytune = exp_nspsytune;

    return 0;
}

int
lame_get_exp_nspsytune( const lame_global_flags*  gfp )
{
    return gfp->exp_nspsytune;
}


int
lame_set_exp_nspsytune2_int( lame_global_flags*  gfp,
			     int adr,int val)
{
  int ret = gfp->exp_nspsytune2.integer[adr];

  gfp->exp_nspsytune2.integer[adr] = val;

  return ret;
}


float
lame_set_exp_nspsytune2_real( lame_global_flags*  gfp,
			      int adr,float val)
{
  float ret = gfp->exp_nspsytune2.real[adr];

  gfp->exp_nspsytune2.real[adr] = val;

  return ret;
}


void *
lame_set_exp_nspsytune2_pointer( lame_global_flags*  gfp,
				 int adr,void *val)
{
  void *ret = gfp->exp_nspsytune2.pointer[adr];

  gfp->exp_nspsytune2.pointer[adr] = val;

  return ret;
}


/********************************************************************
 * VBR control
 ***********************************************************************/

// Types of VBR.  default = vbr_off = CBR
int
lame_set_VBR( lame_global_flags*  gfp,
              vbr_mode            VBR )
{
    if( 0 > VBR || vbr_max_indicator <= VBR )
        return -1;  /* Unknown VBR mode! */

    gfp->VBR = VBR;

    return 0;
}

vbr_mode
lame_get_VBR( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->VBR && vbr_max_indicator > gfp->VBR );

    return gfp->VBR;
}


/*
 * VBR quality level.
 *  0 = highest
 *  9 = lowest 
 */
int
lame_set_VBR_q( lame_global_flags*  gfp,
                int                 VBR_q )
{
    /* XXX: This should be an enum */
    /*  to whoever added this note: why should it be an enum?
        do you want to call a specific setting by name? 
        say VBR quality level red? */
    /* No, but VBR_Q_HIGHEST, VBR_Q_HIGH, ..., VBR_Q_MID, ...
       VBR_Q_LOW, VBR_Q_LOWEST (or something like that )and a
       VBR_Q_DEFAULT, which aliases the default setting of
       e.g. VBR_Q_MID. */


    if( 0 > VBR_q || 10 <= VBR_q )
        return -1;  /* Unknown VBR quality level! */

    gfp->VBR_q = VBR_q;

    return 0;
}

int
lame_get_VBR_q( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->VBR_q && 10 > gfp->VBR_q );

    return gfp->VBR_q;
}


/* Ignored except for VBR = vbr_abr (ABR mode) */
int
lame_set_VBR_mean_bitrate_kbps( lame_global_flags*  gfp,
                                int                 VBR_mean_bitrate_kbps )
{
    gfp->VBR_mean_bitrate_kbps = VBR_mean_bitrate_kbps;

    return 0;
}

int
lame_get_VBR_mean_bitrate_kbps( const lame_global_flags*  gfp )
{
    return gfp->VBR_mean_bitrate_kbps;
}

int
lame_set_VBR_min_bitrate_kbps( lame_global_flags*  gfp,
                               int                 VBR_min_bitrate_kbps )
{
    gfp->VBR_min_bitrate_kbps = VBR_min_bitrate_kbps;

    return 0;
}

int
lame_get_VBR_min_bitrate_kbps( const lame_global_flags*  gfp )
{
    return gfp->VBR_min_bitrate_kbps;
}

int
lame_set_VBR_max_bitrate_kbps( lame_global_flags*  gfp,
                               int                 VBR_max_bitrate_kbps )
{
    gfp->VBR_max_bitrate_kbps = VBR_max_bitrate_kbps;

    return 0;
}

int
lame_get_VBR_max_bitrate_kbps( const lame_global_flags*  gfp )
{
    return gfp->VBR_max_bitrate_kbps;
}


/*
 * Strictly enforce VBR_min_bitrate.
 * Normally it will be violated for analog silence.
 */
int
lame_set_VBR_hard_min( lame_global_flags*  gfp,
                       int                 VBR_hard_min )
{
    /* default = 0 (disabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > VBR_hard_min || 1 < VBR_hard_min )
        return -1;

    gfp->VBR_hard_min = VBR_hard_min;

    return 0;
}

int
lame_get_VBR_hard_min( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->VBR_hard_min && 1 >= gfp->VBR_hard_min );

    return gfp->VBR_hard_min;
}


/********************************************************************
 * Filtering control
 ***********************************************************************/

/*
 * Freqency in Hz to apply lowpass.
 *   0 = default = lame chooses
 *  -1 = disabled
 */
int
lame_set_lowpassfreq( lame_global_flags*  gfp,
                      int                 lowpassfreq )
{
    gfp->lowpassfreq = lowpassfreq;

    return 0;
}

int
lame_get_lowpassfreq( const lame_global_flags*  gfp )
{
    return gfp->lowpassfreq;
}


/*
 * Width of transition band (in Hz).
 *  default = one polyphase filter band
 */
int
lame_set_lowpasswidth( lame_global_flags*  gfp,
                       int                 lowpasswidth )
{
    gfp->lowpasswidth = lowpasswidth;

    return 0;
}

int
lame_get_lowpasswidth( const lame_global_flags*  gfp )
{
    return gfp->lowpasswidth;
}


/*
 * Frequency in Hz to apply highpass.
 *   0 = default = lame chooses
 *  -1 = disabled
 */
int
lame_set_highpassfreq( lame_global_flags*  gfp,
                       int                 highpassfreq )
{
    gfp->highpassfreq = highpassfreq;

    return 0;
}

int
lame_get_highpassfreq( const lame_global_flags*  gfp )
{
    return gfp->highpassfreq;
}


/*
 * Width of transition band (in Hz).
 *  default = one polyphase filter band
 */
int
lame_set_highpasswidth( lame_global_flags*  gfp,
                        int                 highpasswidth )
{
    gfp->highpasswidth = highpasswidth;

    return 0;
}

int
lame_get_highpasswidth( const lame_global_flags*  gfp )
{
    return gfp->highpasswidth;
}




/*
 * psycho acoustics and other arguments which you should not change 
 * unless you know what you are doing
 */

/* Only use ATH for masking. */
int
lame_set_ATHonly( lame_global_flags*  gfp,
                  int                 ATHonly )
{
    gfp->ATHonly = ATHonly;

    return 0;
}

int
lame_get_ATHonly( const lame_global_flags*  gfp )
{
    return gfp->ATHonly;
}


/* Only use ATH for short blocks. */
int
lame_set_ATHshort( lame_global_flags*  gfp,
                   int                 ATHshort )
{
    gfp->ATHshort = ATHshort;

    return 0;
}

int
lame_get_ATHshort( const lame_global_flags*  gfp )
{
    return gfp->ATHshort;
}


/* Disable ATH. */
int
lame_set_noATH( lame_global_flags*  gfp,
                int                 noATH )
{
    gfp->noATH = noATH;

    return 0;
}

int
lame_get_noATH( const lame_global_flags*  gfp )
{
    return gfp->noATH;
}


/* Select ATH formula. */
int
lame_set_ATHtype( lame_global_flags*  gfp,
                  int                 ATHtype )
{
    /* XXX: ATHtype should be converted to an enum. */
    gfp->ATHtype = ATHtype;

    return 0;
}

int
lame_get_ATHtype( const lame_global_flags*  gfp )
{
    return gfp->ATHtype;
}


/* Lower ATH by this many db. */
int
lame_set_ATHlower( lame_global_flags*  gfp,
                   float               ATHlower )
{
    gfp->ATHlower = ATHlower;
    return 0;
}

float
lame_get_ATHlower( const lame_global_flags*  gfp )
{
    return gfp->ATHlower;
}


/* Select ATH adaptive adjustment scheme. */
int
lame_set_athaa_type( lame_global_flags*  gfp,
                      int                athaa_type )
{
    gfp->athaa_type = athaa_type;

    return 0;
}

int
lame_get_athaa_type( const lame_global_flags*  gfp )
{
    return gfp->athaa_type;
}


/* Select the loudness approximation used by the ATH adaptive auto-leveling. */
int
lame_set_athaa_loudapprox( lame_global_flags*  gfp,
                           int                 athaa_loudapprox )
{
    gfp->athaa_loudapprox = athaa_loudapprox;

    return 0;
}

int
lame_get_athaa_loudapprox( const lame_global_flags*  gfp )
{
    return gfp->athaa_loudapprox;
}


/* Adjust (in dB) the point below which adaptive ATH level adjustment occurs. */
int
lame_set_athaa_sensitivity( lame_global_flags*  gfp,
                            float               athaa_sensitivity )
{
    gfp->athaa_sensitivity = athaa_sensitivity;

    return 0;
}

float
lame_get_athaa_sensitivity( const lame_global_flags*  gfp )
{
    return gfp->athaa_sensitivity;
}


/* Predictability limit (ISO tonality formula) */
int
lame_set_cwlimit( lame_global_flags*  gfp,
                  int                 cwlimit )
{
    gfp->cwlimit = cwlimit;

    return 0;
}

int
lame_get_cwlimit( const lame_global_flags*  gfp )
{
    return gfp->cwlimit;
}



/*
 * Allow blocktypes to differ between channels.
 * default:
 *  0 for jstereo => block types coupled
 *  1 for stereo  => block types may differ
 */
int
lame_set_allow_diff_short( lame_global_flags*  gfp,
                           int                 allow_diff_short )
{
    gfp->short_blocks = 
        allow_diff_short ? short_block_allowed : short_block_coupled;

    return 0;
}

int
lame_get_allow_diff_short( const lame_global_flags*  gfp )
{
    if ( gfp->short_blocks == short_block_allowed ) 
        return 1; /* short blocks allowed to differ */
    else 
        return 0; /* not set, dispensed, forced or coupled */
}


/* Use temporal masking effect */
int
lame_set_useTemporal( lame_global_flags*  gfp,
                      int                 useTemporal )
{
    /* default = 1 (enabled) */

    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > useTemporal || 1 < useTemporal )
        return -1;

    gfp->useTemporal = useTemporal;

    return 0;
}

int
lame_get_useTemporal( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->useTemporal && 1 >= gfp->useTemporal );

    return gfp->useTemporal;
}


/* Use temporal masking effect */
int
lame_set_interChRatio( lame_global_flags*  gfp,
			float               ratio )
{
    /* default = 0.0 (no inter-cahnnel maskin) */
    if (! (0 <= ratio && ratio <= 1.0))
        return -1;

    gfp->interChRatio = ratio;

    return 0;
}

float
lame_get_interChRatio( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->interChRatio && gfp->interChRatio <= 1.0);

    return gfp->interChRatio;
}


/* Use pseudo substep shaping method */
int
lame_set_substep( lame_global_flags*  gfp,
		  int                 method )
{
    lame_internal_flags *gfc = gfp->internal_flags;
    /* default = 0.0 (no inter-cahnnel maskin) */
    if (! (0 <= method && method <= 2))
        return -1;

    gfc->substep_shaping = method;
    return 0;
}

int
lame_get_substep(const lame_global_flags*  gfp )
{
    lame_internal_flags *gfc = gfp->internal_flags;
    assert(0 <= gfc->substep_shaping && gfc->substep_shaping <= 2);
    return gfc->substep_shaping;
}

/* Disable short blocks. */
int
lame_set_no_short_blocks( lame_global_flags*  gfp,
                          int                 no_short_blocks )
{
    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > no_short_blocks || 1 < no_short_blocks )
        return -1;

    gfp->short_blocks = 
        no_short_blocks ? short_block_dispensed : short_block_allowed;

    return 0;
}
int
lame_get_no_short_blocks( const lame_global_flags*  gfp )
{
    switch ( gfp->short_blocks ) {
    default:
    case short_block_not_set:   return -1;
    case short_block_dispensed: return 1;
    case short_block_allowed:
    case short_block_coupled:
    case short_block_forced:    return 0;
    }
}


/* Force short blocks. */
int
lame_set_force_short_blocks( lame_global_flags*  gfp,
                          int                 short_blocks )
{
    /* enforce disable/enable meaning, if we need more than two values
       we need to switch to an enum to have an apropriate representation
       of the possible meanings of the value */
    if ( 0 > short_blocks || 1 < short_blocks )
        return -1;

    if (short_blocks == 1)
        gfp->short_blocks = short_block_forced;
    else if (gfp->short_blocks == short_block_forced) 
        gfp->short_blocks = short_block_allowed;

    return 0;
}
int
lame_get_force_short_blocks( const lame_global_flags*  gfp )
{
    switch ( gfp->short_blocks ) {
    default:
    case short_block_not_set:   return -1;
    case short_block_dispensed: 
    case short_block_allowed:
    case short_block_coupled:   return 0;
    case short_block_forced:    return 1;
    }
}


/*
 * Input PCM is emphased PCM
 * (for instance from one of the rarely emphased CDs).
 *
 * It is STRONGLY not recommended to use this, because psycho does not
 * take it into account, and last but not least many decoders
 * ignore these bits
 */
int
lame_set_emphasis( lame_global_flags*  gfp,
                   int                 emphasis )
{
    /* XXX: emphasis should be converted to an enum */
    if ( 0 > emphasis || 4 <= emphasis )
        return -1;

    gfp->emphasis = emphasis;

    return 0;
}

int
lame_get_emphasis( const lame_global_flags*  gfp )
{
    assert( 0 <= gfp->emphasis && 4 > gfp->emphasis );

    return gfp->emphasis;
}




/***************************************************************/
/* internal variables, cannot be set...                        */
/* provided because they may be of use to calling application  */
/***************************************************************/

/* MPEG version.
 *  0 = MPEG-2
 *  1 = MPEG-1
 * (2 = MPEG-2.5)    
 */
int
lame_get_version( const lame_global_flags* gfp )
{
    return gfp->version;
}


/* Encoder delay. */
int
lame_get_encoder_delay( const lame_global_flags*  gfp )
{
    return gfp->encoder_delay;
}

/* padding added to the end of the input */
int
lame_get_encoder_padding( const lame_global_flags*  gfp )
{
    return gfp->encoder_padding;
}


/* Size of MPEG frame. */
int
lame_get_framesize( const lame_global_flags*  gfp )
{
    return gfp->framesize;
}


/* Number of frames encoded so far. */
int
lame_get_frameNum( const lame_global_flags*  gfp )
{
    return gfp->frameNum;
}

int
lame_get_mf_samples_to_encode( const lame_global_flags*  gfp )
{
    lame_internal_flags *gfc = gfp->internal_flags;
    return gfc->mf_samples_to_encode;
}


int CDECL lame_get_size_mp3buffer( const lame_global_flags*  gfp )
{
    int size;
    compute_flushbits(gfp,&size);
    return size;
}



/*
 * LAME's estimate of the total number of frames to be encoded.
 * Only valid if calling program set num_samples.
 */
int
lame_get_totalframes( const lame_global_flags*  gfp )
{
    int totalframes;
    /* estimate based on user set num_samples: */
    totalframes =
        2 + ((double)gfp->num_samples * gfp->out_samplerate) / 
              ((double)gfp->in_samplerate * gfp->framesize);

    /* check to see if we underestimated totalframes */
    //    if (totalframes < gfp->frameNum)
    //        totalframes = gfp->frameNum;

    return totalframes;
}


/*

UNDOCUMENTED, experimental settings.  These routines are not prototyped
in lame.h.  You should not use them, they are experimental and may
change.  

*/


/*
 *  just another daily changing developer switch  
 */
void lame_set_tune( lame_global_flags* gfp, float val ) 
{
    gfp->tune_value_a = val;
    gfp->tune = 1;
}

/* Custom msfix hack */
void
lame_set_msfix( lame_global_flags*  gfp, double msfix )
{
    /* default = 0 */
    gfp->msfix = msfix;

}

int
lame_set_preset_expopts( lame_global_flags*  gfp, int preset_expopts )
{

    lame_internal_flags *gfc = gfp->internal_flags;

    gfc->presetTune.use = 1;

    /* default = 0 (disabled) */
    gfp->preset_expopts = preset_expopts;

    switch (preset_expopts)
    {
        case 1:

          lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | 1);
          lame_set_experimentalX(gfp, 3);
          lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | 2); // safejoint
          lame_set_ATHtype(gfp, 2);

          gfc->presetTune.attackthre   = 35;
          gfc->presetTune.attackthre_s = 150;
          gfc->presetTune.ms_maskadjust = .5;
		  gfc->presetTune.quantcomp_type_s = 3;
          gfc->presetTune.quantcomp_alt_type = 3;
          gfc->presetTune.athadjust_switch_level = 2; // Always switch

          break;

        case 2:

          if (gfp->VBR == vbr_mtrh) {
             lame_set_experimentalX(gfp, 2);
             gfc->presetTune.quantcomp_adjust_mtrh = 9;
             gfc->presetTune.quantcomp_type_s = 4;
             gfc->presetTune.quantcomp_alt_type = 0;
             gfc->presetTune.athadjust_safe_noiseshaping_thre = 0.0;
			 gfc->presetTune.athadjust_safe_athaasensitivity = 8.0;
          }
          else {
             lame_set_experimentalX(gfp, 3);
             gfc->presetTune.quantcomp_adjust_rh_tot = 600;
			 gfc->presetTune.quantcomp_adjust_rh_max = 60;
             gfc->presetTune.quantcomp_type_s = 3;
             gfc->presetTune.quantcomp_alt_type = 1;
          }

          lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | 1);
          lame_set_experimentalZ(gfp, 1);
          lame_set_VBR_q(gfp, 2);
          lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | 2); // safejoint
          lame_set_ATHtype(gfp, 2);				
          // modify sfb21 by 3 dB plus ns-treble=0                 
          lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | (12 << 20));

          gfc->presetTune.attackthre   = 35;
          gfc->presetTune.attackthre_s = 150;
          gfc->presetTune.ms_maskadjust = .5;
          gfc->presetTune.athadjust_switch_level = 1;
          gfc->presetTune.athadjust_msfix = 2.13;

          break;

        case 3:

          if (gfp->VBR == vbr_mtrh) {
             gfc->presetTune.quantcomp_type_s = 4;
             gfc->presetTune.quantcomp_adjust_mtrh = 9;
			 gfc->presetTune.quantcomp_alt_type = 0;
             (void) lame_set_ATHlower( gfp, -2 );
             gfc->presetTune.athadjust_safe_noiseshaping_thre = 0.0;
			 gfc->presetTune.athadjust_safe_athaasensitivity = 8.0;
          }
          else {
             gfc->presetTune.quantcomp_type_s = 3;
             gfc->presetTune.quantcomp_adjust_rh_tot = 600;
			 gfc->presetTune.quantcomp_adjust_rh_max = 60;
             (void) lame_set_ATHlower( gfp, -1 );
          }

          lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | 1);
          lame_set_experimentalZ(gfp, 1);
          lame_set_experimentalX(gfp, 1);
          lame_set_VBR_q(gfp, 2);
          lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | 2); // safejoint
   (void) lame_set_msfix( gfp, 2.13 );
          lame_set_ATHtype(gfp, 4);
          // modify sfb21 by 3.75 dB plus ns-treble=0                 
          lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | (15 << 20));

          gfc->presetTune.attackthre   = 35;
          gfc->presetTune.attackthre_s = 150;
          gfc->presetTune.ms_maskadjust = .5;
          gfc->presetTune.athadjust_switch_level = 1;

          break;
    }
    return 0;
}

int
lame_set_preset_notune( lame_global_flags*  gfp, int preset_notune )
{
    lame_internal_flags *gfc = gfp->internal_flags;

    gfc->presetTune.use = 0;  // Turn off specialized preset tunings

    return 0;
}


int
lame_set_preset( lame_global_flags*  gfp, int preset )
{
    gfp->preset = preset;
    return apply_preset(gfp, preset);
}



int 
lame_set_asm_optimizations( lame_global_flags*  gfp, int optim, int mode)
{
    mode = (mode == 1? 1 : 0);
    switch (optim){
        case MMX: {
            gfp->asm_optimizations.mmx = mode;
            return optim;
        }
        case AMD_3DNOW: {
            gfp->asm_optimizations.amd3dnow = mode;
            return optim;
        }
        case SSE: {
            gfp->asm_optimizations.sse = mode;
            return optim;
        }
        default: return optim;
    }

}
















