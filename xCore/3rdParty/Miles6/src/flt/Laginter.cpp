//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: FLT module for Lagrangian 2nd order interpolator             ##
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
//##  Contact RAD Game Tools at 801-322-4300 for technical support.         ##
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

// Clip Range Macro
#define  FX_CLIPRANGE( p, min, max ) \
            if ( (p) > (max) ) \
               (p) = (max); \
            else \
               if ( (p) < (min) ) \
                  (p) = (min);

#define BUILD_BUFFER_SHIFT(val) ((val)<<11)

//
// Attribute tokens
//

enum ATTRIB
{
   //
   // Additional filter attribs (beyond standard RIB PROVIDER_ attributes)
   //
   _FX_LAG_ORDER
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
   F32    fRate;

   // coefficients
   F32    fC0;
   F32    fC1;
   F32    fC2;

   // sample history
   F32    fXL1;
   F32    fXL2;
   F32    fXL3;
   F32    fXR1;
   F32    fXR2;
   F32    fXR3;

   // fractional part
   F32    fFraction;

   // filter order
   F32    fOrder;

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
      case PROVIDER_NAME:      return (U32) "Lagrangian Interpolator";
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
   DRIVERSTATE FAR * DRV = (DRIVERSTATE FAR *) driver;
   S32               dwRate;

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

   // reset history
   SS->fXL1    =  0.0F;
   SS->fXL2    =  0.0F;
   SS->fXR1    =  0.0F;
   SS->fXR2    =  0.0F;

   // set order
   SS->fOrder  =  3.0f;

   // get rate
   AIL_digital_configuration( DRV->dig, &dwRate, 0, 0 );

   // init driver rate
   SS->fRate   =  (F32)dwRate;

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
   S16 const *  lpOrigSrc      =  (S16 *)*orig_src;
   S16 const *  lpOrigEnd      =  (S16 *)orig_src_end;
   S32   FAR   *  lpBuildDest    =  *build_dest;
   S32   FAR   *  lpBuildDestEnd =  (S32 FAR *)build_dest_end;

   U32    dwBuildBytes   =  AIL_ptr_dif( build_dest_end, *build_dest );
   U32    dwIndex;
   U32    dwDestLen,dwSrcLen;
   F32      fInput;
   F32      fOutput;
   F32      fC0, fC1, fC2, fC3;
   U32    dwInputPtr;
   F32    fFraction;
   F64    fInteger;
   F32    fPointer;

   // calculate the amount the mixer mixed into the build buffer
   // and convert to samples i.e. divide by sample size
   dwDestLen   =  AIL_ptr_dif( lpBuildDestEnd, lpBuildDest ) / 4;

   // we want to stop after we've processed the final sample 
   dwSrcLen   =  AIL_ptr_dif( lpOrigEnd, lpOrigSrc ) / 2;
   
   if ( mixer_operation & M_SRC_STEREO )
     dwSrcLen -= 2;
   else
     dwSrcLen -= 1;

   // get fraction
   SS->fFraction  =  (F32)(AIL_sample_playback_rate(S)) / (F32)(SS->fRate);

   // check if mono or stereo
   if ( mixer_operation & M_DEST_STEREO )
   {
      // reset build index
      dwIndex  =  0;

      // reset pointer
      fPointer =  0.0;

      // process
      do
      {
         // get fractional part
         fFraction   =  (F32)modf( fPointer, &fInteger );

         // advance pointer
         fPointer    += SS->fFraction;

         // determine order, compute appropriate coefficients
         switch( (U32)SS->fOrder )
         {
            case  0  :

                  // compute coefficients
                  fC0      =  fFraction;
                  fC1      =  1.0F  -  fFraction;

                  break;

            case  1  :

                  // compute coefficients
                  fC0      =  0.5F * fFraction * ( fFraction + 1.0F );
                  fC1      =  -( fFraction - 1.0F ) * ( fFraction + 1.0F );
                  fC2      =  0.5F * fFraction * ( fFraction - 1.0F );

                  break;

            case  2  :

                  // compute coefficients
                  fC0      =  0.5F * ( fFraction - 1.0F ) * ( fFraction - 2.0F );
                  fC1      =  -fFraction * ( fFraction - 2.0F );
                  fC2      =  0.5f * fFraction * ( fFraction - 1.0F );

                  break;

            case  3  :

                  // compute coefficients
                  fC0   =  -0.166F  * ( fFraction - 1.0F ) * ( fFraction - 2.0F ) * ( fFraction - 3.0F );
                  fC1   =  0.5F     * fFraction * ( fFraction - 2.0F ) * ( fFraction - 3.0F );
                  fC2   =  -0.5F    * fFraction * ( fFraction - 1.0F ) * ( fFraction - 3.0F );
                  fC3   =  0.166F   * fFraction * ( fFraction - 1.0F ) * ( fFraction - 2.0F );

                  break;
         }

         // determine if mono or stereo
         if ( mixer_operation & M_SRC_STEREO )
         {
            // get pointer
            dwInputPtr  =  (U32)fInteger * 2;

            // determine order
            switch( (U32)SS->fOrder )
            {
               case  0  :

                     // get input sample (left)  
                     fInput   =  (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr ]);

                     // calculate sample
                     fOutput  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     // get input sample (right)
                     fInput   =  (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr + 1 ]);

                     // calculate sample
                     fOutput  =  fInput;

                     // save history
                     SS->fXR3  =  SS->fXR2;
                     SS->fXR2  =  SS->fXR1;
                     SS->fXR1  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     break;

               case  1  :

                     // get input sample (left)
                     fInput   =  (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr ]);

                     // calculate sample
                     fOutput  =  fC0 * SS->fXL1 + fC1 * fInput;

                     // save history
                     SS->fXL3  =  SS->fXL2;
                     SS->fXL2  =  SS->fXL1;
                     SS->fXL1  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     // get input sample (right)
                     fInput   =  (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr + 1 ]);

                     // calculate sample
                     fOutput  =  fC0 * SS->fXR1 + fC1 * fInput;

                     // save history
                     SS->fXR3  =  SS->fXR2;
                     SS->fXR2  =  SS->fXR1;
                     SS->fXR1  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     break;

               case  2  :

                     // get input sample (left)
                     fInput   =  (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr ]);

                     // calculate sample
                     fOutput  =  fC0 * SS->fXL2 + fC1 * SS->fXL1 + fC2 * fInput;

                     // save history
                     SS->fXL3  =  SS->fXL2;
                     SS->fXL2  =  SS->fXL1;
                     SS->fXL1  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     // get input sample (right)
                     fInput   =  (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr + 1 ]);

                     // calculate sample
                     fOutput  =  fC0 * SS->fXR2 + fC1 * SS->fXR1 + fC2 * fInput;

                     // save history
                     SS->fXR3  =  SS->fXR2;
                     SS->fXR2  =  SS->fXR1;
                     SS->fXR1  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     break;

               case  3  :

                     // get input sample (left)
                     fInput   = (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr ]);

                     // calculate sample
                     fOutput  =  fC0 * SS->fXL3 + fC1 * SS->fXL2 + fC2 * SS->fXL1 + fC3 * fInput;

                     // save history
                     SS->fXL3  =  SS->fXL2;
                     SS->fXL2  =  SS->fXL1;
                     SS->fXL1  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     // get input sample (right)
                     fInput   =  (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr + 1 ]);

                     // calculate sample
                     fOutput  =  fC0 * SS->fXR3 + fC1 * SS->fXR2 + fC2 * SS->fXR1 + fC3 * fInput;

                     // save history
                     SS->fXR3  =  SS->fXR2;
                     SS->fXR2  =  SS->fXR1;
                     SS->fXR1  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     break;

            }
         }
         else
         {
            // get pointer
            dwInputPtr  =  (U32)fInteger;

            // determine order
            switch( (U32)SS->fOrder )
            {
               case  0  :

 
                     // get input sample (left)
                     fInput   =  (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr ]);

                     // calculate sample
                     fOutput  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     break;

               case  1  :

                     // get input sample (left)
                     fInput   =  (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr ]);

                     // calculate sample
                     fOutput  =  fC0 * SS->fXL1 + fC1 * fInput;

                     // save history
                     SS->fXL3  =  SS->fXL2;
                     SS->fXL2  =  SS->fXL1;
                     SS->fXL1  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     break;

               case  2  :

                     // get input sample (left)
                     fInput   =  (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr ]);

                     // calculate sample
                     fOutput  =  fC0 * SS->fXL2 + fC1 * SS->fXL1 + fC2 * fInput;

                     // save history
                     SS->fXL3  =  SS->fXL2;
                     SS->fXL2  =  SS->fXL1;
                     SS->fXL1  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     break;

               case  3  :

                     // get input sample (left)
                     fInput   = (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr ]);

                     // calculate sample
                     fOutput  = fC0 * SS->fXL3 + fC1 * SS->fXL2 + fC2 * SS->fXL1 + fC3 * fInput;

                     // save history
                     SS->fXL3  =  SS->fXL2;
                     SS->fXL2  =  SS->fXL1;
                     SS->fXL1  =  fInput;

                     // store output
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );
                     lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );

                     break;

            }
         }

      }
      while( ( dwIndex < dwDestLen ) && ( dwInputPtr < dwSrcLen ) );

      // set new source pointer
      *orig_src   =  lpOrigSrc + dwInputPtr;
   }
   else
   {
      // reset build index
      dwIndex  =  0;

      // reset pointer
      fPointer =  0.0;

      // process
      do
      {
         // get fractional part
         fFraction   =  (F32)modf( fPointer, &fInteger );

         // advance pointer
         fPointer    += SS->fFraction;

         // compute 2nd order coefficients
         fC0      =  0.5F * ( fFraction - 1.0F ) * ( fFraction - 2.0F );
         fC1      =  -fFraction * ( fFraction - 2.0F );
         fC2      =  0.5f * fFraction * ( fFraction - 1.0F );

         // get pointer
         if ( mixer_operation & M_SRC_STEREO )
            dwInputPtr  =  (U32)fInteger * 2;
         else
            dwInputPtr  =  (U32)fInteger;

         // get input sample (left)
         fInput   =  (F32)(S16)LE_SWAP16(&lpOrigSrc[ dwInputPtr ]);

         // calculate sample
         fOutput  =  fC0 * SS->fXL2 + fC1 * SS->fXL1 + fC2 * fInput;

         // save history
         SS->fXL2  =  SS->fXL1;
         SS->fXL1  =  fInput;

         // store output
         lpBuildDest[ dwIndex++ ]  += BUILD_BUFFER_SHIFT( ( S32 ) fOutput );
      }
      while( dwIndex < dwDestLen && ( dwInputPtr < dwSrcLen ));

      // set new source pointer
      *orig_src   =  lpOrigSrc + dwInputPtr;
   }

   // advance pointer
   *build_dest  += dwIndex;

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
         case  _FX_LAG_ORDER  :

               return( (S32) float_as_long(SS->fOrder ));

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

   // determine preference
   switch ((ATTRIB) preference)
      {
      case  _FX_LAG_ORDER  :

            // get previous value and set new
            prev        = (S32) float_as_long(SS->fOrder );
            SS->fOrder  = *(F32 FAR*)value;

            // clip to valid range
            FX_CLIPRANGE( SS->fOrder, 0.0F, 3.0F );
            
            // display order
//            printf( "Order = %f", SS->fOrder );

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

      REG_AT("LaGrange Filter Order",                   _FX_LAG_ORDER,         RIB_FLOAT),
      REG_PR("LaGrange Filter Order",                   _FX_LAG_ORDER,         RIB_FLOAT),
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
