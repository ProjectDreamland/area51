/*
    ReplayGainAnalysis v0.9 - routines to analyze input samples and give the recommended dB change
    Copyright (C) 2001  David Robinson and Glen Sawyer

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA




   concept and filter values by David Robinson ( David@Robinson.org )
       --blame him if you think the idea is flawed

   coding by Glen Sawyer ( glensawyer@hotmail.com )
   442 N 700 E, Provo, UT  84606  USA
       --blame him if you think this runs too slowly, or the coding is
	     otherwise flawed

   For an explanation of the concepts and the basic algorithms involved, go to
   http://www.replaygain.org

*/


/* 
   Here's the deal:
   call InitGainAnalysis(double sampling_frequency_in_kHz) to
   initialize everything

   call AnalyzeSamples(double *left_samples, double *right_samples,
                       long num_samples, int num_channels)
   as many times as you want, with as many or as few samples as you
   want (up to 80 minutes' worth of samples). If mono, pass the
   sample buffer in through left_samples, leave right_samples NULL,
   and make sure num_channels = 1.

   GetRadioGain() will return the recommended dB level change for
   all samples analyzed SINCE THE LAST TIME you called GetRadioGain
   OR GetAudiophileGain

   GetAudiophileGain() will return the recommended dB level change
   for all samples analyzed since InitGainAnalysis was called.

   Pseudo-code to process an album:

		double l_samples[50000], r_samples[50000];
		long num_samples;

		InitGainAnalysis(44.1);
		for (i = 1; i <= num_songs; i++) {
			getSongSamples(song[i],left_samples, right_samples, num_samples);

			AnalyzeSamples(left_samples,right_samples,num_samples,2);

			fprintf("Recommened dB change for song %d: %f",i,GetRadioGain());
		}
		fprintf("Recommended dB change for whole album: %f",GetAudiophileGain());

*/

/*
   So here's the main source of potential code confusion: the filters applied
   to the incoming samples are IIR filters, meaning they rely on up to
   <filter order> number of previous samples AND up to <filter order>
   number of previous filtered samples.
   I set up the AnalyzeSamples routine to minimize memory usage and interface
   complexity. The speed isn't compromised too much (I don't think), but the
   internal complexity is higher than it should be for such a relatively
   simple routine.
   Optimization/clarity suggestions are welcome.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "gain_analysis.h"

#define MAXORDER 10
#define YULEORDER 10
#define BUTTERORDER 2
#define RMSPERCENT 0.95
#define RMSWINDOW 50.0
#define MAXSAMPLES 2400
/*NOTE: MAXSAMPLES = RMSWINDOW * max kHz(48) -- if you change RMSWINDOW,
  you MUST change MAXSAMPLES!*/

#define RMSPERCENT 0.95
#define MAXRMS 216000
/* once this code was working correctly, I ran ref_pink.wav through to get
   the following reference level */
#define PINKREF 58.846298640883795

double linprebuf[MAXORDER * 2], *linpre; /* left input samples, with pre-buffer */
double lstepbuf[MAXSAMPLES + MAXORDER], *lstep; /* left "first step" (i.e. post first filter) samples */
double loutbuf[MAXSAMPLES + MAXORDER], *lout; /* left "out" (i.e. post second filter) samples */
double rinprebuf[MAXORDER * 2], *rinpre; /* right input samples... */
double rstepbuf[MAXSAMPLES + MAXORDER], *rstep;
double routbuf[MAXSAMPLES + MAXORDER], *rout;
long sampleWindow; /* number of samples required to reach number of milliseconds required for RMS window */
long totsamp;
double lsum, rsum;
long rmscount;
long prevsort;
double rms[MAXRMS];
int freqindex;
int first;
int sizeofdouble;
int maxorderXsizeofdouble;

FILE *dump;

/* for each filter, [0] is 48000, [1] is 44100, [2] is 32000, 
                    [3] is 24000, [4] is 22050, [5] is 16000, 
					[6] is 12000, [7] is 11025, [8] is  8000 */
const double AYule[9][11] = {
	{  1.00000000000000,
	  -3.84664617118067,
	   7.81501653005538,
	 -11.34170355132042,
	  13.05504219327545,
	 -12.28759895145294,
	   9.48293806319790,
	  -5.87257861775999,
	   2.75465861874613,
	  -0.86984376593551,
	   0.13919314567432 
	},
	{  1.00000000000000,
	  -3.47845948550071,
	   6.36317777566148,
	  -8.54751527471874,
	   9.47693607801280,
	  -8.81498681370155,
	   6.85401540936998,
	  -4.39470996079559,
	   2.19611684890774,
	  -0.75104302451432,
	  0.13149317958808 
	},
	{  1.00000000000000,
	  -2.37898834973084,
	   2.84868151156327,
	  -2.64577170229825,
	   2.23697657451713,
	  -1.67148153367602,
	   1.00595954808547,
	  -0.45953458054983,
	   0.16378164858596,
	  -0.05032077717131,
	   0.02347897407020 
	},
	{  1.00000000000000,
	  -1.61273165137247,
	   1.07977492259970,
	  -0.25656257754070,
	  -0.16276719120440,
	  -0.22638893773906,
	   0.39120800788284,
	  -0.22138138954925,
	   0.04500235387352,
	   0.02005851806501,
	   0.00302439095741
	},
	{  1.00000000000000,
	  -1.49858979367799,
	   0.87350271418188,
	   0.12205022308084,
	  -0.80774944671438,
	   0.47854794562326,
	  -0.12453458140019,
	  -0.04067510197014,
	   0.08333755284107,
	  -0.04237348025746,
	   0.02977207319925
	},
	{  1.00000000000000,
	  -0.62820619233671,
	   0.29661783706366,
	  -0.37256372942400,
	   0.00213767857124,
	  -0.42029820170918,
	   0.22199650564824,
	   0.00613424350682,
	   0.06747620744683,
	   0.05784820375801,
	   0.03222754072173
	},
	{  1.00000000000000,
	  -1.04800335126349,
	   0.29156311971249,
	  -0.26806001042947,
	   0.00819999645858,
	   0.45054734505008,
	  -0.33032403314006,
	   0.06739368333110,
	  -0.04784254229033,
	   0.01639907836189,
	   0.01807364323573
	},
	{  1.00000000000000,
	  -0.51035327095184,
	  -0.31863563325245,
	  -0.20256413484477,
	   0.14728154134330,
	   0.38952639978999,
	  -0.23313271880868,
	  -0.05246019024463,
	  -0.02505961724053,
	   0.02442357316099,
	   0.01818801111503
	},
	{  1.00000000000000,
	  -0.25049871956020,
	  -0.43193942311114,
	  -0.03424681017675,
	  -0.04678328784242,
	   0.26408300200955,
	   0.15113130533216,
	  -0.17556493366449,
	  -0.18823009262115,
	   0.05477720428674,
	   0.04704409688120
	  }

};
const double BYule[9][11] = {
	{  0.03857599435200,
	  -0.02160367184185,
	  -0.00123395316851,
	  -0.00009291677959,
	  -0.01655260341619,
	   0.02161526843274,
	  -0.02074045215285,
	   0.00594298065125,
	   0.00306428023191,
	   0.00012025322027,
	   0.00288463683916
	},
	{  0.05418656406430,
	  -0.02911007808948,
	  -0.00848709379851,
	  -0.00851165645469,
	  -0.00834990904936,
	   0.02245293253339,
	  -0.02596338512915,
	   0.01624864962975,
	  -0.00240879051584,
	   0.00674613682247,
	  -0.00187763777362
	},
	{  0.15457299681924,
	  -0.09331049056315,
	  -0.06247880153653,
	   0.02163541888798,
	  -0.05588393329856,
	   0.04781476674921,
	   0.00222312597743,
	   0.03174092540049,
	  -0.01390589421898,
	   0.00651420667831,
	  -0.00881362733839
	},
	{  0.30296907319327,
	  -0.22613988682123,
	  -0.08587323730772,
	   0.03282930172664,
	  -0.00915702933434,
	  -0.02364141202522,
	  -0.00584456039913,
	   0.06276101321749,
	  -0.00000828086748,
	   0.00205861885564,
	  -0.02950134983287
	},
	{  0.33642304856132,
	  -0.25572241425570,
	  -0.11828570177555,
	   0.11921148675203,
	  -0.07834489609479,
	  -0.00469977914380,
	  -0.00589500224440,
	   0.05724228140351,
	   0.00832043980773,
	  -0.01635381384540,
	  -0.01760176568150
	},
	{  0.44915256608450,
	  -0.14351757464547,
	  -0.22784394429749,
	  -0.01419140100551,
	   0.04078262797139,
	  -0.12398163381748,
	   0.04097565135648,
	   0.10478503600251,
	  -0.01863887810927,
	  -0.03193428438915,
	   0.00541907748707
	},
	{  0.56619470757641,
	  -0.75464456939302,
	   0.16242137742230,
	   0.16744243493672,
	  -0.18901604199609,
	   0.30931782841830,
	  -0.27562961986224,
	   0.00647310677246,
	   0.08647503780351,
	  -0.03788984554840,
	  -0.00588215443421
	},
	{  0.58100494960553,
	  -0.53174909058578,
	  -0.14289799034253,
	   0.17520704835522,
	   0.02377945217615,
	   0.15558449135573,
	  -0.25344790059353,
	   0.01628462406333,
	   0.06920467763959,
	  -0.03721611395801,
	  -0.00749618797172
	},
	{  0.53648789255105,
	  -0.42163034350696,
	  -0.00275953611929,
	   0.04267842219415,
	  -0.10214864179676,
	   0.14590772289388,
	  -0.02459864859345,
	  -0.11202315195388,
	  -0.04060034127000,
	   0.04788665548180,
	  -0.02217936801134
	}

};
const double AButter[9][3] = { 
	{  1.00000000000000,
	  -1.97223372919527,
	   0.97261396931306
	},
	{  1.00000000000000,
	  -1.96977855582618,
	   0.97022847566350
	},
	{  1.00000000000000,
	  -1.95835380975398,
	   0.95920349965459
	},
	{  1.00000000000000,
	  -1.95002759149878,
	   0.95124613669835
	},
	{  1.00000000000000,
	  -1.94561023566527,
	   0.94705070426118
	},
	{  1.00000000000000,
	  -1.92783286977036,
	   0.93034775234268
	},
	{  1.00000000000000,
	  -1.91858953033784,
	   0.92177618768381
	},
	{  1.00000000000000,
	  -1.91542108074780,
	   0.91885558323625
	},
	{  1.00000000000000,
	  -1.88903307939452,
	   0.89487434461664
	}
};
const double BButter[9][3] = {
	{  0.98621192462708,
	  -1.97242384925416,
	   0.98621192462708
	},
	{  0.98500175787242,
	  -1.97000351574484,
	   0.98500175787242
	},
	{  0.97938932735214,
	  -1.95877865470428,
	   0.97938932735214
	},
	{  0.97531843204928,
	  -1.95063686409857,
	   0.97531843204928
	},
	{  0.97316523498161,
	  -1.94633046996323,
	   0.97316523498161
	},
	{  0.96454515552826,
	  -1.92909031105652,
	   0.96454515552826
	},
	{  0.96009142950541,
	  -1.92018285901082,
	   0.96009142950541
	},
	{  0.95856916599601,
	  -1.91713833199203,
	   0.95856916599601
	},
	{  0.94597685600279,
	  -1.89195371200558,
	   0.94597685600279
	}
};




/* didn't bother trying to optimize qsort, because it's only called once per analyzed file */
int dblcomp( const void *arg1, const void *arg2 )
{	
	return (*(double*)arg1 > *(double*)arg2 ? 1 : *(double*)arg1 == *(double*)arg2 ? 0 : -1);
};


void Filter(double *ip, double *op, long nSamples, const double *a, const double *b, int order) {
	register double y;
	register long i,k;

	/* When calling this procedure, make sure that ip[-order] and op[-order] point to real data! */
    for (i = 0; i < nSamples; i++) {
        y = b[0]*ip[i];
        for (k=1; k<=order; k++)
            y += (b[k]*ip[i-k]) - (a[k]*op[i-k]);
        op[i] = y;
    }

}

/* returns a INIT_GAIN_ANALYSIS_OK if successful, INIT_GAIN_ANALYSIS_ERROR if not */
int InitGainAnalysis(double sampling_kHz) {
	int i;

	if (sampling_kHz > 48.0) {
		fprintf(stderr,"Sorry, can't do greater than 48 kHz samples at this point. If you need this capability, contact the author.\n");
		return INIT_GAIN_ANALYSIS_ERROR;
	}
	/* zero out initial values */
	for (i = 0; i < MAXORDER; i++)
		linprebuf[i] = lstepbuf[i] = loutbuf[i] = rinprebuf[i] = rstepbuf[i] = routbuf[i] = 0.0;

/* for each filter, [0] is 48000, [1] is 44100, [2] is 32000, 
                    [3] is 24000, [4] is 22050, [5] is 16000, 
					[6] is 12000, [7] is 11025, [8] is  8000 */
	switch ((int)(sampling_kHz*1000.0)) {
		case 48000:
			freqindex = 0;
			break;
		case 44100:
			freqindex = 1;
			break;
		case 32000:
			freqindex = 2;
			break;
		case 24000:
			freqindex = 3;
			break;
		case 22050:
			freqindex = 4;
			break;
		case 16000:
			freqindex = 5;
			break;
		case 12000:
			freqindex = 6;
			break;
		case 11025:
			freqindex = 7;
			break;
		case 8000:
			freqindex = 8;
			break;
		default:
			fprintf(stderr, "No filter for this frequency. Using 44.1 kHz filter\n");
			freqindex = 1;
			break;
	}

	sampleWindow = (int)(ceil(sampling_kHz * RMSWINDOW));
	
	linpre = linprebuf+MAXORDER;
	rinpre = rinprebuf+MAXORDER;
	lstep = lstepbuf+MAXORDER;
	rstep = rstepbuf+MAXORDER;
	lout = loutbuf+MAXORDER;
	rout = routbuf+MAXORDER;

	lsum = 0.0;
	rsum = 0.0;
	totsamp = 0;
	rmscount = 0;
	prevsort = 0;
	first = !0;
	sizeofdouble = sizeof(double);
	maxorderXsizeofdouble = MAXORDER * sizeof(double);

	return INIT_GAIN_ANALYSIS_OK;
};


/* returns GAIN_ANALYSIS_OK if successful, GAIN_ANALYSIS_ERROR if not */
int AnalyzeSamples(double *left_samples, double *right_samples, long num_samples, int num_channels) {
	double *curleft, *curright;
	long batchsamples, cursamples, cursamplepos;
	register int i;

	if (num_samples == 0) return GAIN_ANALYSIS_OK;
	if (rmscount >= MAXRMS) {
		fprintf(stderr,"ERROR: Maximum music length (180 minutes) reached.\n");
		/* if you change MAXRMS or RMSWINDOW, change this error message to reflect
		   the appropriate allowable music length */
		return GAIN_ANALYSIS_ERROR;
	}

	cursamplepos = 0;
	batchsamples = num_samples;
	/* yeah, lots of overlapping pieces of code for 1-channel vs. 2-channel,
	   but this way there's only one test for number of channels */

	if (num_channels == 2) {

		if (num_samples < MAXORDER) {
			memcpy(linprebuf+MAXORDER,left_samples,num_samples*sizeofdouble);
			memcpy(rinprebuf+MAXORDER,right_samples,num_samples*sizeofdouble);
		}
		else {
			memcpy(linprebuf+MAXORDER,left_samples,maxorderXsizeofdouble);
			memcpy(rinprebuf+MAXORDER,right_samples,maxorderXsizeofdouble);
		}

		while (batchsamples > 0) {
			cursamples = batchsamples > (sampleWindow-totsamp) ? (sampleWindow-totsamp) : batchsamples;
			if (cursamplepos < MAXORDER) {
				curleft = linpre+cursamplepos;
				curright = rinpre+cursamplepos;
				if (cursamples > (MAXORDER - cursamplepos))
					cursamples = MAXORDER - cursamplepos;
			}
			else {
				curleft = left_samples+cursamplepos;
				curright = right_samples+cursamplepos;
			}

			Filter(curleft,lstep+totsamp,cursamples,AYule[freqindex],BYule[freqindex],YULEORDER);
			Filter(curright,rstep+totsamp,cursamples,AYule[freqindex],BYule[freqindex],YULEORDER);

			Filter(lstep+totsamp,lout+totsamp,cursamples,AButter[freqindex],BButter[freqindex],BUTTERORDER);
			Filter(rstep+totsamp,rout+totsamp,cursamples,AButter[freqindex],BButter[freqindex],BUTTERORDER);

			for (i = 0; i < cursamples; i++) {
				/* Get the squared values */
				lsum = lsum + (lout[totsamp+i] * lout[totsamp+i]);
				rsum = rsum + (rout[totsamp+i] * rout[totsamp+i]);
			}


			batchsamples -= cursamples;
			cursamplepos += cursamples;
			totsamp += cursamples;
			if (totsamp == sampleWindow) {
				/* Get the Root Mean Square (RMS) for this set of samples */
				rms[rmscount] = 10.0 * log10((((lsum+rsum)/(double)totsamp) / 2.0)+0.0000000001);
				rmscount++;
				lsum = 0;
				rsum = 0;
				memmove(loutbuf,loutbuf+totsamp,maxorderXsizeofdouble);
				memmove(routbuf,routbuf+totsamp,maxorderXsizeofdouble);
				memmove(lstepbuf,lstepbuf+totsamp,maxorderXsizeofdouble);
				memmove(rstepbuf,rstepbuf+totsamp,maxorderXsizeofdouble);
				totsamp = 0;
			}
			if (totsamp > sampleWindow) {
				/* somehow I really screwed up! */
				fprintf(stderr,"Error in programming! Contact author about totsamp > sampleWindow");
				return GAIN_ANALYSIS_ERROR;
			}
		}
		if (num_samples < MAXORDER) {
			memmove(linprebuf,linprebuf+num_samples,(MAXORDER-num_samples)*sizeofdouble);
			memcpy(linprebuf+MAXORDER-num_samples,left_samples,num_samples*sizeofdouble);
			memmove(rinprebuf,rinprebuf+num_samples,(MAXORDER-num_samples)*sizeofdouble);
			memcpy(rinprebuf+MAXORDER-num_samples,right_samples,num_samples*sizeofdouble);
		}
		else {
			memcpy(linprebuf,left_samples+num_samples-MAXORDER,maxorderXsizeofdouble);
			memcpy(rinprebuf,right_samples+num_samples-MAXORDER,maxorderXsizeofdouble);
		}
	}
	else {

		if (num_samples < MAXORDER) {
			memcpy(linprebuf+MAXORDER,left_samples,num_samples*sizeofdouble);
		}
		else {
			memcpy(linprebuf+MAXORDER,left_samples,maxorderXsizeofdouble);
		}

		while (batchsamples > 0) {
			cursamples = batchsamples > (sampleWindow-totsamp) ? (sampleWindow-totsamp) : batchsamples;
			if (cursamplepos < MAXORDER) {
				curleft = linpre+cursamplepos;
				if (cursamples > (MAXORDER - cursamplepos))
					cursamples = MAXORDER - cursamplepos;
			}
			else {
				curleft = left_samples+cursamplepos;
			}

			Filter(curleft,lstep+totsamp,cursamples,AYule[freqindex],BYule[freqindex],YULEORDER);

			Filter(lstep+totsamp,lout+totsamp,cursamples,AButter[freqindex],BButter[freqindex],BUTTERORDER);

			for (i = 0; i < cursamples; i++) {
				lsum = lsum + (lout[totsamp+i] * lout[totsamp+i]);
			}


			batchsamples -= cursamples;
			cursamplepos += cursamples;
			totsamp += cursamples;
			if (totsamp == sampleWindow) {
				rms[rmscount] = 10.0 * log10((lsum/(double)totsamp)+0.0000000001);
				rmscount++;
				lsum = 0;
				memmove(loutbuf,loutbuf+totsamp,maxorderXsizeofdouble);
				memmove(lstepbuf,lstepbuf+totsamp,maxorderXsizeofdouble);
				totsamp = 0;
			}
			if (totsamp > sampleWindow) {
				fprintf(stderr,"Error in programming! Contact author about totsamp > sampleWindow");
				return GAIN_ANALYSIS_ERROR;
			}
		}
		if (num_samples < MAXORDER) {
			memmove(linprebuf,linprebuf+num_samples,(MAXORDER-num_samples)*sizeofdouble);
			memcpy(linprebuf+MAXORDER-num_samples,left_samples,num_samples*sizeofdouble);
		}
		else {
			memcpy(linprebuf,left_samples+num_samples-MAXORDER,maxorderXsizeofdouble);
		}
	}

	return GAIN_ANALYSIS_OK;
};

double GetRadioGain() {
	double retval;
	int i;
	

	if (prevsort == rmscount)
		return GAIN_NOT_ENOUGH_SAMPLES;


	qsort((void*)(rms+prevsort),rmscount-prevsort,sizeof(double),dblcomp);
	retval = PINKREF - rms[prevsort + (long)(RMSPERCENT*(rmscount-prevsort)) - 1];

	prevsort = rmscount;
	for (i = 0; i < MAXORDER; i++)
		linprebuf[i] = lstepbuf[i] = loutbuf[i] = rinprebuf[i] = rstepbuf[i] = routbuf[i] = 0.0;
	totsamp = 0;
	lsum = 0.0;
	rsum = 0.0;

	return retval;
};

double GetAudiophileGain() {
	if (rmscount == 0)
		return GAIN_NOT_ENOUGH_SAMPLES;

	qsort((void*)rms,rmscount,sizeof(double),dblcomp);
	prevsort = rmscount;
	return PINKREF - rms[(long)(RMSPERCENT*rmscount) - 1];
};
