//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  MPDEC.CPP: ASI API module for MPEG audio                              ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 8-Apr-98: Initial                                     ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "mss.h"
#include "mp3dec.h"

#include "x_files.hpp"

#define MPDEC
#define DCT_WINDOW
#include "datatbl.h"

#ifdef __WATCOMC__
#pragma warning 7 9
#pragma warning 14 9
#endif

#ifdef IS_WIN16
#pragma warning (disable:4136)
#endif

#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

//############################################################################

//
// Integer frequency coefficients
//
// Note extra 2 words at end to accommodate last half of final quadword 
// in count1 partition
//

S16 ASISTREAM::m_is     [2][578];
U8  ASISTREAM::m_is_sign[2][578];

S32 ASISTREAM::zero_part[2]; 
S32 ASISTREAM::zero_count[2];

//
// Real frequency-coefficient workspace
//

F32 ASISTREAM::m_xr[2][32][18];
F32 ASISTREAM::m_lr[2][32][18];

F32 ASISTREAM::m_sample[2][32][36];

//
// Persistent storage for IMDCT -- must be cleared to zero at seek time
//

//F32 ASISTREAM::m_s[2][32][18];

//
// Input to poly() for a single channel
//
// This also serves as the output array for the IMDCT prefilter stage in
// Layer 3
//

F32 ASISTREAM::m_res[32][18];

//
// Persistent storage for POLY
//

//F32 ASISTREAM::m_u      [2][2][17][16];
//S32 ASISTREAM::m_u_start[2];
//S32 ASISTREAM::m_u_div  [2];

//############################################################################

//
// Table of powers of 4/3, used for decimal-to-float conversion as well
//

F32* power_fn;

//
// Table of 2 to the nth power, where n is successive multiples of -0.5
//

F32 mp3_lfac[256];

//
// Table of 2 to the nth power, where n is successive multiples of -2.0
//

F32 mp3_sfac[256];

#ifdef IS_X86

//
// TRUE to use 3DNow! extensions in math kernel
//

S32 use_AMD = 0;

#endif

//############################################################################
//#                                                                          #
//# Init layer-3 specific data                                               #
//#                                                                          #
//############################################################################

S32 L3_init(void)
{
   //
   // Initialize math engine
   //

   S32 i,j;

   for (i=0; i < 17; i++)
      {
      for (j=0; j < 32; j++)
         {
         mp3_t_dewindow[i][j] *= 16383.5F;
         }
      }

#ifdef IS_X86

   //
   // Detect AMD 3DNow extensions
   //

   use_AMD = 0; //AIL_MMX_available() & 0x0002;

#endif

   //
   // Synthesize IDCT window 0
   //

   for (i=0; i < 36; i++)
      {
      mp3_win[0][i] = (F32) sin(PI36 * (i + 0.5F));
      }

   //
   // Synthesize IDCT window 1
   //

   for (i=0; i < 18; i++)
      {
      mp3_win[1][i] = (F32) sin(PI36 * (i + 0.5F));
      }

   for (i=18; i < 24; i++)
      {
      mp3_win[1][i] = 1.0F;
      }

   for (i=24; i < 30; i++)
      {   
      mp3_win[1][i] = (F32) sin(PI12 * (i + 0.5F - 18.0F));
      }

   for (i=30; i < 36; i++)
      {
      mp3_win[1][i] = 0.0F;
      }

   //
   // Synthesize IDCT window 3
   //

   for (i=0; i < 6; i++)
      {
      mp3_win[3][i] = 0.0F;
      }

   for (i=6; i < 12; i++)
      {
      mp3_win[3][i] = (F32) sin(PI12 * (i + 0.5F - 6.0F));
      }

   for (i=12; i < 18; i++)
      {
      mp3_win[3][i] = 1.0f;
      }

   for (i=18; i < 36; i++)
      {
      mp3_win[3][i] = (F32) sin(PI36 * (i + 0.5F));
      }

   //
   // Build tables for fast quantization
   //

   power_fn = (F32*)x_malloc( 8192 * sizeof(F32) );
   
   if (power_fn == NULL)
      {
      return 0;
      }

   for (i=0; i < 8192; i++)
      {
      power_fn[i] = F32(pow(F32(i), 4.0F / 3.0F));
      }

   for (i=0; i < 256; i++)
      {
      mp3_lfac[i] = pow(2.0F,-0.5F * F32(i));
      mp3_sfac[i] = pow(2.0F,-2.0F * F32(i));
      }

   //
   // Return success
   //

   return 1;
}

//############################################################################
//#                                                                          #
//# Destroy layer-3 specific data                                            #
//#                                                                          #
//############################################################################

S32 L3_destroy(void)
{
   //
   // Free other vars
   //
   if (power_fn != NULL)
      {
      x_free( power_fn );
      }

   //
   // Return success
   //

   return 1;
}

//############################################################################
//#                                                                          #
//# Perform layer 3 scaling and requantization                               #
//#                                                                          #
//############################################################################

static S32 L3_requantize(ASISTREAM* STR, S32 gr, S32 ch)
{
   //
   // Choose correct scalefactor band per block type, and initialize boundary
   //

   S32 sfreq = STR->sampling_frequency + (STR->MPEG1 * 3);

   S32 next_cb_boundary=0, cb_begin=0, cb_width=0;


   if (STR->window_switching_flag[gr][ch] && (STR->block_type[gr][ch] == 2))
      {
      if (STR->mixed_block_flag[gr][ch]) 
         {
         //
         // Long or mixed blocks (0,1,3)
         //

         next_cb_boundary = mp3_sfBandIndex[sfreq].l[1];
         }
      else 
         {
         //
         // Pure short block
         //

         next_cb_boundary = mp3_sfBandIndex[sfreq].s[1] * 3;

         cb_width = mp3_sfBandIndex[sfreq].s[1];
         cb_begin = 0;
         }
      }  
   else 
      {
      //
      // Pure long block
      //

      next_cb_boundary = mp3_sfBandIndex[sfreq].l[1];
      }

   //
   // Build quantization tables for this granule and channel
   //

   const S32 pretab[22] = {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,3,2,0};

   F32 long_scalar[22];
   F32 short_scalar[3][13];

   F32 common = F32(pow(2.0, (0.25 * (STR->global_gain[gr][ch] - 210.0))));

   S32 cb,group;

   S32 sf_scale = STR->scalefac_scale[gr][ch];

   for (cb=0; cb < 22; cb++)
      {
      S32 n = (1 + sf_scale) * (STR->scalefac[gr][ch].l[cb] + STR->preflag[gr][ch] * pretab[cb]);

      long_scalar[cb] = common * mp3_lfac[n];
      }

   for (group=0; group < 3; group++)
      {
      for (cb=0; cb < 13; cb++)
         {
         S32 m = STR->subblock_gain[gr][ch][group];
         S32 n = (1 + sf_scale) * STR->scalefac[gr][ch].s[group][cb];

         short_scalar[group][cb] = common * mp3_sfac[m] * mp3_lfac[n];
         }
      }

   //
   // Apply requantization and scaling formula based on block type
   //

   cb = 0;

   S32 line = 0;
   S32 end  = STR->zero_part[ch];

   F32* xr  = &STR->m_xr[ch][0][0];
   S16* is  = &STR->m_is[ch][0];
   U8*  sgn = &STR->m_is_sign[ch][0];

   S32 pure_short_block = ((STR->block_type[gr][ch] == 2) && 
                          (STR->mixed_block_flag[gr][ch] == 0));

   S32 mixed_block = ((STR->block_type[gr][ch] == 2) && 
                      (STR->mixed_block_flag[gr][ch]));

   S32 window_switch = STR->window_switching_flag[gr][ch];

   while (line < end)
      {
      if (next_cb_boundary > end)
         {
         next_cb_boundary = end;
         }

      S32 group=0;
      S32 group_end=0;

      if (window_switch && (pure_short_block || (mixed_block && (line >= 36))))
         {
         group = (line - cb_begin) / cb_width;
         group_end = cb_begin + cb_width;
         }

      while (line < next_cb_boundary)
         {
         //
         // Do long/short dependent scaling operations
         //
      
         if (window_switch && (pure_short_block || (mixed_block && (line >= 36))))
            {
            if (line >= group_end)
               {
               group_end += cb_width;
               ++group;
               }

            F32 *scalar = &short_scalar[group][0];

            if (*sgn++)
               {
               *xr++ = - (power_fn[*is++] * scalar[cb]);
               }
            else
               {
               *xr++ = power_fn[*is++] * scalar[cb];
               }
            }
         else 
            {
            //
            // LONG block types 0,1,3 & 1st 2 subbands of switched blocks
            //

            if (*sgn++)
               {
               *xr++ = - (power_fn[*is++] * long_scalar[cb]);
               }
            else
               {
               *xr++ = power_fn[*is++] * long_scalar[cb];
               }
            }

         //
         // End processing this line
         //

         ++line;
         }

      //
      // Adjust critical band boundary
      //

      if (STR->window_switching_flag[gr][ch] && 
         (STR->block_type           [gr][ch] == 2))
         {
         //
         // Short/mixed blocks
         //

         if (STR->mixed_block_flag[gr][ch])
            {
            if (line == mp3_sfBandIndex[sfreq].l[8])
               {
               next_cb_boundary = mp3_sfBandIndex[sfreq].s[4]*3; 

               cb = 3;

               cb_width = mp3_sfBandIndex[sfreq].s[cb+1] - 
                          mp3_sfBandIndex[sfreq].s[cb];
               cb_begin = mp3_sfBandIndex[sfreq].s[cb]*3;      
               }
            else
               {
               if (line < mp3_sfBandIndex[sfreq].l[8]) 
                  {
                  next_cb_boundary = mp3_sfBandIndex[sfreq].l[(++cb)+1];
                  }
               else
                  {
                  next_cb_boundary = mp3_sfBandIndex[sfreq].s[(++cb)+1]*3;

                  cb_width = mp3_sfBandIndex[sfreq].s[cb+1] - 
                             mp3_sfBandIndex[sfreq].s[cb];
                  cb_begin = mp3_sfBandIndex[sfreq].s[cb]*3;      
                  }
               }
            }
         else
            {
            next_cb_boundary = mp3_sfBandIndex[sfreq].s[(++cb)+1]*3;

            cb_width = mp3_sfBandIndex[sfreq].s[cb+1] - 
                       mp3_sfBandIndex[sfreq].s[cb];
            cb_begin = mp3_sfBandIndex[sfreq].s[cb]*3;
            } 
         }
      else
         {
         //
         // Long blocks
         //

         next_cb_boundary = mp3_sfBandIndex[sfreq].l[(++cb)+1];
         }
      }

   //
   // Flush zero part
   //

   if (line < 576)
      {
      //
      // Zero part entered -- flush and return success
      //

      x_memset(xr,
                 0,
                 STR->zero_count[ch] * sizeof(F32));

      return 1;
      }

   //
   // Return success
   //

   return 1;
}

//############################################################################
//#                                                                          #
//# Decode layer 3 scalefactors in accordance with MPEG1 specifications      #
//#                                                                          #
//############################################################################

static S32 L3_sf_MP1(ASISTREAM* STR, S32 gr, S32 ch)
{
   S32 i,sfb,window;

   if ((STR->window_switching_flag[gr][ch]) &&
       (STR->block_type           [gr][ch] == 2)) 
      { 
      //
      // Short or mixed block
      //

      if (STR->mixed_block_flag[gr][ch]) 
         {
         for (sfb = 0; sfb < 8; sfb++)
            {
            STR->scalefac[gr][ch].l[sfb] = A(mp3_slen[0][STR->scalefac_compress[gr][ch]]);
            }

         for (sfb = 3; sfb < 6; sfb++)
            {
            for (window=0; window < 3; window++)
               {
               STR->scalefac[gr][ch].s[window][sfb] = A(mp3_slen[0][STR->scalefac_compress[gr][ch]]);
               }
            }

         for (sfb = 6; sfb < 12; sfb++)
            {
            for (window=0; window < 3; window++)
               {
               STR->scalefac[gr][ch].s[window][sfb] = A(mp3_slen[1][STR->scalefac_compress[gr][ch]]);
               }
            }
         }
      else 
         {
         for (i=0; i < 2; i++) 
            {
            for (sfb = mp3_sfbtable.s[i]; sfb < mp3_sfbtable.s[i+1]; sfb++)
               {
               for (window=0; window < 3; window++)
                  {
                  STR->scalefac[gr][ch].s[window][sfb] = A(mp3_slen[i][STR->scalefac_compress[gr][ch]]);
                  }
               }
            }
         }

      for (window=0; window < 3; window++)
         {
         STR->scalefac[gr][ch].s[window][12] = 0;
         }
      }          
   else 
      {
      //
      // Long block only
      //
      // For each scalefactor band partition, scfsi[ch][partition] 
      // determines whether scale factor information for granule 1 
      // appears explicitly, or is copied from granule 0
      //

      for (i=0; i < 4; i++) 
         {
         if ((STR->scfsi[ch][i] == 0) || (gr == 0))
            {
            for (sfb = mp3_sfbtable.l[i]; sfb < mp3_sfbtable.l[i+1]; sfb++)
               {
               STR->scalefac[gr][ch].l[sfb] = A(mp3_slen[(i < 2) ? 0 : 1][STR->scalefac_compress[gr][ch]]);
               }
            }
         else
            {
            for (sfb = mp3_sfbtable.l[i]; sfb < mp3_sfbtable.l[i+1]; sfb++)
               {
               STR->scalefac[1][ch].l[sfb] = STR->scalefac[0][ch].l[sfb];
               }
            }
         }

      STR->scalefac[gr][ch].l[22] = 0; 
      }

   //
   // Return success
   //

   return 1;
}

//############################################################################
//#                                                                          #
//# Decode layer 3 scalefactors in accordance with MPEG2 specifications      #
//#                                                                          #
//############################################################################

static S32 L3_sf_MP2(ASISTREAM* STR, S32 gr, S32 ch)
{
//   S32 start = STR->apos;
   
   gr=gr;

   const S32 mp3_sfb_cnts[2][3][3][4]=
      {
      {{{6,5,5,5},   {9,9,9,9},    {6,9,9,9} },
      {{6,5,7,3},    {9,9,12,6},   {6,9,12,6}},
      {{11,10,0,0},  {18,18,0,0},  {15,18,0,0}}},
      {{{7,7,7,0},   {12,12,12,0}, {6,15,12,0}},
      {{6,6,6,3},    {12,9,9,6},   {6,12,9,6}},
      {{8,8,5,0},    {15,12,9,0},  {6,18,9,0}}}
      };

   S32 slen[4];

   S32      c =        STR->scalefac_compress     [0][ch];
   S32*     p = (S32*)&STR->preflag               [0][ch];
   S32      m =        STR->mixed_block_flag      [0][ch];
   S32      b =        STR->block_type            [0][ch];
   S32      w =        STR->window_switching_flag [0][ch];

   S32 group=0,set=0;
   S32 j,k;

   if (!(((STR->mode_extension == 0x01) || (STR->mode_extension == 0x03)) 
       && 
      (ch == 1)))
      {
      //
      // Either this is the left or mono channel, or this is not an
      // intensity stereo stream
      //

      group = 0;

      if (c < 400) 
         {
         slen[0] = (c >> 4) / 5;
         slen[1] = (c >> 4) % 5;
         slen[2] = (c % 16) >> 2;
         slen[3] = (c % 4);

         *p = 0;
         set = 0;
         }
      else if ((400 <= c) && (c < 500))
         {
         slen[0] = ((c-400) >> 2) / 5;
         slen[1] = ((c-400) >> 2) % 5;
         slen[2] = (c-400) % 4;
         slen[3] = 0;

         *p = 0;
         set = 1;
         }
      else if ((500 <= c) && (c < 512))
         {
         slen[0] = (c-500) / 3;
         slen[1] = (c-500) % 3;
         slen[2] = 0;
         slen[3] = 0;

         *p = 1;
         set = 2;
         }
      }
   

   if (((STR->mode_extension == 0x01) || (STR->mode_extension == 0x03)) 
        && 
       (ch == 1))
      { 
      //
      // This is the right channel of an intensity stereo stream
      //

      STR->intensity_scale = c % 2;

      S32 isc = c >> 1;

      group = 1;

      if (isc < 180)
         {
         slen[0] = isc / 36;
         slen[1] = (isc % 36) / 6;
         slen[2] = (isc % 36) % 6;
         slen[3] = 0;

         *p = 0;
         set = 0;
         }
      else if ((180 <= isc) && (isc < 244))
         {
         slen[0] = ((isc-180) % 64) >> 4;
         slen[1] = ((isc-180) % 16) >> 2;
         slen[2] = (isc-180) % 4;
         slen[3] = 0;

         *p = 0;
         set = 1;
         }
      else if ((244 <= isc) && (isc < 255))
         {
         slen[0] = (isc-244) / 3;
         slen[1] = (isc-244) % 3;
         slen[2] = 0;
         slen[3] = 0;

         *p = 0;
         set = 2;
         }
      }

   //
   // Get scalefactor band counts
   //

   S32 sfb_cnt[4];

   if ((w) && (b == 2))
      {
      x_memcpy(sfb_cnt,
                mp3_sfb_cnts[group][set][m + 1],
                4 * sizeof(S32));
      }
   else
      {
      x_memcpy(sfb_cnt,
                 mp3_sfb_cnts[group][set][0],
                 4 * sizeof(S32));
      }

   //
   // Decode MPEG2 scalefactors, recording limits
   //

   S32 sfb = 0;

   if ((!w) || (w && (b != 2)))
      {
      //
      // Pure long block
      //

      for (j=0; j < 4; j++)
         {
         S32 len = slen[j];
         S32 lim = (1 << len) - 1;

         for (k=0; k < sfb_cnt[j]; k++)
            {
            STR->scalefac[0][ch].l[sfb] = A(len);

            if (ch)
               {
               STR->pos_limit[sfb] = lim;
               }

            sfb++;
            }
         }
      }
   else if (b == 2)
      {
      if (!m)
         {
         //
         // Pure short block
         //

         for (j=0; j < 4; j++)
            {
            S32 len = slen[j];
            S32 lim = (1 << len) - 1;

            for (k=0; k < sfb_cnt[j]; k += 3)
               {
               STR->scalefac[0][ch].s[0][sfb] = A(len);
               STR->scalefac[0][ch].s[1][sfb] = A(len);
               STR->scalefac[0][ch].s[2][sfb] = A(len);

               if (ch)
                  {
                  STR->pos_limit[sfb + 6] = lim;
                  }

               sfb++;
               }
            }
         }
      else
         {
         //
         // Mixed block
         //

         S32 len = slen[0];
         S32 lim = (1 << len) - 1;

         for (k=0; k < 6; k++)
            {
            STR->scalefac[0][ch].l[sfb] = A(len);

            if (ch)
               {
               STR->pos_limit[sfb] = lim;
               }

            sfb++;
            }

         sfb_cnt[0] -= 6;

         sfb = 3;

         for (j=0; j < 4; j++)
            {
            S32 len = slen[j];
            S32 lim = (1 << len) - 1;

            for (k = 0; k < sfb_cnt[j]; k += 3)
               {
               STR->scalefac[0][ch].s[0][sfb] = A(len);
               STR->scalefac[0][ch].s[1][sfb] = A(len);
               STR->scalefac[0][ch].s[2][sfb] = A(len);

               if (ch)
                  {
                  STR->pos_limit[sfb + 6] = lim;
                  }

               sfb++;
               }
            }
         }
      }

   //
   // Return success
   //

   return 1;
}

//############################################################################
//#                                                                          #
//# Decode layer 3 side information and Huffman data                         #
//#                                                                          #
//############################################################################

static S32 L3_parse(ASISTREAM* STR, S32 gr, S32 ch)
{
   S32 part2_start = STR->apos;

   //
   // Fetch and decode scalefactors
   //

// jm: fix the problem with the Xing encoder
/*   S32 i,j,k;

   for (i=0; i < 2; i++)
      {
      for (j=0; j < 2; j++)
         {
         for (k=0; k < 23; k++)
            {
            STR->scalefac[i][j].l[k] = 0;
            }

         for (k=0; k < 13; k++)
            {
            STR->scalefac[i][j].s[0][k] = 0;
            STR->scalefac[i][j].s[1][k] = 0;
            STR->scalefac[i][j].s[2][k] = 0;
            }
         }
      }*/

   if (STR->MPEG1)
      {
      L3_sf_MP1(STR, gr, ch);
      }
   else
      {
      L3_sf_MP2(STR, gr, ch);
      }

   //
   // Init Huffman decode information in accordance with pp. 28,29
   //

   S32 big_value = STR->big_values[gr][ch] * 2;
   S32 r[3];

   //
   // Get Huffman region boundaries
   //
   // r[0] = beginning of region 1
   // r[1] = beginning of region 2
   // r[2] = end of big_value space
   //

   S16* t_l = mp3_sf_bound_l[STR->MPEG1][STR->sampling_frequency];
   S16* t_s = mp3_sf_bound_s[STR->MPEG1][STR->sampling_frequency];

   if ((!STR->window_switching_flag[gr][ch]) &&
        (STR->block_type           [gr][ch] == 0))
      {
      //
      // Long blocks: divided into 3 regions, aligned with scalefactor
      // bands
      //

      r[0] = t_l[STR->region0_count[gr][ch]] + 1;

      if (r[0] > big_value)
         {   
         r[0] = r[1] = big_value;
         }
      else
         {
         r[1] = t_l[STR->region0_count[gr][ch] +
                    STR->region1_count[gr][ch] + 1] + 1;

         if (r[1] > big_value)
            {
            r[1] = big_value;
            }
         }
      }
   else
      {
      //
      // Short or mixed blocks
      //
   
      if ((STR->block_type      [gr][ch] == 2) &&
         (!STR->mixed_block_flag[gr][ch]))
         {
         //
         // p. 26: If window_switching_flag is set, block type == 2, 
         // and !mixed_block_flag, region0_count = 8.  Since this
         // is a short block, each band is counted 3 times, so a 
         // region0_count of 8 indicates that region1 begins at
         // scalefactor band 3.  The beginning of this band is 
         // represented as one past the end of band 2 (t_s[2]+1).
         //

         r[0] = 3 * (t_s[2] + 1);
         }
      else
         {
         //
         // p. 26: If this is a long block with window_switching_flag
         // set, region0_count = 7.  So region1 begins at scalefactor
         // band 8, represented as one past the end of band2 (t_l[7]+1).
         //

         r[0] = t_l[7] + 1;
         }

      if (r[0] > big_value)
         {   
         r[0] = big_value;
         }

      //
      // Regardless of window type, in a short or mixed block 
      // all remaining values in the big_value region are contained
      // in region 1
      //

      r[1] = big_value;
      }

   r[2] = big_value;

   //
   // Decode Huffman frequency-coefficient pairs from big_values
   // partition (regions 0, 1, 2)
   //

   S32 line = 0;

   S16* is      = &STR->m_is[ch][0];
   U8*  is_sign = &STR->m_is_sign[ch][0];

   for (S32 rgn=0; rgn < 3; rgn++)
      {
      S32           n      = STR->table_select[gr][ch][rgn];
      newhuff*      h      = mp3_ht+n;
      S32           esclen = h->linbits;
      S32           end    = r[rgn];

      // prevent overwrites by malformed MP3 frames
      if (((char*)(is+(end-line)))>(((char*)STR->m_is)+sizeof(STR->m_is)))
        end=line+((((char*)STR->m_is)+sizeof(STR->m_is))-((char*)is))/sizeof(is[0]);

      if (((char*)(is_sign+(end-line)))>(((char*)STR->m_is_sign)+sizeof(STR->m_is_sign)))
        end=line+((((char*)STR->m_is_sign)+sizeof(STR->m_is_sign))-((char*)is_sign))/sizeof(is_sign[0]);

         {
         while (line < end)
            {
            S32 x     = 0;
            S32 y     = 0;
            S32 xsign = 0;
            S32 ysign = 0;

            //
            // Decode Huffman pair at current bitstream position
            //
            // Codes of length <= 8 are decoded via a fast hash table; longer
            // codes are decoded via conventional b-tree traversal
            //

          U8*       byte = &STR->audio_buffer[STR->apos >> 3];
          U32       bit  = 0x80 >> (STR->apos & 7);
          short*    val  = h->table;
          while( (y=*val++)<0 )
          {
            if( (*byte)&bit )
              val -= y;
            
            bit >>= 1;
            if( !bit )
            {
                bit = 0x80;
                ++byte;
            }
            ++STR->apos;
          }
          x = y >> 4;
          y &= 0xf;


            //
            // Store coefficient pair, and fetch sign bits and escape codes
            //

            if ((x == 15) && (esclen > 0)) x += A(esclen);
            if ((x)       && (A(1)))       xsign = 1;

            if ((y == 15) && (esclen > 0)) y += A(esclen);
            if ((y)       && (A(1)))       ysign = 1;

            //
            // Store coefficient pair and continue
            //

            *is++ = (S16)(((S16) x)&8191);
            *is++ = (S16)(((S16) y)&8191);

            *is_sign++ = (U8) xsign;
            *is_sign++ = (U8) ysign;

            line += 2;
            }
         }
      }

   //
   // Calculate size of count1 partition
   //
   // count1 size = difference between part2_3_length and # of bits
   // read so far
   //

   S32      n = STR->count1table_select[gr][ch];
   newhuff* h = mp3_htc+n;
   S32      part3_end = part2_start + STR->part2_3_length[gr][ch];

   while (STR->apos < part3_end)
      {
      if (line >= 576) 
         {
         break;
         }

      //
      // Decode Huffman quad at current bitstream position
      //
      // Codes of length <= 8 are decoded via a fast hash table; longer
      // codes are decoded via conventional b-tree traversal
      //
    U8*     byte = &STR->audio_buffer[STR->apos >> 3];
    U32     bit  = 0x80 >> (STR->apos & 7);
    short*  pTable = h->table;
    S32     val;

    while( (val=*pTable++)<0 )
    {
        if( (*byte) & bit )
            pTable -= val;

        bit >>= 1;
        if( !bit )
        {
            bit = 0x80;
            ++byte;
        }
        ++STR->apos;
    }


      //
      // Unpack coefficient quad, and fetch sign bits
      //

      S32 v = (val & 8) >> 3;
      S32 w = (val & 4) >> 2;
      S32 x = (val & 2) >> 1;
      S32 y = (val & 1);

      *is++ = (S16) v;
      *is++ = (S16) w;
      *is++ = (S16) x;
      *is++ = (S16) y;

      *is_sign++ = ((v) && (A(1)));
      *is_sign++ = ((w) && (A(1)));
      *is_sign++ = ((x) && (A(1)));
      *is_sign++ = ((y) && (A(1)));

      line += 4;
      }

   //
   // Skip past any padding or ancillary data to reach next channel
   //

   STR->apos = part3_end;

   //
   // Log beginning of zero part of frequency spectrum 
   //

   if (line > 576)
      {
      STR->zero_part[ch] = 576;
      }
   else
      {
      STR->zero_part[ch] = line;
      }

   //
   // Flush remainder of channel's frequency-line set with zeroes
   //

   if (line < 576)
      {
      STR->zero_count[ch] = (576 - line);

      x_memset(is, 
                 0, 
                 STR->zero_count[ch] * sizeof(S16));
      }
   else
      {
      STR->zero_count[ch] = 0;
      }

   //
   // Return success
   //

   return 1;
}

//############################################################################
//#                                                                          #
//# Calculate scaling values for MPEG 2 Layer 3 stereo processing            #
//#                                                                          #
//############################################################################

inline void L3_stereo_k_values(S32    is_pos, //)
                               F32 io,
                               S32    i,
                               F32 k[2][576])
{
   if (is_pos == 0)
      { 
      k[0][i] = 1.0F;
      k[1][i] = 1.0F;
      }
   else if ((is_pos % 2) == 1)
      {
      k[0][i] = pow(io,(F32) ((is_pos + 1) / 2));
      k[1][i] = 1;
      }
   else
      {
      k[0][i] = 1;
      k[1][i] = pow(io,(F32) (is_pos / 2));
      }
}

//############################################################################
//#                                                                          #
//# Perform stereo processing                                                #
//#                                                                          #
//############################################################################

static S32 L3_stereo(ASISTREAM* STR, S32 gr)
{
   //
   // Initialization
   //

   S32 sfreq  = STR->sampling_frequency + (STR->MPEG1 * 3);
   S32 stereo = STR->nch;

   S32 ms_stereo = (STR->mode == MPG_MD_JOINT_STEREO) &&
                   (STR->mode_extension & 0x2); 

   S32 i_stereo  = (STR->mode == MPG_MD_JOINT_STEREO) &&
                   (STR->mode_extension & 0x1);

   static F32 is_ratio [576];
   static F32 io;
   static F32 k     [2][576];
   static U8  is_pos   [576];
   static S32 i,j,sb,ss,sfb;

   for (i=0; i < 576; i++)
      {
      is_pos[i] = 7;
      }

   if ((STR->scalefac_compress[gr][0] % 2) == 1)
      {
      io = 0.707106781188F;
      }
   else
      {
      io = 0.840896415256F;
      }

   //
   // Perform intensity stereo processing
   //

   if ((stereo == 2) && i_stereo)
      {  
      if (STR->window_switching_flag[gr][0] &&
         (STR->block_type           [gr][0] == 2))
         {  
         if (STR->mixed_block_flag[gr][0])
            {  
            //
            // Mixed short/long block
            //

            S32 max_sfb = 0;

            for (j=0; j < 3; j++)
               {  
               S32 sfbcnt = 2;

               for (sfb = 12; sfb >= 3; sfb--)
                  {  
                  S32 lines = mp3_sfBandIndex[sfreq].s[sfb+1] -
                              mp3_sfBandIndex[sfreq].s[sfb];

                  i = 3 * mp3_sfBandIndex[sfreq].s[sfb] + (j+1) * lines - 1;

                  while (lines > 0)
                     {  
                     if (STR->m_xr[1][i / SSLIMIT][i % SSLIMIT] != 0.0)
                        {  
                        sfbcnt = sfb;
                        sfb    = -10;
                        lines  = -10;
                        }

                     lines--;
                     i--;
                     }
                  }

               sfb = sfbcnt + 1;

               if (sfb > max_sfb)
                  {
                  max_sfb = sfb;
                  }

               while (sfb < 12)
                  {  
                  sb = mp3_sfBandIndex[sfreq].s[sfb+1] -
                       mp3_sfBandIndex[sfreq].s[sfb];

                  i = 3 * mp3_sfBandIndex[sfreq].s[sfb] + j * sb;

                  for (; sb > 0; sb--)
                     {  
                     if (i >= 576) break;

                     is_pos[i] = (U8) STR->scalefac[gr][1].s[j][sfb];

                     if (is_pos[i] != 7)
                        {
                        if (!STR->MPEG1)
                           {
                           L3_stereo_k_values(is_pos[i],io,i,k);
                           }
                        else
                           {
                           is_ratio[i] = tan(is_pos[i] * (PI / 12));
                           }
                        }

                     i++;
                     }

                  sfb++;
                  }

               sb  = mp3_sfBandIndex[sfreq].s[12] - 
                     mp3_sfBandIndex[sfreq].s[11];

               sfb = 3 * mp3_sfBandIndex[sfreq].s[11] + j * sb;

               sb  = mp3_sfBandIndex[sfreq].s[13] - 
                     mp3_sfBandIndex[sfreq].s[12];

               i   = 3 * mp3_sfBandIndex[sfreq].s[11] + j * sb;

               for (; sb > 0; sb--)
                  {  
                  if (i >= 576) break;

                  is_pos  [i] = is_pos[sfb];
                  is_ratio[i] = is_ratio[sfb];
                  k[0][i]     = k[0][sfb];
                  k[1][i]     = k[1][sfb];

                  i++;
                  }
               }

            if (max_sfb <= 3)
               {  
               i  = 2;
               ss = 17;
               sb = -1;

               while (i >= 0)
                  {  
                  if (STR->m_xr[1][i][ss] != 0.0)
                     {  
                     sb = i * 18 + ss;
                     i = -1;
                     } 
                  else
                     {  
                     ss--;

                     if (ss < 0)
                        {  
                        i--;
                        ss = 17;
                        }
                     }
                  }

               i = 0;

               while (mp3_sfBandIndex[sfreq].l[i] <= sb)
                  {
                  i++;
                  if (i >= 576) break;
                  }

               sfb = i;

               i = mp3_sfBandIndex[sfreq].l[i];

               for (; sfb < 8; sfb++)
                  {  
                  sb = mp3_sfBandIndex[sfreq].l[sfb+1] -
                       mp3_sfBandIndex[sfreq].l[sfb];

                  for (; sb > 0; sb--)
                     {  
                     if (i >= 576) break;

                     is_pos[i] = (U8) STR->scalefac[gr][1].l[sfb];

                     if (is_pos[i] != 7)
                        {
                        if (!STR->MPEG1)
                           {
                           L3_stereo_k_values(is_pos[i],io,i,k);
                           }
                        else
                           {
                           is_ratio[i] = tan(is_pos[i] * (PI / 12));
                           }
                        }

                     i++;
                     }
                  }
               }
            } 
         else
            {  
            //
            // Pure short block
            //

            for (j = 0; j < 3; j++)
               {  
               S32 sfbcnt = -1;

               for (sfb = 12; sfb >=0; sfb--)
                  {  
                  S32 lines = mp3_sfBandIndex[sfreq].s[sfb+1] - 
                              mp3_sfBandIndex[sfreq].s[sfb];

                  i = 3 * mp3_sfBandIndex[sfreq].s[sfb] + (j+1) * lines - 1;

                  while (lines > 0)
                     {  
                     if (STR->m_xr[1][i / SSLIMIT][i % SSLIMIT] != 0.0)
                        {  
                        sfbcnt = sfb;
                        sfb    = -10;
                        lines  = -10;
                        }

                     lines--;
                     i--;
                     }
                  }

               sfb = sfbcnt + 1;

               while (sfb < 12)
                  {  
                  sb = mp3_sfBandIndex[sfreq].s[sfb+1] -
                       mp3_sfBandIndex[sfreq].s[sfb];

                  i = 3 * mp3_sfBandIndex[sfreq].s[sfb] + j * sb;

                  for (; sb > 0; sb--)
                     {  
                     if (i >= 576) break;

                     is_pos[i] = (U8) STR->scalefac[gr][1].s[j][sfb];

                     if (is_pos[i] != 7)
                        {
                        if (!STR->MPEG1)
                           {
                           L3_stereo_k_values(is_pos[i],io,i,k);
                           }
                        else
                           {
                           is_ratio[i] = x_tan(is_pos[i] * (PI / 12));
                           }
                        }

                     i++;
                     }
                  sfb++;
                  }

               sb = mp3_sfBandIndex[sfreq].s[12] - 
                    mp3_sfBandIndex[sfreq].s[11];

               sfb = 3 * mp3_sfBandIndex[sfreq].s[11] + j * sb;

               sb = mp3_sfBandIndex[sfreq].s[13] -
                    mp3_sfBandIndex[sfreq].s[12];

               i = 3 * mp3_sfBandIndex[sfreq].s[11] + j * sb;

               for (; sb > 0; sb--)
                  {  
                  if (i >= 576) break;

                  is_pos  [i] = is_pos[sfb];
                  is_ratio[i] = is_ratio[sfb];
                  k[0][i]     = k[0][sfb];
                  k[1][i]     = k[1][sfb];

                  i++;
                  }
               }
            }
         } 
      else
         {  
         //
         // Pure long block
         //

         i  = 31;
         ss = 17;
         sb = 0;

         while (i >= 0)
            {  
            if (STR->m_xr[1][i][ss] != 0.0)
               {  
               sb = i * 18 + ss;
               i = -1;
               } 
            else
               {  
               ss--;

               if (ss < 0)
                  {  
                  i--;
                  ss = 17;
                  }
               }
            }

         i = 0;

         while (mp3_sfBandIndex[sfreq].l[i] <= sb)
            {
            if (i >= 576) break;
            i++;
            }

         sfb = i;
         i   = mp3_sfBandIndex[sfreq].l[i];

         for (; sfb < 21; sfb++)
            {  
            sb = mp3_sfBandIndex[sfreq].l[sfb+1] - 
                 mp3_sfBandIndex[sfreq].l[sfb];

            for (; sb > 0; sb--)
               {  
               if (i >= 576) break;

               is_pos[i] = (U8) STR->scalefac[gr][1].l[sfb];

               if (is_pos[i] != 7)
                  {
                  if (!STR->MPEG1)
                     {
                     L3_stereo_k_values(is_pos[i],io,i,k);
                     }
                  else
                     {
                     is_ratio[i] = x_tan(is_pos[i] * (PI / 12));
                     }
                  }

               i++;
               }
            }

         sfb = mp3_sfBandIndex[sfreq].l[20];

         for (sb = 576 - mp3_sfBandIndex[sfreq].l[21]; sb > 0; sb--)
            {  
            //
            // NB: Derived from ISO reference source, which can crash
            // on some files if check below not made
            //

            if (i >= 576) break;

            is_pos[i]   = is_pos[sfb];
            is_ratio[i] = is_ratio[sfb];
            k[0][i]     = k[0][sfb];
            k[1][i]     = k[1][sfb];

            i++;
            }
         }
      }

   //
   // Write demultiplexed data to output array, reordering as well
   // where appropriate
   //

#if ISO_DCT
   S16* xchg = mp3_reord_ft[STR->sampling_frequency];
#else
   S16* xchg = mp3_reord_ft[STR->MPEG1][STR->sampling_frequency];
#endif

   F32* ld = STR->m_lr[0][0];
   F32* rd = STR->m_lr[1][0];

   F32* ls = STR->m_xr[0][0];
   F32* rs = STR->m_xr[1][0];

   i = 0;

   if (stereo == 2) 
      {
      //
      // Left channel
      //

      if (STR->window_switching_flag[gr][0] && (STR->block_type[gr][0] == 2)) 
         {
         if (STR->mixed_block_flag[gr][0])
            {
            for (; i < 36; i++)
               {
               if (is_pos[i] == 7) 
                  {
                  if (ms_stereo)
                     {
                     ld[i] = (ls[i] + rs[i]) * 0.707106781f;
                     }
                  else 
                     {
                     ld[i] = ls[i];
                     }
                  }
               else 
                  {
                  if (i_stereo) 
                     {
                     if (!STR->MPEG1)
                        {
                        ld[i] = ls[i] * k[0][i];
                        }
                     else
                        {
                        ld[i] = ls[i] * (is_ratio[i] / (1 + is_ratio[i]));
                        }
                     }
                  }
               }
            }

         for (; i < 576; i++)
            {
            if (is_pos[i] == 7)
               {
               if (ms_stereo) 
                  {
                  ld[xchg[i]] = (ls[i] + rs[i]) * 0.707106781f;
                  }
               else 
                  {
                  ld[xchg[i]] = ls[i];
                  }
               }
            else
               {
               if (i_stereo)
                  {
                  if (!STR->MPEG1)
                     {
                     ld[xchg[i]] = ls[i] * k[0][i];
                     }
                  else
                     {
                     ld[xchg[i]] = ls[i] * (is_ratio[i] / (1 + is_ratio[i]));
                     }
                  }
               }
            }
         }
      else
         {
         for (; i < 576; i++)
            {
            if (is_pos[i] == 7) 
               {
               if (ms_stereo) 
                  {
                  ld[i] = (ls[i] + rs[i]) * 0.707106781f;
                  }
               else 
                  {
                  ld[i] = ls[i];
                  }
               }
            else 
               {
               if (i_stereo) 
                  {
                  if (!STR->MPEG1)
                     {
                     ld[i] = ls[i] * k[0][i];
                     }
                  else
                     {
                     ld[i] = ls[i] * (is_ratio[i] / (1 + is_ratio[i]));
                     }
                  }
               }
            }
         }

      //
      // Right channel
      //

      i = 0;

      if (STR->window_switching_flag[gr][1] && (STR->block_type[gr][1] == 2)) 
         {
         if (STR->mixed_block_flag[gr][1])
            {
            for (; i < 36; i++)
               {
               if (is_pos[i] == 7) 
                  {
                  if (ms_stereo) 
                     {
                     rd[i] = (ls[i] - rs[i]) * 0.707106781f;
                     }
                  else 
                     {
                     rd[i] = rs[i];
                     }
                  }
               else 
                  {
                  if (i_stereo) 
                     {
                     if (!STR->MPEG1)
                        {
                        rd[i] = ls[i] * k[1][i];
                        }
                     else
                        {
                        rd[i] = ls[i] * (1 / (1 + is_ratio[i])); 
                        }
                     }
                  }
               }
            }

         for (; i < 576; i++)
            {
            if (is_pos[i] == 7) 
               {
               if (ms_stereo) 
                  {
                  rd[xchg[i]] = (ls[i] - rs[i]) * 0.707106781f;
                  }
               else 
                  {
                  rd[xchg[i]] = rs[i];
                  }
               }
            else 
               {
               if (i_stereo) 
                  {
                  if (!STR->MPEG1)
                     {
                     rd[xchg[i]] = ls[i] * k[1][i];
                     }
                  else
                     {
                     rd[xchg[i]] = ls[i] * (1 / (1 + is_ratio[i])); 
                     }
                  }
               }
            }
         }
      else
         {
         for (; i < 576; i++)
            {
            if (is_pos[i] == 7) 
               {
               if (ms_stereo) 
                  {
                  rd[i] = (ls[i] - rs[i]) * 0.707106781f;
                  }
               else 
                  {
                  rd[i] = rs[i];
                  }
               }
            else 
               {
               if (i_stereo) 
                  {
                  if (!STR->MPEG1)
                     {
                     rd[i] = ls[i] * k[1][i];
                     }
                  else
                     {
                     rd[i] = ls[i] * (1 / (1 + is_ratio[i])); 
                     }
                  }
               }
            }
         }
      }
   else
      {
      //
      // Mono
      //

      if (STR->window_switching_flag[gr][0] && (STR->block_type[gr][0] == 2)) 
         {
         if (STR->mixed_block_flag[gr][0])
            {
            while (i++ < 36)
               {
               *ld++ = *ls++;
               }
            }

         while (i < 576)
            {
            ld[xchg[i++]] = *ls++;
            }
         }
      else
         {
         while (i++ < 576)
            {
            *ld++ = *ls++;
            }
         }
      }

   //
   // Return success
   //

   return 1;
}

//############################################################################
//#                                                                          #
//#  Perform antialiasing pass                                               #
//#                                                                          #
//############################################################################

static S32 L3_antialias(ASISTREAM* STR, S32 gr, S32 ch)
{
   if ((STR->window_switching_flag[gr][ch]) && 
       (STR->block_type           [gr][ch] == 2))
      {
      return 1;
      }

   for (S32 sb=1; sb < 32; sb++) 
      {
      F32 *x = STR->m_lr[ch][sb];
      F32 a, b;

      a = x[0];
      b = x[-1];
      x[-1] = b * mp3_Cs[0] - a * mp3_Ca[0];
      x[0]  = a * mp3_Cs[0] + b * mp3_Ca[0];

      a = x[1];
      b = x[-2];
      x[-2] = b * mp3_Cs[1] - a * mp3_Ca[1];
      x[1]  = a * mp3_Cs[1] + b * mp3_Ca[1];

      a = x[2];
      b = x[-3];
      x[-3] = b * mp3_Cs[2] - a * mp3_Ca[2];
      x[2]  = a * mp3_Cs[2] + b * mp3_Ca[2];

      a = x[3];
      b = x[-4];
      x[-4] = b * mp3_Cs[3] - a * mp3_Ca[3];
      x[3]  = a * mp3_Cs[3] + b * mp3_Ca[3];

      a = x[4];
      b = x[-5];
      x[-5] = b * mp3_Cs[4] - a * mp3_Ca[4];
      x[4]  = a * mp3_Cs[4] + b * mp3_Ca[4];

      a = x[5];
      b = x[-6];
      x[-6] = b * mp3_Cs[5] - a * mp3_Ca[5];
      x[5]  = a * mp3_Cs[5] + b * mp3_Ca[5];

      a = x[6];
      b = x[-7];
      x[-7] = b * mp3_Cs[6] - a * mp3_Ca[6];
      x[6]  = a * mp3_Cs[6] + b * mp3_Ca[6];

      a = x[7];
      b = x[-8];
      x[-8] = b * mp3_Cs[7] - a * mp3_Ca[7];
      x[7]  = a * mp3_Cs[7] + b * mp3_Ca[7];
      }

   return 1;
}

//############################################################################
//#                                                                          #
//# Get statistics for Layer 3 frame                                         #
//#                                                                          #
//############################################################################

S32 L3_frame_info(ASISTREAM* STR)
{
   if (STR->MPEG1)
      {
      //
      // MPEG 1: Mono=17, stereo=32 bytes
      //

      STR->side_info_size = (STR->mode == 3) ? 17 : 32;
      }
   else
      {
      //
      // MPEG2: Mono=9, stereo=17 bytes
      //

      STR->side_info_size = (STR->mode == 3) ? 9 : 17;
      }

   //
   // Read side info block into header buffer
   //

   S32 result = STR->fetch_CB(STR->user,
                             &STR->header_buffer[STR->header_size],
                              STR->side_info_size,
                             -1);

   if (result != STR->side_info_size)
      {
      x_strcpy(ASI_error_text,"Invalid MPEG audio stream");
      return 0;
      }

   STR->current_offset += STR->side_info_size;

   //
   // Get main data offset word and skip private bits
   //

   STR->nch = (STR->mode == 3) ? 1 : 2;
   STR->ngr = (STR->MPEG1) ? 2 : 1;

   if (STR->MPEG1)
      {
      STR->main_data_begin = H(9);
      (STR->nch == 1) ? H(5) : H(3);
      }
   else
      {
      STR->main_data_begin = H(8);
      (STR->nch == 1) ? H(1) : H(2);
      }

   //
   // Get MPEG1 scale factor selection information
   //

   if (STR->MPEG1)
      {
      for (S32 ch=0; ch < STR->nch; ch++)
         {
         for (S32 scfsi_band=0; scfsi_band < 4; scfsi_band++)
            {
            STR->scfsi[ch][scfsi_band] = H(1);
            }
         }
      }

   //
   // Read MPEG1 / MPEG2 BC granule configuration data
   //

   S32 c_bits = (STR->MPEG1) ? 4 : 9;

   for (S32 gr=0; gr < STR->ngr; gr++)
      {
      for (S32 ch=0; ch < STR->nch; ch++)
         {
         STR->part2_3_length       [gr][ch] = H(12);
         STR->big_values           [gr][ch] = H(9);
         STR->global_gain          [gr][ch] = H(8);
         STR->scalefac_compress    [gr][ch] = H(c_bits);
         STR->window_switching_flag[gr][ch] = H(1);

         if (STR->window_switching_flag[gr][ch])
            {
            STR->block_type      [gr][ch] = H(2);
            STR->mixed_block_flag[gr][ch] = H(1);

            for (S32 region=0; region < 2; region++)
               {
               STR->table_select[gr][ch][region] = H(5);
               }

            STR->table_select[gr][ch][2] = 0;

            for (S32 window=0; window < 3; window++)
               {
               STR->subblock_gain[gr][ch][window] = H(3);
               }
            }
         else
            {
            STR->block_type      [gr][ch] = 0;
            STR->mixed_block_flag[gr][ch] = 0;

            for (S32 region=0; region < 3; region++)
               {
               STR->table_select[gr][ch][region] = H(5);
               }

            STR->region0_count[gr][ch] = H(4);
            STR->region1_count[gr][ch] = H(3);
            }

         if (STR->MPEG1)
            {
            STR->preflag[gr][ch] = H(1);
            }

         STR->scalefac_scale    [gr][ch] = H(1);
         STR->count1table_select[gr][ch] = H(1);
         }
      }

   //
   // Calculate average and actual frame size
   //
   // In Layer 3, MPEG2 has only one granule per frame, while MPEG1 has two
   //

   STR->bit_rate = mp3_bit_rate[STR->MPEG1][STR->bitrate_index];

   STR->sample_rate = mp3_sample_rate[STR->MPEG25][STR->MPEG1][STR->sampling_frequency];

   if (STR->MPEG1)
      {
      STR->average_frame_size = (144 * STR->bit_rate) / STR->sample_rate;
      }
   else
      {
      STR->average_frame_size = (72 * STR->bit_rate) / STR->sample_rate;
      }

   STR->data_size = STR->average_frame_size + STR->padding_bit - (STR->header_size + STR->side_info_size);

   if ( STR->data_size < 0 )
     STR->data_size = 0;

   return 1;
}


//############################################################################
//#                                                                          #
//# Decode frame of Layer 3 audio data                                       #
//#                                                                          #
//############################################################################

S32 L3_frame(ASISTREAM* STR)
{
   //
   // Read in audio data associated with frame
   //

   S32 incoming = fetch_audio_data(STR);

   if (incoming == -1)
      {
      return 0;
      }

   //
   // Set read cursor to offset of data for current frame (which is always
   // on a byte boundary, but may be *before* the data just read)
   //

   STR->apos = 8 * (incoming - STR->main_data_begin);

   //
   // If read cursor falls before the start of buffered frame data, we 
   // do not have enough data to process this frame -- skip it
   //

   if (STR->apos < 0)
      {
      return 1;
      }

   //
   // For all granules in all channels...
   //

   for (S32 gr=0; gr < STR->ngr; gr++)
      {
      //
      // Fetch data and requantize it
      //

      S32 ch;
      
      for (ch=0; ch < STR->nch; ch++)
         {
         L3_parse     (STR, gr, ch);
         L3_requantize(STR, gr, ch);
         }

      //
      // Perform stereo processing and reordering
      // 

      L3_stereo(STR, gr);

      //
      // Calculate # of IMDCT operations required to reach zero part of
      // spectrum
      //

      S32 n_dct;

      if ((STR->window_switching_flag[gr][0] && (STR->block_type[gr][0] == 2))
           ||
          (STR->window_switching_flag[gr][1] && (STR->block_type[gr][1] == 2)))
         {
         //
         // Short or mixed blocks use all 32 subbands
         //

         n_dct = SBLIMIT;
         }
      else
         {
         //
         // Pure long blocks can be truncated at zero part
         //

         S32 n0 = ((STR->zero_part[0] - 1) / 18) + 1;
         S32 n1 = ((STR->zero_part[1] - 1) / 18) + 1;

         n_dct = max(n0,n1);
         }

      //
      // Convert to amplitude domain via IMDCT and polyphase filter
      //

      for (ch=0; ch < STR->nch; ch++)
         {
         //
         // Perform alias-reduction
         //

         L3_antialias(STR, gr, ch);

         //
         // Synthesize via IMDCT and overlap-add
         //

         S32 win_type;

         if ((STR->window_switching_flag[gr][ch])      && 
             (STR->block_type           [gr][ch] == 2) &&
             (STR->mixed_block_flag     [gr][ch]))
            {
            win_type = 0;
            }
         else if (!STR->window_switching_flag[gr][ch])
            {
            win_type = 0;
            }
         else
            {
            win_type = STR->block_type[gr][ch];
            }

         //
         // Always do first two subbands, then select long windows if
         // appropriate and continue with remaining subbands up to
         // zero part of spectrum 
         //

         S32 sb;

         if (win_type == 2)
            {
#ifdef IS_X86
/*
            if (use_AMD)
               {
               for (sb=0; sb < 2; sb++)
                  {
                  AMD_IMDCT_3x12(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb]);
                  }
               }
            else
*/
               {
               for (sb=0; sb < 2; sb++)
                  {
                  x86_IMDCT_3x12(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb]);
                  }
               }
#else
           for (sb=0; sb < 2; sb++)
              {
                 PPC_IMDCT_3x12(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb]);
              }
#endif
            }
         else
            {
#ifdef IS_X86
/*
            if (use_AMD)
               {
               for (sb=0; sb < 2; sb++)
                  {
                  AMD_IMDCT_1x36(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb], mp3_win[win_type]);
                  }
               }
            else
*/
               {
               for (sb=0; sb < 2; sb++)
                  {
                  x86_IMDCT_1x36(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb], mp3_win[win_type]);
                  }
               }
#else
           for (sb=0; sb < 2; sb++)
              {
                 PPC_IMDCT_1x36(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb], mp3_win[win_type]);
              }
#endif
            }

         if ((STR->window_switching_flag[gr][ch])      && 
             (STR->block_type           [gr][ch] == 2) &&
             (STR->mixed_block_flag     [gr][ch]))
            {
            win_type = 2;
            }

         if (win_type == 2)
            {
#ifdef IS_X86
/*
            if (use_AMD)
               {
               for (sb=2; sb < n_dct; sb++)
                  {
                  AMD_IMDCT_3x12(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb]);
                  }
               }
            else
*/
               {
               for (sb=2; sb < n_dct; sb++)
                  {
                  x86_IMDCT_3x12(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb]);
                  }
               }
#else
           for (sb=2; sb < n_dct; sb++)
              {
                 PPC_IMDCT_3x12(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb]);
              }
#endif
            }
         else
            {
#ifdef IS_X86
/*
            if (use_AMD)
               {
               for (sb=2; sb < n_dct; sb++)
                  {
                  AMD_IMDCT_1x36(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb], mp3_win[win_type]);
                  }
               }
            else
*/
               {
               for (sb=2; sb < n_dct; sb++)
                  {
                  x86_IMDCT_1x36(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb], mp3_win[win_type]);
                  }
               }
#else
           for (sb=2; sb < n_dct; sb++)
              {
                 PPC_IMDCT_1x36(STR->m_lr[ch][sb], sb, STR->m_res[sb], STR->m_s[ch][sb], mp3_win[win_type]);
              }
#endif
            }

         //
         // Clear zero part explicitly, without performing IMDCTs for
         // subbands known to be empty
         //

         while (sb < SBLIMIT)
            {
            x_memcpy(STR->m_res    [sb],
                       STR->m_s  [ch][sb],
                       18 * sizeof(F32));

            x_memset(STR->m_s[ch][sb],
                       0,
                       18 * sizeof(F32));
            sb++;
            }

         //
         // Execute final synthesis pass with polyphase filterbank
         //
        
         for (S32 f=0; f < 18; f++)
            {
            S32 start = STR->m_u_start[ch];
            S32 div   = STR->m_u_div  [ch];

#ifdef IS_X86
/*
            if (use_AMD)
               {
               AMD_poly_filter(&STR->m_res[0][0], 
                                mp3_b, 
                                f,
                               &STR->m_u[ch][div][0][start],
                               &STR->m_u[ch][!div][0][start]);
               }
            else
*/
               {
               x86_poly_filter(&STR->m_res[0][0], 
                                mp3_b, 
                                f,
                               &STR->m_u[ch][div][0][start],
                               &STR->m_u[ch][!div][0][start]);
               }
#else
               PPC_poly_filter(&STR->m_res[0][0], 
                                mp3_b, 
                                f,
                               &STR->m_u[ch][div][0][start],
                               &STR->m_u[ch][!div][0][start]);
#endif               

            //
            // Calculate index into 576-long * 2-channel sample buffer segment
            //

            STR->samples = (S16*)((u8*)&STR->frame_buffer[STR->frame_size] +
                                       (64 * sizeof(S16) * (f >> (2 - STR->nch))) +
                                       (2 * sizeof(S16) * ((STR->nch == 2) ? 0 : ((f & 1) ? 16 : 0))) +
                                       (sizeof(S16) * ch));

            //
            // Apply inverse window and emit output samples
            //

#ifdef IS_X86
/*
            if (use_AMD)
               {
               AMD_dewindow_and_write(STR->m_u[ch][div][0],
                                      mp3_t_dewindow[0],
                                      start,
                                      STR->samples,
                                      STR->nch * sizeof(S16),
                                      div);
               }
            else
*/
               {
               x86_dewindow_and_write(STR->m_u[ch][div][0],
                                      mp3_t_dewindow[0],
                                      start,
                                      STR->samples,
                                      STR->nch * sizeof(S16),
                                      div);
               }
#else
               PPC_dewindow_and_write(STR->m_u[ch][div][0],
                                      mp3_t_dewindow[0],
                                      start,
                                      STR->samples,
                                      STR->nch * sizeof(S16),
                                      div);
#endif
            STR->m_u_start[ch] = (STR->m_u_start[ch] - 1) & 0x0f;
            STR->m_u_div  [ch] = !STR->m_u_div[ch];
            }
         }

      //
      // Advance frame buffer pointer
      //

      STR->frame_size += (32 * 18 * STR->nch * sizeof(S16));
      }

   //
   // Return success
   //

   return 1;
}

