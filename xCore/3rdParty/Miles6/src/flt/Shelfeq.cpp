//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: FLT module for Shelving EQ Filter                            ##
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

// Shelving EQ Defaults
#define  _FX_LSFREQ_DEFAULT      500.0F
#define  _FX_LSGAIN_DEFAULT      -10.0F
#define  _FX_HSFREQ_DEFAULT      4000.0F
#define  _FX_HSGAIN_DEFAULT      2.0F

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
   _FX_SHELFEQ_LOWFREQ,
   _FX_SHELFEQ_LOWGAIN,
   _FX_SHELFEQ_HIGHFREQ,
   _FX_SHELFEQ_HIGHGAIN,
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
   F32          fLowShelfFreq;
   F32          fLowShelfGain;
   F32          fHighShelfFreq;
   F32          fHighShelfGain;

   // low shelf history / coefficients
   F32          fLSXL[ 3 ];
   F32          fLSYL[ 3 ];
   F32          fLSXR[ 3 ];
   F32          fLSYR[ 3 ];
   F32          fLSA[ 3 ];
   F32          fLSB[ 3 ];

   // high shelf history / coefficients
   F32          fHSXL[ 3 ];
   F32          fHSYL[ 3 ];
   F32          fHSXR[ 3 ];
   F32          fHSYR[ 3 ];
   F32          fHSA[ 3 ];
   F32          fHSB[ 3 ];
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
      F32 fRate;
      F32 fSN, fCS;
      F32 fA;
      F32 fAlpha;
      F32 fBeta;
      F32 fS =  1.0F;
      F32 fOmega;
      F32 fQ =  1.0F;

      // get sample rate
      fRate = SS->driver->rate;

      // compute initial values used to derive coefficients
      fA       =  (F32)sqrt( pow( 10.0F,  ( SS->fLowShelfGain / 20.0F )));
      fOmega   =  2.0F * M_PI * ( SS->fLowShelfFreq / fRate );
      fSN      =  (F32)sin( fOmega );
      fCS      =  (F32)cos( fOmega );
      fAlpha   =  fSN / ( 2.0F * fQ );
      fBeta    =  (F32)sqrt( ( ( fA * fA ) + 1.0F ) / fS - ( ( fA - 1.0F ) * ( fA - 1.0F )) );

      // calculate coefficients for low-shelf
      SS->fLSB[ 0 ] =  fA * ( ( fA + 1.0F ) - ( fA - 1.0F ) * fCS + fBeta * fSN );
      SS->fLSB[ 1 ] =  2.0F * fA * ( ( fA - 1.0F ) - ( fA + 1.0F ) * fCS );
      SS->fLSB[ 2 ] =  fA * ( ( fA + 1.0F ) - ( fA - 1.0F ) * fCS - fBeta * fSN );
      SS->fLSA[ 0 ] =  ( fA + 1.0F ) + ( fA - 1.0F ) * fCS + fBeta * fSN;
      SS->fLSA[ 1 ] =  -2.0F * ( ( fA - 1.0F ) + ( fA + 1.0F ) * fCS );
      SS->fLSA[ 2 ] =  ( fA + 1.0F ) + ( fA - 1.0F ) * fCS - fBeta * fSN;

      // compute initial values used to derive coefficients
      fA       =  (F32)sqrt( pow( 10.0F,  ( SS->fHighShelfGain / 20.0F )));
      fOmega   =  2.0F * M_PI * ( SS->fHighShelfFreq / fRate );
      fSN      =  (F32)sin( fOmega );
      fCS      =  (F32)cos( fOmega );
      fAlpha   =  fSN / ( 2.0F * fQ );
      fBeta    =  (F32)sqrt( ( ( fA * fA ) + 1.0F ) / fS - ( ( fA - 1.0F ) * ( fA - 1.0F )) );

      // calculate coefficients for high-shelf
      SS->fHSB[ 0 ] =  fA * ( ( fA + 1.0F ) + ( fA - 1.0F ) * fCS + fBeta * fSN );
      SS->fHSB[ 1 ] =  -2.0F * fA * ( ( fA - 1.0F ) + ( fA + 1.0F ) * fCS );
      SS->fHSB[ 2 ] =  fA * ( ( fA + 1.0F ) + ( fA -1.0F ) * fCS - fBeta * fSN );
      SS->fHSA[ 0 ] =  ( fA + 1.0F ) - ( fA - 1.0F ) * fCS + fBeta * fSN;
      SS->fHSA[ 1 ] =  2.0F * ( ( fA - 1.0F ) - ( fA + 1.0F ) * fCS );
      SS->fHSA[ 2 ] =  ( fA + 1.0F ) - ( fA - 1.0F ) * fCS - fBeta * fSN;
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
      case PROVIDER_NAME:      return (U32) "ShelvingEQ Filter";
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

   // init values
   SS->fLowShelfFreq    =  _FX_LSFREQ_DEFAULT;
   SS->fLowShelfGain    =  _FX_LSGAIN_DEFAULT;
   SS->fHighShelfFreq   =  _FX_HSFREQ_DEFAULT;
   SS->fHighShelfGain   =  _FX_HSGAIN_DEFAULT;

   // reset sample history
   SS->fLSXL[ 0 ] =  0.0;
   SS->fLSXL[ 1 ] =  0.0;
   SS->fLSXL[ 2 ] =  0.0;
   SS->fLSYL[ 0 ] =  0.0;
   SS->fLSYL[ 1 ] =  0.0;
   SS->fLSYL[ 2 ] =  0.0;
   SS->fLSXR[ 0 ] =  0.0;
   SS->fLSXR[ 1 ] =  0.0;
   SS->fLSXR[ 2 ] =  0.0;
   SS->fLSYR[ 0 ] =  0.0;
   SS->fLSYR[ 1 ] =  0.0;
   SS->fLSYR[ 2 ] =  0.0;
   SS->fHSXL[ 0 ] =  0.0;
   SS->fHSXL[ 1 ] =  0.0;
   SS->fHSXL[ 2 ] =  0.0;
   SS->fHSYL[ 0 ] =  0.0;
   SS->fHSYL[ 1 ] =  0.0;
   SS->fHSYL[ 2 ] =  0.0;
   SS->fHSXR[ 0 ] =  0.0;
   SS->fHSXR[ 1 ] =  0.0;
   SS->fHSXR[ 2 ] =  0.0;
   SS->fHSYR[ 0 ] =  0.0;
   SS->fHSYR[ 1 ] =  0.0;
   SS->fHSYR[ 2 ] =  0.0;

   // update params
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
   F32      fOutput;
   F32      fHSInput;

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

   // check if mono or stereo
   if ( mixer_operation & M_DEST_STEREO )
   {
      // mix into build buffer
      for( dwIndex = 0; dwIndex < dwTemp; dwIndex += 2 )
      {
         // get input sample (left)
         fInput   =  (F32)dwTempBuild[ dwIndex ];

         // calculate output
         fHSInput =  ( SS->fLSB[ 0 ] / SS->fLSA[ 0 ] ) * fInput + \
                     ( SS->fLSB[ 1 ] / SS->fLSA[ 0 ] ) * SS->fLSXL[ 0 ] + \
                     ( SS->fLSB[ 2 ] / SS->fLSA[ 0 ] ) * SS->fLSXL[ 1 ] - \
                     ( SS->fLSA[ 1 ] / SS->fLSA[ 0 ] ) * SS->fLSYL[ 0 ] - \
                     ( SS->fLSA[ 2 ] / SS->fLSA[ 0 ] ) * SS->fLSYL[ 1 ];

         // save history
         SS->fLSXL[ 1 ] =  SS->fLSXL[ 0 ];
         SS->fLSXL[ 0 ] =  fInput;
         SS->fLSYL[ 1 ] =  SS->fLSYL[ 0 ];
         SS->fLSYL[ 0 ] =  fHSInput + _FX_DENORMVAL;

         // calculate output
         fOutput  =  ( SS->fHSB[ 0 ] / SS->fHSA[ 0 ] ) * fHSInput + \
                     ( SS->fHSB[ 1 ] / SS->fHSA[ 0 ] ) * SS->fHSXL[ 0 ] + \
                     ( SS->fHSB[ 2 ] / SS->fHSA[ 0 ] ) * SS->fHSXL[ 1 ] - \
                     ( SS->fHSA[ 1 ] / SS->fHSA[ 0 ] ) * SS->fHSYL[ 0 ] - \
                     ( SS->fHSA[ 2 ] / SS->fHSA[ 0 ] ) * SS->fHSYL[ 1 ];

         // save history
         SS->fHSXL[ 1 ] =  SS->fHSXL[ 0 ];
         SS->fHSXL[ 0 ] =  fHSInput;
         SS->fHSYL[ 1 ] =  SS->fHSYL[ 0 ];
         SS->fHSYL[ 0 ] =  fOutput + _FX_DENORMVAL;

         // store output
         lpBuildDest[ dwIndex ]  += f2i(fOutput);

         // get input sample (right)
         fInput   =  (F32)dwTempBuild[ dwIndex + 1 ];

         // calculate output
         fHSInput =  ( SS->fLSB[ 0 ] / SS->fLSA[ 0 ] ) * fInput + \
                     ( SS->fLSB[ 1 ] / SS->fLSA[ 0 ] ) * SS->fLSXR[ 0 ] + \
                     ( SS->fLSB[ 2 ] / SS->fLSA[ 0 ] ) * SS->fLSXR[ 1 ] - \
                     ( SS->fLSA[ 1 ] / SS->fLSA[ 0 ] ) * SS->fLSYR[ 0 ] - \
                     ( SS->fLSA[ 2 ] / SS->fLSA[ 0 ] ) * SS->fLSYR[ 1 ];

         // save history
         SS->fLSXR[ 1 ] =  SS->fLSXR[ 0 ];
         SS->fLSXR[ 0 ] =  fInput;
         SS->fLSYR[ 1 ] =  SS->fLSYR[ 0 ];
         SS->fLSYR[ 0 ] =  fHSInput + _FX_DENORMVAL;

         // calculate output
         fOutput  =  ( SS->fHSB[ 0 ] / SS->fHSA[ 0 ] ) * fHSInput + \
                     ( SS->fHSB[ 1 ] / SS->fHSA[ 0 ] ) * SS->fHSXR[ 0 ] + \
                     ( SS->fHSB[ 2 ] / SS->fHSA[ 0 ] ) * SS->fHSXR[ 1 ] - \
                     ( SS->fHSA[ 1 ] / SS->fHSA[ 0 ] ) * SS->fHSYR[ 0 ] - \
                     ( SS->fHSA[ 2 ] / SS->fHSA[ 0 ] ) * SS->fHSYR[ 1 ];

         // save history
         SS->fHSXR[ 1 ] =  SS->fHSXR[ 0 ];
         SS->fHSXR[ 0 ] =  fHSInput;
         SS->fHSYR[ 1 ] =  SS->fHSYR[ 0 ];
         SS->fHSYR[ 0 ] =  fOutput + _FX_DENORMVAL;

         // store output
         lpBuildDest[ dwIndex + 1 ]  += f2i(fOutput);
      }
   }
   else
   {
      // mix into build buffer
      for( dwIndex = 0; dwIndex < dwTemp; dwIndex++ )
      {
         // get input sample
         fInput   =  (F32)dwTempBuild[ dwIndex ];

         // calculate output
         fHSInput =  ( SS->fLSB[ 0 ] / SS->fLSA[ 0 ] ) * fInput + \
                     ( SS->fLSB[ 1 ] / SS->fLSA[ 0 ] ) * SS->fLSXL[ 0 ] + \
                     ( SS->fLSB[ 2 ] / SS->fLSA[ 0 ] ) * SS->fLSXL[ 1 ] - \
                     ( SS->fLSA[ 1 ] / SS->fLSA[ 0 ] ) * SS->fLSYL[ 0 ] - \
                     ( SS->fLSA[ 2 ] / SS->fLSA[ 0 ] ) * SS->fLSYL[ 1 ];

         // save history
         SS->fLSXL[ 1 ] =  SS->fLSXL[ 0 ];
         SS->fLSXL[ 0 ] =  fInput;
         SS->fLSYL[ 1 ] =  SS->fLSYL[ 0 ];
         SS->fLSYL[ 0 ] =  fHSInput + _FX_DENORMVAL;

         // calculate output
         fOutput  =  ( SS->fHSB[ 0 ] / SS->fHSA[ 0 ] ) * fHSInput + \
                     ( SS->fHSB[ 1 ] / SS->fHSA[ 0 ] ) * SS->fHSXL[ 0 ] + \
                     ( SS->fHSB[ 2 ] / SS->fHSA[ 0 ] ) * SS->fHSXL[ 1 ] - \
                     ( SS->fHSA[ 1 ] / SS->fHSA[ 0 ] ) * SS->fHSYL[ 0 ] - \
                     ( SS->fHSA[ 2 ] / SS->fHSA[ 0 ] ) * SS->fHSYL[ 1 ];

         // save history
         SS->fHSXL[ 1 ] =  SS->fHSXL[ 0 ];
         SS->fHSXL[ 0 ] =  fHSInput;
         SS->fHSYL[ 1 ] =  SS->fHSYL[ 0 ];
         SS->fHSYL[ 0 ] =  fOutput + _FX_DENORMVAL;

         // store output
         lpBuildDest[ dwIndex ]  += f2i(fOutput);
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
         case  _FX_SHELFEQ_LOWFREQ   :

               return( (S32) float_as_long(SS->fLowShelfFreq ));

               break;

         case  _FX_SHELFEQ_LOWGAIN   :

               return( (S32) float_as_long(SS->fLowShelfGain ));

               break;

         case  _FX_SHELFEQ_HIGHFREQ   :

               return( (S32) float_as_long(SS->fHighShelfFreq ));

               break;

         case  _FX_SHELFEQ_HIGHGAIN   :

               return( (S32) float_as_long(SS->fHighShelfGain ));

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

   S32 prev = -1;
   F32 fRate;

   // determine preference
   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //
      case  _FX_SHELFEQ_LOWFREQ   :

            // get previous value and set new
            prev                = (S32) float_as_long(SS->fLowShelfFreq );
            SS->fLowShelfFreq   = *(F32 FAR*)value;

            // get sample rate
            fRate = SS->driver->rate;

            // clip frequency to valid range
            FX_CLIPRANGE( SS->fLowShelfFreq, 20.0F, ( fRate / 2.0F ) );

            break;

      case  _FX_SHELFEQ_LOWGAIN   :

            // get previous value and set new
            prev                = (S32) float_as_long(SS->fLowShelfGain );
            SS->fLowShelfGain   = *(F32 FAR*)value;

            // get sample rate
            fRate = SS->driver->rate;

            // clip frequency to valid range
            FX_CLIPRANGE( SS->fLowShelfGain, -20.0F, 20.0F );

            break;

      case  _FX_SHELFEQ_HIGHFREQ   :

            // get previous value and set new
            prev                = (S32) float_as_long(SS->fHighShelfFreq );
            SS->fHighShelfFreq  = *(F32 FAR*)value;

            // get sample rate
            fRate = SS->driver->rate;

            // clip frequency to valid range
            FX_CLIPRANGE( SS->fHighShelfFreq, 20.0F, ( fRate / 2.0F ) );

            break;

      case  _FX_SHELFEQ_HIGHGAIN   :

            // get previous value and set new
            prev                = (S32) float_as_long(SS->fHighShelfGain );
            SS->fHighShelfGain  = *(F32 FAR*)value;

            // get sample rate
            fRate = SS->driver->rate;

            // clip frequency to valid range
            FX_CLIPRANGE( SS->fHighShelfGain, -20.0F, 20.0F );

            break;

      }

      // update params
      FXCalcParams( SS, S );

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

      REG_AT("ShelvingEQ LowFreq",             _FX_SHELFEQ_LOWFREQ,         RIB_FLOAT),
      REG_PR("ShelvingEQ LowFreq",             _FX_SHELFEQ_LOWFREQ,         RIB_FLOAT),
      REG_AT("ShelvingEQ LowGain",             _FX_SHELFEQ_LOWGAIN,         RIB_FLOAT),
      REG_PR("ShelvingEQ LowGain",             _FX_SHELFEQ_LOWGAIN,         RIB_FLOAT),
      REG_AT("ShelvingEQ HighFreq",            _FX_SHELFEQ_HIGHFREQ,         RIB_FLOAT),
      REG_PR("ShelvingEQ HighFreq",            _FX_SHELFEQ_HIGHFREQ,         RIB_FLOAT),
      REG_AT("ShelvingEQ HighGain",            _FX_SHELFEQ_HIGHGAIN,         RIB_FLOAT),
      REG_PR("ShelvingEQ HighGain",            _FX_SHELFEQ_HIGHGAIN,         RIB_FLOAT),
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
