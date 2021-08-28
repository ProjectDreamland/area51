
#ifndef LAME_GLOBAL_FLAGS_H
#define LAME_GLOBAL_FLAGS_H

struct lame_internal_flags;
typedef struct lame_internal_flags lame_internal_flags;


typedef enum short_block_e {
    short_block_not_set = -1,   /* allow LAME to decide */
    short_block_allowed = 0,    /* LAME may use them, even different block types for L/R */
    short_block_coupled,        /* LAME may use them, but always same block types in L/R */
    short_block_dispensed,      /* LAME will not use short blocks, long blocks only */
    short_block_forced          /* LAME will not use long blocks, short blocks only */
} short_block_t;

/***********************************************************************
*
*  Control Parameters set by User.  These parameters are here for
*  backwards compatibility with the old, non-shared lib API.  
*  Please use the lame_set_variablename() functions below
*
*
***********************************************************************/
struct lame_global_struct {
  /* input description */
  unsigned long num_samples;  /* number of samples. default=2^32-1           */
  int num_channels;           /* input number of channels. default=2         */
  int in_samplerate;          /* input_samp_rate in Hz. default=44.1 kHz     */
  int out_samplerate;         /* output_samp_rate.
                                   default: LAME picks best value 
                                   at least not used for MP3 decoding:
                                   Remember 44.1 kHz MP3s and AC97           */
  float scale;                /* scale input by this amount before encoding
                                 at least not used for MP3 decoding          */
  float scale_left;           /* scale input of channel 0 (left) by this
			         amount before encoding                      */
  float scale_right;          /* scale input of channel 1 (right) by this
			         amount before encoding                      */

  /* general control params */
  int analysis;               /* collect data for a MP3 frame analyzer?      */
  int bWriteVbrTag;           /* add Xing VBR tag?                           */
  int decode_only;            /* use lame/mpglib to convert mp3/ogg to wav   */
  int ogg;                    /* encode to Vorbis .ogg file                  */

  int quality;                /* quality setting 0=best,  9=worst  default=5 */
  MPEG_mode mode;             /* see enum in lame.h
                                 default = LAME picks best value             */
  int mode_fixed;             /* ignored                                     */
  int mode_automs;            /* use a m/s threshold based on compression
                                 ratio                                       */
  int force_ms;               /* force M/S mode.  requires mode=1            */
  int free_format;            /* use free format? default=0                  */

  /*
   * set either brate>0  or compression_ratio>0, LAME will compute
   * the value of the variable not set.
   * Default is compression_ratio = 11.025
   */
  int brate;                  /* bitrate                                    */
  float compression_ratio;    /* sizeof(wav file)/sizeof(mp3 file)          */


  /* frame params */
  int copyright;                  /* mark as copyright. default=0           */
  int original;                   /* mark as original. default=1            */
  int error_protection;           /* use 2 bytes per frame for a CRC
                                     checksum. default=0                    */
  Padding_type padding_type;      /* PAD_NO = no padding,
                                     PAD_ALL = always pad,
                                     PAD_ADJUST = adjust padding,
                                     default=2                              */
  int extension;                  /* the MP3 'private extension' bit.
                                     Meaningless                            */
  int strict_ISO;                 /* enforce ISO spec as much as possible   */

  /* quantization/noise shaping */
  int disable_reservoir;          /* use bit reservoir?                     */
  int experimentalX;            
  int experimentalY;
  int experimentalZ;
  int exp_nspsytune;

  double newmsfix;
  int preset_expopts;

  int preset;

  /* VBR control */
  vbr_mode VBR;
  int VBR_q;
  int VBR_mean_bitrate_kbps;
  int VBR_min_bitrate_kbps;
  int VBR_max_bitrate_kbps;
  int VBR_hard_min;             /* strictly enforce VBR_min_bitrate
                                   normaly, it will be violated for analog
                                   silence                                 */


  /* resampling and filtering */
  int lowpassfreq;                /* freq in Hz. 0=lame choses.
                                     -1=no filter                          */
  int highpassfreq;               /* freq in Hz. 0=lame choses.
                                     -1=no filter                          */
  int lowpasswidth;               /* freq width of filter, in Hz
                                     (default=15%)                         */
  int highpasswidth;              /* freq width of filter, in Hz
                                     (default=15%)                         */



  /*
   * psycho acoustics and other arguments which you should not change 
   * unless you know what you are doing
   */
  int ATHonly;                    /* only use ATH                         */
  int ATHshort;                   /* only use ATH for short blocks        */
  int noATH;                      /* disable ATH                          */
  int ATHtype;                    /* select ATH formula                   */
  float ATHlower;                 /* lower ATH by this many db            */
  int athaa_type;                 /* select ATH auto-adjust scheme        */
  int athaa_loudapprox;           /* select ATH auto-adjust loudness calc */
  float athaa_sensitivity;        /* dB, tune active region of auto-level */
  int cwlimit;                    /* predictability limit                 */
  short_block_t short_blocks;
/*  int allow_diff_short;            allow blocktypes to differ between
                                     channels?                            */
  int useTemporal;                /* use temporal masking effect          */
  float interChRatio;
/*  int no_short_blocks;             disable short blocks                 */
  int emphasis;                   /* Input PCM is emphased PCM (for
                                     instance from one of the rarely
                                     emphased CDs), it is STRONGLY not
                                     recommended to use this, because
				     psycho does not take it into account,
				     and last but not least many decoders
                                     don't care about these bits          */
  float msfix;              /* Naoki's adjustment of Mid/Side maskings */

  int   tune;               /* 0 off, 1 on */
  float tune_value_a;       /* used to pass values for debugging and stuff */

  
  struct {
    void (*msgf)  (const char *format, va_list ap);
    void (*debugf)(const char *format, va_list ap);
    void (*errorf)(const char *format, va_list ap);
  } report;

  /************************************************************************/
  /* internal variables, do not set...                                    */
  /* provided because they may be of use to calling application           */
  /************************************************************************/

  int version;                    /* 0=MPEG-2/2.5  1=MPEG-1               */
  int encoder_delay;
  int encoder_padding;  /* number of samples of padding appended to input */
  int framesize;                  
  int frameNum;                   /* number of frames encoded             */
  int lame_allocated_gfp;         /* is this struct owned by calling
                                     program or lame?                     */



  /****************************************************************************/
  /* more internal variables, which will not exist after lame_encode_finish() */
  /****************************************************************************/
  lame_internal_flags *internal_flags;

  /* VBR tags.  This data is here because VBR header is writen after
   * input file is closed and *internal_flags struct is free'd */
  int TotalFrameSize;
  //int* pVbrFrames;
  int nVbrNumFrames;
  int nVbrFrameBufferSize;

  struct {
    int integer[16];
    float real[16];
    void *pointer[16];
  } exp_nspsytune2;

  struct {
      int mmx;
      int amd3dnow;
      int sse;

  } asm_optimizations;
} ;

#endif /* LAME_GLOBAL_FLAGS_H */
