/*
 * MP3 quantization
 *
 * Copyright (c) 1999 Mark Taylor
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* $Id: quantize.c,v 1.121 2002/09/30 16:49:30 bouvigne Exp $ */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>
#include <assert.h>
#include "util.h"
#include "l3side.h"
#include "quantize.h"
#include "reservoir.h"
#include "quantize_pvt.h"
#include "lame-analysis.h"
#include "vbrquantize.h"

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif


/* convert from L/R <-> Mid/Side, src == dst allowed */
inline static void 
ms_convert(III_side_info_t *l3_side, int gr)
{
    int i;
    for (i = 0; i < 576; ++i) {
	FLOAT8 l, r;
        l = l3_side->tt[gr][0].xr[i];
        r = l3_side->tt[gr][1].xr[i];
        l3_side->tt[gr][0].xr[i] = (l+r) * (FLOAT8)(SQRT2*0.5);
        l3_side->tt[gr][1].xr[i] = (l-r) * (FLOAT8)(SQRT2*0.5);
    }
}



/************************************************************************
 *
 *      init_outer_loop()
 *  mt 6/99                                    
 *
 *  initializes cod_info, scalefac and xrpow
 *
 *  returns 0 if all energies in xr are zero, else 1                    
 *
 ************************************************************************/

static int 
init_xrpow(
    lame_internal_flags *gfc,
    gr_info *const cod_info, 
    FLOAT8 xrpow[576] )
{
    FLOAT8 tmp, sum = 0;
    int i;
    memset(&gfc->pseudohalf, 0, sizeof(gfc->pseudohalf));
    assert( xrpow != NULL );
    
    /*  check if there is some energy we have to quantize
     *  and calculate xrpow matching our fresh scalefactors
     */
    for (i = 0; i < 576; ++i) {
        tmp = fabs (cod_info->xr[i]);
	sum += tmp;
        xrpow[i] = sqrt (tmp * sqrt(tmp));
    }
    /*  return 1 if we have something to quantize, else 0
     */
    if (sum > (FLOAT8)1E-20)
      return 1;

    memset(&cod_info->l3_enc, 0, sizeof(int)*576);
    return 0;
}

static void
init_outer_loop(
    lame_internal_flags *gfc,
    gr_info *const cod_info)
{
    /*  initialize fresh cod_info
     */
    cod_info->part2_3_length      = 0;
    cod_info->big_values          = 0;
    cod_info->count1              = 0;
    cod_info->global_gain         = 210;
    cod_info->scalefac_compress   = 0;
    /* block_type            was set in psymodel.c? */
    /* mixed_block_flag      would be set in ^      */
    cod_info->table_select [0]    = 0;
    cod_info->table_select [1]    = 0;
    cod_info->table_select [2]    = 0;
    cod_info->subblock_gain[0]    = 0;
    cod_info->subblock_gain[1]    = 0;
    cod_info->subblock_gain[2]    = 0;
    cod_info->region0_count       = 0;
    cod_info->region1_count       = 0;
    cod_info->preflag             = 0;
    cod_info->scalefac_scale      = 0;
    cod_info->count1table_select  = 0;
    cod_info->part2_length        = 0;
    cod_info->sfb_lmax        = SBPSY_l;
    cod_info->sfb_smin        = SBPSY_s;
    cod_info->psy_lmax        = gfc->sfb21_extra ? SBMAX_l : SBPSY_l;
    cod_info->psy_smax        = 0;
    if (cod_info->block_type == SHORT_TYPE) {
	int sfb;
	FLOAT8 ixwork[576];
	FLOAT8 *ix;

        cod_info->sfb_smin        = 0;
        cod_info->psy_smax        = gfc->sfb21_extra ? SBMAX_s : SBPSY_s;
        cod_info->sfb_lmax        = 0;
	if (cod_info->mixed_block_flag) {
            /*
             *  MPEG-1:      sfbs 0-7 long block, 3-12 short blocks 
             *  MPEG-2(.5):  sfbs 0-5 long block, 3-12 short blocks
             */ 
	    cod_info->sfb_smin    = 3;
            cod_info->sfb_lmax    = gfc->is_mpeg1 ? 8 : 6;
	}
	cod_info->psy_lmax    = cod_info->sfb_lmax;
	/* re-order the short blocks, for more efficient encoding below */
	/* By Takehiro TOMINAGA */
	/*
	  Within each scalefactor band, data is given for successive
	  time windows, beginning with window 0 and ending with window 2.
	  Within each window, the quantized values are then arranged in
	  order of increasing frequency...
	*/
	ix = &cod_info->xr[gfc->scalefac_band.l[cod_info->sfb_lmax]];
	memcpy(ixwork, cod_info->xr, 576*sizeof(FLOAT8));
	for (sfb = cod_info->sfb_smin; sfb < SBMAX_s; sfb++) {
	    int start = gfc->scalefac_band.s[sfb];
	    int end   = gfc->scalefac_band.s[sfb + 1];
	    int window, l;
	    for (window = 0; window < 3; window++) {
		for (l = start; l < end; l++) {
		    *ix++ = ixwork[3*l+window];
		}
	    }
	}
    }
    cod_info->count1bits          = 0;  
    cod_info->sfb_partition_table = nr_of_sfb_block[0][0];
    cod_info->slen[0]             = 0;
    cod_info->slen[1]             = 0;
    cod_info->slen[2]             = 0;
    cod_info->slen[3]             = 0;

    /*  fresh scalefactors are all zero
     */
    memset(&cod_info->scalefac, 0, sizeof(III_scalefac_t));
}



/************************************************************************
 *
 *      bin_search_StepSize()
 *
 *  author/date??
 *
 *  binary step size search
 *  used by outer_loop to get a quantizer step size to start with
 *
 ************************************************************************/

typedef enum {
    BINSEARCH_NONE,
    BINSEARCH_UP, 
    BINSEARCH_DOWN
} binsearchDirection_t;

inline
static int 
bin_search_StepSize(
          lame_internal_flags * const gfc,
          gr_info * const cod_info,
    const int             desired_rate, 
    const int             start, 
    const FLOAT8          xrpow [576],
          int             l3enc [576] ) 
{
    int nBits;
    int CurrentStep;
    int flag_GoneOver = 0;
    int StepSize      = start;

    binsearchDirection_t Direction = BINSEARCH_NONE;
    assert(gfc->CurrentStep);
    CurrentStep = gfc->CurrentStep;

    do {
        cod_info->global_gain = StepSize;
        nBits = count_bits(gfc,l3enc,xrpow,cod_info);  

        if (CurrentStep == 1) break; /* nothing to adjust anymore */
    
        if (flag_GoneOver) CurrentStep /= 2;
 
        if (nBits > desired_rate) {  
            /* increase Quantize_StepSize */
            if (Direction == BINSEARCH_DOWN && !flag_GoneOver) {
                flag_GoneOver = 1;
                CurrentStep  /= 2; /* late adjust */
            }
            Direction = BINSEARCH_UP;
            StepSize += CurrentStep;
            if (StepSize > 255) break;
        }
        else if (nBits < desired_rate) {
            /* decrease Quantize_StepSize */
            if (Direction == BINSEARCH_UP && !flag_GoneOver) {
                flag_GoneOver = 1;
                CurrentStep  /= 2; /* late adjust */
            }
            Direction = BINSEARCH_DOWN;
            StepSize -= CurrentStep;
            if (StepSize < 0) break;
        }
        else break; /* nBits == desired_rate;; most unlikely to happen.*/
    } while (1); /* For-ever, break is adjusted. */

    CurrentStep = start - StepSize;
    
    gfc->CurrentStep = CurrentStep/4 != 0 ? 4 : 2;

    return nBits;
}




/*************************************************************************** 
 *
 *         inner_loop ()                                                     
 *
 *  author/date??
 *
 *  The code selects the best global gain for a particular set of scalefacs 
 *
 ***************************************************************************/ 

inline
static int 
inner_loop(
          lame_internal_flags * const gfc,
          gr_info * const cod_info,
    const int             max_bits,
    const FLOAT8          xrpow [576],
          int             l3enc [576] )
{
    int bits;
    
    assert(max_bits >= 0);

    /*  scalefactors may have changed, so count bits
     */
    bits=count_bits(gfc,l3enc,xrpow,cod_info);

    /*  increase quantizer stepsize until needed bits are below maximum
     */
    while (bits > max_bits) {
        cod_info->global_gain++;
        bits = count_bits (gfc, l3enc, xrpow, cod_info);
    } 

    return bits;
}



/*************************************************************************
 *
 *      loop_break()                                               
 *
 *  author/date??
 *
 *  Function: Returns zero if there is a scalefac which has not been
 *            amplified. Otherwise it returns one. 
 *
 *************************************************************************/

inline 
static int
loop_break( 
    const gr_info        * const cod_info,
    const III_scalefac_t * const scalefac ) 
{
    int i, sfb;

    for (sfb = 0; sfb < cod_info->sfb_lmax; sfb++)
        if (scalefac->l[sfb] == 0)
            return 0;

    for (sfb = cod_info->sfb_smin; sfb < SBPSY_s; sfb++)
        for (i = 0; i < 3; i++) 
            if (scalefac->s[sfb][i] + cod_info->subblock_gain[i] == 0)
                return 0;

    return 1;
}




/*************************************************************************
 *
 *      quant_compare()                                               
 *
 *  author/date??
 *
 *  several different codes to decide which quantization is better
 *
 *************************************************************************/

inline 
static int 
quant_compare(
    const int                       experimentalX,
          lame_internal_flags * const gfc,
    const calc_noise_result * const best,
    const calc_noise_result   * const calc,
	const int                         block_type )
{
    /*
       noise is given in decibels (dB) relative to masking thesholds.

       over_noise:  ??? (the previous comment is fully wrong)
       tot_noise:   ??? (the previous comment is fully wrong)
       max_noise:   max quantization noise 

     */
    int better;

    switch (experimentalX) {
        default:
        case 0: 
	    better = calc->over_count  < best->over_count
               ||  ( calc->over_count == best->over_count  &&
                     calc->over_noise  < best->over_noise )
               ||  ( calc->over_count == best->over_count  &&
                     calc->over_noise == best->over_noise  &&
                     calc->tot_noise   < best->tot_noise  ); 
	    break;
        case 1: 
	    better = calc->max_noise < best->max_noise; 
	    break;
        case 2: 
	    better = calc->tot_noise < best->tot_noise; 
	    break;
        case 3: {
            better = (calc->tot_noise < best->tot_noise) &&
                     (calc->max_noise < best->max_noise );
            
            if (gfc->presetTune.use) {
                if ( block_type != NORM_TYPE) {
                    better = (calc->tot_noise < (best->tot_noise - gfc->presetTune.quantcomp_adjust_rh_tot)) &&
                             (calc->max_noise < (best->max_noise - gfc->presetTune.quantcomp_adjust_rh_max));
                }            
            }

            break;
        }
        case 4: 
	    better = ( calc->max_noise <= 0  &&
                       best->max_noise >  2 )
                 ||  ( calc->max_noise <= 0  &&
                       best->max_noise <  0  &&
                       best->max_noise >  calc->max_noise-2  &&
                       calc->tot_noise <  best->tot_noise )
                 ||  ( calc->max_noise <= 0  &&
                       best->max_noise >  0  &&
                       best->max_noise >  calc->max_noise-2  &&
                       calc->tot_noise <  best->tot_noise+best->over_noise )
                 ||  ( calc->max_noise >  0  &&
                       best->max_noise > -0.5  &&
                       best->max_noise >  calc->max_noise-1  &&
                       calc->tot_noise+calc->over_noise < best->tot_noise+best->over_noise )
                 ||  ( calc->max_noise >  0  &&
                       best->max_noise > -1  &&
                       best->max_noise >  calc->max_noise-1.5  &&
                       calc->tot_noise+calc->over_noise+calc->over_noise < best->tot_noise+best->over_noise+best->over_noise );
            break;
        case 5: 
	    better =   calc->over_noise  < best->over_noise
                 ||  ( calc->over_noise == best->over_noise  &&
                       calc->tot_noise   < best->tot_noise ); 
	    break;
        case 6: 
	    better =   calc->over_noise  < best->over_noise
                 ||  ( calc->over_noise == best->over_noise  &&
                     ( calc->max_noise   < best->max_noise  
		     ||  ( calc->max_noise  == best->max_noise  &&
                           calc->tot_noise  <= best->tot_noise )
		      )); 
	    break;
        case 7: 
	    better =   calc->over_count < best->over_count
                   ||  calc->over_noise < best->over_noise; 
	    break;
        case 8: 
	    better =   Max(1e-20, calc->klemm_noise)
	      < Max(1e-20, best->klemm_noise);
            break;
    }   

    return better;
}



/*************************************************************************
 *
 *          amp_scalefac_bands() 
 *
 *  author/date??
 *        
 *  Amplify the scalefactor bands that violate the masking threshold.
 *  See ISO 11172-3 Section C.1.5.4.3.5
 * 
 *  distort[] = noise/masking
 *  distort[] > 1   ==> noise is not masked
 *  distort[] < 1   ==> noise is masked
 *  max_dist = maximum value of distort[]
 *  
 *  Three algorithms:
 *  noise_shaping_amp
 *        0             Amplify all bands with distort[]>1.
 *
 *        1             Amplify all bands with distort[] >= max_dist^(.5);
 *                     ( 50% in the db scale)
 *
 *        2             Amplify first band with distort[] >= max_dist;
 *                       
 *
 *  For algorithms 0 and 1, if max_dist < 1, then amplify all bands 
 *  with distort[] >= .95*max_dist.  This is to make sure we always
 *  amplify at least one band.  
 * 
 *
 *************************************************************************/
static void 
amp_scalefac_bands(
    lame_global_flags *gfp,
    const gr_info  *const cod_info, 
    III_scalefac_t *const scalefac,
    III_psy_xmin *distort,
    FLOAT8 xrpow[576] )
{
  lame_internal_flags *gfc=gfp->internal_flags;
  int j, sfb;
  FLOAT8 ifqstep34, trigger;

  if (cod_info->scalefac_scale == 0) {
    ifqstep34 = 1.29683955465100964055; /* 2**(.75*.5)*/
  } else {
    ifqstep34 = 1.68179283050742922612;  /* 2**(.75*1) */
  }

  /* compute maximum value of distort[]  */
  trigger = 0;
  for (sfb = 0; sfb < cod_info->sfb_lmax; sfb++) {
    if (trigger < distort->l[sfb])
        trigger = distort->l[sfb];
  }
  for (sfb = cod_info->sfb_smin; sfb < SBPSY_s; sfb++) {
      int b;
      for (b = 0; b < 3; b++ ) {
	  if (trigger < distort->s[sfb][b])
	      trigger = distort->s[sfb][b];
      }
  }

  switch (gfc->noise_shaping_amp) {

  case 2:
    /* amplify exactly 1 band */
    break;

  case 1:
    /* amplify bands within 50% of max (on db scale) */
    if (trigger>1.0)
        trigger = pow(trigger, .5);
    else
      trigger *= .95;
    break;

  case 0:
  default:
    /* ISO algorithm.  amplify all bands with distort>1 */
    if (trigger>1.0)
        trigger=1.0;
    else
        trigger *= .95;
    break;
  }

  j = 0;
  for (sfb = 0; sfb < cod_info->sfb_lmax; sfb++ ) {
      int width = gfc->scalefac_band.l[sfb+1] - gfc->scalefac_band.l[sfb];
      int l;
      j += width;
      if (distort->l[sfb] < trigger)
	  continue;

      if (gfc->substep_shaping == 2) {
	  gfc->pseudohalf.l[sfb] = !gfc->pseudohalf.l[sfb];
	  if (!gfc->pseudohalf.l[sfb] && gfc->noise_shaping_amp==2)
	      return;
      }
      scalefac->l[sfb]++;
      for (l = -width; l < 0; l++)
	  xrpow[j+l] *= ifqstep34;
      if (gfc->noise_shaping_amp==2)
	  return;
  }

  for (sfb = cod_info->sfb_smin; sfb < SBPSY_s; sfb++ ) {
      int width = gfc->scalefac_band.s[sfb+1] - gfc->scalefac_band.s[sfb];
      int b;
      for (b = 0; b < 3; b++) {
	  int l;
	  j += width;
	  if (distort->s[sfb][b] < trigger)
	      continue;

	  if (gfc->substep_shaping == 2) {
	      gfc->pseudohalf.s[sfb][b] = !gfc->pseudohalf.s[sfb][b];
	      if (!gfc->pseudohalf.s[sfb][b] && gfc->noise_shaping_amp==2)
		  return;
	  }
	  scalefac->s[sfb][b]++;
	  for (l = -width; l < 0; l++) 
	      xrpow[j+l] *= ifqstep34;
	  if (gfc->noise_shaping_amp==2)
	      return;
      }
  }
}

/*************************************************************************
 *
 *      inc_scalefac_scale()
 *
 *  Takehiro Tominaga 2000-xx-xx
 *
 *  turns on scalefac scale and adjusts scalefactors
 *
 *************************************************************************/
 
static void
inc_scalefac_scale (
    const lame_internal_flags        * const gfc, 
          gr_info        * const cod_info, 
          III_scalefac_t * const scalefac,
          FLOAT8                 xrpow[576] )
{
    int l, j, sfb;
    const FLOAT8 ifqstep34 = 1.29683955465100964055;

    j = 0;
    for (sfb = 0; sfb < cod_info->sfb_lmax; sfb++) {
	int width = gfc->scalefac_band.l[sfb+1] - gfc->scalefac_band.l[sfb];
        int s = scalefac->l[sfb] + (cod_info->preflag ? pretab[sfb] : 0);
	j += width;
        if (s & 1) {
            s++;
            for (l = -width; l < 0; l++) 
                xrpow[j+l] *= ifqstep34;
        }
        scalefac->l[sfb]  = s >> 1;
    }
    cod_info->preflag = 0;

    for (sfb = cod_info->sfb_smin; sfb < SBPSY_s; sfb++) {
	int width = gfc->scalefac_band.s[sfb+1] - gfc->scalefac_band.s[sfb];
	int b;
	for (b = 0; b < 3; b++) {
	    int s = scalefac->s[sfb][b];
	    j += width;
	    if (s & 1) {
		s++;
		for (l = -width; l < 0; l++) 
		    xrpow[j+l] *= ifqstep34;
	    }
	    scalefac->s[sfb][b] = s >> 1;
	}
    }
    cod_info->scalefac_scale = 1;
}



/*************************************************************************
 *
 *      inc_subblock_gain()
 *
 *  Takehiro Tominaga 2000-xx-xx
 *
 *  increases the subblock gain and adjusts scalefactors
 *
 *************************************************************************/
 
static int 
inc_subblock_gain (
    const lame_internal_flags        * const gfc,
          gr_info        * const cod_info,
          III_scalefac_t * const scalefac,
          FLOAT8                 xrpow[576] )
{
    int window;

    for (window = 0; window < 3; window++) {
        int s1, s2, l;
        int sfb;
        s1 = s2 = 0;

        for (sfb = cod_info->sfb_smin; sfb < 6; sfb++) {
            if (s1 < scalefac->s[sfb][window])
            s1 = scalefac->s[sfb][window];
        }
        for (; sfb < SBPSY_s; sfb++) {
            if (s2 < scalefac->s[sfb][window])
            s2 = scalefac->s[sfb][window];
        }

        if (s1 < 16 && s2 < 8)
            continue;

        if (cod_info->subblock_gain[window] >= 7)
            return 1;

        /* even though there is no scalefactor for sfb12
         * subblock gain affects upper frequencies too, that's why
         * we have to go up to SBMAX_s
         */
        cod_info->subblock_gain[window]++;
        for (sfb = cod_info->sfb_smin; sfb < SBMAX_s; sfb++) {
            int i, width;
            int s = scalefac->s[sfb][window];
            FLOAT8 amp;

            if (s < 0)
                continue;
            s = s - (4 >> cod_info->scalefac_scale);
            if (s >= 0) {
                scalefac->s[sfb][window] = s;
                continue;
            }

            scalefac->s[sfb][window] = 0;
            width = gfc->scalefac_band.s[sfb] - gfc->scalefac_band.s[sfb+1];
            i = gfc->scalefac_band.s[sfb] * 3 + width * window;
            amp = IPOW20(210 + (s << (cod_info->scalefac_scale + 1)));
            for (l = 0; l < width; l++) {
                xrpow[i++] *= amp;
            }
        }
    }
    return 0;
}



/********************************************************************
 *
 *      balance_noise()
 *
 *  Takehiro Tominaga /date??
 *  Robert Hegemann 2000-09-06: made a function of it
 *
 *  amplifies scalefactor bands, 
 *   - if all are already amplified returns 0
 *   - if some bands are amplified too much:
 *      * try to increase scalefac_scale
 *      * if already scalefac_scale was set
 *          try on short blocks to increase subblock gain
 *
 ********************************************************************/
inline
static int 
balance_noise (
    lame_global_flags  *const gfp,
    gr_info        * const cod_info,
    III_scalefac_t * const scalefac, 
    III_psy_xmin           *distort,
    FLOAT8                 xrpow[576] )
{
    lame_internal_flags *const gfc = (lame_internal_flags *)gfp->internal_flags;
    int status;
    
    amp_scalefac_bands ( gfp, cod_info, scalefac, distort, xrpow);
    
    /* check to make sure we have not amplified too much 
     * loop_break returns 0 if there is an unamplified scalefac
     * scale_bitcount returns 0 if no scalefactors are too large
     */
    
    status = loop_break (cod_info, scalefac);
    
    if (status) 
        return 0; /* all bands amplified */
    
    /* not all scalefactors have been amplified.  so these 
     * scalefacs are possibly valid.  encode them: 
     */
    if (gfc->is_mpeg1)
        status = scale_bitcount (scalefac, cod_info);
    else 
        status = scale_bitcount_lsf (gfc, scalefac, cod_info);
    
    if (!status) 
        return 1; /* amplified some bands not exceeding limits */
    
    /*  some scalefactors are too large.
     *  lets try setting scalefac_scale=1 
     */
    if ((gfc->noise_shaping > 1) && (!(gfc->presetTune.use &&
                                      gfc->ATH->adjust < gfc->presetTune.athadjust_switch_level))) {
	memset(&gfc->pseudohalf, 0, sizeof(gfc->pseudohalf));
	if (!cod_info->scalefac_scale) {
	    inc_scalefac_scale (gfc, cod_info, scalefac, xrpow);
	    status = 0;
	} else {
	    if (cod_info->block_type == SHORT_TYPE ) {
		status = inc_subblock_gain (gfc, cod_info, scalefac, xrpow)
		    || loop_break (cod_info, scalefac);
	    }
	}
    }

    if (!status) {
        if (gfc->is_mpeg1 == 1) 
            status = scale_bitcount (scalefac, cod_info);
        else 
            status = scale_bitcount_lsf (gfc, scalefac, cod_info);
    }
    return !status;
}



/************************************************************************
 *
 *  outer_loop ()                                                       
 *
 *  Function: The outer iteration loop controls the masking conditions  
 *  of all scalefactorbands. It computes the best scalefac and          
 *  global gain. This module calls the inner iteration loop             
 * 
 *  mt 5/99 completely rewritten to allow for bit reservoir control,   
 *  mid/side channels with L/R or mid/side masking thresholds, 
 *  and chooses best quantization instead of last quantization when 
 *  no distortion free quantization can be found.  
 *  
 *  added VBR support mt 5/99
 *
 *  some code shuffle rh 9/00
 ************************************************************************/

static int 
outer_loop (
   lame_global_flags *gfp,
          gr_info        * const cod_info,
    const III_psy_xmin   * const l3_xmin,   /* allowed distortion of the scalefactor */
          FLOAT8                 xrpow[576], /* coloured magnitudes of spectral values */
    const int                    ch, 
    const int                    targ_bits )  /* maximum allowed bits */
{
    lame_internal_flags *gfc=gfp->internal_flags;
    III_scalefac_t save_scalefac;
    gr_info save_cod_info;
    FLOAT8 save_xrpow[576];
    III_psy_xmin   distort;
    calc_noise_result noise_info;
    calc_noise_result best_noise_info;
    int l3_enc_w[576]; 
    int iteration = 0;
    int bits_found;
    int huff_bits;
    int real_bits;
    int better;
    int over;
    III_scalefac_t * const scalefac = &cod_info->scalefac;

    int copy = 0;
    int age = 0;

    noise_info.over_count = 100;
    noise_info.max_noise  = 0;
    noise_info.tot_noise  = 0;
    noise_info.over_noise = 0;
    
    best_noise_info.over_count = 100;

    real_bits = bits_found =
      bin_search_StepSize (gfc, cod_info, targ_bits, 
			   gfc->OldValue[ch], xrpow, l3_enc_w);
    gfc->OldValue[ch] = cod_info->global_gain;

    /* BEGIN MAIN LOOP */
    do {
        iteration ++;

        /* inner_loop starts with the initial quantization step computed above
         * and slowly increases until the bits < huff_bits.
         * Thus it is important not to start with too large of an inital
         * quantization step.  Too small is ok, but inner_loop will take longer 
         */
        huff_bits = targ_bits - cod_info->part2_length;
        if (huff_bits < 0) {
            assert(iteration != 1);
            /*  scale factors too large, not enough bits. 
             *  use previous quantizaton */
            break;
        }
        /*  if this is the first iteration, 
         *  see if we can reuse the quantization computed in 
         *  bin_search_StepSize above */

        if (iteration == 1) {
            if (bits_found > huff_bits) {
                cod_info->global_gain++;
                real_bits = inner_loop (gfc, cod_info, huff_bits, xrpow, 
                                        l3_enc_w);
            }
        } else {
            real_bits = inner_loop (gfc, cod_info, huff_bits, xrpow,
                                    l3_enc_w);
        }

        cod_info->part2_3_length = real_bits;

        /* compute the distortion in this quantization */
        if (gfc->noise_shaping) 
            /* coefficients and thresholds both l/r (or both mid/side) */
            over = calc_noise (gfc, l3_enc_w, cod_info, l3_xmin, 
                               scalefac, &distort, &noise_info);
        else {
            /* fast mode, no noise shaping, we are ready */
            best_noise_info = noise_info;
            copy = 0;
            memcpy(cod_info->l3_enc, l3_enc_w, sizeof(int)*576);
            break;
        }

        /* check if this quantization is better
         * than our saved quantization */
        if (iteration == 1) /* the first iteration is always better */
            better = 1;
        else
            better = quant_compare ((gfc->presetTune.use ? gfc->presetTune.quantcomp_current
                                                         : gfp->experimentalX), 
                                     gfc, &best_noise_info, &noise_info, cod_info->block_type);
        
        /* save data so we can restore this quantization later */    
        if (better) {
            copy = 0;
            best_noise_info = noise_info;
            memcpy(cod_info->l3_enc, l3_enc_w, sizeof(int)*576);
            age = 0;
        }
        else
            age ++;


        /******************************************************************/
        /* stopping criterion */
        /******************************************************************/
        /* if no bands with distortion and -X0, we are done */
        if (0==gfc->noise_shaping_stop && 
            0==gfp->experimentalX &&
	    (over == 0 || best_noise_info.over_count == 0) )
            break;
        /* Otherwise, allow up to 3 unsuccesful tries in serial, then stop 
         * if our best quantization so far had no distorted bands. This
         * gives us more possibilities for different quant_compare modes.
         * Much more than 3 makes not a big difference, it is only slower.
         */
        if (age > 3 && best_noise_info.over_count == 0) 
            break;    
    
        /* Check if the last scalefactor band is distorted.
         * in VBR mode we can't get rid of the distortion, so quit now
         * and VBR mode will try again with more bits.  
         * (makes a 10% speed increase, the files I tested were
         * binary identical, 2000/05/20 Robert.Hegemann@gmx.de)
         * distort[] > 1 means noise > allowed noise
         */
        if (gfc->sfb21_extra) {
            if (cod_info->block_type == SHORT_TYPE) {
                if (distort.s[SBMAX_s-1][0] > 1 ||
                    distort.s[SBMAX_s-1][1] > 1 ||
                    distort.s[SBMAX_s-1][2] > 1) break;
            } else {
                if (distort.l[SBMAX_l-1] > 1) break;
            }
        }

        /* save data so we can restore this quantization later */    
        if (better) {
            copy = 1;
            save_scalefac = *scalefac;
            save_cod_info = *cod_info;
            if (gfp->VBR == vbr_rh || gfp->VBR == vbr_mtrh) {
                /* store for later reuse */
                memcpy(save_xrpow, xrpow, sizeof(FLOAT8)*576);
            }
        }
            
        if (balance_noise (gfp, cod_info, scalefac, &distort, xrpow) == 0) 
            break;
    }
    while (1); /* main iteration loop, breaks adjusted */
    
    /*  finish up
     */
    if (copy) {
        *cod_info = save_cod_info;
        *scalefac = save_scalefac;
        if (gfp->VBR == vbr_rh || gfp->VBR == vbr_mtrh)
            /* restore for reuse on next try */
            memcpy(xrpow, save_xrpow, sizeof(FLOAT8)*576);
    }
    cod_info->part2_3_length += cod_info->part2_length;
    
    assert (cod_info->global_gain < 256);
    
    return best_noise_info.over_count;
}




/************************************************************************
 *
 *      iteration_finish()                                                    
 *
 *  Robert Hegemann 2000-09-06
 *
 *  update reservoir status after FINAL quantization/bitrate 
 *
 *  rh 2000-09-06: it will not work with CBR due to the bitstream formatter
 *            you will get "Error: MAX_HEADER_BUF too small in bitstream.c"
 *
 ************************************************************************/

static void 
iteration_finish (
    lame_t gfp,
    const int       mean_bits )
{
    lame_internal_flags *gfc = gfp->internal_flags;
    III_side_info_t *l3_side = &gfc->l3_side;
    int gr, ch;
    
    for (gr = 0; gr < gfc->mode_gr; gr++) {
        for (ch = 0; ch < gfc->channels_out; ch++) {
            gr_info *cod_info = &l3_side->tt[gr][ch];

            /*  try some better scalefac storage
             */
            best_scalefac_store (gfc, gr, ch, l3_side);
            
            /*  best huffman_divide may save some bits too
             */
            if (gfc->use_best_huffman == 1 && 
                gfp->VBR != vbr_mt && gfp->VBR != vbr_mtrh)
                best_huffman_divide (gfc, cod_info);
            
            /*  update reservoir status after FINAL quantization/bitrate
             */
            ResvAdjust (gfc, cod_info, l3_side, mean_bits);
	} /* for ch */
    }    /* for gr */
    
    ResvFrameEnd (gfc, l3_side, mean_bits);
}



/*********************************************************************
 *
 *      VBR_encode_granule()
 *
 *  2000-09-04 Robert Hegemann
 *
 *********************************************************************/
 
static void
VBR_encode_granule (
          lame_global_flags *gfp,
          gr_info        * const cod_info,
    const III_psy_xmin   * const l3_xmin,     /* allowed distortion of the scalefactor */
          FLOAT8                 xrpow[576],  /* coloured magnitudes of spectral values */
    const int                    ch, 
          int                    min_bits, 
          int                    max_bits )
{
    lame_internal_flags *gfc=gfp->internal_flags;
    gr_info         bst_cod_info;
    FLOAT8          bst_xrpow [576]; 
    int Max_bits  = max_bits;
    int real_bits = max_bits+1;
    int this_bits = (max_bits+min_bits)/2;
    int dbits, over, found = 0;
    int sfb21_extra = gfc->sfb21_extra;

    assert(Max_bits <= MAX_BITS);

    /*  search within round about 40 bits of optimal
     */
    do {
        assert(this_bits >= min_bits);
        assert(this_bits <= max_bits);
        assert(min_bits <= max_bits);

        if (this_bits > Max_bits-42) 
            gfc->sfb21_extra = 0;
        else
            gfc->sfb21_extra = sfb21_extra;

        over = outer_loop ( gfp, cod_info, l3_xmin,
                            xrpow, ch, this_bits );

        /*  is quantization as good as we are looking for ?
         *  in this case: is no scalefactor band distorted?
         */
        if (over <= 0) {
            found = 1;
            /*  now we know it can be done with "real_bits"
             *  and maybe we can skip some iterations
             */
            real_bits = cod_info->part2_3_length;

            /*  store best quantization so far
             */
            bst_cod_info = *cod_info;
            memcpy(bst_xrpow, xrpow, sizeof(FLOAT8)*576);

            /*  try with fewer bits
             */
            max_bits  = real_bits-32;
            dbits     = max_bits-min_bits;
            this_bits = (max_bits+min_bits)/2;
        } 
        else {
            /*  try with more bits
             */
            min_bits  = this_bits+32;
            dbits     = max_bits-min_bits;
            this_bits = (max_bits+min_bits)/2;
            
            if (found) {
                found = 2;
                /*  start again with best quantization so far
                 */
                *cod_info = bst_cod_info;
                memcpy(xrpow, bst_xrpow, sizeof(FLOAT8)*576);
            }
        }
    } while (dbits>12);

    gfc->sfb21_extra = sfb21_extra;

    /*  found=0 => nothing found, use last one
     *  found=1 => we just found the best and left the loop
     *  found=2 => we restored a good one and have now l3_enc to restore too
     */
    if (found==2) {
        memcpy(cod_info->l3_enc, bst_cod_info.l3_enc, sizeof(int)*576);
    }
    assert(cod_info->part2_3_length <= Max_bits);

}



/************************************************************************
 *
 *      get_framebits()   
 *
 *  Robert Hegemann 2000-09-05
 *
 *  calculates
 *  * how many bits are available for analog silent granules
 *  * how many bits to use for the lowest allowed bitrate
 *  * how many bits each bitrate would provide
 *
 ************************************************************************/

static void 
get_framebits (
    lame_global_flags *gfp,
    int     * const analog_mean_bits,
    int     * const min_mean_bits,
    int             frameBits[15] )
{
    lame_internal_flags *gfc=gfp->internal_flags;
    int bitsPerFrame, mean_bits, i;
    III_side_info_t *l3_side = &gfc->l3_side;
    
    /*  always use at least this many bits per granule per channel 
     *  unless we detect analog silence, see below 
     */
    gfc->bitrate_index = gfc->VBR_min_bitrate;
    getframebits (gfp, &bitsPerFrame, &mean_bits);
    *min_mean_bits = mean_bits / gfc->channels_out;

    /*  bits for analog silence 
     */
    gfc->bitrate_index = 1;
    getframebits (gfp, &bitsPerFrame, &mean_bits);
    *analog_mean_bits = mean_bits / gfc->channels_out;

    for (i = 1; i <= gfc->VBR_max_bitrate; i++) {
        gfc->bitrate_index = i;
        getframebits (gfp, &bitsPerFrame, &mean_bits);
        frameBits[i] = ResvFrameBegin (gfp, l3_side, mean_bits, bitsPerFrame);
    }
}



/************************************************************************
 *
 *      calc_min_bits()   
 *
 *  Robert Hegemann 2000-09-04
 *
 *  determine minimal bit skeleton
 *
 ************************************************************************/
inline
static int 
calc_min_bits (
    lame_global_flags *gfp,
    const gr_info * const cod_info,
    const int             pe,
    const FLOAT8          ms_ener_ratio, 
    const int             bands,    
    const int             mch_bits,
    const int             analog_mean_bits,
    const int             min_mean_bits,
    const int             analog_silence,
    const int             ch )
{
    lame_internal_flags *gfc=gfp->internal_flags;
    int min_bits, min_pe_bits;
    
    if (gfc->nsPsy.use) return 126;
                    /*  changed minimum from 1 to 126 bits
                     *  the iteration loops require a minimum of bits
                     *  for each granule to start with; robert 2001-07-02 */

    /*  base amount of minimum bits
     */
    min_bits = Max (126, min_mean_bits);

    if (gfc->mode_ext == MPG_MD_MS_LR && ch == 1)  
        min_bits = Max (min_bits, mch_bits/5);

    /*  bit skeleton based on PE
     */
    if (cod_info->block_type == SHORT_TYPE) 
        /*  if LAME switches to short blocks then pe is
         *  >= 1000 on medium surge
         *  >= 3000 on big surge
         */
        min_pe_bits = (pe-350) * bands/39;
    else 
        min_pe_bits = (pe-350) * bands/22;
    
    if (gfc->mode_ext == MPG_MD_MS_LR && ch == 1) {
        /*  side channel will use a lower bit skeleton based on PE
         */ 
        FLOAT8 fac  = .33 * (.5 - ms_ener_ratio) / .5;
        min_pe_bits = (int)(min_pe_bits * ((1-fac)/(1+fac)));
    }
    min_pe_bits = Min (min_pe_bits, (1820 * gfp->out_samplerate / 44100));

    /*  determine final minimum bits
     */
    if (analog_silence && !gfp->VBR_hard_min) 
        min_bits = analog_mean_bits;
    else 
        min_bits = Max (min_bits, min_pe_bits);
    
    return min_bits;
}



/*********************************************************************
 *
 *      VBR_prepare()
 *
 *  2000-09-04 Robert Hegemann
 *
 *  * converts LR to MS coding when necessary 
 *  * calculates allowed/adjusted quantization noise amounts
 *  * detects analog silent frames
 *
 *  some remarks:
 *  - lower masking depending on Quality setting
 *  - quality control together with adjusted ATH MDCT scaling
 *    on lower quality setting allocate more noise from
 *    ATH masking, and on higher quality setting allocate
 *    less noise from ATH masking.
 *  - experiments show that going more than 2dB over GPSYCHO's
 *    limits ends up in very annoying artefacts
 *
 *********************************************************************/

/* RH: this one needs to be overhauled sometime */
 
static int 
VBR_prepare (
          lame_global_flags *gfp,
          FLOAT8          pe            [2][2],
          FLOAT8          ms_ener_ratio [2], 
          III_psy_ratio   ratio         [2][2], 
          III_psy_xmin    l3_xmin       [2][2],
          int             frameBits     [16],
          int            *analog_mean_bits,
          int            *min_mean_bits,
          int             min_bits      [2][2],
          int             max_bits      [2][2],
          int             bands         [2][2] )
{
    lame_internal_flags *gfc=gfp->internal_flags;
    
    
    FLOAT8  masking_lower_db, adjust = 0.0;
    int     gr, ch;
    int     analog_silence = 1;
    int     bpf, avg, mxb, bits = 0;
  
    gfc->bitrate_index = gfc->VBR_max_bitrate;
    getframebits (gfp, &bpf, &avg);
    avg = ResvFrameBegin (gfp, &gfc->l3_side, avg, bpf ) / gfc->mode_gr;
    
    get_framebits (gfp, analog_mean_bits, min_mean_bits, frameBits);
    
    for (gr = 0; gr < gfc->mode_gr; gr++) {
        mxb = on_pe (gfp, pe, &gfc->l3_side, max_bits[gr], avg, gr);
        if (gfc->mode_ext == MPG_MD_MS_LR) {
            ms_convert (&gfc->l3_side, gr);
            reduce_side (max_bits[gr], ms_ener_ratio[gr], avg, mxb);
        }
        for (ch = 0; ch < gfc->channels_out; ++ch) {
            gr_info *cod_info = &gfc->l3_side.tt[gr][ch];
      
            if (gfc->nsPsy.use && gfp->VBR == vbr_rh) {
            if (cod_info->block_type == NORM_TYPE) 
                adjust = 1.28/(1+exp(3.5-pe[gr][ch]/300.))-0.05;
            else 
                adjust = 2.56/(1+exp(3.5-pe[gr][ch]/300.))-0.14;
            }
            masking_lower_db   = gfc->VBR->mask_adjust - adjust; 
            gfc->masking_lower = pow (10.0, masking_lower_db * 0.1);
      
            init_outer_loop(gfc, cod_info);
	    bands[gr][ch] = calc_xmin (gfp, &ratio[gr][ch], 
                                       cod_info, l3_xmin[gr]+ch);
            if (bands[gr][ch]) 
                analog_silence = 0;

            min_bits[gr][ch] = calc_min_bits (gfp, cod_info, (int)pe[gr][ch],
                                      ms_ener_ratio[gr], bands[gr][ch],
                                      0, *analog_mean_bits, 
                                      *min_mean_bits, analog_silence, ch);
      
            bits += max_bits[gr][ch];
        }
    }
    for (gr = 0; gr < gfc->mode_gr; gr++) {
        for (ch = 0; ch < gfc->channels_out; ch++) {            
            if (bits > frameBits[gfc->VBR_max_bitrate]) {
                max_bits[gr][ch] *= frameBits[gfc->VBR_max_bitrate];
                max_bits[gr][ch] /= bits;
            }
            if (min_bits[gr][ch] > max_bits[gr][ch]) 
                min_bits[gr][ch] = max_bits[gr][ch];
            
        } /* for ch */
    }  /* for gr */
    
    *min_mean_bits = Max(*min_mean_bits, 126);

    return analog_silence;
}
 
 
static void 
bitpressure_strategy(
    lame_internal_flags * gfc,
    III_psy_xmin l3_xmin[2][2],
    int min_bits[2][2],  
    int max_bits[2][2] )  
{
    int gr, ch, sfb;
    for (gr = 0; gr < gfc->mode_gr; gr++) {
        for (ch = 0; ch < gfc->channels_out; ch++) {
            if (gfc->l3_side.tt[gr][ch].block_type == SHORT_TYPE) {
                for (sfb = 0; sfb < SBMAX_s; sfb++) {
                    l3_xmin[gr][ch].s[sfb][0] *= 1.+.029*sfb*sfb/SBMAX_s/SBMAX_s;
                    l3_xmin[gr][ch].s[sfb][1] *= 1.+.029*sfb*sfb/SBMAX_s/SBMAX_s;
                    l3_xmin[gr][ch].s[sfb][2] *= 1.+.029*sfb*sfb/SBMAX_s/SBMAX_s;
                }
            }
            else {
                for (sfb = 0; sfb < SBMAX_l; sfb++) 
                    l3_xmin[gr][ch].l[sfb] *= 1.+.029*sfb*sfb/SBMAX_l/SBMAX_l;
            }
            max_bits[gr][ch] = Max(min_bits[gr][ch], 0.9*max_bits[gr][ch]);
        }
    }
}

/************************************************************************
 *
 *      VBR_iteration_loop()   
 *
 *  tries to find out how many bits are needed for each granule and channel
 *  to get an acceptable quantization. An appropriate bitrate will then be
 *  choosed for quantization.  rh 8/99                          
 *
 *  Robert Hegemann 2000-09-06 rewrite
 *
 ************************************************************************/

void 
VBR_iteration_loop (
    lame_global_flags *gfp,
    FLOAT8             pe           [2][2],
    FLOAT8             ms_ener_ratio[2],
    III_psy_ratio ratio[2][2])
{
    lame_internal_flags *gfc=gfp->internal_flags;
    III_psy_xmin l3_xmin[2][2];
  
    FLOAT8    xrpow[576];
    int       bands[2][2];
    int       frameBits[15];
    int       bitsPerFrame;
    int       save_bits[2][2];
    int       used_bits, used_bits2;
    int       bits;
    int       min_bits[2][2], max_bits[2][2];
    int       analog_mean_bits, min_mean_bits;
    int       mean_bits;
    int       ch, gr, analog_silence;
    gr_info             *cod_info;
    III_side_info_t     *l3_side  = &gfc->l3_side;

    analog_silence = VBR_prepare (gfp, pe, ms_ener_ratio, ratio, 
                                  l3_xmin, frameBits, &analog_mean_bits,
                                  &min_mean_bits, min_bits, max_bits, bands);

    /*---------------------------------*/
    for(;;) {  
    
    /*  quantize granules with lowest possible number of bits
     */
    
    used_bits = 0;
    used_bits2 = 0;
   
    for (gr = 0; gr < gfc->mode_gr; gr++) {
        for (ch = 0; ch < gfc->channels_out; ch++) {
            int ret; 
            cod_info = &l3_side->tt[gr][ch];
      
            /*  init_outer_loop sets up cod_info, scalefac and xrpow 
             */
            ret = init_xrpow(gfc, cod_info, xrpow);
            if (ret == 0 || max_bits[gr][ch] == 0) {
                /*  xr contains no energy 
                 *  l3_enc, our encoding data, will be quantized to zero
                 */
                save_bits[gr][ch] = 0;
                continue; /* with next channel */
            }
      
            if (gfp->VBR == vbr_mtrh) {
                ret = VBR_noise_shaping (gfc, xrpow,
					 min_bits[gr][ch], max_bits[gr][ch], 
					 &l3_xmin[gr][ch], gr, ch );
                if (ret < 0)
                    cod_info->part2_3_length = 100000;
            } 
            else
	        VBR_encode_granule (gfp, cod_info, &l3_xmin[gr][ch], xrpow,
                                    ch, min_bits[gr][ch], max_bits[gr][ch] );

            used_bits += cod_info->part2_3_length;
            save_bits[gr][ch] = Min(MAX_BITS, cod_info->part2_3_length);
            used_bits2 += Min(MAX_BITS, cod_info->part2_3_length);
        } /* for ch */
    }    /* for gr */

    /*  find lowest bitrate able to hold used bits
     */
    if (analog_silence && !gfp->VBR_hard_min) 
        /*  we detected analog silence and the user did not specify 
         *  any hard framesize limit, so start with smallest possible frame
         */
        gfc->bitrate_index = 1;
    else
        gfc->bitrate_index = gfc->VBR_min_bitrate;
     
    for( ; gfc->bitrate_index < gfc->VBR_max_bitrate; gfc->bitrate_index++) {
        if (used_bits <= frameBits[gfc->bitrate_index]) break; 
    }
    getframebits (gfp, &bitsPerFrame, &mean_bits);
    bits = ResvFrameBegin (gfp, l3_side, mean_bits, bitsPerFrame);
    
    if (used_bits <= bits) break;

    bitpressure_strategy( gfc, l3_xmin, min_bits, max_bits );

    }   /* breaks adjusted */
    /*--------------------------------------*/

    iteration_finish (gfp, mean_bits);
}






/********************************************************************
 *
 *  calc_target_bits()
 *
 *  calculates target bits for ABR encoding
 *
 *  mt 2000/05/31
 *
 ********************************************************************/

static void 
calc_target_bits (
    lame_global_flags * gfp,
    FLOAT8               pe            [2][2],
    FLOAT8               ms_ener_ratio [2],
    int                  targ_bits     [2][2],
    int                 *analog_silence_bits,
    int                 *max_frame_bits )
{
    lame_internal_flags *gfc=gfp->internal_flags;
    III_side_info_t *l3_side = &gfc->l3_side;
    FLOAT8 res_factor;
    int gr, ch, totbits, mean_bits, bitsPerFrame;
    
    gfc->bitrate_index = gfc->VBR_max_bitrate;
    getframebits (gfp, &bitsPerFrame, &mean_bits);
    *max_frame_bits = ResvFrameBegin (gfp, l3_side, mean_bits, bitsPerFrame);

    gfc->bitrate_index = 1;
    getframebits (gfp, &bitsPerFrame, &mean_bits);
    *analog_silence_bits = mean_bits / gfc->channels_out;

    mean_bits  = gfp->VBR_mean_bitrate_kbps * gfp->framesize * 1000;
    mean_bits /= gfp->out_samplerate;
    mean_bits -= gfc->sideinfo_len*8;
    mean_bits /= (gfc->mode_gr*gfc->channels_out);

    /*
        res_factor is the percentage of the target bitrate that should
        be used on average.  the remaining bits are added to the
        bitreservoir and used for difficult to encode frames.  

        Since we are tracking the average bitrate, we should adjust
        res_factor "on the fly", increasing it if the average bitrate
        is greater than the requested bitrate, and decreasing it
        otherwise.  Reasonable ranges are from .9 to 1.0
        
        Until we get the above suggestion working, we use the following
        tuning:
        compression ratio    res_factor
          5.5  (256kbps)         1.0      no need for bitreservoir 
          11   (128kbps)         .93      7% held for reservoir
   
        with linear interpolation for other values.

     */
    res_factor = .93 + .07 * (11.0 - gfp->compression_ratio) / (11.0 - 5.5);
    if (res_factor <  .90)
        res_factor =  .90; 
    if (res_factor > 1.00) 
        res_factor = 1.00;

    for (gr = 0; gr < gfc->mode_gr; gr++) {
        for (ch = 0; ch < gfc->channels_out; ch++) {
            targ_bits[gr][ch] = res_factor * mean_bits;
            
            if (pe[gr][ch] > 700) {
                int add_bits = (pe[gr][ch] - 700) / 1.4;
  
                gr_info *cod_info = &l3_side->tt[gr][ch];
                targ_bits[gr][ch] = res_factor * mean_bits;
 
                /* short blocks use a little extra, no matter what the pe */
                if (cod_info->block_type == SHORT_TYPE) {
                    if (add_bits < mean_bits/2)
                        add_bits = mean_bits/2;
                }
                /* at most increase bits by 1.5*average */
                if (add_bits > mean_bits*3/2)
                    add_bits = mean_bits*3/2;
                else
                if (add_bits < 0) 
                    add_bits = 0;

                targ_bits[gr][ch] += add_bits;
            }
        }/* for ch */
    }   /* for gr */
    
    if (gfc->mode_ext == MPG_MD_MS_LR) 
        for (gr = 0; gr < gfc->mode_gr; gr++) {
            reduce_side (targ_bits[gr], ms_ener_ratio[gr], mean_bits*gfc->channels_out,
			 MAX_BITS);
        }

    /*  sum target bits
     */
    totbits=0;
    for (gr = 0; gr < gfc->mode_gr; gr++) {
        for (ch = 0; ch < gfc->channels_out; ch++) {
            if (targ_bits[gr][ch] > MAX_BITS) 
                targ_bits[gr][ch] = MAX_BITS;
            totbits += targ_bits[gr][ch];
        }
    }

    /*  repartion target bits if needed
     */
    if (totbits > *max_frame_bits) {
        for(gr = 0; gr < gfc->mode_gr; gr++) {
            for(ch = 0; ch < gfc->channels_out; ch++) {
                targ_bits[gr][ch] *= *max_frame_bits; 
                targ_bits[gr][ch] /= totbits; 
            }
        }
    }
}






/********************************************************************
 *
 *  ABR_iteration_loop()
 *
 *  encode a frame with a disired average bitrate
 *
 *  mt 2000/05/31
 *
 ********************************************************************/

void 
ABR_iteration_loop(
    lame_global_flags *gfp,
    FLOAT8             pe           [2][2],
    FLOAT8             ms_ener_ratio[2], 
    III_psy_ratio      ratio        [2][2])
{
    lame_internal_flags *gfc=gfp->internal_flags;
    III_psy_xmin l3_xmin;
    FLOAT8    xrpow[576];
    int       targ_bits[2][2];
    int       bitsPerFrame, mean_bits, totbits, max_frame_bits;
    int       ch, gr, ath_over;
    int       analog_silence_bits;
    gr_info             *cod_info;
    III_side_info_t     *l3_side  = &gfc->l3_side;

    calc_target_bits (gfp, pe, ms_ener_ratio, targ_bits, 
                      &analog_silence_bits, &max_frame_bits);
    
    /*  encode granules
     */
    totbits=0;
    for (gr = 0; gr < gfc->mode_gr; gr++) {

        if (gfc->mode_ext == MPG_MD_MS_LR) 
            ms_convert (&gfc->l3_side, gr);

        for (ch = 0; ch < gfc->channels_out; ch++) {
            cod_info = &l3_side->tt[gr][ch];

            /*  cod_info, scalefac and xrpow get initialized in init_outer_loop
             */
            init_outer_loop(gfc, cod_info);
            if (init_xrpow(gfc, cod_info, xrpow)) {
                /*  xr contains energy we will have to encode 
                 *  calculate the masking abilities
                 *  find some good quantization in outer_loop 
                 */
                ath_over = calc_xmin (gfp, &ratio[gr][ch], cod_info, &l3_xmin);
                if (0 == ath_over) /* analog silence */
                    targ_bits[gr][ch] = analog_silence_bits;

                outer_loop (gfp, cod_info, &l3_xmin,
                            xrpow, ch, targ_bits[gr][ch]);
            }

            totbits += cod_info->part2_3_length;
        } /* ch */
    }  /* gr */
  
    /*  find a bitrate which can handle totbits 
     */
    for (gfc->bitrate_index =  gfc->VBR_min_bitrate ;
         gfc->bitrate_index <= gfc->VBR_max_bitrate;
         gfc->bitrate_index++    ) {
        getframebits (gfp, &bitsPerFrame, &mean_bits);
        max_frame_bits = ResvFrameBegin (gfp, l3_side, mean_bits, bitsPerFrame);
        if (totbits <= max_frame_bits) break; 
    }
    assert (gfc->bitrate_index <= gfc->VBR_max_bitrate);

    iteration_finish (gfp, mean_bits);
}






/************************************************************************
 *
 *      iteration_loop()                                                    
 *
 *  author/date??
 *
 *  encodes one frame of MP3 data with constant bitrate
 *
 ************************************************************************/

void 
iteration_loop(
    lame_global_flags *gfp, 
    FLOAT8             pe           [2][2],
    FLOAT8             ms_ener_ratio[2],  
    III_psy_ratio      ratio        [2][2])
{
    lame_internal_flags *gfc=gfp->internal_flags;
    III_psy_xmin l3_xmin;
    FLOAT8 xrpow[576];
    int    targ_bits[2];
    int    bitsPerFrame;
    int    mean_bits, max_bits;
    int    gr, ch;
    III_side_info_t     *l3_side = &gfc->l3_side;
    gr_info             *cod_info;

    getframebits (gfp, &bitsPerFrame, &mean_bits);
    ResvFrameBegin (gfp, l3_side, mean_bits, bitsPerFrame );

    /* quantize! */
    for (gr = 0; gr < gfc->mode_gr; gr++) {

        /*  calculate needed bits
         */
        max_bits = on_pe (gfp, pe, l3_side, targ_bits, mean_bits, gr);
        
        if (gfc->mode_ext == MPG_MD_MS_LR) {
            ms_convert (&gfc->l3_side, gr);
            reduce_side (targ_bits, ms_ener_ratio[gr], mean_bits, max_bits);
        }
        
        for (ch=0 ; ch < gfc->channels_out ; ch ++) {
            cod_info = &l3_side->tt[gr][ch]; 

            /*  init_outer_loop sets up cod_info, scalefac and xrpow 
             */
            init_outer_loop(gfc, cod_info);
            if (init_xrpow(gfc, cod_info, xrpow)) {
                /*  xr contains energy we will have to encode 
                 *  calculate the masking abilities
                 *  find some good quantization in outer_loop 
                 */
                calc_xmin (gfp, &ratio[gr][ch], cod_info, &l3_xmin);
                outer_loop (gfp, cod_info, &l3_xmin, xrpow, ch, targ_bits[ch]);
            }
            assert (cod_info->part2_3_length <= MAX_BITS);

            /*  try some better scalefac storage
             */
            best_scalefac_store (gfc, gr, ch, l3_side);
            
            /*  best huffman_divide may save some bits too
             */
            if (gfc->use_best_huffman == 1) 
                best_huffman_divide (gfc, cod_info);
            
            /*  update reservoir status after FINAL quantization/bitrate
             */
#undef  NORES_TEST
#ifndef NORES_TEST
            ResvAdjust (gfc, cod_info, l3_side, mean_bits);
#endif      
        } /* for ch */
    }    /* for gr */
    
#ifdef NORES_TEST
    /* replace ResvAdjust above with this code if you do not want
       the second granule to use bits saved by the first granule.
       Requires using the --nores.  This is useful for testing only */
    for (gr = 0; gr < gfc->mode_gr; gr++) {
        for (ch =  0; ch < gfc->channels_out; ch++) {
            cod_info = &l3_side->tt[gr][ch];
            ResvAdjust (gfc, cod_info, l3_side, mean_bits);
        }
    }
#endif

    ResvFrameEnd (gfc, l3_side, mean_bits);
}



