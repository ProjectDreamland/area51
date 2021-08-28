/*
 *	Layer 3 side include file
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

#ifndef LAME_L3SIDE_H
#define LAME_L3SIDE_H

#include "encoder.h"
#include "machine.h"

/* Layer III side information. */

typedef struct 
{
   int l[1+SBMAX_l];
   int s[1+SBMAX_s];
} scalefac_struct;


typedef struct {
	FLOAT8	l[SBMAX_l];
	FLOAT8	s[SBMAX_s][3];
} III_psy_xmin;

typedef struct {
    III_psy_xmin thm;
    III_psy_xmin en;
} III_psy_ratio;

/* Layer III scale factors. */
/* note: there are only SBPSY_l=(SBMAX_l-1) and SBPSY_s=(SBMAX_s-1) scalefactors.
 * for the faster address calculation, use SBMAX_l/SBMAX_s instead of them.
 * (it will be the same calculation of III_psy_ratio, etc */

typedef struct {
	int l[SBMAX_l];            /* [cb] */
	int s[SBMAX_s][3];         /* [window][cb] */
} III_scalefac_t;  /* [gr][ch] */

typedef struct {
    FLOAT8 xr[576];
    int l3_enc[576];
    III_scalefac_t scalefac;

    int part2_3_length;
    int big_values;
    int count1;
    int global_gain;
    int scalefac_compress;
    int block_type;
    int mixed_block_flag;
    int table_select[3];
    int subblock_gain[3];
    int region0_count;
    int region1_count;
    int preflag;
    int scalefac_scale;
    int count1table_select;

    int part2_length;
    int sfb_lmax;
    int sfb_smin;
    int psy_lmax;
    int psy_smax;
    int count1bits;
    /* added for LSF */
    const int *sfb_partition_table;
    int slen[4];
} gr_info;

typedef struct {
	int main_data_begin; 
	int private_bits;
	int resvDrain_pre;
	int resvDrain_post;
	int scfsi[2][4];
	gr_info tt[2][2];
} III_side_info_t;

#endif
