//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: FLT module for Reverb3                                       ##
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

//
// Epsilon value used for FP comparisons with 0
//

#define EPSILON 0.0001F

// Pentium Floating Point Processor Denormal Protect Value
#define  _FX_DENORMVAL     0.0000000001F

// PI definition
#define  M_PI  3.14159265F

// Maximum time (seconds)
#define  _FX_MAX_TIME            10000.0F
#define  _FX_MAX_PREDELAY        250.0F

// Reverb Defaults
#define  _FX_DEFAULT_TIME        3000.0F
#define  _FX_DEFAULT_PREDELAY    0.0F
#define  _FX_DEFAULT_DAMPING     5000.0F
#define  _FX_DEFAULT_MIX         0.0F
#define  _FX_DEFAULT_EAXMAPPING  0.0F

// Reverb Parameters for EAX table
typedef  struct   _tagFXREVERBPARAMS
         {
            // reverb mix, in %
            F32    fReverbMix;

            // reverb time in seconds
            F32    fReverbTime;

            // reverb damping in % of frequency range
            F32    fReverbDamping;

            F32    fPredelay;

         } FXREVERBPARAMS, *PFXREVERBPARAMS;

// Delay Line
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

// Reverb3 Comb/AllPass Filters
#define  _FX_REVERB3_DELAYS   6

// reverb mapping table for EAX->Reverb3
FXREVERBPARAMS sEAXMappingTable[ ENVIRONMENT_COUNT ]   =
                  {
                     { 0.0F,1.493F,0.5F,0.0F },            // generic
                     { 0.25F,0.1F,0.4F,0.0F },             //  1 padded cell
                     { 0.417F,0.4F,0.666F,0.0F },          //  2 room
                     { 0.5F,1.499F,0.166F,0.0F },          //  3 bathroom
                     { 0.208F,0.478F,0.1F,0.0F },          //  4 living room
                     { 0.3F,2.309F,0.6F,0.0F },            //  5 stone room
                     { 0.203F,4.279F,0.5F,0.0F },          //  6 auditorium
                     { 0.5F,3.961F,0.5F,0.0F },            //  7 concert hall
                     { 0.5F,2.886F,1.3F,0.0F },            //  8 cave
                     { 0.361F,7.284F,0.332F,0.0F },        //  9 arena
                     { 0.5F,10.0F,0.3F,0.0F },             // 10 hanger
                     { 0.153F,0.259F,0.55F,0.0F },         // 11 carpeted hallway
                     { 0.361F,1.493F,0.50F,0.0F },         // 12 hallway
                     { 0.444F,2.697F,0.638F,0.0F },        // 13 stone corridor
                     { 0.25F,1.490F,0.5F,100.0F },         // 14 alley
                     { 0.111F,1.490F,0.5F,100.0F },        // 15 forest
                     { 0.08F,1.2F,0.5F,100.0F },           // 16 city
                     { 0.05F,0.8F,0.7F,200.0F },           // 17 mountains
                     { 0.55F,3.0F,0.45F,150.0F },          // 18 quarry
                     { 0.12F,0.8F,0.18F,150.0F },          // 19 plain
                     { 0.208F,1.652F,1.5F,0.0F },          // 20 parkinglot
                     { 0.652F,2.886F,0.25F,0.0F },         // 21 sewerpipe
                     { 1.0F,1.499F,0.1F,0.0F },            // 22 underwater
                     { 0.875F,8.392F,1.39F,0.0F },         // 23 drugged
                     { 0.139F,10.00F,1.5F,0.0F },          // 24 dizzy
                     { 0.486F,7.563F,4.0F,0.0F },          // 25 psychotic
                  };

// global reverb table for time of the comb/all pass
// filters
static   F32 sReverb3Table[] =
         {
            0.0297F,
            0.0371F,
            0.0411F,
            0.0437F,
            0.005F,
            0.0017F
         };

// all pass reverb times
static   F32 sReverb3AllPass[]  =
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
   _FX_REVERB3_TIME,
   _FX_REVERB3_PREDELAY,
   _FX_REVERB3_DAMPING,
   _FX_REVERB3_MIX,
   _FX_REVERB3_EAXMAP,
   _FX_LOWPASS_CUTOFF,
   _FX_REVERB_QUALITY
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

   S32              dorv;
   S32              rvquality;

   // reverb time
   F32    fReverbTime;

   // pre delay time
   F32    fPreDelay;

   // bytes of maximum delay line
   U32    dwDelayLineBytes;

   // delay lines for reverb
   FXDELAYLINE sDelayLinesL[ _FX_REVERB3_DELAYS ];
   FXDELAYLINE sDelayLinesR[ _FX_REVERB3_DELAYS ];

   // pre delay bytes to clear
   U32    dwPreDelayLineBytes;

   // pre-delay line
   FXDELAYLINE sPreDelayLineL;
   FXDELAYLINE sPreDelayLineR;

   // damping freq
   F32    fDampingFreq;

   // sample history
   F32            fXL[ 2 ];
   F32            fYL[ 2 ];
   F32            fXR[ 2 ];
   F32            fYR[ 2 ];

   // coefficients
   F32            fA[ 3 ];
   F32            fB[ 2 ];

   // sample rate
   F32       fRate;

   // mix
   F32       fMix;

   // current EAX preset mapping, -1.0 for none
   F32       fEAXMapping;

   //
   // Members associated with specific filter provider
   //
   S32              dolp;
   F32            lpfCutoffFreq;
   F32            lpfXL[ 2 ];
   F32            lpfYL[ 2 ];
   F32            lpfXR[ 2 ];
   F32            lpfYR[ 2 ];
   F32            lpfA[ 3 ];
   F32            lpfB[ 2 ];
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
      F32          fC;

      static   int  fFirstTime  =  1;

      // get sample rate
      fRate =  (F32)SS->driver->rate;

      if (SS->fMix<EPSILON)
      {
        if (SS->dorv==1)
        {
          SS->fXL[ 0 ]  =  0.0;
          SS->fXL[ 1 ]  =  0.0;
          SS->fYL[ 0 ]  =  0.0;
          SS->fYL[ 1 ]  =  0.0;
          SS->fXR[ 0 ]  =  0.0;
          SS->fXR[ 1 ]  =  0.0;
          SS->fYR[ 0 ]  =  0.0;
          SS->fYR[ 1 ]  =  0.0;
        }
        SS->dorv=0;
      }
      else
      {
        SS->dorv=1;

        // get reverb time
        fRT   =  SS->fReverbTime / 1000.0F;

        // calculate samples for reverb time
        fSamples =  fRate;

        // reset all delay lines to silence
        for( dwIndex = 0; dwIndex < _FX_REVERB3_DELAYS; dwIndex++ )
        {
           // calculate delay line length
           fTemp =  fSamples * sReverb3Table[ dwIndex ];

           // set delay line length
           SS->sDelayLinesL[ dwIndex ].dwLength =  (U32)fTemp;
           SS->sDelayLinesR[ dwIndex ].dwLength =  (U32)fTemp;

           // check if first time
           if ( fFirstTime )
           {
             // clear delay line buffer
             AIL_memset( SS->sDelayLinesL[ dwIndex ].lpBuffer, 0, (U32)fTemp * sizeof( F32 ) );
             AIL_memset( SS->sDelayLinesR[ dwIndex ].lpBuffer, 0, (U32)fTemp * sizeof( F32 ) );

             // set input index location
             SS->sDelayLinesL[ dwIndex ].dwInput  =  0;
             SS->sDelayLinesR[ dwIndex ].dwInput  =  0;
           }

           // set coef for reverb time
           if ( dwIndex < 4 )
           {
              // set left/right
              SS->sDelayLinesL[ dwIndex ].fCoef =  (F32)pow( .001F, sReverb3Table[ dwIndex ] / fRT );
              SS->sDelayLinesR[ dwIndex ].fCoef =  (F32)pow( .001F, sReverb3Table[ dwIndex ] / fRT );
           }
           else
           {
              // set left/right
              SS->sDelayLinesL[ dwIndex ].fCoef =  (F32)pow( .001F, sReverb3Table[ dwIndex ] / sReverb3AllPass[ dwIndex - 4 ] );
              SS->sDelayLinesR[ dwIndex ].fCoef =  (F32)pow( .001F, sReverb3Table[ dwIndex ] / sReverb3AllPass[ dwIndex - 4 ] );
           }
        }

        // calculate samples for pre-delay line
        fSamples =  ( SS->fPreDelay / 1000.0F ) * fRate;

        // set samples
        SS->sPreDelayLineL.dwLength =  (U32)fSamples;
        SS->sPreDelayLineR.dwLength =  (U32)fSamples;

        // check if first time
        if ( fFirstTime )
        {
          // clear predelay line
          AIL_memset( SS->sPreDelayLineL.lpBuffer, 0, SS->dwPreDelayLineBytes );
          AIL_memset( SS->sPreDelayLineR.lpBuffer, 0, SS->dwPreDelayLineBytes );

          // reset predelay index
          SS->sPreDelayLineL.dwInput  =  0;
          SS->sPreDelayLineR.dwInput  =  0;
        }

        // calculate new coeffs
        fC =  1.0F / (F32)tan( M_PI * SS->fDampingFreq / fRate );

        // calculate coefficients
        SS->fA[ 0 ] =  1.0F / ( 1.0F + (F32)sqrt( 2.0F ) * fC + fC * fC );
        SS->fA[ 1 ] =  (F32)2.0 * SS->fA[ 0 ];
        SS->fA[ 2 ] =  SS->fA[ 0 ];

        // calculate coefficients
        SS->fB[ 0 ] =  2.0F * ( 1.0F - fC * fC ) * SS->fA[ 0 ];
        SS->fB[ 1 ] =  ( 1.0F - (F32)sqrt( 2.0 ) * fC + fC * fC ) * SS->fA[ 0 ];

        // reset first time flag
        fFirstTime  =  FALSE;
     }

   if (SS->lpfCutoffFreq>=((fRate/2)-EPSILON))
   {
     if (SS->dolp==1)
     {
       SS->lpfXL[ 0 ]  =  0.0;
       SS->lpfXL[ 1 ]  =  0.0;
       SS->lpfYL[ 0 ]  =  0.0;
       SS->lpfYL[ 1 ]  =  0.0;
       SS->lpfXR[ 0 ]  =  0.0;
       SS->lpfXR[ 1 ]  =  0.0;
       SS->lpfYR[ 0 ]  =  0.0;
       SS->lpfYR[ 1 ]  =  0.0;
     }
     SS->dolp=0;
   }
   else
   {
     SS->dolp=1;

     // calculate new coeffs
     fC =  1.0F / (F32)tan( M_PI * SS->lpfCutoffFreq / fRate );

     // calculate coefficients
     SS->lpfA[ 0 ] =  1.0F / ( 1.0F + (F32)sqrt( 2.0F ) * fC + fC * fC );
     SS->lpfA[ 1 ] =  (F32)2.0 * SS->lpfA[ 0 ];
     SS->lpfA[ 2 ] =  SS->lpfA[ 0 ];

     // calculate coefficients
     SS->lpfB[ 0 ] =  2.0F * ( 1.0F - fC * fC ) * SS->lpfA[ 0 ];
     SS->lpfB[ 1 ] =  ( 1.0F - (F32)sqrt( 2.0 ) * fC + fC * fC ) * SS->lpfA[ 0 ];

   }
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
//# Return floating-point type as unsigned long U32 (without actually        #
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
//# Return signed long U32 as single-precision float (without actually       #
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
      case PROVIDER_NAME:      return (U32) "Reverb3 Filter";
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
   SS->fDampingFreq  =  _FX_DEFAULT_DAMPING;
   SS->fMix          =  _FX_DEFAULT_MIX;
   SS->fEAXMapping   =  _FX_DEFAULT_EAXMAPPING;

   // get rate
   fRate =  SS->driver->rate;

   // save rate
   SS->fRate   =  fSamples =  fRate;

   // convert MS to samples
   dwSamples   =  (U32)(( _FX_MAX_TIME * fRate ) / 1000.0F);

   // set buffer size
   SS->dwDelayLineBytes =  dwSamples * sizeof( F32 );

   // allocate delay lines
   for( dwIndex = 0; dwIndex < _FX_REVERB3_DELAYS; dwIndex++ )
   {
      // calculate delay line length
      fTemp    =  fSamples * sReverb3Table[ dwIndex ];

      // get bytes to allocate
      dwBytes  =  (U32)fTemp * sizeof( F32 );

      // allocate delay lines
      SS->sDelayLinesL[ dwIndex ].lpBuffer    =  (F32 *)AIL_mem_alloc_lock( dwBytes );
      SS->sDelayLinesR[ dwIndex ].lpBuffer    =  (F32 *)AIL_mem_alloc_lock( dwBytes );

      AIL_memset(SS->sDelayLinesL[ dwIndex ].lpBuffer, 0 , dwBytes );
      AIL_memset(SS->sDelayLinesR[ dwIndex ].lpBuffer, 0 , dwBytes );
   }

   // calculate max predelay line length
   fSamples =  (F32)( (F32)_FX_MAX_PREDELAY / 1000.0 ) * fRate;

   // allocate pre-delay
   dwBytes  =  (U32)( fSamples * sizeof( F32 ));

   // save bytes
   SS->dwPreDelayLineBytes =  dwBytes;

   // allocate predelay
   SS->sPreDelayLineL.lpBuffer    =  (F32 *)AIL_mem_alloc_lock( dwBytes );
   SS->sPreDelayLineR.lpBuffer    =  (F32 *)AIL_mem_alloc_lock( dwBytes );

   AIL_memset(SS->sPreDelayLineL.lpBuffer, 0 , dwBytes );
   AIL_memset(SS->sPreDelayLineR.lpBuffer, 0 , dwBytes );

   // reset sample history
   SS->fXL[ 0 ]  =  0.0;
   SS->fXL[ 1 ]  =  0.0;
   SS->fYL[ 0 ]  =  0.0;
   SS->fYL[ 1 ]  =  0.0;
   SS->fXR[ 0 ]  =  0.0;
   SS->fXR[ 1 ]  =  0.0;
   SS->fYR[ 0 ]  =  0.0;
   SS->fYR[ 1 ]  =  0.0;

   // reset lowpass sample history
   SS->lpfXL[ 0 ]  =  0.0;
   SS->lpfXL[ 1 ]  =  0.0;
   SS->lpfYL[ 0 ]  =  0.0;
   SS->lpfYL[ 1 ]  =  0.0;
   SS->lpfXR[ 0 ]  =  0.0;
   SS->lpfXR[ 1 ]  =  0.0;
   SS->lpfYR[ 0 ]  =  0.0;
   SS->lpfYR[ 1 ]  =  0.0;

   // init low pass values
   SS->lpfCutoffFreq        =  SS->driver->rate/2.0F;

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

   // free memory   // free memory
   for( dwIndex = 0; dwIndex < _FX_REVERB3_DELAYS; dwIndex++ )
   {
      // free delay lines
      AIL_mem_free_lock( SS->sDelayLinesL[ dwIndex ].lpBuffer );
      AIL_mem_free_lock( SS->sDelayLinesR[ dwIndex ].lpBuffer );
   }

   // free predelays
   AIL_mem_free_lock( SS->sPreDelayLineL.lpBuffer );
   AIL_mem_free_lock( SS->sPreDelayLineR.lpBuffer );

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

   if ((SS->dorv==0) && (SS->dolp==0))
     return(1);

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
   F32          fOutput;
   F32          fOutputL;
   F32          fOutputR;
   F32          fSInput;
   F32          fSInputR;
   F32          fLPInput;

   // temp variables
   F32          fTemp, fTemp1;

   // comb sum
   F32          fCombSum;

   // comb coeff
   F32          fC1, fC2, fC3, fC4, fC5, fC6;

   // comb pointers
   F32         *pfLC1, *pfLC2, *pfLC3, *pfLC4, *pfLC5, *pfLC6;
   F32         *pfRC1, *pfRC2, *pfRC3, *pfRC4, *pfRC5, *pfRC6;

   // temp
   F32          fY1, fY2;

   // get coefficents
   fC1   =  SS->sDelayLinesL[ 0 ].fCoef;
   fC2   =  SS->sDelayLinesL[ 1 ].fCoef;
   fC3   =  SS->sDelayLinesL[ 2 ].fCoef;
   fC4   =  SS->sDelayLinesL[ 3 ].fCoef;
   fC5   =  SS->sDelayLinesL[ 4 ].fCoef;
   fC6   =  SS->sDelayLinesL[ 5 ].fCoef;

   // get buffer pointers
   pfLC1  =  SS->sDelayLinesL[ 0 ].lpBuffer;
   pfLC2  =  SS->sDelayLinesL[ 1 ].lpBuffer;
   pfLC3  =  SS->sDelayLinesL[ 2 ].lpBuffer;
   pfLC4  =  SS->sDelayLinesL[ 3 ].lpBuffer;
   pfLC5  =  SS->sDelayLinesL[ 4 ].lpBuffer;
   pfLC6  =  SS->sDelayLinesL[ 5 ].lpBuffer;

   pfRC1  =  SS->sDelayLinesR[ 0 ].lpBuffer;
   pfRC2  =  SS->sDelayLinesR[ 1 ].lpBuffer;
   pfRC3  =  SS->sDelayLinesR[ 2 ].lpBuffer;
   pfRC4  =  SS->sDelayLinesR[ 3 ].lpBuffer;
   pfRC5  =  SS->sDelayLinesR[ 4 ].lpBuffer;
   pfRC6  =  SS->sDelayLinesR[ 5 ].lpBuffer;

   F32      fA0, fA1, fA2, fB0, fB1;

   // get coeffs
   fA0   =  SS->lpfA[ 0 ];
   fA1   =  SS->lpfA[ 1 ];
   fA2   =  SS->lpfA[ 2 ];
   fB0   =  SS->lpfB[ 0 ];
   fB1   =  SS->lpfB[ 1 ];

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
         if (SS->dorv)
         {
            // get input sample (left) (right)
            fSInput   =  (F32)dwTempBuild[ dwIndex ];
            fSInputR   =  (F32)dwTempBuild[ dwIndex + 1 ];

            // get input from pre-delay line
            fInput   =  SS->sPreDelayLineL.lpBuffer[ SS->sPreDelayLineL.dwInput ];

            // save new input sample
            SS->sPreDelayLineL.lpBuffer[ SS->sPreDelayLineL.dwInput++ ]   =
              (SS->rvquality)?(F32)fSInput:(F32)(fSInput+fSInputR);

            // process clip for buffer wrap
            if ( SS->sPreDelayLineL.dwInput >= SS->sPreDelayLineL.dwLength )
               SS->sPreDelayLineL.dwInput  =  0;

            // calculate sample
            fLPInput =  SS->fA[ 0 ] * fInput + SS->fA[ 1 ] * SS->fXL[ 0 ] + \
                        SS->fA[ 2 ] * SS->fXL[ 1 ] - SS->fB[ 0 ] * SS->fYL[ 0 ] - \
                        SS->fB[ 1 ] * SS->fYL[ 1 ];

            // save sample history
            SS->fXL[ 1 ]   =  SS->fXL[ 0 ];
            SS->fXL[ 0 ]   =  fInput;
            SS->fYL[ 1 ]   =  SS->fYL[ 0 ];
            SS->fYL[ 0 ]   =  fLPInput + _FX_DENORMVAL;

            // process sum of comb filters
            fCombSum =  pfLC1[ SS->sDelayLinesL[ 0 ].dwInput ] + \
                        pfLC2[ SS->sDelayLinesL[ 1 ].dwInput ] + \
                        pfLC3[ SS->sDelayLinesL[ 2 ].dwInput ] + \
                        pfLC4[ SS->sDelayLinesL[ 3 ].dwInput ];

            // process coef for each comb filter
            pfLC1[ SS->sDelayLinesL[ 0 ].dwInput ]   =  fC1 * pfLC1[ SS->sDelayLinesL[ 0 ].dwInput ] + fLPInput;
            pfLC2[ SS->sDelayLinesL[ 1 ].dwInput ]   =  fC2 * pfLC2[ SS->sDelayLinesL[ 1 ].dwInput ] + fLPInput;
            pfLC3[ SS->sDelayLinesL[ 2 ].dwInput ]   =  fC3 * pfLC3[ SS->sDelayLinesL[ 2 ].dwInput ] + fLPInput;
            pfLC4[ SS->sDelayLinesL[ 3 ].dwInput ]   =  fC4 * pfLC4[ SS->sDelayLinesL[ 3 ].dwInput ] + fLPInput;

            // advance inputs
            SS->sDelayLinesL[ 0 ].dwInput++;
            SS->sDelayLinesL[ 1 ].dwInput++;
            SS->sDelayLinesL[ 2 ].dwInput++;
            SS->sDelayLinesL[ 3 ].dwInput++;

            // process clip for buffer wrap
            if ( SS->sDelayLinesL[ 0 ].dwInput >= SS->sDelayLinesL[ 0 ].dwLength )
               SS->sDelayLinesL[ 0 ].dwInput  =  0;
            if ( SS->sDelayLinesL[ 1 ].dwInput >= SS->sDelayLinesL[ 1 ].dwLength )
               SS->sDelayLinesL[ 1 ].dwInput  =  0;
            if ( SS->sDelayLinesL[ 2 ].dwInput >= SS->sDelayLinesL[ 2 ].dwLength )
               SS->sDelayLinesL[ 2 ].dwInput  =  0;
            if ( SS->sDelayLinesL[ 3 ].dwInput >= SS->sDelayLinesL[ 3 ].dwLength )
               SS->sDelayLinesL[ 3 ].dwInput  =  0;

            // get allpass value
            fY1   =  pfLC5[ SS->sDelayLinesL[ 4 ].dwInput ];

            // all pass
            pfLC5[ SS->sDelayLinesL[ 4 ].dwInput++ ] =  fTemp = fC5 * fY1 + fCombSum;

            // adjust
            fY1   -= fC5 * fTemp;
            fY2   =  pfLC6[ SS->sDelayLinesL[ 5 ].dwInput ];

            // all pass
            pfLC6[ SS->sDelayLinesL[ 5 ].dwInput++ ] =  fTemp =  fC6 * fY2 + ( fY1 * .25F );

            // compute left
            fOutputL =  ( fY2 - fC6 * fTemp ) * fWetOut;

            // adjust LR
            fOutputL += (fDryOut * fSInput);

            // process clip for buffer wrap
            if ( SS->sDelayLinesL[ 4 ].dwInput >= SS->sDelayLinesL[ 4 ].dwLength )
               SS->sDelayLinesL[ 4 ].dwInput  =  0;
            if ( SS->sDelayLinesL[ 5 ].dwInput >= SS->sDelayLinesL[ 5 ].dwLength )
               SS->sDelayLinesL[ 5 ].dwInput  =  0;

            if (!SS->rvquality)
            {
              fTemp1   =  fC6 * fY2 + ( fY1 * .15F );

              // compute right
              fOutputR =  ( fY2 - fC6 * fTemp1 ) * fWetOut;

              // adjust LR
              fOutputR += (fDryOut * fSInputR);

            }
            else
            {

              // get input from pre-delay line
              fInput   =  SS->sPreDelayLineR.lpBuffer[ SS->sPreDelayLineR.dwInput ];

              // save new input sample
              SS->sPreDelayLineR.lpBuffer[ SS->sPreDelayLineR.dwInput++ ]   =  (F32)fSInputR;

              // process clip for buffer wrap
              if ( SS->sPreDelayLineR.dwInput >= SS->sPreDelayLineR.dwLength )
                 SS->sPreDelayLineR.dwInput  =  0;

              // calculate sample
              fLPInput =  SS->fA[ 0 ] * fInput + SS->fA[ 1 ] * SS->fXR[ 0 ] + \
                          SS->fA[ 2 ] * SS->fXR[ 1 ] - SS->fB[ 0 ] * SS->fYR[ 0 ] - \
                          SS->fB[ 1 ] * SS->fYR[ 1 ];

              // save sample history
              SS->fXR[ 1 ]   =  SS->fXR[ 0 ];
              SS->fXR[ 0 ]   =  fInput;
              SS->fYR[ 1 ]   =  SS->fYR[ 0 ];
              SS->fYR[ 0 ]   =  fLPInput + _FX_DENORMVAL;

              // process sum of comb filters
              fCombSum =  pfRC1[ SS->sDelayLinesR[ 0 ].dwInput ] + \
                          pfRC2[ SS->sDelayLinesR[ 1 ].dwInput ] + \
                          pfRC3[ SS->sDelayLinesR[ 2 ].dwInput ] + \
                          pfRC4[ SS->sDelayLinesR[ 3 ].dwInput ];

              // process coef for each comb filter
              pfRC1[ SS->sDelayLinesR[ 0 ].dwInput ]   =  fC1 * pfRC1[ SS->sDelayLinesR[ 0 ].dwInput ] + fLPInput;
              pfRC2[ SS->sDelayLinesR[ 1 ].dwInput ]   =  fC2 * pfRC2[ SS->sDelayLinesR[ 1 ].dwInput ] + fLPInput;
              pfRC3[ SS->sDelayLinesR[ 2 ].dwInput ]   =  fC3 * pfRC3[ SS->sDelayLinesR[ 2 ].dwInput ] + fLPInput;
              pfRC4[ SS->sDelayLinesR[ 3 ].dwInput ]   =  fC4 * pfRC4[ SS->sDelayLinesR[ 3 ].dwInput ] + fLPInput;

              // advance inputs
              SS->sDelayLinesR[ 0 ].dwInput++;
              SS->sDelayLinesR[ 1 ].dwInput++;
              SS->sDelayLinesR[ 2 ].dwInput++;
              SS->sDelayLinesR[ 3 ].dwInput++;

              // process clip for buffer wrap
              if ( SS->sDelayLinesR[ 0 ].dwInput >= SS->sDelayLinesR[ 0 ].dwLength )
                 SS->sDelayLinesR[ 0 ].dwInput  =  0;
              if ( SS->sDelayLinesR[ 1 ].dwInput >= SS->sDelayLinesR[ 1 ].dwLength )
                 SS->sDelayLinesR[ 1 ].dwInput  =  0;
              if ( SS->sDelayLinesR[ 2 ].dwInput >= SS->sDelayLinesR[ 2 ].dwLength )
                 SS->sDelayLinesR[ 2 ].dwInput  =  0;
              if ( SS->sDelayLinesR[ 3 ].dwInput >= SS->sDelayLinesR[ 3 ].dwLength )
                 SS->sDelayLinesR[ 3 ].dwInput  =  0;

              // get allpass value
              fY1   =  pfRC5[ SS->sDelayLinesR[ 4 ].dwInput ];

              // all pass
              pfRC5[ SS->sDelayLinesR[ 4 ].dwInput++ ] =  fTemp = fC5 * fY1 + fCombSum;

              // adjust
              fY1   -= fC5 * fTemp;
              fY2   =  pfRC6[ SS->sDelayLinesR[ 5 ].dwInput ];

              // all pass
              pfRC6[ SS->sDelayLinesR[ 5 ].dwInput++ ] = /* fTemp = */ fC6 * fY2 + ( fY1 * .25F );
              fTemp1   =  fC6 * fY2 + ( fY1 * .15F );

              // compute right
              fOutputR =  ( fY2 - fC6 * fTemp1 ) * fWetOut;

              // compute left
    //          fOutputL =  ( fY2 - fC6 * fTemp ) * fWetOut;

              // add in dry signal
              fOutputR += (fDryOut * fSInputR);

              // process clip for buffer wrap
              if ( SS->sDelayLinesR[ 4 ].dwInput >= SS->sDelayLinesR[ 4 ].dwLength )
                 SS->sDelayLinesR[ 4 ].dwInput  =  0;
              if ( SS->sDelayLinesR[ 5 ].dwInput >= SS->sDelayLinesR[ 5 ].dwLength )
                 SS->sDelayLinesR[ 5 ].dwInput  =  0;
            }
         }
         else
         {
           fOutputL   =  (F32)dwTempBuild[ dwIndex ];
           fOutputR   =  (F32)dwTempBuild[ dwIndex + 1 ];
         }

         if (SS->dolp)
         {
           // calculate sample after low pass
           fOutput  =  fA0 * fOutputL + fA1 * SS->lpfXL[ 0 ] + \
                       fA2 * SS->lpfXL[ 1 ] - fB0 * SS->lpfYL[ 0 ] - \
                       fB1 * SS->lpfYL[ 1 ];

           // save samples in history
           SS->lpfXL[ 1 ]   =  SS->lpfXL[ 0 ];
           SS->lpfXL[ 0 ]   =  fOutputL;
           SS->lpfYL[ 1 ]   =  SS->lpfYL[ 0 ];
           SS->lpfYL[ 0 ]   =  fOutput + _FX_DENORMVAL;

           // store output
           lpBuildDest[ dwIndex ]     += f2i(fOutput);

            // calculate sample after low pass
           fOutput  =  fA0 * fOutputR + fA1 * SS->lpfXR[ 0 ] + \
                       fA2 * SS->lpfXR[ 1 ] - fB0 * SS->lpfYR[ 0 ] - \
                       fB1 * SS->lpfYR[ 1 ];

           // save samples in history
           SS->lpfXR[ 1 ]   =  SS->lpfXR[ 0 ];
           SS->lpfXR[ 0 ]   =  fOutputR;
           SS->lpfYR[ 1 ]   =  SS->lpfYR[ 0 ];
           SS->lpfYR[ 0 ]   =  fOutput + _FX_DENORMVAL;

           // store output
           lpBuildDest[ dwIndex + 1 ] += f2i(fOutput);
         }
         else
         {

           // store output
           lpBuildDest[ dwIndex ]     += f2i(fOutputL);

           // store output
           lpBuildDest[ dwIndex + 1 ] += f2i(fOutputR);

         }

      }
   }
   else
   {
      // mix into build buffer
      for( dwIndex = 0; dwIndex < dwTemp; dwIndex++ )
      {
         if (SS->dorv)
         {
           // get input sample
           fSInput  =  (F32)dwTempBuild[ dwIndex ];

           // get input from pre-delay line
           fInput   =  SS->sPreDelayLineL.lpBuffer[ SS->sPreDelayLineL.dwInput ];

           // save new input sample
           SS->sPreDelayLineL.lpBuffer[ SS->sPreDelayLineL.dwInput++ ]   =  (F32)fSInput;

           // process clip for buffer wrap
           if ( SS->sPreDelayLineL.dwInput >= SS->sPreDelayLineL.dwLength )
              SS->sPreDelayLineL.dwInput  =  0;

           // calculate sample
           fLPInput =  SS->fA[ 0 ] * fInput + SS->fA[ 1 ] * SS->fXL[ 0 ] + \
                       SS->fA[ 2 ] * SS->fXL[ 1 ] - SS->fB[ 0 ] * SS->fYL[ 0 ] - \
                       SS->fB[ 1 ] * SS->fYL[ 1 ];

           // save sample history
           SS->fXL[ 1 ]   =  SS->fXL[ 0 ];
           SS->fXL[ 0 ]   =  fInput;
           SS->fYL[ 1 ]   =  SS->fYL[ 0 ];
           SS->fYL[ 0 ]   =  fLPInput + _FX_DENORMVAL;

           // process sum of comb filters
           fCombSum =  pfLC1[ SS->sDelayLinesL[ 0 ].dwInput ] + \
                       pfLC2[ SS->sDelayLinesL[ 1 ].dwInput ] + \
                       pfLC3[ SS->sDelayLinesL[ 2 ].dwInput ] + \
                       pfLC4[ SS->sDelayLinesL[ 3 ].dwInput ];

           // process coef for each comb filter
           pfLC1[ SS->sDelayLinesL[ 0 ].dwInput ]   =  fC1 * pfLC1[ SS->sDelayLinesL[ 0 ].dwInput ] + fLPInput;
           pfLC2[ SS->sDelayLinesL[ 1 ].dwInput ]   =  fC2 * pfLC2[ SS->sDelayLinesL[ 1 ].dwInput ] + fLPInput;
           pfLC3[ SS->sDelayLinesL[ 2 ].dwInput ]   =  fC3 * pfLC3[ SS->sDelayLinesL[ 2 ].dwInput ] + fLPInput;
           pfLC4[ SS->sDelayLinesL[ 3 ].dwInput ]   =  fC4 * pfLC4[ SS->sDelayLinesL[ 3 ].dwInput ] + fLPInput;

           // advance inputs
           SS->sDelayLinesL[ 0 ].dwInput++;
           SS->sDelayLinesL[ 1 ].dwInput++;
           SS->sDelayLinesL[ 2 ].dwInput++;
           SS->sDelayLinesL[ 3 ].dwInput++;

           // process clip for buffer wrap
           if ( SS->sDelayLinesL[ 0 ].dwInput >= SS->sDelayLinesL[ 0 ].dwLength )
              SS->sDelayLinesL[ 0 ].dwInput  =  0;
           if ( SS->sDelayLinesL[ 1 ].dwInput >= SS->sDelayLinesL[ 1 ].dwLength )
              SS->sDelayLinesL[ 1 ].dwInput  =  0;
           if ( SS->sDelayLinesL[ 2 ].dwInput >= SS->sDelayLinesL[ 2 ].dwLength )
              SS->sDelayLinesL[ 2 ].dwInput  =  0;
           if ( SS->sDelayLinesL[ 3 ].dwInput >= SS->sDelayLinesL[ 3 ].dwLength )
              SS->sDelayLinesL[ 3 ].dwInput  =  0;

           // get allpass value
           fY1   =  pfLC5[ SS->sDelayLinesL[ 4 ].dwInput ];

           // all pass
           pfLC5[ SS->sDelayLinesL[ 4 ].dwInput++ ] =  fTemp = fC5 * fY1 + fCombSum;

           // adjust
           fY1   -= fC5 * fTemp;
           fY2   =  pfLC6[ SS->sDelayLinesL[ 5 ].dwInput ];

           // all pass
           pfLC6[ SS->sDelayLinesL[ 5 ].dwInput++ ] =  fTemp =  fC6 * fY2 + ( fY1 * .25F );
//           fTemp1   =  fC6 * fY2 + ( fY1 * .15F );

           // compute right
  //         fOutputR =  ( fY2 - fC6 * fTemp1 ) * fWetOut;

           // compute left
           fOutputL =  ( fY2 - fC6 * fTemp ) * fWetOut;

           // set up fTemp
           fTemp  =  fDryOut * fSInput;

           // adjust LR
           fOutputL += fTemp;

           // process clip for buffer wrap
           if ( SS->sDelayLinesL[ 4 ].dwInput >= SS->sDelayLinesL[ 4 ].dwLength )
              SS->sDelayLinesL[ 4 ].dwInput  =  0;
           if ( SS->sDelayLinesL[ 5 ].dwInput >= SS->sDelayLinesL[ 5 ].dwLength )
              SS->sDelayLinesL[ 5 ].dwInput  =  0;
         }
         else
         {
           fOutputL   =  (F32)dwTempBuild[ dwIndex ];
         }

         if (SS->dolp)
         {
           // calculate sample after low pass
           fOutput  =  fA0 * fOutputL + fA1 * SS->lpfXL[ 0 ] + \
                       fA2 * SS->lpfXL[ 1 ] - fB0 * SS->lpfYL[ 0 ] - \
                       fB1 * SS->lpfYL[ 1 ];

           // save samples in history
           SS->lpfXL[ 1 ]   =  SS->lpfXL[ 0 ];
           SS->lpfXL[ 0 ]   =  fOutputL;
           SS->lpfYL[ 1 ]   =  SS->lpfYL[ 0 ];
           SS->lpfYL[ 0 ]   =  fOutput + _FX_DENORMVAL;

           // store output
           lpBuildDest[ dwIndex ]  += f2i(fOutput);
         }
         else
         {
           // store output
           lpBuildDest[ dwIndex ]  += f2i(fOutputL);
         }

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
         case  _FX_REVERB3_TIME :

               return( (S32) float_as_long(SS->fReverbTime ));

               break;

         case  _FX_REVERB3_PREDELAY :

               return( (S32) float_as_long(SS->fPreDelay ));

               break;

         case  _FX_REVERB3_DAMPING :

               return( (S32) float_as_long(SS->fDampingFreq ));

               break;

         case  _FX_REVERB3_MIX :

               return( (S32) float_as_long(SS->fMix ));

               break;


         case  _FX_REVERB3_EAXMAP :

               return( (S32) float_as_long(SS->fEAXMapping ));

               break;

         case  _FX_LOWPASS_CUTOFF   :

               return( (S32) float_as_long(SS->lpfCutoffFreq ));

               break;

         case  _FX_REVERB_QUALITY   :

               return( (S32) float_as_long((F32)SS->rvquality ) );
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
   F32 fRate          =  (F32)SS->driver->rate;
   
   S32 prev = -1;

   // determine preference
   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //

      case  _FX_REVERB3_TIME     :

            // get previous value and set new
            prev           = (S32) float_as_long(SS->fReverbTime );
            SS->fReverbTime       = *(F32 FAR*)value;

            // clip to valid range
            FX_CLIPRANGE( SS->fReverbTime, 0.01F, _FX_MAX_TIME );

            // set new parameters
            FXCalcParams( SS, S );

            break;

      case  _FX_REVERB3_PREDELAY :

            // get previous value and set new
            prev           = (S32) float_as_long(SS->fPreDelay );
            SS->fPreDelay  = *(F32 FAR*)value;

            // clip to valid range
            FX_CLIPRANGE( SS->fPreDelay, 0.0F, _FX_MAX_PREDELAY );

            // set new parameters
            FXCalcParams( SS, S );

            break;

      case  _FX_REVERB3_DAMPING :

            // get previous value and set new
            prev              = (S32) float_as_long(SS->fDampingFreq );
            SS->fDampingFreq  = *(F32 FAR*)value;

            // clip to valid range
            FX_CLIPRANGE( SS->fDampingFreq, 20.0F, fRate / 2.0F );

            // set new parameters
            FXCalcParams( SS, S );

            break;

      case  _FX_REVERB3_MIX       :

            // get previous value and set new
            prev           = (S32) float_as_long(SS->fMix );
            SS->fMix       = *(F32 FAR*)value;

            // clip to valid range
            FX_CLIPRANGE( SS->fMix, 0.0F, 1.0F );

            // set new parameters
            FXCalcParams( SS, S );

            break;

      case  _FX_REVERB3_EAXMAP    :

            // get previous value and set new
            prev           = (S32) float_as_long(SS->fEAXMapping );
            SS->fEAXMapping       = *(F32 FAR*)value;
            
            // clip to valid range
            FX_CLIPRANGE( SS->fEAXMapping, 0.0F, (F32)ENVIRONMENT_COUNT );

            // now look up the values and assign
            SS->fReverbTime   =  sEAXMappingTable[ (U32)SS->fEAXMapping ].fReverbTime * 1000.0F;
            SS->fMix          =  sEAXMappingTable[ (U32)SS->fEAXMapping ].fReverbMix;
            SS->fDampingFreq  =  sEAXMappingTable[ (U32)SS->fEAXMapping ].fReverbDamping * ( fRate / 8.0F );
            SS->fPreDelay  = sEAXMappingTable[ (U32)SS->fEAXMapping ].fPredelay;

            // clip to valid range
            FX_CLIPRANGE( SS->fPreDelay, 0.0F, _FX_MAX_PREDELAY );

            // clip damping to make sure we don't go below 20.0 Hz
            FX_CLIPRANGE( SS->fDampingFreq, 200.0F, ( fRate / 8.0F ) );

            // clip mix to valid range
            FX_CLIPRANGE( SS->fMix, 0.0F, 1.0F );

            // clip time to valid range
            FX_CLIPRANGE( SS->fReverbTime, 0.01F, _FX_MAX_TIME );

            // set new parameters
            FXCalcParams( SS, S );

            break;

      case  _FX_LOWPASS_CUTOFF   :

            // get previous value and set new
            prev = (S32) float_as_long(SS->lpfCutoffFreq );
            SS->lpfCutoffFreq = *(F32 FAR*)value;

            if (SS->lpfCutoffFreq<20.0F)
              SS->lpfCutoffFreq=20.0F;

            // set new parameters
            FXCalcParams( SS, S );
            break;
      
      case  _FX_REVERB_QUALITY   :

            // get previous value and set new
            prev = (S32) float_as_long((F32)SS->rvquality );
            SS->rvquality = ((*(F32 FAR*)value)>EPSILON)?1:0;

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

      REG_AT("Reverb Time",              _FX_REVERB3_TIME  ,        RIB_FLOAT),
      REG_PR("Reverb Time",              _FX_REVERB3_TIME  ,        RIB_FLOAT),
      REG_AT("Reverb PreDelay",          _FX_REVERB3_PREDELAY  ,    RIB_FLOAT),
      REG_PR("Reverb PreDelay",          _FX_REVERB3_PREDELAY  ,    RIB_FLOAT),
      REG_AT("Reverb Damping",           _FX_REVERB3_DAMPING  ,     RIB_FLOAT),
      REG_PR("Reverb Damping",           _FX_REVERB3_DAMPING  ,     RIB_FLOAT),
      REG_AT("Reverb Mix",               _FX_REVERB3_MIX  ,         RIB_FLOAT),
      REG_PR("Reverb Mix",               _FX_REVERB3_MIX  ,         RIB_FLOAT),
      REG_AT("Reverb EAX Environment",   _FX_REVERB3_EAXMAP ,       RIB_FLOAT),
      REG_PR("Reverb EAX Environment",   _FX_REVERB3_EAXMAP ,       RIB_FLOAT),
      REG_AT("Lowpass Cutoff",           _FX_LOWPASS_CUTOFF ,       RIB_FLOAT),
      REG_PR("Lowpass Cutoff",           _FX_LOWPASS_CUTOFF ,       RIB_FLOAT),
      REG_AT("Reverb Quality",           _FX_REVERB_QUALITY ,       RIB_FLOAT),
      REG_PR("Reverb Quality",           _FX_REVERB_QUALITY ,       RIB_FLOAT),
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
