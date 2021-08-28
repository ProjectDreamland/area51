/*
 *	LAME MP3 encoding engine
 *
 *	Copyright (c) 1999 Mark Taylor
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* $Id: encoder.c,v 1.77 2002/09/30 16:49:30 bouvigne Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>

#include "lame.h"
#include "util.h"
#include "newmdct.h"
#include "psymodel.h"
#include "quantize.h"
#include "quantize_pvt.h"
#include "bitstream.h"
#include "VbrTag.h"
#include "vbrquantize.h"

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif


/*
 * auto-adjust of ATH, useful for low volume
 * Gabriel Bouvigne 3 feb 2001
 *
 * modifies some values in
 *   gfp->internal_flags->ATH
 *   (gfc->ATH)
 */
void
adjust_ATH( lame_global_flags* const  gfp,
            FLOAT8              tot_ener[2][4] )
{
    lame_internal_flags* const  gfc = gfp->internal_flags;
    int gr, channel;
    FLOAT max_pow, max_pow_alt;
    FLOAT8 max_val;

    if (gfc->ATH->use_adjust == 0) {
        gfc->ATH->adjust = 1.0;	/* no adjustment */
        return;
    }
    
    switch( gfp->athaa_loudapprox ) {
    case 1:
                                /* flat approximation for loudness (squared) */
        max_pow = 0;
        for ( gr = 0; gr < gfc->mode_gr; ++gr ) 
            for ( channel = 0; channel < gfc->channels_out; ++channel ) 
                max_pow = Max( max_pow, tot_ener[gr][channel] );
        max_pow *= 0.25/ 5.6e13; /* scale to 0..1 (5.6e13), and tune (0.25) */
        break;
    
    case 2:                     /* jd - 2001 mar 12, 27, jun 30 */
    {				            /* loudness based on equal loudness curve; */
                                /* use granule with maximum combined loudness*/
	    FLOAT gr2_max;
        max_pow = gfc->loudness_sq[0][0];
	    gr2_max = gfc->loudness_sq[1][0];
        if( gfc->channels_out == 2 ) {
            max_pow += gfc->loudness_sq[0][1];
	        gr2_max += gfc->loudness_sq[1][1];
	    } else {
	        max_pow += max_pow;
	        gr2_max += gr2_max;
	    }
	    if( gfc->mode_gr == 2 ) {
	        max_pow = Max( max_pow, gr2_max );
	    }
	    max_pow *= 0.5;		    /* max_pow approaches 1.0 for full band noise*/
        break;
    }

    default:
        gfc->ATH->adjust = 1.0;	/* no adjustment */
        return;
    }

                                /* jd - 2001 mar 31, jun 30 */
                                /* user tuning of ATH adjustment region */
    max_pow_alt = max_pow;
    max_pow *= gfc->athaa_sensitivity_p;
    if (gfc->presetTune.use)
        max_pow_alt *= pow( 10.0, gfc->presetTune.athadjust_safe_athaasensitivity / -10.0 );

    /*  adjust ATH depending on range of maximum value
        The first mode was just implemented for rough
        experimentation. Third mode is recommended
     */
    switch ( gfc->ATH->use_adjust ) {

    case  1:                                    /* by Gabriel Bouvigne */
        max_val = sqrt( max_pow );              /* max_val should be between 0 and 1 */
                                                
        if      (max_val > 0.5) {               /* value above 50 % */
                gfc->ATH->adjust = 1.0;         /* do not reduce ATH */
        }
        else if (max_val > 0.3) {               /* value above 30 % */
                gfc->ATH->adjust *= 0.955;      /* reduce by ~0.2 dB */
                if (gfc->ATH->adjust < 0.3)     /* but ~5 dB in maximum */
                    gfc->ATH->adjust = 0.3;            
        }
        else {                                  /* value below 30 % */
                gfc->ATH->adjust *= 0.93;       /* reduce by ~0.3 dB */
                if (gfc->ATH->adjust < 0.01)    /* but 20 dB in maximum */
                    gfc->ATH->adjust = 0.01;
        }
        break;

    case  2:   
        max_val = Min( max_pow, 1.0 ) * 32768; /* adapt for RH's adjust */

      {                         /* by Robert Hegemann */
        /*  this code reduces slowly the ATH (speed of 12 dB per second)
         */
        FLOAT8 
        //x = Max (640, 320*(int)(max_val/320));
        x = Max (32, 32*(int)(max_val/32));
        x = x/32768;
        gfc->ATH->adjust *= gfc->ATH->decay;
        if (gfc->ATH->adjust < x)       /* but not more than f(x) dB */
            gfc->ATH->adjust = x;
      }
        break;

    case  3:
      {                         /* jd - 2001 feb27, mar12,20, jun30, jul22 */
                                /* continuous curves based on approximation */
                                /* to GB's original values. */
        FLOAT8 adj_lim_new;
                                /* For an increase in approximate loudness, */
                                /* set ATH adjust to adjust_limit immediately*/
                                /* after a delay of one frame. */
                                /* For a loudness decrease, reduce ATH adjust*/
                                /* towards adjust_limit gradually. */
                                /* max_pow is a loudness squared or a power. */
        if( max_pow > 0.03125) { /* ((1 - 0.000625)/ 31.98) from curve below */
            if( gfc->ATH->adjust >= 1.0) {
                gfc->ATH->adjust = 1.0;
            } else {
                                /* preceding frame has lower ATH adjust; */
                                /* ascend only to the preceding adjust_limit */
                                /* in case there is leading low volume */
                if( gfc->ATH->adjust < gfc->ATH->adjust_limit) {
                    gfc->ATH->adjust = gfc->ATH->adjust_limit;
                }
            }
            if (gfc->presetTune.use) {
		        if (max_pow_alt > gfc->presetTune.athadjust_safe_noiseshaping_thre)
			      gfc->presetTune.athadjust_safe_noiseshaping = 1;
		        else
			      gfc->presetTune.athadjust_safe_noiseshaping = 0;
            }
            gfc->ATH->adjust_limit = 1.0;
        } else {                /* adjustment curve */
                                /* about 32 dB maximum adjust (0.000625) */
            adj_lim_new = 31.98 * max_pow + 0.000625;
            if( gfc->ATH->adjust >= adj_lim_new) { /* descend gradually */
                gfc->ATH->adjust *= adj_lim_new * 0.075 + 0.925;
                if( gfc->ATH->adjust < adj_lim_new) { /* stop descent */
                    gfc->ATH->adjust = adj_lim_new;
                }
            } else {            /* ascend */
                if( gfc->ATH->adjust_limit >= adj_lim_new) {
                    gfc->ATH->adjust = adj_lim_new;
                } else {        /* preceding frame has lower ATH adjust; */
                                /* ascend only to the preceding adjust_limit */
                    if( gfc->ATH->adjust < gfc->ATH->adjust_limit) {
                        gfc->ATH->adjust = gfc->ATH->adjust_limit;
                    }
                }
            }
            gfc->ATH->adjust_limit = adj_lim_new;
        }
      }
        break;
        
    default:
        gfc->ATH->adjust = 1.0;	/* no adjustment */
        break;
    }   /* switch */
}

/************************************************************************
*
* encodeframe()           Layer 3
*
* encode a single frame
*
************************************************************************
lame_encode_frame()


                       gr 0            gr 1
inbuf:           |--------------|---------------|-------------|
MDCT output:  |--------------|---------------|-------------|

FFT's                    <---------1024---------->
                                         <---------1024-------->



    inbuf = buffer of PCM data size=MP3 framesize
    encoder acts on inbuf[ch][0], but output is delayed by MDCTDELAY
    so the MDCT coefficints are from inbuf[ch][-MDCTDELAY]

    psy-model FFT has a 1 granule delay, so we feed it data for the 
    next granule.
    FFT is centered over granule:  224+576+224
    So FFT starts at:   576-224-MDCTDELAY

    MPEG2:  FFT ends at:  BLKSIZE+576-224-MDCTDELAY
    MPEG1:  FFT ends at:  BLKSIZE+2*576-224-MDCTDELAY    (1904)

    FFT starts at 576-224-MDCTDELAY (304)  = 576-FFTOFFSET

*/

typedef FLOAT8 chgrdata[2][2];

int  lame_encode_mp3_frame (				// Output
	lame_global_flags* const  gfp,			// Context
	sample_t*                 inbuf_l,              // Input
	sample_t*                 inbuf_r,              // Input
	unsigned char*            mp3buf, 		// Output
	int                    mp3buf_size )		// Output
{
  int mp3count;
  III_psy_ratio masking_LR[2][2];    /*LR masking & energy */
  III_psy_ratio masking_MS[2][2]; /*MS masking & energy */
  III_psy_ratio (*masking)[2][2];  /*pointer to selected maskings*/
  const sample_t *inbuf[2];
  lame_internal_flags *gfc=gfp->internal_flags;

  FLOAT8 tot_ener[2][4];
  FLOAT8 ms_ener_ratio[2]={.5,.5};
  chgrdata pe,pe_MS;
  chgrdata *pe_use;

  int ch,gr,mean_bits;
  int bitsPerFrame;

  int check_ms_stereo;
  FLOAT8 ms_ratio_next = 0.;
  FLOAT8 ms_ratio_prev = 0.;


  memset((char *) masking_LR, 0, sizeof(masking_LR));
  memset((char *) masking_MS, 0, sizeof(masking_MS));
  inbuf[0]=inbuf_l;
  inbuf[1]=inbuf_r;

  check_ms_stereo =  (gfp->mode == JOINT_STEREO);
  gfc->mode_ext = MPG_MD_LR_LR;

  if (gfc->lame_encode_frame_init==0 )  {
    gfc->lame_encode_frame_init=1;
    
    /* padding method as described in 
     * "MPEG-Layer3 / Bitstream Syntax and Decoding"
     * by Martin Sieler, Ralph Sperschneider
     *
     * note: there is no padding for the very first frame
     *
     * Robert.Hegemann@gmx.de 2000-06-22
     */
        
    gfc->frac_SpF = ((gfp->version+1)*72000L*gfp->brate) % gfp->out_samplerate;
    gfc->slot_lag  = gfc->frac_SpF;
    
    switch (gfp->padding_type) {
    default:
    case PAD_NO:
        gfc->padding = FALSE;
        break;
    case PAD_ALL:
        gfc->padding = TRUE;
        break;
    }
    
    /* check FFT will not use a negative starting offset */
#if 576 < FFTOFFSET
# error FFTOFFSET greater than 576: FFT uses a negative offset
#endif
    /* check if we have enough data for FFT */
    assert(gfc->mf_size>=(BLKSIZE+gfp->framesize-FFTOFFSET));
    /* check if we have enough data for polyphase filterbank */
    /* it needs 1152 samples + 286 samples ignored for one granule */
    /*          1152+576+286 samples for two granules */
    assert(gfc->mf_size>=(286+576*(1+gfc->mode_gr)));

    /* prime the MDCT/polyphase filterbank with a short block */
    { 
      int i,j;
      sample_t primebuff0[286+1152+576];
      sample_t primebuff1[286+1152+576];
      for (i=0, j=0; i<286+576*(1+gfc->mode_gr); ++i) {
	if (i<576*gfc->mode_gr) {
	  primebuff0[i]=0;
	  if (gfc->channels_out==2) 
	    primebuff1[i]=0;
	}else{
	  primebuff0[i]=inbuf[0][j];
	  if (gfc->channels_out==2) 
	    primebuff1[i]=inbuf[1][j];
	  ++j;
	}
      }
      /* polyphase filtering / mdct */
      for ( gr = 0; gr < gfc->mode_gr; gr++ ) {
	for ( ch = 0; ch < gfc->channels_out; ch++ ) {
	  gfc->l3_side.tt[gr][ch].block_type=SHORT_TYPE;
	}
      }
      mdct_sub48(gfc, primebuff0, primebuff1);
    }
    
    iteration_init(gfp);
    
    /*  prepare for ATH auto adjustment:
     *  we want to decrease the ATH by 12 dB per second
     */ {
        FLOAT8 frame_duration = 576. * gfc->mode_gr / gfp->out_samplerate;
        gfc->ATH->decay = pow(10., -12./10. * frame_duration);
        gfc->ATH->adjust = 0.01; /* minimum, for leading low loudness */
        gfc->ATH->adjust_limit = 1.0; /* on lead, allow adjust up to maximum */
    }
  }


  /********************** padding *****************************/
  if (gfp->padding_type == PAD_ADJUST) {
        /* padding method as described in 
         * "MPEG-Layer3 / Bitstream Syntax and Decoding"
         * by Martin Sieler, Ralph Sperschneider
         *
         * note: there is no padding for the very first frame
         *
         * Robert.Hegemann@gmx.de 2000-06-22
         */

        gfc->slot_lag -= gfc->frac_SpF;
        if (gfc->slot_lag < 0) {
            gfc->slot_lag += gfp->out_samplerate;
            gfc->padding = TRUE;
        } 
        else {
            gfc->padding = FALSE;
        }
  }


  if (gfc->psymodel) {
    /* psychoacoustic model
     * psy model has a 1 granule (576) delay that we must compensate for
     * (mt 6/99).
     */
    int ret;
    const sample_t *bufp[2];  /* address of beginning of left & right granule */
    int blocktype[2];

    ms_ratio_prev=gfc->ms_ratio[gfc->mode_gr-1];
    for (gr=0; gr < gfc->mode_gr ; gr++) {

      for ( ch = 0; ch < gfc->channels_out; ch++ )
	bufp[ch] = &inbuf[ch][576 + gr*576-FFTOFFSET];

      if (gfc->nsPsy.use) {
	ret=L3psycho_anal_ns( gfp, bufp, gr, 
			      &gfc->ms_ratio[gr],&ms_ratio_next,
			      masking_LR, masking_MS,
			      pe[gr],pe_MS[gr],tot_ener[gr],blocktype);
      } else {
	ret=L3psycho_anal( gfp, bufp, gr, 
			   &gfc->ms_ratio[gr],&ms_ratio_next,
			   masking_LR, masking_MS,
			   pe[gr],pe_MS[gr],tot_ener[gr],blocktype);
      }
      if (ret!=0) return -4;

      for ( ch = 0; ch < gfc->channels_out; ch++ )
	gfc->l3_side.tt[gr][ch].block_type=blocktype[ch];

      if (check_ms_stereo) {
	  ms_ener_ratio[gr] = tot_ener[gr][2]+tot_ener[gr][3];
	  if (ms_ener_ratio[gr]>0)
	      ms_ener_ratio[gr] = tot_ener[gr][3]/ms_ener_ratio[gr];
      }

    }
  }else{
    for (gr=0; gr < gfc->mode_gr ; gr++)
      for ( ch = 0; ch < gfc->channels_out; ch++ ) {
	gfc->l3_side.tt[gr][ch].block_type=NORM_TYPE;
	pe_MS[gr][ch]=pe[gr][ch]=700;
      }
  }



  /* auto-adjust of ATH, useful for low volume */
  adjust_ATH( gfp, tot_ener );



  /* block type flags */
  for( gr = 0; gr < gfc->mode_gr; gr++ ) {
    for ( ch = 0; ch < gfc->channels_out; ch++ ) {
      gr_info *cod_info = &gfc->l3_side.tt[gr][ch];
      cod_info->mixed_block_flag = 0;     /* never used by this model */
    }
  }


  /* polyphase filtering / mdct */
  mdct_sub48(gfc, inbuf[0], inbuf[1]);

  /* use m/s gfc->channels_out? */
  if (check_ms_stereo) {
    int gr0 = 0, gr1 = gfc->mode_gr-1;
    /* make sure block type is the same in each channel */
    check_ms_stereo =
      (gfc->l3_side.tt[gr0][0].block_type==gfc->l3_side.tt[gr0][1].block_type) &&
      (gfc->l3_side.tt[gr1][0].block_type==gfc->l3_side.tt[gr1][1].block_type);
  }
  
  /* Here will be selected MS or LR coding of the 2 stereo channels */

  assert (  gfc->mode_ext == MPG_MD_LR_LR );
  gfc->mode_ext = MPG_MD_LR_LR;
  
  if (gfp->force_ms) {
    gfc->mode_ext = MPG_MD_MS_LR;
  } else if (check_ms_stereo) {
      /* ms_ratio = is scaled, for historical reasons, to look like
	 a ratio of side_channel / total.  
         0 = signal is 100% mono
         .5 = L & R uncorrelated
      */
      
      /* [0] and [1] are the results for the two granules in MPEG-1,
       * in MPEG-2 it's only a faked averaging of the same value
       * _prev is the value of the last granule of the previous frame
       * _next is the value of the first granule of the next frame
       */
      FLOAT8  ms_ratio_ave1;
      FLOAT8  ms_ratio_ave2;
      FLOAT8  threshold1    = 0.35;
      FLOAT8  threshold2    = 0.45;

      /* take an average */
      if (gfc->mode_gr==1) {
	  /* MPEG2 - no second granule */
	  ms_ratio_ave1 = 0.33 * ( gfc->ms_ratio[0] + ms_ratio_prev + ms_ratio_next );
	  ms_ratio_ave2 = gfc->ms_ratio[0];
      }else{
	  ms_ratio_ave1 = 0.25 * ( gfc->ms_ratio[0] + gfc->ms_ratio[1] + ms_ratio_prev + ms_ratio_next );
	  ms_ratio_ave2 = 0.50 * ( gfc->ms_ratio[0] + gfc->ms_ratio[1] );
      }
      
      if (gfp->mode_automs) {
	  if ( gfp->compression_ratio < 11.025 ) {
	      /* 11.025 => 1, 6.3 => 0 */
	      double thr = (gfp->compression_ratio - 6.3) / (11.025 - 6.3);
	      if (thr<0) thr=0;
	      threshold1   *= thr;
	      threshold2   *= thr;
	  }
      }
      
      if ((ms_ratio_ave1 < threshold1  &&  ms_ratio_ave2 < threshold2) || gfc->nsPsy.use) {
	  int  sum_pe_MS = 0;
	  int  sum_pe_LR = 0;
	  for ( gr = 0; gr < gfc->mode_gr; gr++ ) {
	      for ( ch = 0; ch < gfc->channels_out; ch++ ) {
		  sum_pe_MS += pe_MS[gr][ch];
		  sum_pe_LR += pe[gr][ch];
	      }
	  }
	  
	  /* based on PE: M/S coding would not use much more bits than L/R coding */
	  
	  if (sum_pe_MS <= 1.07 * sum_pe_LR && !gfc->nsPsy.use) gfc->mode_ext = MPG_MD_MS_LR;
	  if (sum_pe_MS <= 1.00 * sum_pe_LR &&  gfc->nsPsy.use) gfc->mode_ext = MPG_MD_MS_LR;
      }
  }


#if defined(HAVE_GTK)
  /* copy data for MP3 frame analyzer */
  if (gfp->analysis && gfc->pinfo != NULL) {
    for ( gr = 0; gr < gfc->mode_gr; gr++ ) {
      for ( ch = 0; ch < gfc->channels_out; ch++ ) {
	gfc->pinfo->ms_ratio[gr]=gfc->ms_ratio[gr];
	gfc->pinfo->ms_ener_ratio[gr]=ms_ener_ratio[gr];
	gfc->pinfo->blocktype[gr][ch]=gfc->l3_side.tt[gr][ch].block_type;
	memcpy(gfc->pinfo->xr[gr][ch], &gfc->l3_side.tt[gr][ch].xr,
	       sizeof(FLOAT8)*576);
	/* in psymodel, LR and MS data was stored in pinfo.  
	   switch to MS data: */
	if (gfc->mode_ext==MPG_MD_MS_LR) {
	  gfc->pinfo->pe[gr][ch]=gfc->pinfo->pe[gr][ch+2];
	  gfc->pinfo->ers[gr][ch]=gfc->pinfo->ers[gr][ch+2];
	  memcpy(gfc->pinfo->energy[gr][ch],gfc->pinfo->energy[gr][ch+2],
		 sizeof(gfc->pinfo->energy[gr][ch]));
	}
      }
    }
  }
#endif




  /* bit and noise allocation */
  if (MPG_MD_MS_LR == gfc->mode_ext) {
    masking = &masking_MS;    /* use MS masking */
    pe_use = &pe_MS;
  } else {
    masking = &masking_LR;    /* use LR masking */
    pe_use = &pe;
  }


  if (gfc->nsPsy.use && (gfp->VBR == vbr_off || gfp->VBR == vbr_abr)) {
    static FLOAT fircoef[19] = {
      -0.0207887,-0.0378413,-0.0432472,-0.031183,
      7.79609e-18,0.0467745,0.10091,0.151365,
      0.187098,0.2,0.187098,0.151365,
      0.10091,0.0467745,7.79609e-18,-0.031183,
      -0.0432472,-0.0378413,-0.0207887,
    };
    int i;
    FLOAT8 f;

    for(i=0;i<18;i++) gfc->nsPsy.pefirbuf[i] = gfc->nsPsy.pefirbuf[i+1];

    i=0;
    gfc->nsPsy.pefirbuf[18] = 0;
    for ( gr = 0; gr < gfc->mode_gr; gr++ ) {
      for ( ch = 0; ch < gfc->channels_out; ch++ ) {
	gfc->nsPsy.pefirbuf[18] += (*pe_use)[gr][ch];
	i++;
      }
    }

    gfc->nsPsy.pefirbuf[18] = gfc->nsPsy.pefirbuf[18] / i;
    f = 0;
    for(i=0;i<19;i++) f += gfc->nsPsy.pefirbuf[i] * fircoef[i];

    for ( gr = 0; gr < gfc->mode_gr; gr++ ) {
      for ( ch = 0; ch < gfc->channels_out; ch++ ) {
	(*pe_use)[gr][ch] *= 670 / f;
      }
    }
  }

  switch (gfp->VBR){ 
  default:
  case vbr_off:
    iteration_loop( gfp,*pe_use,ms_ener_ratio, *masking);
    break;
  case vbr_mt:
  case vbr_rh:
  case vbr_mtrh:
    VBR_iteration_loop( gfp,*pe_use,ms_ener_ratio, *masking);
    break;
  case vbr_abr:
    ABR_iteration_loop( gfp,*pe_use,ms_ener_ratio, *masking);
    break;
  }

  /*  write the frame to the bitstream  */
  getframebits(gfp, &bitsPerFrame, &mean_bits);

  format_bitstream( gfp, bitsPerFrame);

  /* copy mp3 bit buffer into array */
  mp3count = copy_buffer(gfc,mp3buf,mp3buf_size,1);




  if (gfp->bWriteVbrTag) AddVbrFrame(gfp);


#if defined(HAVE_GTK)
  /* copy data for MP3 frame analyzer */
  if (gfp->analysis && gfc->pinfo != NULL) {
    int j;
    for ( ch = 0; ch < gfc->channels_out; ch++ ) {
      for ( j = 0; j < FFTOFFSET; j++ )
	gfc->pinfo->pcmdata[ch][j] = gfc->pinfo->pcmdata[ch][j+gfp->framesize];
      for ( j = FFTOFFSET; j < 1600; j++ ) {
	gfc->pinfo->pcmdata[ch][j] = inbuf[ch][j-FFTOFFSET];
      }
    }
    set_frame_pinfo (gfp, *masking);
  }
#endif
  
  updateStats( gfc );

  return mp3count;
}
