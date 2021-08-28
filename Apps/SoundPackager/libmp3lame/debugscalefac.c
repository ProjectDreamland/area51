/* $Id: debugscalefac.c,v 1.5 2001/01/05 15:20:33 aleidinger Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif

{
/*******************************************************************

DEBUG INFORMATION 
criticalbands (scalefactor bands)
partition bands
FFT bands
MDCT bands 

*******************************************************************/

#include "tables.h"
 int *scalefac_band_long; 
 int *scalefac_band_short;
 FLOAT fstart,fstop;
 FLOAT mstart,mstop;
 

 scalefac_band_long  = &sfBandIndex[info->sampling_frequency + (info->version * 3)].l[0];
 scalefac_band_short = &sfBandIndex[info->sampling_frequency + (info->version * 3)].s[0];

 DEBUGF("\n\n long block partition bands \n");
 for (b = 0; b<CBANDS; b++) {
   /* look for FFT partition band bu_l[b]*/
   for (i=0;i<HBLKSIZE;i++, fstart=i) {
     if (partition_l[i]==b) break;
   }
   fstop=0;
   for (i=HBLKSIZE-1;i>=0;i--, fstart=i) {
     if (partition_l[i]==b) break;
   }

   if(fstart>0) fstart -= .5;
   fstop += .5;
   DEBUGF("pb=%2i  FFT=(%4.1f,%4.1f)  numlines=%i  norm=%f  \n",
         b,fstart,fstop,numlines_l[b],norm_l[b]);

 }


 
 DEBUGF("\n\n long block critial bands \n");
 DEBUGF("crit band,part_band, part_band range  FFT range MDCT range \n");
 for ( b = 0;b < SBPSY_l; b++ ) {

   /* look for FFT partition band bu_l[b]*/
   for (i=0;i<HBLKSIZE;i++, fstart=i) {
     if (partition_l[i]==bu_l[b]) break;
   }
   for (i=HBLKSIZE-1;i>=0;i--, fstop=i) {
     if (partition_l[i]==bo_l[b]) break;
   }


   /* numlines_l[pb] = number of FFT lines in partition band pb */
   /* w1 = 1, then all FFT lines belong to this partition band */
   fstart += ( (1-w1_l[b])*numlines_l[ bu_l[b] ] ) ;

   /* w2 = 1, then all FFT lines belong to this partition band */
   fstop -= ( (1-w2_l[b])*numlines_l[ bo_l[b] ] ) ;


   if (fstart>0) fstart -= .5;  /* contribution extends to center of freq band */
   fstop += .5;

   mstart = scalefac_band_long[b];
   mstop  = scalefac_band_long[b+1]-1;
   if (mstart>0) mstart -= .5;
   mstop += .5;

   DEBUGF("cb=%2i(%2i)(%2i,%2i)(%3.2f,%3.2f)  FFT:(%4.1f,%4.1f)(%5.0f,%5.0f)Hz   MDCT:(%5.1f,%5.1f)(%5.0f,%5.0f)Hz \n",
           b,npart_l_orig,bu_l[b],bo_l[b],w1_l[b],w2_l[b],
	  fstart,fstop,sfreq*fstart/BLKSIZE,sfreq*fstop/BLKSIZE,
	  mstart,mstop,mstart*.5*sfreq/576,mstop*.5*sfreq/576
           );
 }





 DEBUGF("\n\n short block partition bands \n");
 for (b = 0; b<CBANDS; b++) {
   /* look for FFT partition band bu_s[b]*/
   for (i=0;i<HBLKSIZE_s;i++,fstart=i) {
     if (partition_s[i]==b) break;
   }
   for (i=HBLKSIZE_s-1; i>=0; i--,fstop=i) {
     if (partition_s[i]==b) break;
   }

   if(fstart>0) fstart -= .5;
   fstop += .5;
   DEBUGF("pb=%2i  FFT=(%4.1f,%4.1f)  numlines=%i norm=%f \n",
	  b,fstart,fstop,numlines_s[b],norm_s[b]);

 }


 
 DEBUGF("\n\n short block critial bands \n");
 DEBUGF("crit band,part_band, part_band range  FFT range MDCT range \n");
 for ( b = 0;b < SBPSY_s; b++ ) {

   /* look for FFT partition band bu_s[b]*/
   for (i=0;i<HBLKSIZE_s;i++)  
     if (partition_s[i]==bu_s[b]) {fstart=i;break;}
   for (i=HBLKSIZE_s-1;i>=0;i--) 
     if (partition_s[i]==bo_s[b]) {fstop=i; break;}


   /* numlines_s[pb] = number of FFT lines in partition band pb */
   /* w1 = 1, then all FFT lines belong to this partition band */
   fstart += ( (1-w1_s[b])*numlines_s[ bu_s[b] ] ) ;

   /* w2 = 1, then all FFT lines belong to this partition band */
   fstop -= ( (1-w2_s[b])*numlines_s[ bo_s[b] ] ) ;


   if (fstart>0) fstart -= .5;  /* contribution extends to center of freq band */
   fstop += .5;

   mstart = scalefac_band_short[b];
   mstop  = scalefac_band_short[b+1]-1;
   if (mstart>0) mstart -= .5;
   mstop += .5;

   DEBUGF("cb=%2i(%2i)(%2i,%2i)(%3.2f,%3.2f)  FFT:(%4.1f,%4.1f)(%5.0f,%5.0f)Hz   MDCT:(%5.1f,%5.1f)(%5.0f,%5.0f)Hz \n",
           b,npart_s_orig,bu_s[b],bo_s[b],w1_s[b],w2_s[b],
	  fstart,fstop,sfreq*fstart/BLKSIZE_s,sfreq*fstop/BLKSIZE_s,
	  mstart,mstop,mstart*.5*sfreq/192,mstop*.5*sfreq/192
           );
 }


/*******************************************************************
END DEBUG INFORMATION 
*******************************************************************/
     }
