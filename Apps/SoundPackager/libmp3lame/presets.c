/*
 * presets.c -- Apply presets
 *
 * Copyright (C) 2002 Gabriel Bouvigne / Lame project
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "lame.h"


#define         Min(A, B)       ((A) < (B) ? (A) : (B))
#define         Max(A, B)       ((A) > (B) ? (A) : (B))





int apply_abr_preset(lame_global_flags*  gfp, int preset)
{
    int k; 

    typedef struct {
        int    abr_kbps;
        int    expZ;
        int    expX;
        int    lowpass;
        int    safejoint;
        double nsmsfix;
        double nsbass;
        double scale;
    } dm_abr_presets_t;


    // Switch mappings for ABR mode
    const dm_abr_presets_t abr_switch_map [] = {
        // kbps Z  X  lowpass safejoint nsmsfix ns-bass scale
        {   8,  1, 6,  2000,  0,        0   ,   -3,      0.85 }, //   8 //impossible to use in stereo
        {  16,  1, 6,  3700,  0,        0   ,   -3,      0.85 }, //  16
        {  24,  1, 1,  3900,  0,        0   ,   -3,      0.85 }, //  24
        {  32,  1, 1,  5500,  0,        0   ,   -3,      0.85 }, //  32
        {  40,  1, 1,  7000,  0,        0   ,   -3,      0.85 }, //  40
        {  48,  1, 1,  7500,  0,        0   ,   -3,      0.85 }, //  48
        {  56,  1, 1, 10000,  0,        0   ,   -3,      0.85 }, //  56
        {  64,  1, 1, 11000,  0,        0   ,   -3,      0.85 }, //  64
        {  80,  1, 1, 13500,  0,        0   ,   -3,      0.85 }, //  80
        {  96,  1, 1, 15300,  0,        0   ,   -4,      0.85 }, //  96
        { 112,  1, 1, 16000,  0,        0   ,   -5,      0.87 }, // 112
        { 128,  1, 1, 17500,  0,        0   ,   -6,      0.93 }, // 128
        { 160,  1, 1, 18000,  0,        0   ,   -4,      0.95 }, // 160
        { 192,  1, 1, 19500,  1,        1.7 ,   -2,      0.97 }, // 192
        { 224,  1, 1, 20000,  1,        1.25,    0,      0.98 }, // 224
        { 256,  0, 3, 20500,  1,        0   ,    0,      1.00 }, // 256
        { 320,  0, 3, 21000,  1,        0   ,    0,      1.00 }  // 320
                                       };

    // Variables for the ABR stuff
    int actual_bitrate = preset;

    int lower_range = 0, lower_range_kbps = 0,
        upper_range = 0, upper_range_kbps = 0;
    
    int r; // r = resolved range

    int b;




    // We assume specified bitrate will be 320kbps
    upper_range_kbps = abr_switch_map[16].abr_kbps;
    upper_range = 16;
    lower_range_kbps = abr_switch_map[16].abr_kbps;
    lower_range = 16;
 
    // Determine which significant bitrates the value specified falls between,
    // if loop ends without breaking then we were correct above that the value was 320
    for (b = 1; b < 17; b++) {
        if ((Max(actual_bitrate, abr_switch_map[b].abr_kbps)) != actual_bitrate) {
              upper_range_kbps = abr_switch_map[b].abr_kbps;
              upper_range = b;
              lower_range_kbps = abr_switch_map[b-1].abr_kbps;
              lower_range = (b-1);
              break; // We found upper range 
        }
    }

    // Determine which range the value specified is closer to
    if ((upper_range_kbps - actual_bitrate) > (actual_bitrate - lower_range_kbps))
        r = lower_range;
    else
        r = upper_range;


    lame_set_VBR(gfp, vbr_abr); 
    lame_set_VBR_mean_bitrate_kbps(gfp, (actual_bitrate));
    lame_set_VBR_mean_bitrate_kbps(gfp, Min(lame_get_VBR_mean_bitrate_kbps(gfp), 320)); 
    lame_set_VBR_mean_bitrate_kbps(gfp, Max(lame_get_VBR_mean_bitrate_kbps(gfp), 8)); 
    lame_set_brate(gfp, lame_get_VBR_mean_bitrate_kbps(gfp));


    lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | 1);
    lame_set_experimentalZ(gfp, abr_switch_map[r].expZ);
    lame_set_experimentalX(gfp, abr_switch_map[r].expX);
    lame_set_quality(gfp, 2);
    lame_set_lowpassfreq(gfp, abr_switch_map[r].lowpass);
    lame_set_mode(gfp, JOINT_STEREO);

    if (abr_switch_map[r].safejoint > 0)
        lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | 2); // safejoint

    if (abr_switch_map[r].nsmsfix > 0)
            (void) lame_set_msfix( gfp, abr_switch_map[r].nsmsfix );

    // ns-bass tweaks
    if (abr_switch_map[r].nsbass != 0) {
        k = (int)(abr_switch_map[r].nsbass * 4);
        if (k < 0) k += 64;
        lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | (k << 2));
    }

    // ABR seems to have big problems with clipping, especially at low bitrates
    // so we compensate for that here by using a scale value depending on bitrate
    if (abr_switch_map[r].scale != 1)
        (void) lame_set_scale( gfp, abr_switch_map[r].scale );

    lame_set_ATHtype(gfp, 2);

    return preset;
}





int apply_preset(lame_global_flags*  gfp, int preset)
{
    switch (preset) {
    case MEDIUM: {
                lame_set_VBR(gfp, vbr_rh);

                lame_set_preset_expopts(gfp, 3);
                lame_set_quality(gfp, 2);
                lame_set_lowpassfreq(gfp, 18000);
                lame_set_mode(gfp, JOINT_STEREO);
                lame_set_VBR_min_bitrate_kbps(gfp, 64);
                lame_set_athaa_sensitivity(gfp, -11);
                lame_set_msfix(gfp, 3);
                lame_set_VBR_q(gfp, 3);
                lame_set_experimentalY(gfp, 1);
                return preset;
           }
    case MEDIUM_FAST: {
                lame_set_VBR(gfp, vbr_mtrh);

                lame_set_preset_expopts(gfp, 3);
                lame_set_quality(gfp, 2);
                lame_set_lowpassfreq(gfp, 18000);
                lame_set_mode(gfp, JOINT_STEREO);
                lame_set_VBR_min_bitrate_kbps(gfp, 64);
                lame_set_athaa_sensitivity(gfp, -11);
                lame_set_msfix(gfp, 3);
                lame_set_VBR_q(gfp, 3);
                lame_set_experimentalY(gfp, 1);
                return preset;
           }
    case STANDARD: {
                lame_set_VBR(gfp, vbr_rh);

                lame_set_preset_expopts(gfp, 3);
                lame_set_quality(gfp, 2);
                lame_set_lowpassfreq(gfp, 19000);
                lame_set_mode(gfp, JOINT_STEREO);
                lame_set_VBR_min_bitrate_kbps(gfp, 128);
                return preset;
           }
    case STANDARD_FAST: {
                lame_set_VBR(gfp, vbr_mtrh);

                lame_set_preset_expopts(gfp, 3);
                lame_set_quality(gfp, 2);
                lame_set_lowpassfreq(gfp, 19000);
                lame_set_mode(gfp, JOINT_STEREO);
                lame_set_VBR_min_bitrate_kbps(gfp, 128);
                return preset;
           }
    case EXTREME: {
                lame_set_VBR(gfp, vbr_rh);

                lame_set_preset_expopts(gfp, 2);
                lame_set_quality(gfp, 2);
                lame_set_lowpassfreq(gfp, 19500);
                lame_set_mode(gfp, JOINT_STEREO);
                lame_set_VBR_min_bitrate_kbps(gfp, 128);					
                return preset;
           }
    case EXTREME_FAST: {
                lame_set_VBR(gfp, vbr_mtrh);

                lame_set_preset_expopts(gfp, 2);
                lame_set_quality(gfp, 2);
                lame_set_lowpassfreq(gfp, 19500);
                lame_set_mode(gfp, JOINT_STEREO);
                lame_set_VBR_min_bitrate_kbps(gfp, 128);					
                return preset;
           }
    case INSANE: {
                lame_set_preset_expopts(gfp, 1);
                lame_set_brate(gfp, 320);
                lame_set_quality(gfp, 2);
                lame_set_mode(gfp, JOINT_STEREO);
                lame_set_lowpassfreq(gfp, 20500);
                return preset;
           }
    case R3MIX: {
                lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | 1); /*nspsytune*/
                /*  lame_set_experimentalX(gfp,1); (test CVS) */

                (void) lame_set_scale( gfp, 0.98 ); /* --scale 0.98*/

                lame_set_exp_nspsytune(gfp, lame_get_exp_nspsytune(gfp) | (8 << 20));

                lame_set_VBR(gfp,vbr_mtrh); 
                lame_set_VBR_q(gfp,1);
                lame_set_quality( gfp, 2 );
                lame_set_lowpassfreq(gfp,19500);
                lame_set_mode( gfp, JOINT_STEREO );
                lame_set_ATHtype( gfp, 3 );
                lame_set_VBR_min_bitrate_kbps(gfp,96);
                return preset;
           }
    default: break;
    };

    if ((preset >= 8) && (preset <=320))
        return apply_abr_preset(gfp, preset);


    return preset;
}



