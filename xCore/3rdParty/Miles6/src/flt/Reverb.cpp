//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: FLT module for demonstration echo effect                     ##
//##                                                                        ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 5-Feb-99: Initial                                     ##
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
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "mss.h"
#include "imssapi.h"

#define DEFAULT_REVERB_BUFFER_SIZE 65536*16     // 1 meg reverb buffer by default

#define DEFAULT_REVERB_LEVEL 0.0F               // Reverb off by default
#define DEFAULT_REFLECT_TIME 0.030F             // 30-millisecond reflections by default
#define DEFAULT_DECAY_TIME   1.493F             // Equivalent to EAX_PRESET_GENERIC

//
// Attribute tokens
//

enum ATTRIB
{
   //
   // Additional filter attribs (beyond standard RIB PROVIDER_ attributes)
   //

   REQUESTED_REVERB_BUFFER_SIZE,  // S32 size of reverb buffer in bytes

   //
   // Sample-specific filter attributes/preferences
   //

   REVERB_LEVEL,              // F32 level, from 0.0F to 1.0F
   REVERB_REFLECT_TIME,       // F32 reflection time in milliseconds
   REVERB_DECAY_TIME,         // F32 time over which reflections decay to -60 dB

   ACTUAL_REVERB_SIZE,        // S32 actual size of buffer associated with this sample's driver
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

   S32 FAR *reverb_buffer;
   S32      reverb_buffer_size;
   S32      reverb_buffer_position;
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

   F32 reverb_level;          // Level [0.0, 1.0]
   F32 reverb_reflect_time;   // Reflect time in milliseconds
   F32 reverb_decay_time;     // Decay time [0.1, 20.0]
};

//
// Globals
//

S32 FLT_started = 0;

C8 FLT_error_text[256];

S32 requested_reverb_buffer_size;

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
      case PROVIDER_NAME:      return (U32) "Miles Reverb";
      case PROVIDER_VERSION:   return 0x100;

      case REQUESTED_REVERB_BUFFER_SIZE: return requested_reverb_buffer_size;
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

   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //

      case REQUESTED_REVERB_BUFFER_SIZE:

         prev = requested_reverb_buffer_size;
         requested_reverb_buffer_size = *(S32 FAR*)value;
         break;
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
   requested_reverb_buffer_size = DEFAULT_REVERB_BUFFER_SIZE;

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
   // Initialize provider-specific members to their default values
   //

   DRV->reverb_buffer_size     = requested_reverb_buffer_size;
   DRV->reverb_buffer_position = 0;

   //
   // Round actual reverb buffer size down to the nearest multiple of the
   // build buffer size (our algorithm doesn't handle wraparound)
   //

   DRV->reverb_buffer_size = 
      (DRV->reverb_buffer_size / DRV->build_buffer_size) * 
       DRV->build_buffer_size;

   if (DRV->reverb_buffer_size == 0)
      {
      DRV->reverb_buffer_size = DRV->build_buffer_size;
      }

   //
   // Allocate and initialize reverb buffer
   //

   DRV->reverb_buffer = (S32 FAR *) AIL_mem_alloc_lock(DRV->reverb_buffer_size);

   if (DRV->reverb_buffer == NULL)
      {
      strcpy(FLT_error_text,"Could not allocate reverb buffer");
      return NULL;
      }

   AIL_memset(DRV->reverb_buffer,
              0,
              DRV->reverb_buffer_size);

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

   if (DRV->reverb_buffer != NULL)
      {
      AIL_mem_free_lock(DRV->reverb_buffer);
      DRV->reverb_buffer = NULL;
      }

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

static void AILEXPORT FLT_postmix_process (HDRIVERSTATE driver
#ifdef IS_MAC
                                           ,U32 buffer_size
#endif
)
{
   DRIVERSTATE FAR *DRV = (DRIVERSTATE FAR *) driver;

#ifdef IS_MAC  
   DRV->build_buffer_size = buffer_size;
#endif
  
   //
   // Add reverb buffer contents, if any, to build buffer
   //

   S32 FAR *src  = (S32 FAR *) AIL_ptr_add(DRV->reverb_buffer,
                                           DRV->reverb_buffer_position);
   S32 FAR *dest =  DRV->build_buffer;

   S32 i = DRV->build_buffer_size / sizeof(S32);

   while (i--)
      {
      dest[i] += src[i];
      }

   //
   // Clear this segment of reverb buffer and advance write pointer
   //

   AIL_memset(src,
              0,
              DRV->build_buffer_size);

   DRV->reverb_buffer_position += DRV->build_buffer_size;

   if (DRV->reverb_buffer_position >= DRV->reverb_buffer_size)
      {
      DRV->reverb_buffer_position = 0;
      }
}

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

   SS->reverb_level        = DEFAULT_REVERB_LEVEL;
   SS->reverb_reflect_time = DEFAULT_REFLECT_TIME;
   SS->reverb_decay_time   = DEFAULT_DECAY_TIME;

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
                                    void FAR * FAR *orig_src,
                                    U32        FAR *orig_src_fract,
                                    void FAR       *orig_src_end,
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
   //
   // Reverb algorithm details:
   //
   // Apply reflections to contents of this buffer segment, until
   // either (a) the volume scalar has dropped below 60 dB down (< 2 
   // on a 2048-unit linear scale); or (b) the end of the reverb buffer
   // is reached via wraparound to the reverb buffer's write cursor.
   //
   // The period of each reflection is determined by the reverb voice's
   // reflection time.
   //
   // The initial volume is determined by scaling the base sample volume
   // by the reverb voice's level.
   //
   // Prior to each reflection, the 0-2048 volume is dropped by a factor 
   // which will reduce it to 2 in the number of reflection intervals which
   // equals the period specified by the reverb buffer's decay time.
   //
   // The initial dest buffer offset for the first reflection is determined
   // by adding the reflection time to the reverb buffer's current_offset.
   // Subsequent reflection offsets are determined by adding the reflection
   // period to the last reflection's offset.
   //
   // The reverb buffer's write cursor, however, is advanced only when the
   // reverb buffer data is added to the build buffer contents in the postmix
   // stage.  In the postmix stage, the contents of the reverb buffer are
   // added into the output build buffer, and the buffer's write cursor
   // is advanced accordingly.
   //

   SAMPLESTATE FAR *SS = (SAMPLESTATE FAR *) state;

   HSAMPLE          S   = SS->sample;
   DRIVERSTATE FAR *DRV = SS->driver;

   //
   // Fail if reverb parms invalid or disabled
   //

   if ((SS->reverb_reflect_time < 0.0001F) ||
       (SS->reverb_level        < 0.001F))
      {
      return 1;
      }

   //
   // Get initial destination offset
   //
   // The size of the reverb buffer is a multiple of the build buffer size,
   // and this position is guaranteed to be aligned to a 
   // build-buffer-size boundary
   //

   S32 dest_offset = DRV->reverb_buffer_position;

   //
   // Adjust destination offset by difference between build_dest pointer and
   // beginning of build buffer
   //
   // This handles the cases where buffer-switching, looping, etc.
   // require multiple calls to the mixer to fill the build buffer
   // completely
   //

   dest_offset += AIL_ptr_dif(*build_dest,
                               DRV->build_buffer);

   S32 FAR *write_cursor = (S32 FAR *) AIL_ptr_add(DRV->reverb_buffer,
                                                   dest_offset);

   //
   // Calculate # of bytes in reverb buffer that correspond to 
   // reflection time interval
   //

   F32 adv = F32(AIL_sample_playback_rate(S)) * SS->reverb_reflect_time;

   S32 reflection_advance = (S32)(adv) * 
                           ((mixer_operation & M_DEST_STEREO) ? 8 : 4);

   //
   // Calculate factor by which to decrease amplitude each reflection
   //
   // We want the volume to fall to 2/2048 (1/1024=-60 dB) after 
   // reverb_decay_time seconds elapses.  Each reflection interval is
   // reverb_reflect_time seconds long, so the d/r quotient is the total
   // number of reflection periods over which the volume decay should occur.
   //

   S32 decay_intervals = (S32)(SS->reverb_decay_time / SS->reverb_reflect_time);

   if (decay_intervals == 0)
      {
      decay_intervals = 1;
      }

   S32 volume_decay = (base_scale - 2) / decay_intervals;

   if (volume_decay == 0)
      {
      volume_decay = 1;
      }

   //
   // Get (potential) amount of destination data to write to reverb 
   // buffer for each reflection interval
   //

   S32 max_dest_bytes = AIL_ptr_dif(build_dest_end, *build_dest);

   S32 FAR *reverb_end = (S32 FAR *) AIL_ptr_add(DRV->reverb_buffer, 
                                                 DRV->reverb_buffer_size);

   //
   // Modify mixer operation code to support reverb (always use volume 
   // scaling, never do filtering)
   //

   mixer_operation |=  M_VOL_SCALING;
   mixer_operation  &= ~M_FILTER;

   //
   // Generate reflections in reverb buffer until buffer full or sound
   // dies out
   // 

   S32 l_scale = (S32) (((F32) left_scale)  * SS->reverb_level);
   S32 r_scale = (S32) (((F32) right_scale) * SS->reverb_level);
   S32 b_scale = (S32) (((F32) base_scale)  * SS->reverb_level);

   S32  FAR *dest;
   void FAR *dest_end;

   S32 wrapped = 0;
   
   while (1)
      {
      //
      // Add one reflection period to dest buffer offset
      //

      dest_offset += reflection_advance;

      if (dest_offset >= DRV->reverb_buffer_size)
         {
         dest_offset -= DRV->reverb_buffer_size;
         }

      //
      // Decrease amplitude by decay value; exit if -60 dB point reached
      //

      b_scale = max(b_scale  - volume_decay, 0);
      l_scale = max(l_scale  - volume_decay, 0);
      r_scale = max(r_scale - volume_decay, 0);

      if (b_scale <= 2)
         {
         break;
         }

      --decay_intervals;

      //
      // Add reflection to buffer, handling buffer wrap as needed
      //

      S32 l_val = *left_val;
      S32 r_val = *right_val;

      dest     = (S32 FAR *) AIL_ptr_add(DRV->reverb_buffer, dest_offset);
      dest_end =             AIL_ptr_add(dest,              max_dest_bytes);

      void FAR *source        = *orig_src;
      U32       source_fract  = *orig_src_fract;
      void FAR *source_end    = orig_src_end;

      while (1)
         {
         if (AIL_ptr_dif(dest, reverb_end) == 0)
            {
            S32 size = AIL_ptr_dif(dest_end, dest);
            dest     = DRV->reverb_buffer;
            dest_end = AIL_ptr_add(dest, size);
            continue;
            }

         S32 remnant = AIL_ptr_dif(dest_end, reverb_end);

         if (remnant > 0)
            {
            dest_end = reverb_end;
            }

         if ((AIL_ptr_dif(write_cursor, dest)     > 0) &&
             (AIL_ptr_dif(dest_end, write_cursor) > 0))
            {
            dest_end = write_cursor;
            remnant  = 0;
            }

         mixer_provider->MSS_mixer_merge((void const**)&source,
                                         &source_fract,
                                          source_end,
                       (S32 FAR * FAR *) &dest,
                                          dest_end,
                                         &l_val,
                                         &r_val,
                                          playback_ratio,
                                          l_scale,
                                          r_scale,
                                          mixer_operation
#ifdef IS_X86
                                    ,AIL_MMX_available()
#endif
                                    );

         if (remnant > 0)
            {
            dest     = DRV->reverb_buffer;
            dest_end = AIL_ptr_add(dest, remnant);
            }
         else
            {
            break;
            }
         }
      }

   //
   // Return 1 to allow mixing process to run normally
   //

   return 1;
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
      case REVERB_LEVEL:         return float_as_long(SS->reverb_level);
      case REVERB_REFLECT_TIME:  return float_as_long(SS->reverb_reflect_time);
      case REVERB_DECAY_TIME:    return float_as_long(SS->reverb_decay_time);
      case ACTUAL_REVERB_SIZE:   return SS->driver->reverb_buffer_size;
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
   SAMPLESTATE FAR *SS = (SAMPLESTATE FAR *) state;

   S32 prev = -1;

   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //

      case REVERB_LEVEL:

         prev = (S32) float_as_long(SS->reverb_level);
         SS->reverb_level = *(F32 FAR*)value;
         break;

      case REVERB_REFLECT_TIME:

         prev = (S32) float_as_long(SS->reverb_reflect_time);
         SS->reverb_reflect_time = *(F32 FAR*)value;
         break;

      case REVERB_DECAY_TIME:

         prev = (S32) float_as_long(SS->reverb_decay_time);
         SS->reverb_decay_time = *(F32 FAR*)value;
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

      REG_AT("Requested reverb buffer size", REQUESTED_REVERB_BUFFER_SIZE,   RIB_DEC),
      REG_PR("Requested reverb buffer size", REQUESTED_REVERB_BUFFER_SIZE,   RIB_DEC),
      };

   const RIB_INTERFACE_ENTRY FLTSMP[] =
      {
      REG_FN(FLTSMP_open_sample),
      REG_FN(FLTSMP_close_sample),

      REG_FN(FLTSMP_sample_process),

      REG_FN(FLTSMP_sample_attribute),
      REG_FN(FLTSMP_set_sample_preference),

      REG_AT("Reverb level",             REVERB_LEVEL,         RIB_FLOAT),
      REG_AT("Reverb reflect time",      REVERB_REFLECT_TIME,  RIB_FLOAT),
      REG_AT("Reverb decay time",        REVERB_DECAY_TIME,    RIB_FLOAT),
      REG_AT("Actual reverb buffer size",ACTUAL_REVERB_SIZE,   RIB_DEC),

      REG_PR("Reverb level",             REVERB_LEVEL,         RIB_FLOAT),
      REG_PR("Reverb reflect time",      REVERB_REFLECT_TIME,  RIB_FLOAT),
      REG_PR("Reverb decay time",        REVERB_DECAY_TIME,    RIB_FLOAT),
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
