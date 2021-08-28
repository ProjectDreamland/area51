//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: FLT module for Reverb2                                       ##
//##                                                                        ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 5-Feb-99: Initial                                     ##
//##                                                                        ##
//##  Author: John Miles, Nick Skrepetos                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "mss.h"
#include "imssapi.h"

// Pentium Floating Point Processor Denormal Protect Value
#define  _FX_DENORMVAL     0.0000000001F

// PI definition
#define  M_PI  3.14159265F

// Maximum time (seconds)
#define  _FX_MAX_TIME            8000.0F
#define  _FX_MAX_PREDELAY        250.0F

// Reverb Defaults
#define  _FX_DEFAULT_TIME        3000.0F
#define  _FX_DEFAULT_PREDELAY    0.0F
#define  _FX_DEFAULT_MIX         0.3F

// Infinite Audio Delay Line
typedef  struct   _FXDELAYLINE
         {
            // pointer to buffer
            F32*   lpBuffer;

            // input location
            U32    dwInput;

            // output loction
            U32    dwOutput;

            // length of line, in samples
            U32    dwLength;

            // last output sample
            F32    fLastOutput;

            // coefficient
            F32    fCoef;

         } FXDELAYLINE, *LPFXDELAYLINE;

// Infinite Audio Reverb2 Comb/AllPass Filters
#define  _FX_REVERB2_DELAYS   6

// global reverb table for time of the comb/all pass
// filters
static   F32 sReverb2Table[] =
         {
            0.0297F,
            0.0371F,
            0.0411F,
            0.0437F,
            0.005F,
            0.0017F
         };

// all pass reverb times
static   F32 sReverb2AllPass[]  =
         {
            0.0968535F,
            0.032924F
         };

// Clip Range Macro
#define  FX_CLIPRANGE( p, min, max ) \
            if ( (p) > (max) ) \
               (p) = (max); \
            else \
               if ( (p) < (min) ) \
                  (p) = (min);

// global buffer
static   S32*   dwTempBuild=0;
static   U32    dwLastBytes=0;

//
// Attribute tokens
//

enum ATTRIB
{
   //
   // Additional filter attribs (beyond standard RIB PROVIDER_ attributes)
   //
   _FX_REVERB2_TIME,
   _FX_REVERB2_PREDELAY,
   _FX_REVERB2_MIX,
};

//
// Driver state descriptor
//
// One state descriptor is associated with each HDIGDRIVER
//
struct DRIVERSTATE
{
   //
   // Members common to all pipeline filter providers
   //

   HDIGDRIVER       dig;                  // Driver with which this descriptor is associated

   S32 FAR         *build_buffer;         // Location and size of driver's mixer output buffer
   S32              build_buffer_size;

   //
   // Members associated with specific filter provider
   //

   F32 rate;
};

//
// Per-sample filter state descriptor
//
// One state descriptor is associated with each HSAMPLE
//
struct SAMPLESTATE
{
   //
   // Members common to all pipeline filter providers
   //

   HSAMPLE          sample;   // HSAMPLE with which this descriptor is associated
   DRIVERSTATE FAR *driver;   // Driver with which this descriptor is associated

   //
   // Members associated with specific filter provider
   //

   // reverb time
   F32    fReverbTime;

   // pre delay time
   F32    fPreDelay;

   // bytes of maximum delay line
   U32    dwDelayLineBytes;

   // delay lines for reverb
   FXDELAYLINE sDelayLines[ _FX_REVERB2_DELAYS ];

   // pre delay bytes to clear
   U32    dwPreDelayLineBytes;

   // pre-delay line
   FXDELAYLINE sPreDelayLine;

   // sample rate
   F32       fRate;

   // mix
   F32       fMix;
};

//
// Globals
//

S32 FLT_started = 0;

C8 FLT_error_text[256];

//############################################################################
//#                                                                          #
//# Calculate coefficients and values based on parameter set                 #
//#                                                                          #
//############################################################################
static void  FXCalcParams( SAMPLESTATE FAR * SS, HSAMPLE  S )
   {
      F32          fRate;
      U32          dwIndex;
      F32          fTemp;
      F32          fSamples;
      F32          fRT;

      static   int  fFirstTime  =  1;

      // get sample rate
      fRate =  SS->driver->rate;

      // get reverb time
      fRT   =  SS->fReverbTime / 1000.0F;

      // calculate samples for reverb time
      fSamples =  fRate;

      // reset all delay lines to silence
      for( dwIndex = 0; dwIndex < _FX_REVERB2_DELAYS; dwIndex++ )
      {
         // calculate delay line length
         fTemp =  fSamples * sReverb2Table[ dwIndex ];

         // set delay line length
         SS->sDelayLines[ dwIndex ].dwLength =  (U32)fTemp;

         // check if first time
         if ( fFirstTime )
         {
            // clear delay line buffer
            memset( SS->sDelayLines[ dwIndex ].lpBuffer, 0, (U32)fTemp * sizeof( F32 ) );

            // set input index location
            SS->sDelayLines[ dwIndex ].dwInput  =  0;
         }

         // set coef for reverb time
         if ( dwIndex < 4 )
            SS->sDelayLines[ dwIndex ].fCoef =  (F32)pow( .001F, sReverb2Table[ dwIndex ] / fRT );
         else
            SS->sDelayLines[ dwIndex ].fCoef =  (F32)pow( .001F, sReverb2Table[ dwIndex ] / sReverb2AllPass[ dwIndex - 4 ] );
      }

      // calculate samples for pre-delay line
      fSamples =  ( SS->fPreDelay / 1000.0F ) * fRate;

      // set samples
      SS->sPreDelayLine.dwLength =  (U32)fSamples;

      // check if first time
      if ( fFirstTime )
      {
         // clear predelay line
         memset( SS->sPreDelayLine.lpBuffer, 0, SS->dwPreDelayLineBytes );

         // reset predelay index
         SS->sPreDelayLine.dwInput  =  0;
      }

      // reset first time flag
      fFirstTime  =  FALSE;
   }

// handle delay line
static F32	inline FXReverbDelayTick( LPFXDELAYLINE lpsDelay, F32 fSample )
	{
		// sample sample and advance input point
      lpsDelay->lpBuffer[ lpsDelay->dwInput++ ] =  fSample;

      // check for wrap around
      if ( lpsDelay->dwInput == lpsDelay->dwLength )
         lpsDelay->dwInput =  0;

      // set last output value
      lpsDelay->fLastOutput   =  lpsDelay->lpBuffer[ lpsDelay->dwOutput++ ];

      // check for wrap around
      if ( lpsDelay->dwOutput == lpsDelay->dwLength )
         lpsDelay->dwOutput   =  0;

      // return last output sample
      return( lpsDelay->fLastOutput );
	}

//############################################################################
//#                                                                          #
//# Return floating-point type as unsigned long U32 (without actually      #
//# converting the value)                                                    #
//#                                                                          #
//############################################################################

static U32 float_as_long(F32 FP)
{
   static U32 val;

   *(F32 FAR *) (&val) = FP;

   return val;
}

//############################################################################
//#                                                                          #
//# Return signed long U32 as single-precision float (without actually     #
//# converting the value)                                                    #
//#                                                                          #
//############################################################################

static F32 long_as_float(S32 integer)
{
   static F32 val;

   *(S32 FAR *) (&val) = integer;

   return val;
}

//############################################################################
//#                                                                          #
//# Retrieve a standard RIB provider attribute by index                      #
//#                                                                          #
//############################################################################

static U32 AILEXPORT PROVIDER_query_attribute (HATTRIB index)
{
   switch ((ATTRIB) index)
      {
      case PROVIDER_NAME:      return (U32) "Reverb2 Filter";
      case PROVIDER_VERSION:   return 0x100;
      }

   return 0;
}

//############################################################################
//#                                                                          #
//# Set provider preference value, returning previous setting                #
//#                                                                          #
//############################################################################

static S32 AILEXPORT FLT_set_provider_preference (HATTRIB    preference, //)
                                           void FAR*  value)
{
   S32 prev = -1;

//   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //
      }

   return prev;
}

//############################################################################
//#                                                                          #
//# Return FLT filter error text                                             #
//#                                                                          #
//############################################################################

static C8 FAR *       AILEXPORT FLT_error       (void)
{
   if (!AIL_strlen(FLT_error_text))
      {
      return NULL;
      }

   return FLT_error_text;
}

//############################################################################
//#                                                                          #
//# Initialize FLT sample filter                                             #
//#                                                                          #
//############################################################################

static FLTRESULT AILEXPORT FLT_startup     (void)
{
   if (FLT_started++)
      {
      strcpy(FLT_error_text,"Already started");
      return FLT_ALREADY_STARTED;
      }

   //
   // Init static prefs/attributes
   //

   FLT_error_text[0]            = 0;

   return FLT_NOERR;
}

//############################################################################
//#                                                                          #
//# Shut down FLT sample filter                                              #
//#                                                                          #
//############################################################################

static FLTRESULT      AILEXPORT FLT_shutdown    (void)
{
   if (!FLT_started)
      {
      strcpy(FLT_error_text,"Not initialized");
      return FLT_NOT_INIT;
      }

   --FLT_started;

   return FLT_NOERR;
}

//############################################################################
//#                                                                          #
//# Allocate driver-specific descriptor                                      #
//#                                                                          #
//############################################################################

static HDRIVERSTATE AILEXPORT FLT_open_driver (HDIGDRIVER dig, //)
                                        S32 FAR   *build_buffer,
                                        S32        build_buffer_size)

{
   if (dwTempBuild==0)
   {
     dwTempBuild=(S32*)AIL_mem_alloc_lock(128000*sizeof(S32));
     if (dwTempBuild==0)
       return(0);
     dwLastBytes = 128000*sizeof(S32); // clear the entire buffer
   }

   DRIVERSTATE FAR *DRV = (DRIVERSTATE *) AIL_mem_alloc_lock(sizeof(DRIVERSTATE));

   if (DRV == NULL)
      {
      strcpy(FLT_error_text,"Out of memory");
      return NULL;
      }

   AIL_memset(DRV,
              0,
              sizeof(DRIVERSTATE));

   //
   // Initialize generic members
   //

   DRV->dig               = dig;
   DRV->build_buffer      = build_buffer;
   DRV->build_buffer_size = build_buffer_size;

   S32 rate;
   AIL_digital_configuration(dig,&rate,0,0);

   DRV->rate              = (F32) rate;

   //
   // Return descriptor address cast to handle
   //

   return (HSAMPLESTATE) DRV;
}
                                            
//############################################################################
//#                                                                          #
//# Close filter driver instance                                             #
//#                                                                          #
//############################################################################

static FLTRESULT     AILEXPORT FLT_close_driver (HDRIVERSTATE state)
{
   DRIVERSTATE FAR *DRV = (DRIVERSTATE FAR *) state;

   AIL_mem_free_lock(DRV);

   if ( dwTempBuild )
   {
     AIL_mem_free_lock( dwTempBuild );
     dwTempBuild = 0;
   }

   return FLT_NOERR;
}

//############################################################################
//#                                                                          #
//#  Perform any needed processing before per-sample mixing begins           #
//#                                                                          #
//#  Called after the build buffer has been flushed prior to the mixing      #
//#  phase, but before any samples have been mixed into it                   #
//#                                                                          #
//############################################################################

#ifdef IS_MAC
static void AILEXPORT FLT_premix_process (HDRIVERSTATE driver, U32 buffer_size )
{
  ((DRIVERSTATE*)driver)->build_buffer_size = buffer_size;
}
#else
static void AILEXPORT FLT_premix_process (HDRIVERSTATE driver)
{
}
#endif

//############################################################################
//#                                                                          #
//#  Process data after mixing                                               #
//#                                                                          #
//#  Called after all samples have been mixed into the 32-bit build buffer,  #
//#  prior to copying the build-buffer contents to the driver's output       #
//#  buffer                                                                  #
//#                                                                          #
//############################################################################


#ifdef IS_MAC
static void AILEXPORT FLT_postmix_process (HDRIVERSTATE driver, U32 buffer_size )
{
  ((DRIVERSTATE*)driver)->build_buffer_size = buffer_size;
}
#else
static void AILEXPORT FLT_postmix_process (HDRIVERSTATE driver)
{
}
#endif

//############################################################################
//#                                                                          #
//# Assign filter to specified sample                                        #
//#                                                                          #
//############################################################################

static HSAMPLESTATE AILEXPORT FLTSMP_open_sample (HDRIVERSTATE driver, //)
                                           HSAMPLE      S)
{
   U32    dwSamples;
   F32    fRate;
   F32    fTemp;
   U32    dwIndex;
   U32    dwBytes;
   F32    fSamples;

   //
   // Allocate SAMPLESTATE descriptor
   //

   SAMPLESTATE FAR *SS = (SAMPLESTATE FAR *) AIL_mem_alloc_lock(sizeof(SAMPLESTATE));

   if (SS == NULL)
      {
      strcpy(FLT_error_text,"Out of memory");
      return NULL;
      }

   AIL_memset(SS,
              0, 
              sizeof(SAMPLESTATE));

   //
   // Initialize generic members
   //

   SS->driver = (DRIVERSTATE FAR *) driver;
   SS->sample = S;

   //
   // Initialize provider-specific members to their default values
   //

   SS->fReverbTime   =  _FX_DEFAULT_TIME;
   SS->fPreDelay     =  _FX_DEFAULT_PREDELAY;
   SS->fMix          =  _FX_DEFAULT_MIX;

   // get rate
   fRate =  SS->driver->rate;

   // save rate
   SS->fRate   =  fSamples =  fRate;

   // convert MS to samples
   dwSamples   =  (U32)(( _FX_MAX_TIME * fRate ) / 1000.0F);

   // set buffer size
   SS->dwDelayLineBytes =  dwSamples * sizeof( F32 );

   // allocate delay lines
   for( dwIndex = 0; dwIndex < _FX_REVERB2_DELAYS; dwIndex++ )
   {
      // calculate delay line length
      fTemp    =  fSamples * sReverb2Table[ dwIndex ];

      // get bytes to allocate
      dwBytes  =  (U32)fTemp * sizeof( F32 );

      // allocate delay line
      SS->sDelayLines[ dwIndex ].lpBuffer    =  (F32 *)AIL_mem_alloc_lock( dwBytes );
      
      AIL_memset(SS->sDelayLines[ dwIndex ].lpBuffer, 0 , dwBytes );
   }

   // calculate max predelay line length
   fSamples =  (F32)( (F32)_FX_MAX_PREDELAY / 1000.0 ) * fRate;

   // allocate pre-delay
   dwBytes  =  (U32)( fSamples * sizeof( F32 ));

   // save bytes
   SS->dwPreDelayLineBytes =  dwBytes;

   // allocate predelay
   SS->sPreDelayLine.lpBuffer    =  (F32 *)AIL_mem_alloc_lock( dwBytes );

   AIL_memset(SS->sPreDelayLine.lpBuffer, 0 , dwBytes );

   // calculate params
   FXCalcParams( SS, S );

   //
   // Return descriptor address cast to handle
   //

   return (HSAMPLESTATE) SS;
}
                                            
//############################################################################
//#                                                                          #
//# Close filter sample instance                                             #
//#                                                                          #
//############################################################################

static FLTRESULT     AILEXPORT FLTSMP_close_sample (HSAMPLESTATE state)
{
   SAMPLESTATE FAR *SS = (SAMPLESTATE FAR *) state;
   U32       dwIndex;

   // free memory
   for( dwIndex = 0; dwIndex < _FX_REVERB2_DELAYS; dwIndex++ )
      AIL_mem_free_lock( SS->sDelayLines[ dwIndex ].lpBuffer );

   // free predelay
   AIL_mem_free_lock( SS->sPreDelayLine.lpBuffer );

   // free state
   AIL_mem_free_lock(SS);

   return FLT_NOERR;
}

#ifdef NOQIFIST
static S32 __inline f2i(F32 val)
{
  S32 retval;
  __asm
  {
    fld [val]
    fistp [retval]
  }
  return(retval);
}
#else
#define f2i(val) ((S32)val)
#endif


//############################################################################
//#                                                                          #
//#  Process sample data just prior to mixing                                #
//#                                                                          #
//#  Return 0 to force MSS to skip mixing stage and update returned pointers #
//#  with stored values (orig_src, orig_src_fract, build_dest, left_val, and #
//#  right_val).  Return 1 to allow mixer to operate normally on original    #
//#  source and dest pointers, ignoring any pointer values written back by   #
//#  the filter.                                                             #
//#                                                                          #
//#  Parameters:                                                             #
//#                                                                          #
//#    state is the sample descriptor.  You can retrieve the HSAMPLE via     #
//#    the state.sample member, if needed.                                   #
//#                                                                          #
//#    orig_src is the pointer to the data being submitted to the mixer      #
//#    pipeline.  This is normally in the application's own data space.  The #
//#    filter should advance this pointer if it is going to return 0 to      #
//#    inhibit mixing.                                                       #
//#                                                                          #
//#    orig_src_fract is the fractional (16:16) source address.  Normally    #
//#    the integer part of this fraction is 0, unless the mixer's smoothing  #
//#    algorithm is undersampling the data.  The fractional source address   #
//#    is not usually especially critical, except for DLS synthesis and      #
//#    similar applications with tight subblock loops.  Filters which do not #
//#    maintain fractional source addresses do not need to update this       #
//#    pointer.                                                              #
//#                                                                          #
//#    orig_src_end is the end of the source data + 1 byte.                  #
//#                                                                          #
//#    build_dest is the destination of the current mixing operation in the  #
//#    build buffer.  The build buffer consists of 32-bit DWORDS which are   #
//#    16-bit samples shifted left 11 times (*2048).  The lower 11 bits are  #
//#    used for volume scaling, while the upper 5 bits provide headroom for  #
//#    clipping.  Like orig_src, the filter should advance this pointer if   #
//#    mixing will be inhibited.                                             #
//#                                                                          #
//#    build_dest_end is the end of the destination data area + 1 byte.      #
//#    Due to resampling and format conversion performed by the mixer, the   #
//#    amount of data to be processed in a given frame may be limited by     #
//#    EITHER build_dest_end OR orig_src_end.  The only assumption you can   #
//#    make regarding the amount of data that may be passed to this routine  #
//#    is that it will not exceed the driver's build buffer size.  As little #
//#    as a single mono sample may be passed to this routine.                #
//#                                                                          #
//#    left_val and right_val are the left and right amplitude values from   #
//#    the end of the last mixing operation, respectively.  They are needed  #
//#    by the mixer's smoothing algorithm.  Filters which do not require     #
//#    these values do not need to update them.                              #
//#                                                                          #
//#    playback_ratio is the fixed-point resampling constant, given at 16:16 #
//#    precision.  A value of 0x10000 indicates no resampling.               #
//#                                                                          #
//#    left_scale and right_scale are the 11-bit left and right channel      #
//#    amplitude scalars, ranging from 0 to 2047.  base_scale is the scalar  #
//#    derived from the sample's volume setting, before stereo panning is    #
//#    applied.                                                              #
//#                                                                          #
//#    mixer_provider is provided for the convenience of filters which need  #
//#    a general-purpose mixer kernel (such as the reverb example).  This is #
//#    the same mixer provider that will be called immediately after         #
//#    FLTSMP_sample_process() returns with a nonzero value.                 #
//#                                                                          #
//#    mixer_operation is the operation code for the current mixing phase.   #
//#    The operation flags are defined as follows:                           #
//#                                                                          #
//#       M_DEST_STEREO 1  // Set to enable stereo mixer output              #
//#       M_SRC_16      2  // Set to enable mixing of 16-bit samples         #
//#       M_FILTER      4  // Set to enable smoothing when resampling        #
//#       M_SRC_STEREO  8  // Set to enable mixing of stereo input samples   #
//#       M_VOL_SCALING 16 // Set to enable volume scalars other than 2047   #
//#       M_RESAMPLE    32 // Set to enable playback ratios other than 65536 #
//#       M_ORDER       64 // Set to reverse L/R stereo order for sample     #
//#                                                                          #
//############################################################################

static S32 AILEXPORT FLTSMP_sample_process(HSAMPLESTATE    state,  //)
                                    void const FAR * FAR *orig_src,
                                    U32        FAR *orig_src_fract,
                                    void const FAR *orig_src_end,
                                    S32  FAR * FAR *build_dest,
                                    void FAR       *build_dest_end,
                                    S32        FAR *left_val,
                                    S32        FAR *right_val,
                                    S32             playback_ratio,
                                    S32             left_scale,
                                    S32             right_scale,
                                    S32             base_scale,
                                    MIXSTAGE   FAR *mixer_provider,
                                    U32             mixer_operation)
{
   SAMPLESTATE FAR *SS           = (SAMPLESTATE FAR *) state;
   HSAMPLE          S            = SS->sample;
   DRIVERSTATE FAR *DRV          = SS->driver;
   void  const FAR   *  lpOrigSrc=  *orig_src;
   S32   FAR   *  lpBuildDest    =  *build_dest;
   S32   FAR   *  lpBuildTemp;
   void  FAR   *  lpBuildTempEnd;

   U32    dwBuildBytes   =  AIL_ptr_dif( build_dest_end, *build_dest );
   U32    dwIndex;
   U32    dwTemp;
   F32      fInput;
   F32      fDryOut;
   F32      fWetOut;

   // temp outputs
   F32          fOutputL;
   F32          fOutputR;
   F32          fSInput;

   // temp variables
   F32          fTemp, fTemp1;

   // comb sum
   F32          fCombSum;

   // comb coeff
   F32          fC1, fC2, fC3, fC4, fC5, fC6;

   // comb pointers
   F32         *pfC1, *pfC2, *pfC3, *pfC4, *pfC5, *pfC6;

   // temp 
   F32          fY1, fY2;

   // get coefficents
   fC1   =  SS->sDelayLines[ 0 ].fCoef;
   fC2   =  SS->sDelayLines[ 1 ].fCoef;
   fC3   =  SS->sDelayLines[ 2 ].fCoef;
   fC4   =  SS->sDelayLines[ 3 ].fCoef;
   fC5   =  SS->sDelayLines[ 4 ].fCoef;
   fC6   =  SS->sDelayLines[ 5 ].fCoef;

   // get buffer pointers
   pfC1  =  SS->sDelayLines[ 0 ].lpBuffer;
   pfC2  =  SS->sDelayLines[ 1 ].lpBuffer;
   pfC3  =  SS->sDelayLines[ 2 ].lpBuffer;
   pfC4  =  SS->sDelayLines[ 3 ].lpBuffer;
   pfC5  =  SS->sDelayLines[ 4 ].lpBuffer;
   pfC6  =  SS->sDelayLines[ 5 ].lpBuffer;

   // clear buffer
   AIL_memset( dwTempBuild, 0, dwLastBytes );

   // get pointer to build buffer
   lpBuildTemp    =  dwTempBuild;
   lpBuildTempEnd =  ( char FAR * )lpBuildTemp + dwBuildBytes;

   // mix into temp buffer
   mixer_provider->MSS_mixer_merge( orig_src,
                                    orig_src_fract,
                                    orig_src_end,
                 (S32 FAR * FAR *)  &lpBuildTemp,
                                    lpBuildTempEnd,
                                    left_val,
                                    right_val,
                                    playback_ratio,
                                    left_scale,
                                    right_scale,
                                    mixer_operation
#ifdef IS_X86
                                    ,AIL_MMX_available()
#endif
                                    );

   // calculate the amount the mixer mixed into the build buffer
   // and convert to samples i.e. divide by sample size
   dwLastBytes = AIL_ptr_dif( lpBuildTemp, dwTempBuild );
   dwTemp      = dwLastBytes / 4;

   // set wet/dry mix
   fDryOut  =  1.0F - SS->fMix;
   fWetOut  =  SS->fMix;

   // check if mono or stereo
   if ( mixer_operation & M_DEST_STEREO )
   {
      // mix into build buffer
      for( dwIndex = 0; dwIndex < dwTemp; dwIndex += 2 )
      {
         // get input sample (left)
         fSInput   =  (F32)dwTempBuild[ dwIndex ];

         // get input from pre-delay line
         fInput   =  SS->sPreDelayLine.lpBuffer[ SS->sPreDelayLine.dwInput ];

         // save new input sample
         SS->sPreDelayLine.lpBuffer[ SS->sPreDelayLine.dwInput++ ]   =  (F32)fSInput;

         // process clip for buffer wrap
         if ( SS->sPreDelayLine.dwInput >= SS->sPreDelayLine.dwLength )
            SS->sPreDelayLine.dwInput  =  0;

         // process sum of comb filters
         fCombSum =  pfC1[ SS->sDelayLines[ 0 ].dwInput ] + \
                     pfC2[ SS->sDelayLines[ 1 ].dwInput ] + \
                     pfC3[ SS->sDelayLines[ 2 ].dwInput ] + \
                     pfC4[ SS->sDelayLines[ 3 ].dwInput ];

         // process coef for each comb filter
         pfC1[ SS->sDelayLines[ 0 ].dwInput ]   =  fC1 * pfC1[ SS->sDelayLines[ 0 ].dwInput ] + fInput;
         pfC2[ SS->sDelayLines[ 1 ].dwInput ]   =  fC2 * pfC2[ SS->sDelayLines[ 1 ].dwInput ] + fInput;
         pfC3[ SS->sDelayLines[ 2 ].dwInput ]   =  fC3 * pfC3[ SS->sDelayLines[ 2 ].dwInput ] + fInput;
         pfC4[ SS->sDelayLines[ 3 ].dwInput ]   =  fC4 * pfC4[ SS->sDelayLines[ 3 ].dwInput ] + fInput;

         // advance inputs
         SS->sDelayLines[ 0 ].dwInput++;
         SS->sDelayLines[ 1 ].dwInput++;
         SS->sDelayLines[ 2 ].dwInput++;
         SS->sDelayLines[ 3 ].dwInput++;

         // process clip for buffer wrap
         if ( SS->sDelayLines[ 0 ].dwInput >= SS->sDelayLines[ 0 ].dwLength )
            SS->sDelayLines[ 0 ].dwInput  =  0;
         if ( SS->sDelayLines[ 1 ].dwInput >= SS->sDelayLines[ 1 ].dwLength )
            SS->sDelayLines[ 1 ].dwInput  =  0;
         if ( SS->sDelayLines[ 2 ].dwInput >= SS->sDelayLines[ 2 ].dwLength )
            SS->sDelayLines[ 2 ].dwInput  =  0;
         if ( SS->sDelayLines[ 3 ].dwInput >= SS->sDelayLines[ 3 ].dwLength )
            SS->sDelayLines[ 3 ].dwInput  =  0;

         // get allpass value
         fY1   =  pfC5[ SS->sDelayLines[ 4 ].dwInput ];

         // all pass
         pfC5[ SS->sDelayLines[ 4 ].dwInput++ ] =  fTemp = fC5 * fY1 + fCombSum;

         // adjust
         fY1   -= fC5 * fTemp;
         fY2   =  pfC6[ SS->sDelayLines[ 5 ].dwInput ];

         // all pass
         pfC6[ SS->sDelayLines[ 5 ].dwInput++ ] =  fTemp =  fC6 * fY2 + ( fY1 * .25F );
         fTemp1   =  fC6 * fY2 + ( fY1 * .15F );
         
         // compute right
         fOutputR =  ( fY2 - fC6 * fTemp1 ) * fWetOut;

         // compute left
         fOutputL =  ( fY2 - fC6 * fTemp ) * fWetOut;

         // set up fTemp
         fTemp  =  fDryOut * fSInput;

         // adjst LR
         fOutputL += fTemp;
         fOutputR += fTemp;

         // process clip for buffer wrap
         if ( SS->sDelayLines[ 4 ].dwInput >= SS->sDelayLines[ 4 ].dwLength )
            SS->sDelayLines[ 4 ].dwInput  =  0;
         if ( SS->sDelayLines[ 5 ].dwInput >= SS->sDelayLines[ 5 ].dwLength )
            SS->sDelayLines[ 5 ].dwInput  =  0;

         // store output
         lpBuildDest[ dwIndex ]     += f2i(fOutputL);

         // store output
         lpBuildDest[ dwIndex + 1 ] += f2i(fOutputR);
      }
   }
   else
   {
      // mix into build buffer
      for( dwIndex = 0; dwIndex < dwTemp; dwIndex++ )
      {
         // get input sample
         fSInput  =  (F32)dwTempBuild[ dwIndex ];

         // get input from pre-delay line
         fInput   =  SS->sPreDelayLine.lpBuffer[ SS->sPreDelayLine.dwInput ];

         // save new input sample
         SS->sPreDelayLine.lpBuffer[ SS->sPreDelayLine.dwInput++ ]   =  (F32)fSInput;

         // process clip for buffer wrap
         if ( SS->sPreDelayLine.dwInput >= SS->sPreDelayLine.dwLength )
            SS->sPreDelayLine.dwInput  =  0;

         // process sum of comb filters
         fCombSum =  pfC1[ SS->sDelayLines[ 0 ].dwInput ] + \
                     pfC2[ SS->sDelayLines[ 1 ].dwInput ] + \
                     pfC3[ SS->sDelayLines[ 2 ].dwInput ] + \
                     pfC4[ SS->sDelayLines[ 3 ].dwInput ];

         // process coef for each comb filter
         pfC1[ SS->sDelayLines[ 0 ].dwInput ]   =  fC1 * pfC1[ SS->sDelayLines[ 0 ].dwInput ] + fInput;
         pfC2[ SS->sDelayLines[ 1 ].dwInput ]   =  fC2 * pfC2[ SS->sDelayLines[ 1 ].dwInput ] + fInput;
         pfC3[ SS->sDelayLines[ 2 ].dwInput ]   =  fC3 * pfC3[ SS->sDelayLines[ 2 ].dwInput ] + fInput;
         pfC4[ SS->sDelayLines[ 3 ].dwInput ]   =  fC4 * pfC4[ SS->sDelayLines[ 3 ].dwInput ] + fInput;

         // advance inputs
         SS->sDelayLines[ 0 ].dwInput++;
         SS->sDelayLines[ 1 ].dwInput++;
         SS->sDelayLines[ 2 ].dwInput++;
         SS->sDelayLines[ 3 ].dwInput++;

         // process clip for buffer wrap
         if ( SS->sDelayLines[ 0 ].dwInput >= SS->sDelayLines[ 0 ].dwLength )
            SS->sDelayLines[ 0 ].dwInput  =  0;
         if ( SS->sDelayLines[ 1 ].dwInput >= SS->sDelayLines[ 1 ].dwLength )
            SS->sDelayLines[ 1 ].dwInput  =  0;
         if ( SS->sDelayLines[ 2 ].dwInput >= SS->sDelayLines[ 2 ].dwLength )
            SS->sDelayLines[ 2 ].dwInput  =  0;
         if ( SS->sDelayLines[ 3 ].dwInput >= SS->sDelayLines[ 3 ].dwLength )
            SS->sDelayLines[ 3 ].dwInput  =  0;

         // get allpass value
         fY1   =  pfC5[ SS->sDelayLines[ 4 ].dwInput ];

         // all pass
         pfC5[ SS->sDelayLines[ 4 ].dwInput++ ] =  fTemp = fC5 * fY1 + fCombSum;

         // adjust
         fY1   -= fC5 * fTemp;
         fY2   =  pfC6[ SS->sDelayLines[ 5 ].dwInput ];

         // all pass
         pfC6[ SS->sDelayLines[ 5 ].dwInput++ ] =  fTemp =  fC6 * fY2 + ( fY1 * .25F );
         fTemp1   =  fC6 * fY2 + ( fY1 * .15F );
         
         // compute left
         fOutputL =  ( fY2 - fC6 * fTemp ) * fWetOut;

         // set up fTemp
         fTemp  =  fDryOut * fSInput;

         // adjst LR
         fOutputL += fTemp;

         // process clip for buffer wrap
         if ( SS->sDelayLines[ 4 ].dwInput >= SS->sDelayLines[ 4 ].dwLength )
            SS->sDelayLines[ 4 ].dwInput  =  0;
         if ( SS->sDelayLines[ 5 ].dwInput >= SS->sDelayLines[ 5 ].dwLength )
            SS->sDelayLines[ 5 ].dwInput  =  0;

         // store output
         lpBuildDest[ dwIndex ]  += f2i(fOutputL);
      }
   }

   // advance pointer
   *build_dest  += dwTemp;

   //
   // Return 1 to allow mixing process to run normally
   //

   return 0;
}

//############################################################################
//#                                                                          #
//# Retrieve an FLT sample attribute or preference value by index            #
//#                                                                          #
//############################################################################

static S32     AILEXPORT FLTSMP_sample_attribute (HSAMPLESTATE state, //)
                                           HATTRIB      attribute)
{
   SAMPLESTATE FAR *SS = (SAMPLESTATE FAR *) state;

   switch ((ATTRIB) attribute)
      {
         case  _FX_REVERB2_TIME :

               return( (S32) float_as_long(SS->fReverbTime ));

               break;

         case  _FX_REVERB2_PREDELAY :

               return( (S32) float_as_long(SS->fPreDelay ));

               break;

         case  _FX_REVERB2_MIX :

               return( (S32) float_as_long(SS->fMix ));

               break;


      }

   return -1;
}

//############################################################################
//#                                                                          #
//# Set sample preference value, returning previous setting                  #
//#                                                                          #
//############################################################################

static S32 AILEXPORT FLTSMP_set_sample_preference (HSAMPLESTATE state, //)
                                            HATTRIB      preference,
                                            void FAR*    value)
{
   SAMPLESTATE FAR *SS  = (SAMPLESTATE FAR *) state;
   HSAMPLE          S   = SS->sample;
   F32            fRate  =  SS->driver->rate;

   S32 prev = -1;

   // determine preference
   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //

      case  _FX_REVERB2_TIME     :

            // get previous value and set new
            prev           = (S32) float_as_long(SS->fReverbTime );
            SS->fReverbTime       = *(F32 FAR*)value;

            // clip to valid range
            FX_CLIPRANGE( SS->fReverbTime, 0.01F, _FX_MAX_TIME );

            // set new parameters
            FXCalcParams( SS, S );

            break;

      case  _FX_REVERB2_PREDELAY :

            // get previous value and set new
            prev           = (S32) float_as_long(SS->fPreDelay );
            SS->fPreDelay  = *(F32 FAR*)value;

            // clip to valid range
            FX_CLIPRANGE( SS->fPreDelay, 0.0F, _FX_MAX_PREDELAY );

            // set new parameters
            FXCalcParams( SS, S );

            break;

      case  _FX_REVERB2_MIX       :

            // get previous value and set new
            prev           = (S32) float_as_long(SS->fMix );
            SS->fMix       = *(F32 FAR*)value;

            // clip to valid range
            FX_CLIPRANGE( SS->fMix, 0.0F, 1.0F );

            break;
      }


   return prev;
}

extern "C" S32 MSS_RIB_Main( HPROVIDER provider_handle, U32 up_down );
extern "C" S32 MSS_RIB_Main( HPROVIDER provider_handle, U32 up_down )
{
   const RIB_INTERFACE_ENTRY FLT[] =
      {                                                        
      REG_AT("Name",                     PROVIDER_NAME,        RIB_STRING),
      REG_AT("Version",                  PROVIDER_VERSION,     RIB_HEX),

      REG_FN(PROVIDER_query_attribute),

      REG_FN(FLT_startup),
      REG_FN(FLT_error),
      REG_FN(FLT_shutdown),

      REG_FN(FLT_open_driver),
      REG_FN(FLT_close_driver),

      REG_FN(FLT_premix_process),
      REG_FN(FLT_postmix_process),

      REG_FN(FLT_set_provider_preference),

      };

   const RIB_INTERFACE_ENTRY FLTSMP[] =
      {
      REG_FN(FLTSMP_open_sample),
      REG_FN(FLTSMP_close_sample),

      REG_FN(FLTSMP_sample_process),

      REG_FN(FLTSMP_sample_attribute),
      REG_FN(FLTSMP_set_sample_preference),

      REG_AT("Reverb Time",               _FX_REVERB2_TIME  ,         RIB_FLOAT),
      REG_PR("Reverb Time",               _FX_REVERB2_TIME  ,         RIB_FLOAT),
      REG_AT("Reverb PreDelay",           _FX_REVERB2_PREDELAY  ,     RIB_FLOAT),
      REG_PR("Reverb PreDelay",           _FX_REVERB2_PREDELAY  ,     RIB_FLOAT),
      REG_AT("Reverb Mix",               _FX_REVERB2_MIX  ,         RIB_FLOAT),
      REG_PR("Reverb Mix",               _FX_REVERB2_MIX  ,         RIB_FLOAT),
      };

   static HPROVIDER self;

   if (up_down)
      {

         self = RIB_provider_library_handle();

         RIB_register(self,
                     "MSS pipeline filter",
                      FLT);

         RIB_register(self,
                     "Pipeline filter sample services",
                      FLTSMP);
      }
   else
      {
         RIB_unregister_all(self);
      }

   return TRUE;
}


#ifdef IS_WINDOWS

//############################################################################
//#                                                                          #
//# DLLMain registers FLT API interface at load time                         #
//#                                                                          #
//############################################################################

BOOL WINAPI DllMain(HINSTANCE hinstDll, //)
                          U32     fdwReason,
                          LPVOID    plvReserved)
{
   switch (fdwReason)
      {
      case DLL_PROCESS_ATTACH:

         DisableThreadLibraryCalls( hinstDll );
         return( MSS_RIB_Main( 0, 1 ) );

      case DLL_PROCESS_DETACH:

         return( MSS_RIB_Main( 0, 0 ) );
      }

   return TRUE;
}

#endif
