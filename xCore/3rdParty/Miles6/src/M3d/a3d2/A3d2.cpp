//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: M3D module for Aureal A3D 2.0 provider                       ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 26-Apr-99: Initial                                    ##
//##                                                                        ##
//##  Author: Dan Teven                                                     ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#define INITGUID

#define diag_printf(str) {}
//#define  CHECK_API_RETURN_VALUES 1     // Sanity-check all A3D return codes
//#define  CYCLE_ROOM_TYPES        1     // Try a new room type on every sound
//#define  DEBUG_STREAMING         1     // Monitor buffer streaming
//#define  DEBUG_STARVATION        1     // Check for buffer starvation
//#define  EXPAND_A3D_ERRORS       1     // Expand error codes into text

#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <process.h>
#include <objbase.h>

#include "ia3dapi.h"
#include "ia3dutil.h"

#include "mss.h"


S32 M3D_started = 0;

C8 M3D_error_text[256];

//
// Additional attributes and preferences
//

enum ATTRIB
{
   //
   // Provider attribs
   //

   MAX_SUPPORTED_SAMPLES,        // Maximum number of samples supported
   TECHNOLOGY_VERSION,           // Version of the mixer DLL
   TECHNOLOGY_INTERFACE,         // Top-level A3D interface
   RENDER_1ST_REFLECTIONS,       // Rendering of first reflections supported?
   RENDER_OCCLUSIONS,            // Rendering of occlusions supported?

   //
   // Provider prefs
   //

   SOFTWARE_MIXING,              // Mix in software if no A3D hardware
   PRIORITY_BIAS,                // Priority/audibility bias for resource mgr
   OUTPUT_GAIN,                  // Master output gain
   FLUSH_TIMEOUT,                // Milliseconds between background Flush()
   MAX_REFLECTION_DELAY,         // Maximum reflection delay time
   UNITS_PER_METER,              // Application units per meter
   VIRTUAL_GEOMETRY,             // Create virtual geometry for room types

   //
   // Provider and per-sample preferences
   //

   DISTANCE_SCALE,               // Distance model scale
   DOPPLER_SCALE,                // Doppler scale
   EQUALIZATION,                 // Equalization

   //
   // Per-sample attribs
   //

   AUDIBILITY,                   // Distance-adjusted volume
   OCCLUSION_FACTOR,             // Occlusion factor

   //
   // Per-sample preferences
   //

   REFLECTION_DELAY_SCALE,       // Reflection delay scale
   REFLECTION_GAIN_SCALE,        // Reflection gain scale
   PRIORITY,                     // Priority
   RENDER_MODE,                  // A3DSOURCE_RENDERMODE_xxx
};

//
// Event IDs for thread communication
//

#define BUFFER1_EVENT (0)
#define BUFFER2_EVENT (1)

#define NUM_EVENTS    (2)

//
// Use 16K half-buffers by default
//

#define BUFF_SIZE 16384

//
// Service object positions every 100 milliseconds
//

#define SERVICE_MSECS 100

//
// Epsilon value used for FP comparisons with 0
//

#define EPSILON 0.0001F

//
// Object types
//

enum OBJTYPE
{
   IS_SAMPLE,
   IS_LISTENER,
   IS_OBJECT
};

//
// Sample descriptor
//

struct SAMPLE3D
{
   OBJTYPE  type;                // Runtime type of object

   U32      index;               // index of this sample
   U32      status;              // SMP_ flags: _FREE, _DONE, _PLAYING

   void const FAR *start;        // Sample buffer address (W)
   U32       len  ;              // Sample buffer size in bytes (W)
   U32       pos  ;              // Index to next byte (R/W)
   U32       done ;              // Nonzero if buffer with len=0 sent by app

   S32      loop_count;          // # of cycles-1 (1=one-shot, 0=indefinite)
   S32      loop_start;          // Starting offset of loop block (0=SOF)
   S32      loop_end;            // End offset of loop block (-1=EOF)

   S32      volume;              // Sample volume 0-127
   S32      playback_rate;       // Playback rate in samples/sec

   S32      bytes_per_sample;    // 1 or 2 for 8- or 16-bit samples

   S32      buffers_past_end;    // # of silent buffers appended

   //
   // Positioning
   //

   A3DVECTOR   position;         // 3D position
   A3DVECTOR   face;             // 3D orientation
   A3DVECTOR   up;               // 3D up-vector
   A3DVECTOR   velocity;         // 3D velocity
   S32         auto_update;      // TRUE to automatically update in background

   F32         max_dist;         // Sample distances
   F32         min_dist;

   volatile S32 cancel_pending;      // we took an early exit from sleep_sample(), don't do anything else

   LPA3DSOURCE lpSrc;                  // Audio source object

   U32         lastblockdone;          // estimated time when last mix will be done

   F32         obstruction;            // amount to attenuate primary signal
   F32         effects_level;          // amount to attenuate reflections
   F32         occlusion;              // amount to attenuate both

   F32         inner_angle;
   F32         outer_angle;
   S32         outer_volume;

   AIL3DSAMPLECB eos;
   H3DSAMPLE clientH3D;
   S32         docb;
};


//
// Limit to 64 voices, or DIG_MIXER_CHANNELS, whichever is lower
//

#define N_SAMPLES 64

SAMPLE3D samples[N_SAMPLES];

S32 avail_samples = 0;

//
// Globals
//

S32 active = 0;

LPA3D4        lpA3D;
LPA3DLISTENER lpA3DListener;
LPA3DGEOM     lpA3DGeom;
LPA3DMATERIAL lpA3DWallMaterial;

#define     X_VAL(A3DVEC)     (A3DVEC)[0]
#define     Y_VAL(A3DVEC)     (A3DVEC)[1]
#define     Z_VAL(A3DVEC)     (A3DVEC)[2]
A3DVECTOR   listen_position;
A3DVECTOR   listen_face;
A3DVECTOR   listen_up;
A3DVECTOR   listen_velocity;
A3DVECTOR   room_vertices[8];

//
// Acoustic properties for six "materials" that we use to simulate
// different room types.  This data is mostly guesswork.
//
// The transmittance properties, unfortunately, can't be used
// to simulate specific room types, because the imaginary walls
// we create (to produce reflections) need to have occlusions
// disabled to prevent side effects.
//
#define   REFLECTANCE(M)            (M)[0]
#define   REFLECTANCE_HIFREQ(M)     (M)[1]
#define   TRANSMITTANCE(M)          (M)[2]
#define   TRANSMITTANCE_HIFREQ(M)   (M)[3]
A3DVECTOR open_air = { 0.0F, 1.0F, 1.0F, 1.0F };
A3DVECTOR carpet   = { 0.4F, 0.4F, 0.4F, 0.4F };
A3DVECTOR water    = { 0.7F, 0.3F, 0.8F, 0.3F };
A3DVECTOR wood     = { 0.8F, 0.7F, 0.5F, 0.8F };
A3DVECTOR brick    = { 0.9F, 0.8F, 0.3F, 0.9F };
A3DVECTOR glass    = { 0.9F, 0.9F, 0.2F, 1.0F };
A3DVECTOR metal    = { 1.0F, 0.9F, 0.6F, 0.9F };

S32			speaker_type;

S32         listen_auto_update        = 0;
S32         flush_timeout             = 30;
S32         g_room_type;                // values defined by EAX.H
F32         room_size;                // <= 0 means no virtual geometry
F32         units_per_meter           = 1.0F;

HTIMER      service_timer;
HTIMER		buffer_timer;

HANDLE      hFlushEvent;
HANDLE      hFlushThread;
HANDLE      hA3DMutex;

//
// Thread synchronization
//

U32 num_events=0;
HANDLE m_lpThread=0;					                            // Thread for buffer maintenance
HANDLE m_hmaster=0;                                          // master win32 event
HANDLE m_hBufferEvents[NUM_EVENTS*N_SAMPLES+1]={0};          // Buffer events for this sample
volatile S32 is_safe;			             	                // Thread is waiting for next event
volatile SAMPLE3D FAR* current_sample;

//
// These two functions are used to prevent multiple threads from
// making simultaneous calls to the A3D API, which is not reentrant,
// as well as to MSS.
//
// API_lock will block the calling thread is the mutex is already owned
// by another thread.
//

void API_lock(void)
{
	if (AIL_get_preference(AIL_LOCK_PROTECTION))
     AIL_lock();

   if (hA3DMutex)
      WaitForSingleObject (hA3DMutex, INFINITE);
}

void API_unlock(void)
{
   if (hA3DMutex)
      ReleaseMutex (hA3DMutex);

	if (AIL_get_preference(AIL_LOCK_PROTECTION))
      AIL_unlock();
}


static void OurSetEvent(SAMPLE3D FAR* S, U32 eventnum)
{
  SetEvent(m_hBufferEvents[eventnum+(S->index*NUM_EVENTS)]);
  SetEvent(m_hmaster);
}


#if defined(CHECK_API_RETURN_VALUES) && CHECK_API_RETURN_VALUES
   #define  CHECK_RESULT(code)   if (code != S_OK) A3D_set_error (code);
#else
   #define  CHECK_RESULT(code)
#endif

static void A3D_set_error (HRESULT code)
{
   char errmsg[256];

   wsprintf (errmsg, "A3D error [%08lxh]\r\n", code);
   diag_printf (errmsg);
   AIL_set_error (errmsg);

#if defined(EXPAND_A3D_ERRORS) && EXPAND_A3D_ERRORS
   char *errstr;

   switch (code)
      {
      case A3DERROR_MEMORY_ALLOCATION:
         errstr = "A3DERROR_MEMORY_ALLOCATION";
         break;
      case A3DERROR_FAILED_CREATE_PRIMARY_BUFFER:
         errstr = "A3DERROR_FAILED_CREATE_PRIMARY_BUFFER";
         break;
      case A3DERROR_FAILED_CREATE_SECONDARY_BUFFER:
         errstr = "A3DERROR_FAILED_CREATE_SECONDARY_BUFFER";
         break;
      case A3DERROR_FAILED_INIT_A3D_DRIVER:
         errstr = "A3DERROR_FAILED_INIT_A3D_DRIVER";
         break;
      case A3DERROR_FAILED_QUERY_DIRECTSOUND:
         errstr = "A3DERROR_FAILED_QUERY_DIRECTSOUND";
         break;
      case A3DERROR_FAILED_QUERY_A3D3:
         errstr = "A3DERROR_FAILED_QUERY_A3D3";
         break;
      case A3DERROR_FAILED_INIT_A3D3:
         errstr = "A3DERROR_FAILED_INIT_A3D3";
         break;
      case A3DERROR_FAILED_QUERY_A3D2:
         errstr = "A3DERROR_FAILED_QUERY_A3D2";
         break;
      case A3DERROR_FAILED_FILE_OPEN:
         errstr = "A3DERROR_FAILED_FILE_OPEN";
         break;
      case A3DERROR_FAILED_CREATE_SOUNDBUFFER:
         errstr = "A3DERROR_FAILED_CREATE_SOUNDBUFFER";
         break;
      case A3DERROR_FAILED_QUERY_3DINTERFACE:
         errstr = "A3DERROR_FAILED_QUERY_3DINTERFACE";
         break;
      case A3DERROR_FAILED_LOCK_BUFFER:
         errstr = "A3DERROR_FAILED_LOCK_BUFFER";
         break;
      case A3DERROR_FAILED_UNLOCK_BUFFER:
         errstr = "A3DERROR_FAILED_UNLOCK_BUFFER";
         break;
      case A3DERROR_UNRECOGNIZED_FORMAT:
         errstr = "A3DERROR_UNRECOGNIZED_FORMAT";
         break;
      case A3DERROR_NO_WAVE_DATA:
         errstr = "A3DERROR_NO_WAVE_DATA";
         break;
      case A3DERROR_UNKNOWN_PLAYMODE:
         errstr = "A3DERROR_UNKNOWN_PLAYMODE";
         break;
      case A3DERROR_FAILED_PLAY:
         errstr = "A3DERROR_FAILED_PLAY";
         break;
      case A3DERROR_FAILED_STOP:
         errstr = "A3DERROR_FAILED_STOP";
         break;
      case A3DERROR_NEEDS_FORMAT_INFORMATION:
         errstr = "A3DERROR_NEEDS_FORMAT_INFORMATION";
         break;
      case A3DERROR_FAILED_ALLOCATE_WAVEDATA:
         errstr = "A3DERROR_FAILED_ALLOCATE_WAVEDATA";
         break;
      case A3DERROR_NOT_VALID_SOURCE:
         errstr = "A3DERROR_NOT_VALID_SOURCE";
         break;
      case A3DERROR_FAILED_DUPLICATION:
         errstr = "A3DERROR_FAILED_DUPLICATION";
         break;
      case A3DERROR_FAILED_INIT:
         errstr = "A3DERROR_FAILED_INIT";
         break;
      case A3DERROR_FAILED_SETCOOPERATIVE_LEVEL:
         errstr = "A3DERROR_FAILED_SETCOOPERATIVE_LEVEL";
         break;
      case A3DERROR_FAILED_INIT_QUERIED_INTERFACE:
         errstr = "A3DERROR_FAILED_INIT_QUERIED_INTERFACE";
         break;
      case A3DERROR_GEOMETRY_INPUT_OUTSIDE_BEGIN_END_BLOCK:
         errstr = "A3DERROR_GEOMETRY_INPUT_OUTSIDE_BEGIN_END_BLOCK";
         break;
      case A3DERROR_INVALID_NORMAL:
         errstr = "A3DERROR_INVALID_NORMAL";
         break;
      case A3DERROR_END_BEFORE_VALID_BEGIN_BLOCK:
         errstr = "A3DERROR_END_BEFORE_VALID_BEGIN_BLOCK";
         break;
      case A3DERROR_INVALID_BEGIN_MODE:
         errstr = "A3DERROR_INVALID_BEGIN_MODE";
         break;
      case A3DERROR_INVALID_ARGUMENT:
         errstr = "A3DERROR_INVALID_ARGUMENT";
         break;
      case A3DERROR_INVALID_INDEX:
         errstr = "A3DERROR_INVALID_INDEX";
         break;
      case A3DERROR_INVALID_VERTEX_INDEX:
         errstr = "A3DERROR_INVALID_VERTEX_INDEX";
         break;
      case A3DERROR_INVALID_PRIMITIVE_INDEX:
         errstr = "A3DERROR_INVALID_PRIMITIVE_INDEX";
         break;
      case A3DERROR_MIXING_2D_AND_3D_MODES:
         errstr = "A3DERROR_MIXING_2D_AND_3D_MODES";
         break;
      case A3DERROR_2DWALL_REQUIRES_EXACTLY_ONE_LINE:
         errstr = "A3DERROR_2DWALL_REQUIRES_EXACTLY_ONE_LINE";
         break;
      case A3DERROR_NO_PRIMITIVES_DEFINED:
         errstr = "A3DERROR_NO_PRIMITIVES_DEFINED";
         break;
      case A3DERROR_PRIMITIVES_NON_PLANAR:
         errstr = "A3DERROR_PRIMITIVES_NON_PLANAR";
         break;
      case A3DERROR_PRIMITIVES_OVERLAPPING:
         errstr = "A3DERROR_PRIMITIVES_OVERLAPPING";
         break;
      case A3DERROR_PRIMITIVES_NOT_ADJACENT:
         errstr = "A3DERROR_PRIMITIVES_NOT_ADJACENT";
         break;
      case A3DERROR_OBJECT_NOT_FOUND:
         errstr = "A3DERROR_OBJECT_NOT_FOUND";
         break;
      case A3DERROR_ROOM_HAS_NO_SHELL_WALLS:
         errstr = "A3DERROR_ROOM_HAS_NO_SHELL_WALLS";
         break;
      case A3DERROR_WALLS_DO_NOT_ENCLOSE_ROOM:
         errstr = "A3DERROR_WALLS_DO_NOT_ENCLOSE_ROOM";
         break;
      case A3DERROR_INVALID_WALL:
         errstr = "A3DERROR_INVALID_WALL";
         break;
      case A3DERROR_ROOM_HAS_LESS_THAN_4SHELL_WALLS:
         errstr = "A3DERROR_ROOM_HAS_LESS_THAN_4SHELL_WALLS";
         break;
      case A3DERROR_ROOM_HAS_LESS_THAN_3UNIQUE_NORMALS:
         errstr = "A3DERROR_ROOM_HAS_LESS_THAN_3UNIQUE_NORMALS";
         break;
      case A3DERROR_INTERSECTING_WALL_EDGES:
         errstr = "A3DERROR_INTERSECTING_WALL_EDGES";
         break;
      case A3DERROR_INVALID_ROOM:
         errstr = "A3DERROR_INVALID_ROOM";
         break;
      case A3DERROR_SCENE_HAS_ROOMS_INSIDE_ANOTHER_ROOMS:
         errstr = "A3DERROR_SCENE_HAS_ROOMS_INSIDE_ANOTHER_ROOMS";
         break;
      case A3DERROR_SCENE_HAS_OVERLAPPING_STATIC_ROOMS:
         errstr = "A3DERROR_SCENE_HAS_OVERLAPPING_STATIC_ROOMS";
         break;
      case A3DERROR_DYNAMIC_OBJ_UNSUPPORTED:
         errstr = "A3DERROR_DYNAMIC_OBJ_UNSUPPORTED";
         break;
      case A3DERROR_DIR_AND_UP_VECTORS_NOT_PERPENDICULAR:
         errstr = "A3DERROR_DIR_AND_UP_VECTORS_NOT_PERPENDICULAR";
         break;
      case A3DERROR_INVALID_ROOM_INDEX:
         errstr = "A3DERROR_INVALID_ROOM_INDEX";
         break;
      case A3DERROR_INVALID_WALL_INDEX:
         errstr = "A3DERROR_INVALID_WALL_INDEX";
         break;
      case A3DERROR_SCENE_INVALID:
         errstr = "A3DERROR_SCENE_INVALID";
         break;
      case A3DERROR_UNIMPLEMENTED_FUNCTION:
         errstr = "A3DERROR_UNIMPLEMENTED_FUNCTION";
         break;
      case A3DERROR_NO_ROOMS_IN_SCENE:
         errstr = "A3DERROR_NO_ROOMS_IN_SCENE";
         break;
      case A3DERROR_2D_GEOMETRY_UNIMPLEMENTED:
         errstr = "A3DERROR_2D_GEOMETRY_UNIMPLEMENTED";
         break;
      case A3DERROR_OPENING_NOT_WALL_COPLANAR:
         errstr = "A3DERROR_OPENING_NOT_WALL_COPLANAR";
         break;
      case A3DERROR_OPENING_NOT_VALID:
         errstr = "A3DERROR_OPENING_NOT_VALID";
         break;
      case A3DERROR_INVALID_OPENING_INDEX:
         errstr = "A3DERROR_INVALID_OPENING_INDEX";
         break;
      case A3DERROR_FEATURE_NOT_REQUESTED:
         errstr = "A3DERROR_FEATURE_NOT_REQUESTED";
         break;
      case A3DERROR_FEATURE_NOT_SUPPORTED:
         errstr = "A3DERROR_FEATURE_NOT_SUPPORTED";
         break;
      case A3DERROR_FUNCTION_NOT_VALID_BEFORE_INIT:
         errstr = "A3DERROR_FUNCTION_NOT_VALID_BEFORE_INIT";
         break;
      case A3DERROR_INVALID_NUMBER_OF_CHANNELS:
         errstr = "A3DERROR_INVALID_NUMBER_OF_CHANNELS";
         break;
      case A3DERROR_SOURCE_IN_NATIVE_MODE:
         errstr = "A3DERROR_SOURCE_IN_NATIVE_MODE";
         break;
      case A3DERROR_SOURCE_IN_A3D_MODE:
         errstr = "A3DERROR_SOURCE_IN_A3D_MODE";
         break;
      case A3DERROR_BBOX_CANNOT_ENABLE_AFTER_BEGIN_LIST_CALL:
         errstr = "A3DERROR_BBOX_CANNOT_ENABLE_AFTER_BEGIN_LIST_CALL";
         break;
      case A3DERROR_CANNOT_CHANGE_FORMAT_FOR_ALLOCATED_BUFFER:
         errstr = "A3DERROR_CANNOT_CHANGE_FORMAT_FOR_ALLOCATED_BUFFER";
         break;
      case A3DERROR_FAILED_QUERY_DIRECTSOUNDNOTIFY:
         errstr = "A3DERROR_FAILED_QUERY_DIRECTSOUNDNOTIFY";
         break;
      case A3DERROR_DIRECTSOUNDNOTIFY_FAILED:
         errstr = "A3DERROR_DIRECTSOUNDNOTIFY_FAILED";
         break;
      case A3DERROR_RESOURCE_MANAGER_ALWAYS_ON:
         errstr = "A3DERROR_RESOURCE_MANAGER_ALWAYS_ON";
         break;
      case A3DERROR_CLOSED_LIST_CANNOT_BE_CHANGED:
         errstr = "A3DERROR_CLOSED_LIST_CANNOT_BE_CHANGED";
         break;
      case A3DERROR_END_CALLED_BEFORE_BEGIN:
         errstr = "A3DERROR_END_CALLED_BEFORE_BEGIN";
         break;
      case A3DERROR_UNMANAGED_BUFFER:
         errstr = "A3DERROR_UNMANAGED_BUFFER";
         break;
      case A3DERROR_COORD_SYSTEM_CAN_ONLY_BE_SET_ONCE:
         errstr = "A3DERROR_COORD_SYSTEM_CAN_ONLY_BE_SET_ONCE";
         break;
      default:
         errstr = "unknown error";
         break;
      }
   diag_printf ("  (%s)\r\n", errstr);
   __asm int   3
#endif
}

//##############################################################################
//#		                                                                         #
//# M3D_set_3D_speaker_type                                                    #
//#		                                                                         #
//##############################################################################

void		  AILEXPORT M3D_set_3D_speaker_type (S32 spk_type)
{
  API_lock();

  speaker_type=spk_type;

  switch (spk_type)
  {
	 case AIL_3D_HEADPHONE:
      lpA3D->SetOutputMode(OUTPUT_HEADPHONES,OUTPUT_HEADPHONES,OUTPUT_MODE_STEREO);
      break;
	 case AIL_3D_4_SPEAKER:
      lpA3D->SetOutputMode(OUTPUT_SPEAKERS_WIDE,OUTPUT_SPEAKERS_WIDE,OUTPUT_MODE_QUAD);
      break;
	 case AIL_3D_SURROUND:
      lpA3D->SetOutputMode(OUTPUT_SPEAKERS_WIDE,OUTPUT_SPEAKERS_NARROW,OUTPUT_MODE_QUAD);
      break;
	 case AIL_3D_2_SPEAKER:
	 default:
      lpA3D->SetOutputMode(OUTPUT_SPEAKERS_WIDE,OUTPUT_SPEAKERS_WIDE,OUTPUT_MODE_STEREO);
      break;
  }

  API_unlock();
}

//##############################################################################
//#		                                                                         #
//# M3D_3D_speaker_type                                                        #
//#                                                                            #
//##############################################################################

S32		 AILEXPORT M3D_3D_speaker_type (void)
{
	return speaker_type;
}


//##############################################################################
//#                                                                            #
//# M3D_set_3D_rolloff_factor                                                  #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_rolloff_factor (F32 factor)
{
  API_lock();

  if (lpA3D)
  {
    lpA3D->SetDistanceModelScale( factor );
  }

  API_unlock();
}

//##############################################################################
//#                                                                            #
//# M3D_3D_rolloff_factor                                                      #
//#                                                                            #
//##############################################################################

F32       AILEXPORT M3D_3D_rolloff_factor (void)
{
   F32 result = 1.0F;

   API_lock();

   if (lpA3D)
   {
     lpA3D->GetDistanceModelScale( &result );
   }

   API_unlock();

   return( result );
}


//##############################################################################
//#                                                                            #
//# M3D_set_3D_doppler_factor                                                  #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_doppler_factor (F32 factor)
{
  API_lock();

  if (lpA3D)
  {
    lpA3D->SetDopplerScale( factor);
  }

  API_unlock();
}

//##############################################################################
//#                                                                            #
//# M3D_3D_doppler_factor                                                      #
//#                                                                            #
//##############################################################################

F32       AILEXPORT M3D_3D_doppler_factor (void)
{
   F32 result = 1.0F;

   API_lock();

   if (lpA3D)
   {
     lpA3D->GetDopplerScale( &result );
   }

   API_unlock();

   return( result );
}


//##############################################################################
//#                                                                            #
//# M3D_set_3D_distance_factor                                                 #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_distance_factor (F32 factor)
{
  API_lock();

  if (lpA3D)
  {
    lpA3D->SetUnitsPerMeter( 1.0F/factor);
  }

  API_unlock();
}

//##############################################################################
//#                                                                            #
//# M3D_3D_distance_factor                                                     #
//#                                                                            #
//##############################################################################

F32       AILEXPORT M3D_3D_distance_factor (void)
{
   F32 result = 1.0F;

   API_lock();

   if (lpA3D)
   {
     lpA3D->GetUnitsPerMeter( &result );
     result = 1.0f / result;
   }

   API_unlock();

   return( result );
}


//############################################################################
//##                                                                        ##
//## Convert linear voltage level to a value in the range 0.0 to 1.0.       ##
//## A3D uses a linear scale, as does MSS.                                  ##
//##                                                                        ##
//############################################################################

static F32 A3D_scale_volume(const S32 linear_min, const S32 linear_max, S32 linear_level)
{
   //
   // Ensure extreme values return max/min results
   //

   if (linear_level == linear_min)
      return 0.0F;

   if (linear_level == linear_max)
      return 1.0F;

   return (F32) ((linear_level - linear_min) /
                 (double)(linear_max - linear_min));
}

//############################################################################
//##                                                                        ##
//## Set volume level of secondary buffer                                   ##
//##                                                                        ##
//############################################################################

static void A3D_set_volume(SAMPLE3D FAR *S)
{
   //
   // Check against volume limits
   //

   if (S->volume > 127)
      {
      S->volume = 127;
      }
   else if (S->volume < 0)
      {
      S->volume = 0;
      }

   //
   // Set secondary buffer volume
   //

   if (S->lpSrc != NULL)
      {
      API_lock();

      //
      // Currently, we model obstructions by attenuating
      // the sample by the specified amount.
      //

      F32 net_volume = A3D_scale_volume (0, 127, S->volume) *
                       (1.0F-S->obstruction);

      if (net_volume < EPSILON)
         net_volume = 0.0F;

      HRESULT result = S->lpSrc->SetGain (net_volume);
      CHECK_RESULT(result);

      API_unlock();
      }
}


//############################################################################
//##                                                                        ##
//## Set cone of secondary buffer                                           ##
//##                                                                        ##
//############################################################################

static void A3D_set_cone(SAMPLE3D FAR *S)
{
   //
   // Check against volume limits
   //

   if (S->outer_volume > 127)
      {
      S->outer_volume = 127;
      }
   else if (S->outer_volume < 0)
      {
      S->outer_volume = 0;
      }

   //
   // Set secondary buffer volume
   //

   if (S->lpSrc != NULL)
      {

      F32 net_volume = A3D_scale_volume (0, 127, S->outer_volume);

      if (net_volume < EPSILON)
         net_volume = 0.0F;

      API_lock();

      HRESULT result = S->lpSrc->SetCone ( S->inner_angle,S->outer_angle,net_volume);
      CHECK_RESULT(result);

      API_unlock();
      }
}


//##############################################################################
//#																									 #
//# M3D_set_3D_EOS                                                             #
//#																									 #
//##############################################################################

AIL3DSAMPLECB AILEXPORT M3D_set_3D_EOS     (H3DSAMPLE client,
                                            H3DSAMPLE samp,
														  AIL3DSAMPLECB eos)
{
  SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

  AIL3DSAMPLECB prev=S->eos;

  S->eos=eos;

  S->clientH3D=client;

  return prev;
}

//############################################################################
//##                                                                        ##
//## Set playback rate of secondary buffer                                  ##
//##                                                                        ##
//############################################################################

static void A3D_set_frequency(SAMPLE3D FAR *S, S32 playback_rate)
{
   //
   // Set the frequency only if it changes, because resampling is
   // expensive.  A3D 2.0 takes the pitch as a ratio between the
   // new value and the old one.
   //

   if ((S->lpSrc != NULL) && (playback_rate != S->playback_rate))
      {
      API_lock();

      HRESULT result =
         S->lpSrc->SetPitch (playback_rate / ((F32) S->playback_rate));
      CHECK_RESULT(result);

      API_unlock();
      }
}

//############################################################################
//##                                                                        ##
//## Lock region of secondary buffer, returning write cursor information    ##
//##                                                                        ##
//############################################################################

static S32 A3D_lock_secondary_region(SAMPLE3D FAR *S, //)
                                     S32           offset,
                                     S32           size,
                                     void        **p1,
                                     U32          *l1,
                                     void        **p2,
                                     U32          *l2)
{
   HRESULT result;

   if (S->lpSrc == NULL)
      return 0;

   //
   // Lock the buffer, returning 0 on failure
   //

   API_lock();

   result = S->lpSrc->Lock (offset, size, p1, l1, p2, l2, 0);
   CHECK_RESULT(result);

   API_unlock();

   if (FAILED(result))
      return 0;

   if ((*l1) + (*l2) != (U32) size)
      {
      A3D_set_error (A3DERROR_FAILED_LOCK_BUFFER);

      //
      // Be sure to unlock if the wrong amount was locked, which
      // should never happen anyway.
      //

      API_lock();

      HRESULT result = S->lpSrc->Unlock(*p1, *l1, *p2, *l2);
      CHECK_RESULT(result);

      API_unlock();

      return 0;
      }

   return 1;
}

//############################################################################
//##                                                                        ##
//## Unlock region of secondary buffer                                      ##
//##                                                                        ##
//############################################################################

static void A3D_unlock_secondary_region(SAMPLE3D FAR *S, //)
                                        void         *p1,
                                        U32           l1,
                                        void         *p2,
                                        U32           l2)
{
   if (S->lpSrc == NULL)
      return;

   API_lock();

   HRESULT result = S->lpSrc->Unlock(p1, l1, p2, l2);
   CHECK_RESULT(result);

   API_unlock();
}

//############################################################################
//##                                                                        ##
//## Start playback of secondary buffer at beginning                        ##
//##                                                                        ##
//############################################################################

static void A3D_start_secondary(SAMPLE3D FAR *S)
{
   if (S->lpSrc == NULL)
      return;

   API_lock();

#if defined(CYCLE_ROOM_TYPES) && CYCLE_ROOM_TYPES
   //
   // Each time a sample is started, cycle through the available
   // room types, for debugging.
   //
   void AILEXPORT M3D_set_3D_room_type (S32 room_type);
   static int room_value = 0;

   diag_printf ("Using room type %d\r\n", room_value % ENVIRONMENT_COUNT);
   M3D_set_3D_room_type (room_value % ENVIRONMENT_COUNT);
   room_value++;
#endif

   S->buffers_past_end = 0;

   HRESULT result = S->lpSrc->Play (A3D_LOOPED);
   CHECK_RESULT(result);

   API_unlock();
}

//############################################################################
//##                                                                        ##
//## Stop playback of secondary buffer                                      ##
//##                                                                        ##
//############################################################################

static void A3D_stop_secondary(SAMPLE3D FAR *S)
{
   if (S->lpSrc == NULL)
      return;

   API_lock();

   HRESULT result = S->lpSrc->Stop();
   CHECK_RESULT(result);

   API_unlock();
}

//############################################################################
//##                                                                        ##
//## Rewind secondary buffer to beginning                                   ##
//##                                                                        ##
//############################################################################

static void A3D_rewind_secondary(SAMPLE3D FAR *S)
{
   if (S->lpSrc == NULL)
      return;

   API_lock();

   HRESULT result = S->lpSrc->Rewind();
   CHECK_RESULT(result);

   API_unlock();
}

//############################################################################
//##                                                                        ##
//## Flush sample's secondary buffer with silence                           ##
//##                                                                        ##
//############################################################################

static void A3D_flush_secondary(SAMPLE3D FAR *S)
{
   U32     dwDummy;
   void   *lpDummy;
   U32     cnt;
   void   *dest;
   U32     silence;
   HRESULT result;

   if (S->lpSrc == NULL)
      return;

   //
   // Request lock on entire buffer
   //

   API_lock();

   result = S->lpSrc->Lock(0,
                           BUFF_SIZE * 2,
                          &dest,
                          &cnt,
                          &lpDummy,
                          &dwDummy,
                           0);
   CHECK_RESULT(result);

   API_unlock();

   if (FAILED(result))
      return;

   if (cnt != (U32) BUFF_SIZE * 2)
      {
      //
      // Be sure to unlock, although this case should never happen.
      //

      API_lock();

      result = S->lpSrc->Unlock (dest, cnt, lpDummy, dwDummy);
      CHECK_RESULT(result);

      API_unlock();

      return;
      }

   //
   // Flush with silence
   //

   silence = (S->bytes_per_sample == 2) ? 0 : 0x80808080;

   memset (dest, silence, cnt);

   //
   // Release lock
   //

   API_lock();

   result = S->lpSrc->Unlock(dest,
                             cnt,
                             lpDummy,
                             dwDummy);
   CHECK_RESULT(result);

   API_unlock();
}

//############################################################################
//#                                                                          #
//# Abort any pending sample events                                          #
//#                                                                          #
//############################################################################

void reset_sample_events(SAMPLE3D FAR *S)
{
	S32 j;

	volatile HANDLE * addr=&m_hBufferEvents[S->index*NUM_EVENTS];

   for (j = 0; j < NUM_EVENTS; j++)
   {
      ResetEvent(*addr++);
	}
}

//############################################################################
//#                                                                          #
//# Cancel all pending events and return as soon as no events are being      #
//# processed                                                                #
//#                                                                          #
//############################################################################

void sleep_sample(SAMPLE3D FAR *S)
{
   S->cancel_pending = 1;

   reset_sample_events(S);

   AIL_unlock_mutex();

   while (!is_safe)
      {
      if ((current_sample) && (current_sample!=S))
        break;

      Sleep(1);
      }

   AIL_lock_mutex();

}

//############################################################################
//#                                                                          #
//# Re-enable event processing                                               #
//#                                                                          #
//############################################################################

void wake_sample(SAMPLE3D FAR *S)
{
   reset_sample_events(S);

   S->cancel_pending = 0;
}

//############################################################################
//#                                                                          #
//# Flush sample buffers                                                     #
//#                                                                          #
//############################################################################

void flush_sample(SAMPLE3D FAR *S)
{
   A3D_stop_secondary(S);
   A3D_flush_secondary(S);
   A3D_rewind_secondary(S);

   reset_sample_events(S);

   S->buffers_past_end = 0;
}

//############################################################################
//##                                                                        ##
//## Copy data from source sample to target secondary buffer                ##
//##                                                                        ##
//## Backfill target secondary buffer with silence to end of source data    ##
//##                                                                        ##
//############################################################################

static void A3D_stream_to_buffer(SAMPLE3D FAR *S, U32 half, U32 len)
{
   void *out;
   U32   out_len;
   void *in;
   U32   in_len;
   U32   copy_len;
   void *dest1;
   U32   len1;
   void *dest2;
   U32   len2;
   U32   amtmixed=0;

   //
   // Lock segment to fill
   //

   if (!A3D_lock_secondary_region(S,
                                  half * len,
                                  len,
                                 &dest1,
                                 &len1,
                                 &dest2,
                                 &len2))
      {
      return;
      }

   //
   // Init output pointer to beginning of secondary buffer segment
   //

   out     = dest1;
   out_len = len1;

   //
   // Copy source data to output buffer until source exhausted or output full
   //
   // Loop can be exited under the following conditions:
   //
   // 1) Output buffer full (normal condition)
   //
   // 2) Source sample ended (normal condition)
   //
   // 3) Source stream "starved" (abnormal condition)
   //
   // If source stream ended in previous call, simply flush segment with
   // silence and return
   //

   while (1)
      {
      //
      // Exit if output buffer full
      //

      if (out_len == 0)
         {
         break;
         }

      //
      // Set input pointer and len = initial source block to copy, based on
      // size and playback position of source sample
      //

      in      = (U8 *) S->start + S->pos;
      in_len  =        S->len   - S->pos;

      //
      // Initial block may terminate before end of source buffer, if loop
      // endpoint has been declared
      //

      if (S->loop_count != 1)
         {
         if ((S->loop_end != -1) && ((U32) S->loop_end < S->len))
            {
            in_len = S->loop_end - S->pos;
            }
         }

      //
      // If no input data left, check for buffer switching and loop iteration
      //

      if (in_len == 0)
         {
         //
         // If looping active, go back to beginning of loop to fetch next
         // source data block
         //

         if (S->loop_count != 1)
            {
            //
            // Reset source sample position to beginning of loop
            //

            S->pos = S->loop_start;

            //
            // Decrement loop count if not infinite
            //

            if (S->loop_count != 0)
               {
               --S->loop_count;
               }

            //
            // Recalculate size and address of source block
            //

            continue;
            }

         //
         // If new input data is still not available after looping,
         // set up to terminate sample after last two valid buffers have
         // been completely processed
         //

         if (S->buffers_past_end++ > 2)
         {
            U32 oldstatus=S->status;

#if defined(DEBUG_STREAMING) && DEBUG_STREAMING
            diag_printf("%X DONE\r\n",S);
#endif

            S->status = SMP_DONE;

            flush_sample(S);

            if (oldstatus==SMP_PLAYING)
              S->docb=1;
          }

         break;
         }

      //
      // Copy block of data directly from source sample to buffer
      //
      // Size of block to copy is determined by smaller amount
      // of available or needed data
      //

      copy_len = min(out_len, in_len);

      memcpy(out, in, copy_len);

#if defined(DEBUG_STREAMING) && DEBUG_STREAMING
      #undef   DEBUG_STARVATION
      #define  DEBUG_STARVATION  1

      diag_printf ("read %6lu-%6lu, ",
                    (U32)in - (U32)S->start,
                    (U32)in - (U32)S->start + copy_len - 1);
      if (out_len > copy_len)
         diag_printf ("wrapped,\r\n");
#endif

      //
      // Update source sample position index
      //

      S->pos += copy_len;

      amtmixed += copy_len;

      out_len -= copy_len;
      out      = (U8 *) out + copy_len;
      }

   //
   // If insufficient source data was available to fill output buffer,
   // flush remainder of buffer with silence
   //

   if (out_len > 0)
      {
      memset(out,
             (S->bytes_per_sample == 2) ? 0 : 0x80808080,
             out_len);

      out_len = 0;
      }

   // setup to monitor the "SMP_DONE" time
   if (amtmixed) {
      U32 timer=timeGetTime();

      //
      // This code is to provide a reasonable approximation of when the
      // sound is actually done - Miles is done with the sound once it
      // has been handled off to the underlying technology, but if we
      // say the sound is done early, the user may free the sound and
      // then the DirectSound buffer would get freed, which would cause
      // the audio to stop early.
      //

      // if not enough time has past for the last block assume this block will start after it
      if (S->lastblockdone>timer)
         timer=S->lastblockdone;

      S->lastblockdone=timer+((amtmixed*1000)/(S->playback_rate*S->bytes_per_sample));
   }

   //
   // Unlock the previously-locked buffer segment
   //

   A3D_unlock_secondary_region(S,
                              dest1,
                              len1,
                              dest2,
                              len2);

#if defined(DEBUG_STARVATION) && DEBUG_STARVATION
   U32 p;

   //
   // Since this is time-sensitive debugging code, we don't protect
   // against reentrancy -- waiting for a mutex here could cause us
   // to misread the position
   //

   HRESULT result = S->lpSrc->GetWavePosition(&p);
   CHECK_RESULT(result);

#if defined(DEBUG_STREAMING) && DEBUG_STREAMING
   diag_printf ("wrote %6lu-%6lu, playing %6lu\r\n",
      half * BUFF_SIZE,
      (half + 1) * BUFF_SIZE - 1,
      p);
#endif

   if (((S->status&255) == SMP_PLAYING) &&
       (p >=  (half      * BUFF_SIZE)) &&
       (p <  ((half + 1) * BUFF_SIZE)))
      {
      diag_printf ("Play cursor caught up to write cursor\r\n");
      }

#endif
}

//############################################################################
//#                                                                          #
//# AIL timer callback for buffer callbacks                                  #
//#                                                                          #
//############################################################################

void AILCALLBACK notify_timer(U32 user)
{
   S32 i;

   //
   // Update sample positions
   //

   for (i=0; i < avail_samples; i++)
   {
      SAMPLE3D FAR *S = &samples[i];

      if (S->eos)
      {
        if (S->docb)
        {
          S->docb=0;
          S->eos(S->clientH3D);
        }
        else
        {
          if ((S->status==SMP_PLAYING) && (S->pos) && (S->pos == S->len))
            if (timeGetTime()>=S->lastblockdone)
            {
              S->status=SMP_PLAYING|256;
              S->docb=0;
              S->eos(S->clientH3D);
            }
        }
      }
   }

}


//############################################################################
//#                                                                          #
//# Return floating-point type as unsigned long DWORD (without actually      #
//# converting the value)                                                    #
//#                                                                          #
//############################################################################

U32 float_as_long(F32 FP)
{
   static U32 val;

   *(F32 FAR *) (&val) = FP;

   return val;
}

//############################################################################
//#                                                                          #
//# Return signed long DWORD as single-precision float (without actually     #
//# converting the value)                                                    #
//#                                                                          #
//############################################################################

F32 long_as_float(S32 integer)
{
   static F32 val;

   *(S32 FAR *) (&val) = integer;

   return val;
}

//############################################################################
//#                                                                          #
//# Destroy system-level emitter object(s) associated with sample            #
//#                                                                          #
//############################################################################

void destroy_sample_emitter(SAMPLE3D FAR *S)
{
   API_lock();

   if (S->lpSrc != NULL)
      {
      S->lpSrc->Release();
      S->lpSrc = NULL;
      }

   API_unlock();
}


//############################################################################
//#                                                                          #
//# Destroy system-level events and threads associated with all samples      #
//#                                                                          #
//############################################################################

void destroy_all_sample_events(void)
{
   if (m_lpThread)
   {
      SetEvent(m_hBufferEvents[num_events]);
      SetEvent(m_hmaster);

      WaitForSingleObject(m_lpThread,
                          INFINITE);

      CloseHandle(m_lpThread);

      m_lpThread=0;

   }

   //
	// Destroy event handle associated with sample
	//

   if (num_events)
   {
     U32 i;

     for(i=0;i<=num_events;i++)
     {
       CloseHandle(m_hBufferEvents[i]);
       m_hBufferEvents[i]=0;
     }

     CloseHandle(m_hmaster);
   }

   //
   // Set hFlushEvent to cause the flush thread to terminate.  Wait
   // for it to terminate, and discard its resources.
   //

   if (hFlushThread)
      {
      SetEvent (hFlushEvent);
      WaitForSingleObject (hFlushThread, INFINITE);
      CloseHandle (hFlushThread);
      hFlushThread = NULL;
      }

   if (hFlushEvent)
      {
      CloseHandle (hFlushEvent);
      hFlushEvent = NULL;
      }
}

//############################################################################
//#                                                                          #
//# Initialize sample                                                        #
//#                                                                          #
//############################################################################

void init_sample(SAMPLE3D FAR *S)
{
   S->type            =  IS_SAMPLE;

   S->start           =  NULL;
   S->len             =  0;
   S->pos             =  0;
   S->done            =  0;

   S->buffers_past_end = 0;

   S->loop_count      =  1;
   S->loop_start      =  0;
   S->loop_end        = -1;

   S->volume          =  AIL_get_preference(DIG_DEFAULT_VOLUME);
   S->playback_rate   =  22050;

   S->status          =  SMP_DONE;

   S->auto_update     =  0;
}

//############################################################################
//#                                                                          #
//# Terminate playing sample immediately, aborting all pending events        #
//#                                                                          #
//# Do NOT call from within background thread -- will cause deadlocks due    #
//# to sleep_sample() call!                                                  #
//#                                                                          #
//############################################################################

void reset_sample_voice(SAMPLE3D FAR *S)
{
   //
   // Put sample to sleep
   //

   sleep_sample(S);

   //
   // Mark sample as DONE to inhibit position notifications
   //

   S->status = SMP_DONE;

   //
   // Flush sample buffers
   //

   flush_sample(S);

   //
   // Resume normal sample thread execution
   //

   wake_sample(S);
}

//############################################################################
//#                                                                          #
//# This function is the worker thread that                                  #
//# feeds buffers to our streaming emitter                                   #
//# In this sample we use a simple double buffering                          #
//# scheme.                                                                  #
//# When the thread is told to start playing the 2 buffers                   #
//# are filled with data and both are submitted to the streaming emitter     #
//# The thread is then signalled when a buffer is finished.                  #
//# When we wake up when a buffer is finished we send another                #
//# buffer                                                                   #
//#                                                                          #
//############################################################################

DWORD WINAPI pfnThreadProc(LPVOID pParam)
{
	DWORD dwReturnedEvent = 0;
   U32 loops,final;

   loops=(num_events+1)/MAXIMUM_WAIT_OBJECTS;
   final=(num_events+1)%MAXIMUM_WAIT_OBJECTS;

  keepwaiting:

   _asm
   {
     lock inc dword ptr [is_safe]
   }

   current_sample=0;

   WaitForSingleObject(m_hmaster,INFINITE);

   _asm
   {
     lock dec dword ptr [is_safe]
   }

   while (1)
		{
      U32 i,j=0;

      for(i=0;i<loops;i++)
      {
        dwReturnedEvent=WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS,m_hBufferEvents+j,0,0);

        if (dwReturnedEvent<MAXIMUM_WAIT_OBJECTS)
        {
          goto found;
        }
        j+=MAXIMUM_WAIT_OBJECTS;
      }

      if (final)
      {
        dwReturnedEvent=WaitForMultipleObjects(final,m_hBufferEvents+j,0,0);

        if (dwReturnedEvent>=final)
        {
          goto keepwaiting;
        }
      }

     found:

      dwReturnedEvent+=j;

      //
		// DIE event may be issued for any sample, with or without an
		// assigned emitter
		//

		if (dwReturnedEvent == num_events)
			{
			diag_printf("%X DIE\n");

			return 0;
			}

    	SAMPLE3D FAR *S		 = &samples[dwReturnedEvent/NUM_EVENTS];

      dwReturnedEvent%=NUM_EVENTS;

      //
      // is this sample being set up right now?
      //

      if (S->cancel_pending)
			{
			continue;
			}

      current_sample = S;

      //
      // All others require an assigned emitter -- if no emitter assigned,
      // no operations can be executed
      //

      if (S->lpSrc == NULL)
         continue;

      //
      // Process event on sample with valid assigned emitter
      //

      switch (dwReturnedEvent)
         {

         case BUFFER1_EVENT:

#if defined(DEBUG_STREAMING) && DEBUG_STREAMING
            diag_printf("%X BUFFER 1\r\n",S);
#endif

            if ((S->status&255) == SMP_PLAYING)
               {
               A3D_stream_to_buffer(S, 0, BUFF_SIZE);
               }
            break;

         case BUFFER2_EVENT:

#if defined(DEBUG_STREAMING) && DEBUG_STREAMING
            diag_printf("%X BUFFER 2\r\n",S);
#endif

            if ((S->status&255) == SMP_PLAYING)
               {
               A3D_stream_to_buffer(S, 1, BUFF_SIZE);
               }
            break;
         }
      }
}

//##############################################################################
//#                                                                            #
//# A3D_make_wall                                                              #
//#                                                                            #
//##############################################################################

static void A3D_make_wall (int wall, int v1, int v2, int v3, int v4)
{
   //
   // Tag the walls so A3D can keep track of them from frame to
   // frame.  This is mandatory in order to use reflections.
   //

   HRESULT result = lpA3DGeom->Tag (wall);
   CHECK_RESULT (result);

   //
   // Now a wall from the four referenced vertices.
   //

   lpA3DGeom->Vertex3fv (room_vertices[v1]);
   lpA3DGeom->Vertex3fv (room_vertices[v2]);
   lpA3DGeom->Vertex3fv (room_vertices[v3]);
   lpA3DGeom->Vertex3fv (room_vertices[v4]);
}

//##############################################################################
//#                                                                            #
//# A3D_render_scene                                                           #
//#                                                                            #
//# This function renders the audio scene.  It's normally called 30 times per  #
//# second by flushThreadProc.                                                 #
//#                                                                            #
//##############################################################################

static void A3D_render_scene (void)
{
   HRESULT result;
   int i;
   F32 plus_half, minus_half;
   U32 original_mode;

   API_lock ();

   //
   // Clear the room geometry from the previous frame.
   //

   result = lpA3D->Clear ();
   CHECK_RESULT (result);

   //
   // If we have defined a room size -- which will never be the case
   // if we are unable to process reflections, or are using an
   // open-air environment like ENVIRONMENT_GENERIC, or virtual
   // geometry is disabled -- then we need to recreate the room geometry
   // every frame.
   //

   if (room_size >= EPSILON)
      {
      result = lpA3DGeom->Begin (A3D_QUADS);
      CHECK_RESULT (result);

      //
      // If the default render mode is not reflections-but-not-occlusions,
      // set it that way while we render the walls.  SetRenderMode is not
      // guaranteed to be click-free, so we avoid making the call when
      // it's not necessary.
      //

      result = lpA3DGeom->GetRenderMode (&original_mode);
      CHECK_RESULT (result);

      if (original_mode != A3D_1ST_REFLECTIONS)
         {
         result = lpA3DGeom->SetRenderMode (A3D_1ST_REFLECTIONS);
         CHECK_RESULT (result);
         }

      //
      // Create the walls using the reflectance and transmittance
      // properties corresponding to the current room type, but
      // don't permit them to occlude anything.
      //

      result = lpA3DGeom->BindMaterial (lpA3DWallMaterial);
      CHECK_RESULT (result);

      //
      // Create a cube around the listener of the desired size, which
      // we convert from meters to application units.
      //

      plus_half  = (room_size * units_per_meter) / 2;
      minus_half = -plus_half;

      //
      // Now create the vertices.  Rooms are always cubical.
      //

      for (i = 0; i < 8; i++)
         {
         X_VAL(room_vertices[i]) =
            X_VAL(listen_position) + (i & 1 ? plus_half : minus_half);
         Y_VAL(room_vertices[i]) =
            Y_VAL(listen_position) + (i & 2 ? plus_half : minus_half);
         Z_VAL(room_vertices[i]) =
            Z_VAL(listen_position) + (i & 4 ? plus_half : minus_half);
         }

      //
      // Now create the walls.
      //

      A3D_make_wall (1, 0, 1, 3, 2);
      A3D_make_wall (2, 2, 3, 7, 6);
      A3D_make_wall (3, 6, 7, 5, 4);
      A3D_make_wall (4, 4, 6, 2, 0);
      A3D_make_wall (5, 0, 4, 5, 1);
      A3D_make_wall (6, 1, 5, 7, 3);

      if (original_mode != A3D_1ST_REFLECTIONS)
         {
         result = lpA3DGeom->SetRenderMode (original_mode);
         CHECK_RESULT (result);
         }

      result = lpA3DGeom->End ();
      CHECK_RESULT (result);
      }

   //
   // Flush the scene.  This always happens, whether or not there's
   // any geometry in the scene, or A3D plays no sound.
   //

   result = lpA3D->Flush ();
   CHECK_RESULT(result);

   API_unlock ();
}

//############################################################################
//#                                                                          #
//# This function is the thread that periodically flushes the audio scene    #
//# when the application hasn't taken control itself.                        #
//#                                                                          #
//# It wakes up every so often (30 ms, but the interval is configurable)     #
//# and if the audio scene has changed, it flushes the output and begins     #
//# a new scene.
//#                                                                          #
//############################################################################

DWORD WINAPI flushThreadProc (LPVOID pParam)
{
   DWORD dwReturnedEvent = 0;
   S32 AILEXPORT M3D_active_3D_sample_count (void);

   if (! hFlushEvent)
      return 0;

   //
   // Wait for the shutdown event, timing out after 30 ms, then repeat.
   //

   while (WaitForSingleObject (hFlushEvent, flush_timeout) == WAIT_TIMEOUT)
      {
      API_lock ();

      if (M3D_active_3D_sample_count () > 0)
         A3D_render_scene ();

      API_unlock ();
      }
   return 0;
}

//############################################################################
//#                                                                          #
//# Retrieve a standard RIB provider attribute by index                      #
//#                                                                          #
//############################################################################

U32 AILEXPORT PROVIDER_query_attribute (HATTRIB index)
{
   HRESULT result = S_OK;
   U32 retval = 0;

   //
   // Only a few attributes can be queried before A3D is initialized.
   //

   if ((! lpA3D) &&
      ((ATTRIB)index != PROVIDER_NAME) &&
      ((ATTRIB)index != PROVIDER_VERSION))
      return retval;

   API_lock ();

   switch ((ATTRIB) index)
      {
      case PROVIDER_NAME:
         {
         retval = (U32) "Aureal A3D 2.0 (TM)";
         break;
         }
      case PROVIDER_VERSION:
         {
         retval = 0x100;
         break;
         }
      case MAX_SUPPORTED_SAMPLES:
         {
         retval = avail_samples;
         break;
         }
      case TECHNOLOGY_VERSION:
         {
         A3DCAPS_SOFTWARE software_caps;

         memset (&software_caps, 0, sizeof(software_caps));
         software_caps.dwSize = sizeof(software_caps);
         result = lpA3D->GetSoftwareCaps (&software_caps);
         retval = software_caps.dwVersion;
         break;
         }
      case TECHNOLOGY_INTERFACE:
         {
         retval = (U32) lpA3D;
         lpA3D->AddRef ();
         break;
         }
      case RENDER_1ST_REFLECTIONS:
         {
         retval = (lpA3D->IsFeatureAvailable (A3D_1ST_REFLECTIONS) != 0);
         break;
         }
      case RENDER_OCCLUSIONS:
         {
         retval = (lpA3D->IsFeatureAvailable (A3D_OCCLUSIONS) != 0);
         break;
         }
      case SOFTWARE_MIXING:
         {
         DWORD sources;

         result = lpA3D->GetNumFallbackSources (&sources);
         retval = (sources != 0);
         break;
         }
      case PRIORITY_BIAS:
         {
         F32 bias;

         result = lpA3D->GetRMPriorityBias (&bias);
         retval = float_as_long (bias);
         break;
         }
      case OUTPUT_GAIN:
         {
         F32 gain;

         result = lpA3D->GetOutputGain (&gain);
         retval = float_as_long (gain);
         break;
         }
      case FLUSH_TIMEOUT:
         {
         retval = flush_timeout;
         break;
         }
      case MAX_REFLECTION_DELAY:
         {
         F32 delay;

         result = lpA3D->GetMaxReflectionDelayTime (&delay);
         retval = float_as_long (delay);
         break;
         }
      case UNITS_PER_METER:
         {
         F32 units;

         result = lpA3D->GetUnitsPerMeter (&units);
         retval = float_as_long (units);
         break;
         }
      case VIRTUAL_GEOMETRY:
         {
         retval = (room_size >= EPSILON);
         break;
         }
      case DISTANCE_SCALE:
         {
         F32 scale;

         result = lpA3D->GetDistanceModelScale (&scale);
         retval = float_as_long (scale);
         break;
         }
      case DOPPLER_SCALE:
         {
         F32 scale;

         result = lpA3D->GetDopplerScale (&scale);
         retval = float_as_long (scale);
         break;
         }
      case EQUALIZATION:
         {
         F32 eq;

         result = lpA3D->GetEq (&eq);
         retval = float_as_long (eq);
         break;
         }
      }

   CHECK_RESULT(result);

   API_unlock ();

   return retval;
}

//############################################################################
//#                                                                          #
//# Return M3D error text                                                    #
//#                                                                          #
//############################################################################

C8 FAR *       AILEXPORT M3D_error       (void)
{
   if (!strlen(M3D_error_text))
      {
      return NULL;
      }

   return M3D_error_text;
}

//############################################################################
//#                                                                          #
//# Initialize M3D stream decoder                                            #
//#                                                                          #
//############################################################################

M3DRESULT AILEXPORT M3D_startup     (void)
{
   if (M3D_started++)
      {
      strcpy(M3D_error_text,"Already started");
      return M3D_ALREADY_STARTED;
      }

   //
   // Init static prefs/attributes
   //

   M3D_error_text[0] = 0;

   return M3D_NOERR;
}

//############################################################################
//#                                                                          #
//# Shut down M3D stream decoder                                             #
//#                                                                          #
//############################################################################

M3DRESULT      AILEXPORT M3D_shutdown    (void)
{
   if (!M3D_started)
      {
      strcpy(M3D_error_text,"Not initialized");
      return M3D_NOT_INIT;
      }

   --M3D_started;

   return M3D_NOERR;
}

//############################################################################
//#                                                                          #
//# Set provider preference value, returning previous setting                #
//#                                                                          #
//############################################################################

S32 AILEXPORT M3D_set_provider_preference (HATTRIB    preference, //)
                                           void FAR * value)
{
   void AILEXPORT M3D_set_3D_room_type (S32 room_type);
   HRESULT result = S_OK;
   S32 prev = -1;

   if (! lpA3D)
      return prev;

   API_lock ();

   switch ((ATTRIB) preference)
      {
      case SOFTWARE_MIXING:
         {

         //
         // The SOFTWARE_MIXING preference is TRUE to enable 64
         // software-mixed channels (A2D), FALSE to restrict A3D
         // to use A3D hardware mixing only.
         //

         DWORD sources;

         result = lpA3D->GetNumFallbackSources (&sources);
         CHECK_RESULT(result);
         prev = (sources != 0);

         result = lpA3D->SetNumFallbackSources (*(U32 FAR *)value ?
                                                N_SAMPLES :
                                                0);
         break;
         }
      case PRIORITY_BIAS:
         {
         F32 bias;

         result = lpA3D->GetRMPriorityBias (&bias);
         CHECK_RESULT(result);
         prev = float_as_long (bias);

         result = lpA3D->SetRMPriorityBias (*(F32 FAR *)value);
         break;
         }
      case OUTPUT_GAIN:
         {
         F32 gain;

         result = lpA3D->GetOutputGain (&gain);
         CHECK_RESULT(result);
         prev = float_as_long (gain);

         result = lpA3D->SetOutputGain (*(F32 FAR *)value);
         break;
         }
      case FLUSH_TIMEOUT:
         {
         prev = flush_timeout;
         flush_timeout = *(U32 FAR *)value;
         break;
         }
      case MAX_REFLECTION_DELAY:
         {
         F32 delay;

         result = lpA3D->GetMaxReflectionDelayTime (&delay);
         CHECK_RESULT(result);
         prev = float_as_long (delay);

         result = lpA3D->SetMaxReflectionDelayTime (*(F32 FAR *)value);
         break;
         }
      case UNITS_PER_METER:
         {
         F32 units;

         result = lpA3D->GetUnitsPerMeter (&units);
         CHECK_RESULT(result);
         prev = float_as_long (units);

         units_per_meter = *(F32 FAR *)value;
         result = lpA3D->SetUnitsPerMeter (units_per_meter);
         break;
         }
      case VIRTUAL_GEOMETRY:
         {
         prev = (room_size >= EPSILON);

         if (*(U32 FAR *)value)
            M3D_set_3D_room_type (ENVIRONMENT_ROOM);
         else
            M3D_set_3D_room_type (ENVIRONMENT_GENERIC);
         break;
         }
      case DISTANCE_SCALE:
         {
         F32 scale;

         result = lpA3D->GetDistanceModelScale (&scale);
         CHECK_RESULT(result);
         prev = float_as_long (scale);

         result = lpA3D->SetDistanceModelScale (*(F32 FAR *)value);
         break;
         }
      case DOPPLER_SCALE:
         {
         F32 scale;

         result = lpA3D->GetDopplerScale (&scale);
         CHECK_RESULT(result);
         prev = float_as_long (scale);

         result = lpA3D->SetDopplerScale (*(F32 FAR *)value);
         break;
         }
      case EQUALIZATION:
         {
         F32 eq;

         result = lpA3D->GetEq (&eq);
         CHECK_RESULT(result);
         prev = float_as_long (eq);

         result = lpA3D->SetEq (*(F32 FAR *)value);
         break;
         }
      }

   CHECK_RESULT(result);

   API_unlock ();

   return prev;
}

//##############################################################################
//#                                                                            #
//# M3D_allocate_3D_sample_handle                                              #
//#                                                                            #
//##############################################################################

H3DSAMPLE  AILEXPORT M3D_allocate_3D_sample_handle (void)
{
   //
   // Look for an unallocated sample structure
   //

   S32 i;

   for (i=0; i < avail_samples; i++)
      {
      if (samples[i].status == SMP_FREE)
         {
         break;
         }
      }

   //
   // If all structures in use, return NULL
   //
   // (Unlike in the 2D case, this does not set the MSS error string, since
   // the application has no control or knowledge of the number of available
   // handles from various 3D providers)
   //

   if (i == avail_samples)
      {
      return NULL;
      }

   SAMPLE3D FAR *S = &samples[i];

   //
   // Initialize sample to default (SMP_DONE) status with nominal
   // sample attributes
   //

   init_sample(S);

   S->eos=0;
   S->docb=0;

   return (H3DSAMPLE) S;
}

//##############################################################################
//#                                                                            #
//# M3D_release_3D_sample_handle                                               #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_release_3D_sample_handle (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   reset_sample_voice(S);

   //
   // Mark sample available for immediate reallocation
   //

   S->status = SMP_FREE;
}

//##############################################################################
//#                                                                            #
//# M3D_start_3D_sample                                                        #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_start_3D_sample         (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   //
   // Make sure valid sample data exists
   //

   if ((S->len   == 0) ||
       (S->start == NULL))
      {
      return;
      }

   //
   // Initialize sample voice
   //

   reset_sample_voice(S);

   //
   // Rewind source buffer to beginning
   //

   S->pos = 0;

   //
   // Activate sample and wait for command to take effect
   //

#if defined(DEBUG_STREAMING) && DEBUG_STREAMING
   diag_printf("%X START\r\n",S);
#endif

   //
   // We only read one half-buffer's worth before starting
   // the sample because A3D will notify us immediately to
   // read the second half-buffer.
   //
   A3D_stream_to_buffer(S, 0, BUFF_SIZE);

   A3D_start_secondary(S);

   S->docb=0;
   S->status = SMP_PLAYING;
}

//##############################################################################
//#                                                                            #
//# M3D_stop_3D_sample                                                         #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_stop_3D_sample          (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status != SMP_PLAYING)
      {
      return;
      }

   //
   // M3D_stop_3D_sample was called; we need to stop playing
   // the sample but not flush it.  It's OK to flush the
   // emitter, but we remember where we are in the sample
   // in case we have to restart.
   //

#if defined(DEBUG_STREAMING) && DEBUG_STREAMING
   diag_printf("%X STOP\r\n",S);
#endif

   S->status = SMP_STOPPED;

   flush_sample(S);
}

//##############################################################################
//#                                                                            #
//# M3D_resume_3D_sample                                                       #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_resume_3D_sample        (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if ((S->status == SMP_FREE) || (S->status == SMP_PLAYING))
      {
      return;
      }

   //
   // Make sure valid sample data exists
   //

   if ((S->len   == 0) ||
       (S->start == NULL))
      {
      return;
      }

   //
   // Initialize sample voice
   //

   reset_sample_voice(S);

   //
   // Activate sample and wait for command to take effect
   //

#if defined(DEBUG_STREAMING) && DEBUG_STREAMING
   diag_printf("%X RESUME\r\n",S);
#endif

   A3D_stream_to_buffer(S, 0, BUFF_SIZE);

   A3D_start_secondary(S);

   S->docb=0;
   S->status = SMP_PLAYING;
}

//##############################################################################
//#                                                                            #
//# M3D_end_3D_sample                                                          #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_end_3D_sample        (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   reset_sample_voice(S);

   //
   // Mark sample available for immediate reuse
   //

   S->status = SMP_DONE;
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_volume                                                   #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_sample_volume    (H3DSAMPLE samp, //)
                                                  S32       volume)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S->volume = volume;

   if (S->lpSrc)
   {
     A3D_set_volume(S);
   }
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_playback_rate                                            #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_sample_playback_rate    (H3DSAMPLE samp, //)
                                                         S32       playback_rate)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   if (S->lpSrc)
   {
     A3D_set_frequency(S, playback_rate);
   }

   S->playback_rate = playback_rate;
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_offset                                                   #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_sample_offset    (H3DSAMPLE samp, //)
                                                  U32       offset)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S->pos = offset;
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_loop_count                                               #
//#                                                                            #
//#  1: Single iteration, no looping                                           #
//#  0: Loop indefinitely                                                      #
//#  n: Play sample n times                                                    #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_sample_loop_count(H3DSAMPLE samp, //)
                                                  U32       loops)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S->loop_count = loops;
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_loop_block                                               #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_sample_loop_block(H3DSAMPLE samp, //)
                                                  S32       loop_start_offset,
                                                  S32       loop_end_offset)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S32 gran = S->bytes_per_sample;

   S->loop_start = ((loop_start_offset+gran/2) / gran)*gran;
   S->loop_end   = loop_end_offset;
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_obstruction                                              #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_sample_obstruction(H3DSAMPLE samp, //)
                                                   F32       obstruction)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S->obstruction = obstruction;
   if (S->lpSrc)
   {
     A3D_set_volume(S);
   }
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_occlusion                                                #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_sample_occlusion(H3DSAMPLE samp, //)
                                                 F32       occlusion)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S->occlusion = occlusion;

   occlusion=1.01F-occlusion;
   if (occlusion>1.0F)
     occlusion=1.0F;

   if (S->lpSrc)
   {
     API_lock();
     HRESULT result = S->lpSrc->SetEq (occlusion);
     API_unlock();
   }
   CHECK_RESULT(result);
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_effects_level                                            #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_sample_effects_level(H3DSAMPLE samp, //)
                                                     F32       effects_level)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   if (fabs(effects_level+1.0F)<EPSILON)
     effects_level=1.0F;

   S->effects_level = effects_level;


   if (S->lpSrc)
   {
     API_lock ();
     HRESULT result = S->lpSrc->SetReflectionGainScale(effects_level);
     CHECK_RESULT(result);
     API_unlock ();
   }
}

//##############################################################################
//#																									 #
//# M3D_3D_sample_cone     																	 #
//#																									 #
//##############################################################################

void		  AILEXPORT M3D_3D_sample_cone      (H3DSAMPLE samp,
                                              F32 FAR* inner_angle,
                                              F32 FAR* outer_angle,
                                              S32 FAR* outer_volume   )
{
	SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

	if (S->status == SMP_FREE)
		{
		return;
		}

   if (inner_angle)
     *inner_angle=S->inner_angle;
   if (outer_angle)
     *outer_angle=S->outer_angle;
   if (outer_volume)
     *outer_volume=S->outer_volume;
}

//##############################################################################
//#																									 #
//# M3D_set_3D_sample_cone     																 #
//#																									 #
//##############################################################################

void		  AILEXPORT M3D_set_3D_sample_cone     (H3DSAMPLE samp, //)
																 F32		  inner_angle,
                                                 F32       outer_angle,
                                                 S32       outer_volume)
{
	SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

	if (S->status == SMP_FREE)
		{
		return;
		}

	S->inner_angle = inner_angle;
	S->outer_angle = outer_angle;
	S->outer_volume = outer_volume;

   if (S->lpSrc)
   {
     A3D_set_cone(S);
   }
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_volume                                                       #
//#                                                                            #
//##############################################################################

S32        AILEXPORT M3D_3D_sample_volume        (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   return S->volume;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_playback_rate                                                #
//#                                                                            #
//##############################################################################

S32        AILEXPORT M3D_3D_sample_playback_rate        (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   return S->playback_rate;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_status                                                       #
//#                                                                            #
//##############################################################################

U32        AILEXPORT M3D_3D_sample_status        (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (((S->status&255)==SMP_PLAYING) && (S->pos) && (S->pos == S->len))
     if (timeGetTime()>=S->lastblockdone)
       return SMP_DONE;

   return S->status;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_offset                                                       #
//#                                                                            #
//##############################################################################

U32        AILEXPORT M3D_3D_sample_offset        (H3DSAMPLE     samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   return S->pos;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_length                                                       #
//#                                                                            #
//##############################################################################

U32        AILEXPORT M3D_3D_sample_length        (H3DSAMPLE     samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   return S->len;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_loop_count                                                   #
//#                                                                            #
//##############################################################################

U32        AILEXPORT M3D_3D_sample_loop_count    (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   return S->loop_count;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_obstruction                                                  #
//#                                                                            #
//##############################################################################

F32        AILEXPORT M3D_3D_sample_obstruction (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   return S->obstruction;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_occlusion                                                    #
//#                                                                            #
//##############################################################################

F32        AILEXPORT M3D_3D_sample_occlusion (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   return S->occlusion;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_effects_level                                                #
//#                                                                            #
//##############################################################################

F32        AILEXPORT M3D_3D_sample_effects_level (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   return S->effects_level;
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_distances                                                #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_sample_distances (H3DSAMPLE samp, //)
                                                  F32       max_dist,
                                                  F32       min_dist)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S->max_dist = max_dist;
   S->min_dist = min_dist;

   if (S->lpSrc != NULL)
      {
      API_lock();

      HRESULT result = S->lpSrc->SetMinMaxDistance(min_dist,
                                                   max_dist,
                                                   A3D_MUTE);
      CHECK_RESULT(result);

      API_unlock();
      }
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_distances                                                    #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_3D_sample_distances     (H3DSAMPLE samp, //)
                                                  F32 FAR * max_dist,
                                                  F32 FAR * min_dist)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   if (max_dist) *max_dist = S->max_dist;
   if (min_dist) *min_dist = S->min_dist;
}

//############################################################################
//#                                                                          #
//# Retrieve a sample attribute by index                                     #
//#                                                                          #
//############################################################################

U32 AILEXPORT M3D_3D_sample_query_attribute (H3DSAMPLE samp, //)
                                             HATTRIB   index)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;
   HRESULT result = S_OK;
   U32 retval = 0;

   if ((S->status == SMP_FREE) || (! S->lpSrc))
      {
      return 0;
      }

   API_lock ();

   switch ((ATTRIB) index)
      {
      case AUDIBILITY:
         {
         F32 audibility;

         result = S->lpSrc->GetAudibility (&audibility);
         retval = float_as_long (audibility);
         break;
         }
      case OCCLUSION_FACTOR:
         {
         F32 factor;

         result = S->lpSrc->GetOcclusionFactor (&factor);
         retval = float_as_long (factor);
         break;
         }
      case DISTANCE_SCALE:
         {
         F32 scale;

         result = S->lpSrc->GetDistanceModelScale (&scale);
         retval = float_as_long (scale);
         break;
         }
      case DOPPLER_SCALE:
         {
         F32 scale;

         result = S->lpSrc->GetDopplerScale (&scale);
         retval = float_as_long (scale);
         break;
         }
      case EQUALIZATION:
         {
         F32 eq;

         result = S->lpSrc->GetEq (&eq);
         retval = float_as_long (eq);
         break;
         }
      case REFLECTION_DELAY_SCALE:
         {
         F32 scale;

         result = S->lpSrc->GetReflectionDelayScale (&scale);
         retval = float_as_long (scale);
         break;
         }
      case REFLECTION_GAIN_SCALE:
         {
         F32 scale;

         result = S->lpSrc->GetReflectionGainScale (&scale);
         retval = float_as_long (scale);
         break;
         }
      case PRIORITY:
         {
         F32 priority;

         result = S->lpSrc->GetPriority (&priority);
         retval = float_as_long (priority);
         break;
         }
      case RENDER_MODE:
         {
         result = S->lpSrc->GetRenderMode (&retval);
         break;
         }
      }

   CHECK_RESULT(result);

   API_unlock ();

   return retval;
}

//############################################################################
//#                                                                          #
//# Set provider preference value, returning previous setting                #
//#                                                                          #
//############################################################################

S32 AILEXPORT M3D_3D_set_sample_preference (H3DSAMPLE  samp, //)
                                            HATTRIB    preference,
                                            void FAR*  value)
{
   HRESULT result = S_OK;
   S32 prev = -1;

   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if ((S->status == SMP_FREE) || (! S->lpSrc))
      {
      return -1;
      }

   API_lock ();

   switch ((ATTRIB) preference)
      {
      case DISTANCE_SCALE:
         {
         F32 scale;

         result = S->lpSrc->GetDistanceModelScale (&scale);
         CHECK_RESULT(result);
         prev = float_as_long (scale);

         result = S->lpSrc->SetDistanceModelScale (*(F32 FAR *)value);
         break;
         }
      case DOPPLER_SCALE:
         {
         F32 scale;

         result = S->lpSrc->GetDopplerScale (&scale);
         CHECK_RESULT(result);
         prev = float_as_long (scale);

         result = S->lpSrc->SetDopplerScale (*(F32 FAR *)value);
         break;
         }
      case EQUALIZATION:
         {
         F32 eq;

         result = S->lpSrc->GetEq (&eq);
         CHECK_RESULT(result);
         prev = float_as_long (eq);

         result = S->lpSrc->SetEq (*(F32 FAR *)value);
         break;
         }
      case REFLECTION_DELAY_SCALE:
         {
         F32 scale;

         result = S->lpSrc->GetReflectionDelayScale (&scale);
         CHECK_RESULT(result);
         prev = float_as_long (scale);

         result = S->lpSrc->SetReflectionDelayScale (*(F32 FAR *)value);
         break;
         }
      case REFLECTION_GAIN_SCALE:
         {
         F32 scale;

         result = S->lpSrc->GetReflectionGainScale (&scale);
         CHECK_RESULT(result);
         prev = float_as_long (scale);

         result = S->lpSrc->SetReflectionGainScale (*(F32 FAR *)value);
         break;
         }
      case PRIORITY:
         {
         F32 priority;

         result = S->lpSrc->GetPriority (&priority);
         CHECK_RESULT(result);
         prev = float_as_long (priority);

         result = S->lpSrc->SetPriority (*(F32 FAR *)value);
         break;
         }
      case RENDER_MODE:
         {
         result = S->lpSrc->GetRenderMode ((U32 *)&prev);
         CHECK_RESULT(result);

         result = S->lpSrc->SetRenderMode (*(U32 FAR *)value);
         break;
         }
      }

   CHECK_RESULT(result);

   API_unlock ();

   return prev;
}


//##############################################################################
//#                                                                            #
//# M3D_active_3D_sample_count                                                 #
//#                                                                            #
//##############################################################################

S32      AILEXPORT M3D_active_3D_sample_count   (void)
{
   S32 i,n;

   n = 0;

   for (i=0; i < avail_samples; i++)
      {
      if (samples[i].status == SMP_PLAYING)
         {
         ++n;
         }
      }

   return n;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_open_listener                                                       #
//#                                                                            #
//##############################################################################

H3DPOBJECT AILEXPORT M3D_3D_open_listener        (void)
{
   static OBJTYPE listener = IS_LISTENER;

   return (H3DPOBJECT) &listener;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_close_listener                                                      #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_3D_close_listener       (H3DPOBJECT listener)
{
   //
   // Not supported
   //
}

//##############################################################################
//#                                                                            #
//# M3D_3D_open_object                                                         #
//#                                                                            #
//##############################################################################

H3DPOBJECT AILEXPORT M3D_3D_open_object          (void)
{
   //
   // Not supported
   //

   return NULL;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_close_object                                                        #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_3D_close_object         (H3DPOBJECT obj)
{
   //
   // Not supported
   //
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_room_type                                                       #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_room_type (S32 room_type)
{
   A3DVECTOR *material;
   HRESULT result;
   F32 emphasis = 1.0F;

   //
   // Reflection processing must be enabled for this call to do anything.
   //

   if (! lpA3DGeom->IsEnabled (A3D_1ST_REFLECTIONS))
      {
      room_size = 0.0F;
      return;
      }


   API_lock ();

   switch (room_type)
      {
      //
      // Outside, no reflections
      //
      default:
      case ENVIRONMENT_GENERIC:
         {
         material = &open_air;
         room_size = 0.0F;
         break;
         }

      //
      // Small rooms
      //
      case ENVIRONMENT_STONECORRIDOR:
         {
         material = &brick;
         room_size = 3.0F;
         break;
         }
      case ENVIRONMENT_CARPETEDHALLWAY:
      case ENVIRONMENT_PADDEDCELL:
         {
         material = &carpet;
         room_size = 3.0F;
         break;
         }
      case ENVIRONMENT_BATHROOM:
         {
         material = &glass;
         room_size = 3.0F;
         break;
         }
      case ENVIRONMENT_SEWERPIPE:
         {
         material = &metal;
         room_size = 3.0F;
         break;
         }
      case ENVIRONMENT_UNDERWATER:
      case ENVIRONMENT_DRUGGED:
      case ENVIRONMENT_DIZZY:
      case ENVIRONMENT_PSYCHOTIC:
         {
         if (room_type == ENVIRONMENT_DRUGGED)
            emphasis = 2.0F;
         else if (room_type == ENVIRONMENT_DIZZY)
            emphasis = 4.0F;
         else if (room_type == ENVIRONMENT_PSYCHOTIC)
            emphasis = 8.0F;
         material = &water;
         room_size = 3.0F;
         break;
         }
      case ENVIRONMENT_HALLWAY:
         {
         material = &wood;
         room_size = 3.0F;
         break;
         }

      //
      // Medium rooms
      //
      case ENVIRONMENT_STONEROOM:
         {
         material = &brick;
         room_size = 6.0F;
         break;
         }
      case ENVIRONMENT_LIVINGROOM:
      case ENVIRONMENT_ROOM:
         {
         material = &wood;
         room_size = 6.0F;
         break;
         }

      //
      // Large rooms
      //
      case ENVIRONMENT_CAVE:
         {
         material = &brick;
         room_size = 16.0F;
         break;
         }
      case ENVIRONMENT_ALLEY:
         {
         material = &metal;
         room_size = 16.0F;
         break;
         }

      //
      // Very large rooms
      //
      case ENVIRONMENT_QUARRY:
         {
         material = &brick;
         room_size = 64.0F;
         break;
         }
      case ENVIRONMENT_CONCERTHALL:
         {
         material = &carpet;
         room_size = 64.0F;
         break;
         }
      case ENVIRONMENT_ARENA:
         {
         material = &glass;
         room_size = 64.0F;
         break;
         }
      case ENVIRONMENT_HANGAR:
         {
         material = &metal;
         room_size = 64.0F;
         break;
         }
      case ENVIRONMENT_AUDITORIUM:
         {
         material = &wood;
         room_size = 64.0F;
         break;
         }

      //
      // Outside, with some reflections
      //
      case ENVIRONMENT_MOUNTAINS:
         {
         material = &brick;
         room_size = 256.0F;
         break;
         }
      case ENVIRONMENT_PLAIN:
         {
         material = &carpet;
         room_size = 256.0F;
         break;
         }
      case ENVIRONMENT_CITY:
         {
         material = &glass;
         room_size = 256.0F;
         break;
         }
      case ENVIRONMENT_PARKINGLOT:
         {
         material = &metal;
         room_size = 256.0F;
         break;
         }
      case ENVIRONMENT_FOREST:
         {
         material = &wood;
         room_size = 256.0F;
         break;
         }
      }

   //
   // Apply the global reflection scale to both the delay and
   // the gain, so that reflections are really obvious.
   //

   result = lpA3DGeom->SetReflectionDelayScale (emphasis);
   CHECK_RESULT(result);

   result = lpA3DGeom->SetReflectionGainScale (emphasis);
   CHECK_RESULT(result);

   //
   // Apply the properties to the wall material.  They won't take
   // effect until the next frame, when we recreate the walls.
   //

   lpA3DWallMaterial->SetReflectance   (REFLECTANCE(*material),
                                        REFLECTANCE_HIFREQ(*material));
   lpA3DWallMaterial->SetTransmittance (TRANSMITTANCE(*material),
                                        TRANSMITTANCE_HIFREQ(*material));

   g_room_type = room_type;

   API_unlock ();
}

//##############################################################################
//#                                                                            #
//# M3D_3D_room_type                                                           #
//#                                                                            #
//##############################################################################

S32       AILEXPORT M3D_3D_room_type (void)
{
   return g_room_type;
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_position                                                        #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_position         (H3DPOBJECT obj, //)
                                                  F32     X,
                                                  F32     Y,
                                                  F32     Z)
{
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;
   HRESULT result;

   if (t == NULL)
      {
      return;
      }

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      X_VAL(S->position) = X;
      Y_VAL(S->position) = Y;
      Z_VAL(S->position) = -Z;

      if (S->lpSrc != NULL)
         {
         API_lock();

         result = S->lpSrc->SetPosition3fv (S->position);
         CHECK_RESULT(result);

         API_unlock();
         }

      Z_VAL(S->position) = Z;

      }
   else if (*t == IS_LISTENER)
      {
      X_VAL(listen_position) = X;
      Y_VAL(listen_position) = Y;
      Z_VAL(listen_position) = -Z;

      API_lock();

      result = lpA3DListener->SetPosition3fv (listen_position);
      CHECK_RESULT(result);

      API_unlock();

      Z_VAL(listen_position) = Z;
      }
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_velocity_vector                                                 #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_velocity_vector  (H3DPOBJECT obj, //)
                                                  F32     dX_per_ms,
                                                  F32     dY_per_ms,
                                                  F32     dZ_per_ms)
{
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;
   A3DVECTOR per_second;
   HRESULT result;

   if (t == NULL)
      {
      return;
      }

   X_VAL(per_second) = dX_per_ms * 1000.0F;
   Y_VAL(per_second) = dY_per_ms * 1000.0F;
   Z_VAL(per_second) = -dZ_per_ms * 1000.0F;

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      X_VAL(S->velocity) = dX_per_ms;
      Y_VAL(S->velocity) = dY_per_ms;
      Z_VAL(S->velocity) = dZ_per_ms;

      if (S->lpSrc != NULL)
         {
         API_lock();

         result = S->lpSrc->SetVelocity3fv (per_second);
         CHECK_RESULT(result);

         API_unlock();
         }
      }
   else if (*t == IS_LISTENER)
      {
      X_VAL(listen_velocity) = dX_per_ms;
      Y_VAL(listen_velocity) = dY_per_ms;
      Z_VAL(listen_velocity) = dZ_per_ms;

      API_lock();

      result = lpA3DListener->SetVelocity3fv (per_second);
      CHECK_RESULT(result);

      API_unlock();
      }
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_velocity                                                        #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_velocity         (H3DPOBJECT obj, //)
                                                  F32     dX_per_ms,
                                                  F32     dY_per_ms,
                                                  F32     dZ_per_ms,
                                                  F32     magnitude)
{
   M3D_set_3D_velocity_vector(obj,
                              dX_per_ms * magnitude,
                              dY_per_ms * magnitude,
                              dZ_per_ms * magnitude);
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_orientation                                                     #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_orientation      (H3DPOBJECT obj, //)
                                                  F32     X_face,
                                                  F32     Y_face,
                                                  F32     Z_face,
                                                  F32     X_up,
                                                  F32     Y_up,
                                                  F32     Z_up)
{
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;

   if (t == NULL)
      {
      return;
      }

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      X_VAL(S->face) = X_face;
      Y_VAL(S->face) = Y_face;
      Z_VAL(S->face) = Z_face;

      X_VAL(S->up) = X_up;
      Y_VAL(S->up) = Y_up;
      Z_VAL(S->up) = Z_up;

      API_lock();

//{char buf[128];sprintf(buf,"%f %f %f %f %f %f\n",X_VAL(S->position),Y_VAL(S->position),Z_VAL(S->position),X_face,Y_face,Z_face);OutputDebugString(buf);}

      HRESULT result = S->lpSrc->SetOrientation6f      (X_face,
                                                        Y_face,
                                                        -Z_face,
                                                        X_up,
                                                        Y_up,
                                                        -Z_up);
      CHECK_RESULT(result);

      API_unlock();

      }
   else if (*t == IS_LISTENER)
      {
      //
      // "face" vector - points in facing direction
      //
      // "up" vector - perpendicular to orientation vector
      // This vector points to which direction is up
      // for the listener - it can not be parallel to the
      // listener orientation vector
      //

      X_VAL(listen_face) = X_face;
      Y_VAL(listen_face) = Y_face;
      Z_VAL(listen_face) = Z_face;

      X_VAL(listen_up) = X_up;
      Y_VAL(listen_up) = Y_up;
      Z_VAL(listen_up) = Z_up;

      API_lock();

      HRESULT result = lpA3DListener->SetOrientation6f (X_face,
                                                        Y_face,
                                                        -Z_face,
                                                        X_up,
                                                        Y_up,
                                                        -Z_up);
      CHECK_RESULT(result);

      API_unlock();
      }
}

//##############################################################################
//#                                                                            #
//# M3D_3D_position                                                            #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_3D_position             (H3DPOBJECT  obj, //)
                                                  F32 FAR *X,
                                                  F32 FAR *Y,
                                                  F32 FAR *Z)
{
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;

   if (t == NULL)
      {
      return;
      }

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      if (X) *X = X_VAL(S->position);
      if (Y) *Y = Y_VAL(S->position);
      if (Z) *Z = Z_VAL(S->position);
      }
   else if (*t == IS_LISTENER)
      {
      if (X) *X = X_VAL(listen_position);
      if (Y) *Y = Y_VAL(listen_position);
      if (Z) *Z = Z_VAL(listen_position);
      }
}

//##############################################################################
//#                                                                            #
//# M3D_3D_velocity                                                            #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_3D_velocity             (H3DPOBJECT  obj, //)
                                                  F32 FAR *dX_per_ms,
                                                  F32 FAR *dY_per_ms,
                                                  F32 FAR *dZ_per_ms)
{
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;

   if (t == NULL)
      {
      return;
      }

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      if (dX_per_ms) *dX_per_ms = X_VAL(S->velocity);
      if (dY_per_ms) *dY_per_ms = Y_VAL(S->velocity);
      if (dZ_per_ms) *dZ_per_ms = Z_VAL(S->velocity);
      }
   else if (*t == IS_LISTENER)
      {
      if (dX_per_ms) *dX_per_ms = X_VAL(listen_velocity);
      if (dY_per_ms) *dY_per_ms = Y_VAL(listen_velocity);
      if (dZ_per_ms) *dZ_per_ms = Z_VAL(listen_velocity);
      }
}

//##############################################################################
//#                                                                            #
//# M3D_3D_orientation                                                         #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_3D_orientation          (H3DPOBJECT  obj, //)
                                                  F32 FAR *X_face,
                                                  F32 FAR *Y_face,
                                                  F32 FAR *Z_face,
                                                  F32 FAR *X_up,
                                                  F32 FAR *Y_up,
                                                  F32 FAR *Z_up)
{
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;

   if (t == NULL)
      {
      return;
      }

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      if (X_face) *X_face = X_VAL(S->face);
      if (Y_face) *Y_face = Y_VAL(S->face);
      if (Z_face) *Z_face = Z_VAL(S->face);
      if (X_up)   *X_up   = X_VAL(S->up);
      if (Y_up)   *Y_up   = Y_VAL(S->up);
      if (Z_up)   *Z_up   = Z_VAL(S->up);
      }
   else if (*t == IS_LISTENER)
      {
      if (X_face) *X_face = X_VAL(listen_face);
      if (Y_face) *Y_face = Y_VAL(listen_face);
      if (Z_face) *Z_face = Z_VAL(listen_face);
      if (X_up)   *X_up   = X_VAL(listen_up);
      if (X_up)   *Y_up   = Y_VAL(listen_up);
      if (X_up)   *Z_up   = Z_VAL(listen_up);
      }
}

//##############################################################################
//#                                                                            #
//# M3D_3D_update_position                                                     #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_3D_update_position      (H3DPOBJECT obj, //)
                                                  F32     dt_milliseconds)
{
   if (obj == NULL)
      {
      return;
      }

   F32 X,Y,Z;
   F32 dX_dt,dY_dt,dZ_dt;

   M3D_3D_velocity(obj,&dX_dt,&dY_dt,&dZ_dt);

   if ((fabs(dX_dt) < EPSILON) &&
       (fabs(dY_dt) < EPSILON) &&
       (fabs(dZ_dt) < EPSILON))
       {
       return;
       }

   M3D_3D_position(obj,&X,&Y,&Z);

   X += (dX_dt * dt_milliseconds);
   Y += (dY_dt * dt_milliseconds);
   Z += (dZ_dt * dt_milliseconds);

   M3D_set_3D_position(obj,X,Y,Z);
}

//##############################################################################
//#                                                                            #
//# M3D_3D_auto_update_position                                                #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_3D_auto_update_position (H3DPOBJECT obj, //)
                                                  S32        enable)
{
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;

   if (t == NULL)
      {
      return;
      }

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      S->auto_update = enable;
      }
   else if (*t == IS_LISTENER)
      {
      listen_auto_update = enable;
      }
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_data                                                     #
//#                                                                            #
//##############################################################################

S32        AILEXPORT M3D_set_3D_sample_data      (H3DSAMPLE         samp, //)
                                                  AILSOUNDINFO FAR *info)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;
   HRESULT result1, result2, result3, result4, result5;

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   //
   // Validate format
   //

   if (info->format != WAVE_FORMAT_PCM)
      {
      AIL_set_error("Not linear PCM data");
      return 0;
      }

   if (info->channels != 1)
      {
      AIL_set_error("Not monaural data");
      return 0;
      }

   S->eos=0;
   S->docb=0;

   //
   // Ensure that the background thread does not try to access the
   // sample structure while we are manipulating it
   //

   sleep_sample(S);

   //
   // Abort any pending events
   //

   reset_sample_events(S);

   //
   // If sample already has an assigned emitter, kill it
   //

   destroy_sample_emitter(S);

   //
   // Initialize sample fields
   //

   init_sample(S);

   S->bytes_per_sample = info->bits / 8;

   S->start         =  info->data_ptr;
   S->len           =  info->data_len;
   S->pos           =  0;
   S->done          =  0;

   S->buffers_past_end = 0;

   S->loop_count    =  1;
   S->loop_start    =  0;
   S->loop_end      = -1;

   S->volume        =  AIL_get_preference(DIG_DEFAULT_VOLUME);
   S->playback_rate = info->rate;

   S->status        =  SMP_DONE;

   S->lpSrc         = NULL;

   //
   // Set up WAVEFORMATEX structure
   //

   WAVEFORMATEX wf;

   wf.wFormatTag      = WAVE_FORMAT_PCM;
   wf.nChannels       = (S16) info->channels;
   wf.nSamplesPerSec  = info->rate;
   wf.nAvgBytesPerSec = (info->rate * info->channels * info->bits) / 8;
   wf.nBlockAlign     = (info->channels * info->bits) / 8;
   wf.wBitsPerSample  = (S16) info->bits;
   wf.cbSize          = 0;

   //
   // Create A3D source object
   //

   API_lock ();

   //
   // We currently create only static (unmanaged) sources because of a
   // bug in the A3D resource manager that causes managed sources to skip.
   // This doesn't mean the sources will only play on A3D hardware, but it
   // does mean A3D's prioritization is unavailable.  We request A3D
   // rendering to get the highest quality output.
   //

   result1 = lpA3D->NewSource (A3DSOURCE_INITIAL_RENDERMODE_A3D |
                               A3DSOURCE_TYPESTREAMED |
                               A3DSOURCE_TYPEUNMANAGED,
                              &S->lpSrc);
   CHECK_RESULT(result1);

   if (result1 == S_OK)
      {
      result2 = S->lpSrc->SetWaveFormat ((LPVOID) &wf);
      CHECK_RESULT (result2);

      if (result2 == S_OK)
         {
         result3 = S->lpSrc->AllocateWaveData (BUFF_SIZE * 2);
         CHECK_RESULT (result3);

         if (result3 == S_OK)
            {

            //
            // Instruct A3D to signal the appropriate events when the play
            // cursor moves from one half of the buffer to the other.
            //
            // A3D will signal the BUFFER2 event as soon as the sample
            // begins playing, because the play cursor starts at zero.
            // We don't fill the second half-buffer until this event
            // is set.
            //

            result4 = S->lpSrc->SetWaveEvent (BUFF_SIZE,
                                              m_hBufferEvents[BUFFER1_EVENT+(S->index*NUM_EVENTS)]);
            CHECK_RESULT (result4);

            // trick to cause our multiple layer events to trigger
            result4 = S->lpSrc->SetWaveEvent (BUFF_SIZE+1,
                                              m_hmaster);
            CHECK_RESULT (result4);

            result5 = S->lpSrc->SetWaveEvent (0,
                                              m_hBufferEvents[BUFFER2_EVENT+(S->index*NUM_EVENTS)]);
            CHECK_RESULT (result5);

            // trick to cause our multiple layer events to trigger
            result4 = S->lpSrc->SetWaveEvent (1,
                                              m_hmaster);
            CHECK_RESULT (result4);

            }
         }
      }

   API_unlock();

   if (FAILED(result1) || FAILED(result2) || FAILED(result3) ||
       FAILED(result4) || FAILED(result5))
      {
      AIL_set_error("Could not create secondary A3D buffer");
      return 0;
      }

   //
   // Set emitter distances
   //

   M3D_set_3D_sample_distances(samp,
                               200.0,
                               1.0);
   //
   // Set default emitter volume, orientation, and velocity
   //
   // We do not set a default position here, to avoid Doppler artifacts
   // when the application sets the actual emitter position
   //

   M3D_set_3D_sample_volume(samp,
                            S->volume);

   M3D_set_3D_orientation(samp,
                          0.0F, 0.0F, 1.0F,
                          0.0F, 1.0F, 0.0F);

   //
   // Set velocity
   //

   M3D_set_3D_velocity_vector(samp,
                              0.0F,
                              0.0F,
                              0.0F);

   //
   // Clear all special attenuation effects
   //
   M3D_set_3D_sample_obstruction  (samp, 0.0F);
   M3D_set_3D_sample_occlusion    (samp, 0.0F);
   M3D_set_3D_sample_effects_level(samp, 0.0F);

	//
	// Set cone
	//

	M3D_set_3D_sample_cone    (samp,
										360.0F,
										360.0F,
                              127);

   //
   // Release the background thread from its sleep state
   //

   wake_sample(S);

   return 1;
}


//############################################################################
//#                                                                          #
//# AIL timer callback for automatic position updates                        #
//#                                                                          #
//############################################################################

void AILCALLBACK M3D_serve(U32 user)
{
   S32 i;

   //
   // Update sample positions
   //

   for (i=0; i < avail_samples; i++)
      {
      SAMPLE3D FAR *S = &samples[i];

      if (!S->auto_update)
         {
         continue;
         }

      if ((S->status != SMP_PLAYING) && (S->status != SMP_STOPPED))
         {
         continue;
         }

      M3D_3D_update_position(H3DPOBJECT(S),
                             F32(SERVICE_MSECS));
      }

   //
   // Update listener positions
   //

   if (listen_auto_update)
      {
      OBJTYPE type = IS_LISTENER;

      M3D_3D_update_position(H3DPOBJECT(&type),
                             F32(SERVICE_MSECS));
      }
}

//############################################################################
//#                                                                          #
//# Shut down A3D cleaning up all resources.                                 #
//#                                                                          #
//############################################################################

void shut_down (void)
{
   HRESULT result;

   destroy_all_sample_events();

   if (lpA3DWallMaterial)
      {
      result = lpA3DWallMaterial->Release ();
      CHECK_RESULT(result);
      lpA3DWallMaterial = NULL;
      }

   if (lpA3DListener)
      {
      result = lpA3DListener->Release ();
      CHECK_RESULT(result);
      lpA3DListener = NULL;
      }

   if (lpA3DGeom)
      {
      result = lpA3DGeom->Release ();
      CHECK_RESULT(result);
      lpA3DGeom = NULL;
      }

   if (lpA3D)
      {
      A3dUninitialize ();
      lpA3D = NULL;
      }

   if (hA3DMutex)
      {
      CloseHandle (hA3DMutex);
      hA3DMutex = NULL;
      }
}

//##############################################################################
//#                                                                            #
//# M3D_activate                                                               #
//#                                                                            #
//##############################################################################

static S32 saved=0;
static DWORD oldscreen=0;
static DWORD oldaudio=0;

M3DRESULT AILEXPORT M3D_activate                 (S32 enable)
{
   HRESULT result;
   AILLPDIRECTSOUND prev_lpds = NULL;     // MSS's DirectSound pointer

   if (enable)
      {
      // -----------------------------------------
      // Activate
      // -----------------------------------------

      if (active)
         {
         AIL_set_error("M3D provider already activated");
         return M3D_ALREADY_STARTED;
         }

      //
      // Create a mutex in preparation for multithreaded access to A3D.
      // A3D actually seems to work pretty well without the mutex, so a
      // failure to create it -- which is very unlikely anyway -- is not
      // currently considered a fatal error.
      //
      hA3DMutex = CreateMutex (NULL, FALSE, NULL);
      if (hA3DMutex == NULL)
         diag_printf ("Failed to create A3D mutex\r\n");

      //
      // Get DirectSound pointer from MSS; if MSS is already
      // using DirectSound, we must temporarily disable it
      // across the calls to initialize and activate A3D.  This
      // is presumably because A3D initializes DirectSound using
      // DDSCL_PRIORITY.  Anyway, DirectSound hangs if we have
      // it open already at a lower priority level.
      //
      AILLPDIRECTSOUNDBUFFER dummy = NULL;
      AIL_get_DirectSound_info (NULL, &prev_lpds, &dummy);

      if (prev_lpds)
         {
         AIL_digital_handle_release (NULL);
         AIL_get_DirectSound_info (NULL, &prev_lpds, &dummy);
         if (prev_lpds)
            {
            AIL_digital_handle_reacquire (NULL);
            AIL_set_error("Unable to release DirectSound object");
            shut_down ();
            return M3D_NOT_INIT;
            }
         prev_lpds = (AILLPDIRECTSOUNDBUFFER) ~NULL;
         }

      //
      // turn off the stupid splash sound and graphic
      //

		// A3D registry key
		#define REG_SETTINGS_KEY				 TEXT("Software\\Aureal\\A3D")
		// A3D splash registry values
		#define REG_SETTING_SPLASH_SCREEN  TEXT("SplashScreen")
		#define REG_SETTING_SPLASH_AUDIO	 TEXT("SplashAudio")

      HKEY hReg;
      DWORD dwCreateDisposition;

      // Save current settings to our registry key
      if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REG_SETTINGS_KEY,
                     0, NULL, 0, KEY_WRITE, NULL, &hReg,
                     &dwCreateDisposition)==ERROR_SUCCESS)
      {

        saved=1;
        DWORD typ,size=sizeof(DWORD);

        // read original value
        RegQueryValueEx(hReg,REG_SETTING_SPLASH_SCREEN, 0,
                        &typ,(LPBYTE)&oldscreen,&size);
        RegQueryValueEx(hReg,REG_SETTING_SPLASH_AUDIO, 0,
                        &typ,(LPBYTE)&oldaudio,&size);


        // Save the "SplashScreen" flag
        DWORD dwVal = (DWORD) false;        // "true" to enable
        RegSetValueEx(hReg, REG_SETTING_SPLASH_SCREEN, 0, REG_DWORD,
                    (LPBYTE) &dwVal, sizeof(DWORD));
        RegSetValueEx(hReg, REG_SETTING_SPLASH_AUDIO, 0, REG_DWORD,
                    (LPBYTE) &dwVal, sizeof(DWORD));

        RegCloseKey(hReg);
        hReg = NULL;
      }


      //
      // Initialize A3D.  Internally, the steps are 1) initializing COM;
      // 2) creating A3D registry keys for COM; 3) creating the root
      // IA3d4 object.
      //
      // To specify the GUID that A3D should use, pass it as the first
      // argument.
      //
      // To disable the A3D splash screen, or A3D_DISABLE_SPLASHSCREEN
      // into the last argument.
      //
      AIL_unlock_mutex();
      result = A3dCreate (NULL,
                          (LPVOID *)&lpA3D,
                          NULL,
                          A3D_1ST_REFLECTIONS | A3D_OCCLUSIONS);
      AIL_lock_mutex();
      CHECK_RESULT(result);

      if (FAILED(result))
         {
         if (prev_lpds)
           AIL_digital_handle_reacquire (NULL);

         AIL_set_error("The A3D 2 library could not be loaded");
         shut_down ();
         return M3D_NOT_INIT;
         }

      //
      // Create geometry interface early on.  There appears to be
      // an undocumented order dependency in the A3D initialization
      // sequence.  If the geometry object is created later, then
      // hardware reflections aren't available.
      //
      result = lpA3D->QueryInterface (IID_IA3dGeom,
                                      (LPVOID *) &lpA3DGeom);
      CHECK_RESULT(result);

      if (FAILED(result))
         {
         if (prev_lpds)
           AIL_digital_handle_reacquire (NULL);

         AIL_set_error("Failed to create A3D geometry object");
         shut_down ();
         return M3D_NOT_INIT;
         }

      //
      // Set the cooperative level to allow other applications access
      // to the audio device at the same time, unless the USE_PRIMARY
      // preference is set.
      //

      AIL_unlock_mutex();
      result = lpA3D->SetCooperativeLevel (AIL_HWND(),
               AIL_get_preference (DIG_DS_USE_PRIMARY) ?
               A3D_CL_EXCLUSIVE : A3D_CL_NORMAL);
      AIL_lock_mutex();
      CHECK_RESULT(result);

      //
      // If MSS had DirectSound open before, reacquire a handle
      // for it to use.
      //
      if (prev_lpds)
         {
         AIL_set_preference (DIG_DS_DSBCAPS_CTRL3D, TRUE);
         AIL_digital_handle_reacquire (NULL);
         AIL_get_DirectSound_info (NULL, &prev_lpds, &dummy);
         if (! prev_lpds)
            {
            AIL_set_error("Unable to reacquire DirectSound object");
            shut_down ();
            return M3D_NOT_INIT;
            }
         }

#if defined(CHECK_API_RETURN_VALUES) && CHECK_API_RETURN_VALUES
      //
      // Get hardware and software capability information, just for
      // debugging.
      //
      A3DCAPS_SOFTWARE software_caps;
      A3DCAPS_HARDWARE hardware_caps;

      memset (&software_caps, 0, sizeof(software_caps));
      software_caps.dwSize = sizeof(software_caps);
      result = lpA3D->GetSoftwareCaps (&software_caps);
      CHECK_RESULT(result);

      memset (&hardware_caps, 0, sizeof(hardware_caps));
      hardware_caps.dwSize = sizeof(hardware_caps);
      result = lpA3D->GetHardwareCaps (&hardware_caps);
      CHECK_RESULT(result);

      diag_printf ("A3D initialized.\r\n"
                   "Buffers = %u hw 3D, %u hw 2D, %u sw 3D, %u sw 2D\r\n",
                   hardware_caps.dwMax3DBuffers,
                   hardware_caps.dwMax2DBuffers,
                   software_caps.dwMax3DBuffers,
                   software_caps.dwMax2DBuffers);
#endif

      //
      // Set other global preferences.  Many of these are commented out
      // because they are the defaults.
      //
      // The following global preferences are listed in the header file,
      // but not documented for A3D 2.0:
      //
      //   SetOutputMode
      //   SetResourceManagerMode
      //   SetHFAbsorbFactor
      //

      result = lpA3D->SetCoordinateSystem (A3D_RIGHT_HANDED_CS);
      CHECK_RESULT(result);
      result = lpA3D->SetMaxReflectionDelayTime (0.5F);
      CHECK_RESULT(result);
      units_per_meter = 1.0F;
      //result = lpA3D->SetUnitsPerMeter (units_per_meter);
      //CHECK_RESULT(result);
      //result = lpA3D->SetDistanceModelScale (1.0F);
      //CHECK_RESULT(result);
      //result = lpA3D->SetDopplerScale (1.0F);
      //CHECK_RESULT(result);
      //result = lpA3D->SetEq (1.0F);
      //CHECK_RESULT(result);
      //result = lpA3D->SetOutputGain (1.0F);
      //CHECK_RESULT(result);
      //result = lpA3D->SetRMPriorityBias (0.5F);
      //CHECK_RESULT(result);

      //
      // We leave the debug viewer enabled.  If the user wants to disable
      // it, he can obtain the lpA3D pointer with the TECHNOLOGY_INTERFACE
      // attribute.
      //
      //lpA3D->DisableViewer ();

      //
      // Configure for the desired number of channels, according to
      // the AIL preference (default = 16).  We use A3D's resource
      // manager, so some number of these may be accelerated in hardware,
      // but we have no way of knowing how many.  Therefore, we ask
      // for enough fallback (software-mixed) sources to accommodate
      // all of our channels.
      //
      avail_samples = AIL_get_preference (DIG_MIXER_CHANNELS);
      if (avail_samples > N_SAMPLES)
         avail_samples = N_SAMPLES;

      result = lpA3D->SetNumFallbackSources (avail_samples);
      CHECK_RESULT(result);

      API_lock ();

      //
      // Create listener object
      //

      result = lpA3D->QueryInterface (IID_IA3dListener,
                                      (LPVOID *) &lpA3DListener);
      CHECK_RESULT(result);

      if (FAILED(result))
         {
         API_unlock();
         AIL_set_error("Failed to create A3D listener object");
         shut_down ();
         return M3D_INTERNAL_ERR;
         }

      //
      // Enable occlusions and reflections, if the features are
      // available.  We will only use reflections with the wall
      // geometry we create, and we will only create that geometry
      // if reflections are available.
      //

      if (lpA3D->IsFeatureAvailable (A3D_OCCLUSIONS))
         {
         result = lpA3DGeom->Enable (A3D_OCCLUSIONS);
         CHECK_RESULT(result);
         }

      if (lpA3D->IsFeatureAvailable (A3D_1ST_REFLECTIONS))
         {
         result = lpA3DGeom->Enable (A3D_1ST_REFLECTIONS);
         CHECK_RESULT(result);

         //
         // Set the default render mode to include reflections but not
         // occlusions.  The walls don't really exist in the world, so they
         // must not be permitted to occlude anything.
         //

         result = lpA3DGeom->SetRenderMode (A3D_1ST_REFLECTIONS);
         CHECK_RESULT (result);

         //
         // Create a material to be used for the room walls.
         //

         result = lpA3DGeom->NewMaterial (&lpA3DWallMaterial);
         CHECK_RESULT(result);

         if (FAILED(result))
            {
            API_unlock();
            AIL_set_error("Failed to create A3D wall material");
            shut_down ();
            return M3D_INTERNAL_ERR;
            }

         result = lpA3DWallMaterial->SetNameID ("walls");
         CHECK_RESULT(result);
         }

      API_unlock();

      //
      // Set default listener position
      //

      OBJTYPE type = IS_LISTENER;

      M3D_set_3D_position(H3DPOBJECT(&type),
                          0.0F,
                          0.0F,
                          0.0F);

      //
      // Set default listener orientation and up-vector
      //

      M3D_set_3D_orientation(H3DPOBJECT(&type),
                             0.0F, 0.0F, 1.0F,
                             0.0F, 1.0F, 0.0F);

      //
      // Set default listener velocity
      //

      M3D_set_3D_velocity_vector(H3DPOBJECT(&type),
                                 0.0F,
                                 0.0F,
                                 0.0F);

      //
      // Set default room type
      //

      M3D_set_3D_room_type (ENVIRONMENT_GENERIC);

      //
      // Initialize samples
      //

      S32 i;

      for (i=0; i < avail_samples; i++)
         {
         SAMPLE3D FAR *S = &samples[i];

         memset(S, 0, sizeof(struct SAMPLE3D));

         S->index  = i;
         S->status = SMP_FREE;

         S->cancel_pending = 0;

         //
         // Clear sample emitter
         //

         S->lpSrc   = NULL;

         }

       num_events=NUM_EVENTS*avail_samples;

       //
       // Create Win32 signal events for sample
       //

       U32 j;

       for (j = 0; j <= num_events; j++)
       {
          //
          // Events are initially non-signaled
          //

          m_hBufferEvents[j] = CreateEvent(0,0,0,0);
          if (!m_hBufferEvents[j])
          {
            AIL_set_error("Error creating events");
            return M3D_INTERNAL_ERR;
          }

       }

      m_hmaster=CreateEvent(0,0,0,0);
      if (!m_hmaster)
      {
        AIL_set_error("Error creating event");
        return M3D_INTERNAL_ERR;
      }

      ResetEvent(m_hmaster);

      //
		  // Clear thread variable
	    //

      DWORD stupId;

      is_safe=0;
      current_sample=0;
      m_lpThread=CreateThread(0,0,pfnThreadProc,0,0,&stupId);
      if (m_lpThread==0)
      {
         AIL_set_error("Error creating thread");
         return M3D_INTERNAL_ERR;
      }

      //
      // Register and start service timer
      //

      service_timer = AIL_register_timer(M3D_serve);

      AIL_set_timer_period(service_timer, SERVICE_MSECS * 1000);

      AIL_start_timer(service_timer);

		//
		// Register and start buffer-notification timer at 20 Hz
		//

		buffer_timer = AIL_register_timer(notify_timer);

		AIL_set_timer_frequency(buffer_timer, 20);

		AIL_start_timer(buffer_timer);

      //
      // Create a thread to flush the audio frame at 30Hz
      //
      DWORD threadId;

      hFlushEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
      if (! hFlushEvent)
         {
         AIL_set_error("Error creating events");
         shut_down ();
         return M3D_INTERNAL_ERR;
         }

      hFlushThread = CreateThread (NULL,
                                   NULL,
                                   flushThreadProc,
                                   0,
                                   0,
                                   &threadId);
      if (!hFlushThread)
         {
         AIL_set_error("Error creating flush thread");
         shut_down ();
         return M3D_INTERNAL_ERR;
         }

      M3D_set_3D_speaker_type (AIL_3D_2_SPEAKER);

      active = 1;
      }
   else
      {
      // -----------------------------------------
      // Deactivate
      // -----------------------------------------

      if (!active)
         {
         AIL_set_error("M3D provider not activated");
         return M3D_NOT_INIT;
         }

      //
      // Stop service timer and buffer timer
      //

      AIL_stop_timer(service_timer);
      AIL_release_timer_handle(service_timer);

		AIL_stop_timer(buffer_timer);
		AIL_release_timer_handle(buffer_timer);

      AIL_unlock_mutex();

      Sleep(100);

      AIL_lock_mutex();

      //
      // Deallocate resources associated with samples
      //

      S32 i;

      for (i=0; i < avail_samples; i++)
         {
         SAMPLE3D FAR *S = &samples[i];

         reset_sample_voice(S);

         //
         // Mark handle free
         //

         S->status = SMP_FREE;

         //
         // Destroy emitter associated with sample
         //

         destroy_sample_emitter(S);
         }

      destroy_all_sample_events();

      //
      // Release the listener
      //

      lpA3DListener->Release();
      lpA3DListener = NULL;

      //
      // Go through the same gyrations we did when starting up, to
      // prevent A3D from hanging because of cooperative level
      // problems.
      //

      AILLPDIRECTSOUNDBUFFER dummy = NULL;
      AIL_get_DirectSound_info (NULL, &prev_lpds, &dummy);

      if (prev_lpds)
         {
         AIL_digital_handle_release (NULL);
         AIL_get_DirectSound_info (NULL, &prev_lpds, &dummy);
         if (prev_lpds)
            {
            AIL_set_error("Unable to release DirectSound object");
            shut_down ();
            return M3D_CLOSE_ERR;
            }
         prev_lpds = (AILLPDIRECTSOUNDBUFFER) ~NULL;
         }

      //
      // Deactivate A3D.
      //

      lpA3DGeom->Release ();
      lpA3DGeom = NULL;

      AIL_unlock_mutex();
      lpA3D->Release();
      A3dUninitialize ();
      AIL_lock_mutex();
      lpA3D = NULL;

      //
      // Reacquire a DirectSound handle for MSS.
      //

      if (prev_lpds)
         {
         AIL_digital_handle_reacquire (NULL);
         AIL_get_DirectSound_info (NULL, &prev_lpds, &dummy);
         if (! prev_lpds)
            {
            AIL_set_error("Unable to reacquire DirectSound object");
            shut_down ();
            return M3D_CLOSE_ERR;
            }
         }

      CloseHandle (hA3DMutex);
      hA3DMutex = NULL;

		// restore the registry
		if (saved)
		{
		  saved=0;

		  HKEY hReg;
		  DWORD dwCreateDisposition;

		  // Save current settings to our registry key
		  if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REG_SETTINGS_KEY,
							  0, NULL, 0, KEY_WRITE, NULL, &hReg,
							  &dwCreateDisposition)==ERROR_SUCCESS)
		  {

			 // Save the "SplashScreen" flag
			 RegSetValueEx(hReg, REG_SETTING_SPLASH_SCREEN, 0, REG_DWORD,
							 (LPBYTE) &oldscreen, sizeof(DWORD));
			 RegSetValueEx(hReg, REG_SETTING_SPLASH_AUDIO, 0, REG_DWORD,
							 (LPBYTE) &oldaudio, sizeof(DWORD));

			 RegCloseKey(hReg);
			 hReg = NULL;
		  }

		}

      active = 0;
      diag_printf ("Successfully closed A3D.\r\n");
      }

   return M3D_NOERR;
}

//############################################################################
//#                                                                          #
//# DLLMain registers M3D API interface at load time                         #
//#                                                                          #
//############################################################################

BOOL WINAPI DllMain(HINSTANCE hinstDll, //)
                          DWORD     fdwReason,
                          LPVOID    plvReserved)
{
   const RIB_INTERFACE_ENTRY M3DSAMPLE[] =
      {
      REG_AT("Audibility",                 AUDIBILITY,            RIB_FLOAT),
      REG_AT("Occlusion factor",           OCCLUSION_FACTOR,      RIB_FLOAT),
      REG_AT("Distance model scale",       DISTANCE_SCALE,        RIB_FLOAT),
      REG_AT("Doppler scale",              DOPPLER_SCALE,         RIB_FLOAT),
      REG_AT("Equalization",               EQUALIZATION,          RIB_FLOAT),
      REG_AT("Reflection delay scale",     REFLECTION_DELAY_SCALE,RIB_FLOAT),
      REG_AT("Reflection gain scale",      REFLECTION_GAIN_SCALE, RIB_FLOAT),
      REG_AT("Priority",                   PRIORITY,              RIB_FLOAT),
      REG_AT("Render mode",                RENDER_MODE,           RIB_HEX),

      REG_PR("Distance model scale",       DISTANCE_SCALE,        RIB_FLOAT),
      REG_PR("Doppler scale",              DOPPLER_SCALE,         RIB_FLOAT),
      REG_PR("Equalization",               EQUALIZATION,          RIB_FLOAT),
      REG_PR("Reflection delay scale",     REFLECTION_DELAY_SCALE,RIB_FLOAT),
      REG_PR("Reflection gain scale",      REFLECTION_GAIN_SCALE, RIB_FLOAT),
      REG_PR("Priority",                   PRIORITY,              RIB_FLOAT),
      REG_PR("Render mode",                RENDER_MODE,           RIB_HEX),

      REG_FN(M3D_3D_sample_query_attribute),
      REG_FN(M3D_3D_set_sample_preference),

	    REG_FN(M3D_set_3D_sample_cone),
	  	REG_FN(M3D_3D_sample_cone),

      REG_FN(M3D_set_3D_EOS),
      REG_FN(M3D_set_3D_sample_obstruction),
      REG_FN(M3D_3D_sample_obstruction),
      REG_FN(M3D_set_3D_sample_occlusion),
      REG_FN(M3D_3D_sample_occlusion),
      REG_FN(M3D_set_3D_sample_effects_level),
      REG_FN(M3D_3D_sample_effects_level),
      REG_FN(M3D_set_3D_room_type),
      REG_FN(M3D_3D_room_type),

      REG_FN(M3D_set_3D_speaker_type),
      REG_FN(M3D_3D_speaker_type),

      REG_FN(M3D_set_3D_rolloff_factor),
      REG_FN(M3D_3D_rolloff_factor),

      REG_FN(M3D_set_3D_doppler_factor),
      REG_FN(M3D_3D_doppler_factor),

      REG_FN(M3D_set_3D_distance_factor),
      REG_FN(M3D_3D_distance_factor),
      };

   const RIB_INTERFACE_ENTRY M3D[] =
      {
      REG_AT("Name",                       PROVIDER_NAME,         RIB_STRING),
      REG_AT("Version",                    PROVIDER_VERSION,      RIB_HEX),
      REG_AT("Maximum supported samples",  MAX_SUPPORTED_SAMPLES, RIB_DEC),
      REG_AT("Technology version",         TECHNOLOGY_VERSION,    RIB_HEX),
      REG_AT("A3D technology interface",   TECHNOLOGY_INTERFACE,  RIB_HEX),
      REG_AT("Render first reflections",   RENDER_1ST_REFLECTIONS,RIB_BOOL),
      REG_AT("Render occlusions",          RENDER_OCCLUSIONS,     RIB_BOOL),
      REG_AT("Software mixing",            SOFTWARE_MIXING,       RIB_BOOL),
      REG_AT("Priority bias",              PRIORITY_BIAS,         RIB_FLOAT),
      REG_AT("Output gain",                OUTPUT_GAIN,           RIB_FLOAT),
      REG_AT("Flush timeout",              FLUSH_TIMEOUT,         RIB_DEC),
      REG_AT("Maximum reflection delay",   MAX_REFLECTION_DELAY,  RIB_FLOAT),
      REG_AT("Units per meter",            UNITS_PER_METER,       RIB_FLOAT),
      REG_AT("Virtual geometry",           VIRTUAL_GEOMETRY,      RIB_BOOL),
      REG_AT("Distance model scale",       DISTANCE_SCALE,        RIB_FLOAT),
      REG_AT("Doppler scale",              DOPPLER_SCALE,         RIB_FLOAT),
      REG_AT("Equalization",               EQUALIZATION,          RIB_FLOAT),

      REG_PR("Software mixing",            SOFTWARE_MIXING,       RIB_BOOL),
      REG_PR("Priority bias",              PRIORITY_BIAS,         RIB_FLOAT),
      REG_PR("Output gain",                OUTPUT_GAIN,           RIB_FLOAT),
      REG_PR("Flush timeout",              FLUSH_TIMEOUT,         RIB_DEC),
      REG_PR("Maximum reflection delay",   MAX_REFLECTION_DELAY,  RIB_FLOAT),
      REG_PR("Units per meter",            UNITS_PER_METER,       RIB_FLOAT),
      REG_PR("Virtual geometry",           VIRTUAL_GEOMETRY,      RIB_BOOL),
      REG_PR("Distance model scale",       DISTANCE_SCALE,        RIB_FLOAT),
      REG_PR("Doppler scale",              DOPPLER_SCALE,         RIB_FLOAT),
      REG_PR("Equalization",               EQUALIZATION,          RIB_FLOAT),

      REG_FN(PROVIDER_query_attribute),
      REG_FN(M3D_startup),
      REG_FN(M3D_error),
      REG_FN(M3D_shutdown),
      REG_FN(M3D_set_provider_preference),
      REG_FN(M3D_activate),
      REG_FN(M3D_allocate_3D_sample_handle),
      REG_FN(M3D_release_3D_sample_handle),
      REG_FN(M3D_start_3D_sample),
      REG_FN(M3D_stop_3D_sample),
      REG_FN(M3D_resume_3D_sample),
      REG_FN(M3D_end_3D_sample),
      REG_FN(M3D_set_3D_sample_data),
      REG_FN(M3D_set_3D_sample_volume),
      REG_FN(M3D_set_3D_sample_playback_rate),
      REG_FN(M3D_set_3D_sample_offset),
      REG_FN(M3D_set_3D_sample_loop_count),
      REG_FN(M3D_set_3D_sample_loop_block),
      REG_FN(M3D_3D_sample_status),
      REG_FN(M3D_3D_sample_volume),
      REG_FN(M3D_3D_sample_playback_rate),
      REG_FN(M3D_3D_sample_offset),
      REG_FN(M3D_3D_sample_length),
      REG_FN(M3D_3D_sample_loop_count),
      REG_FN(M3D_set_3D_sample_distances),
      REG_FN(M3D_3D_sample_distances),
      REG_FN(M3D_active_3D_sample_count),
      REG_FN(M3D_3D_open_listener),
      REG_FN(M3D_3D_close_listener),
      REG_FN(M3D_3D_open_object),
      REG_FN(M3D_3D_close_object),
      REG_FN(M3D_set_3D_position),
      REG_FN(M3D_set_3D_velocity),
      REG_FN(M3D_set_3D_velocity_vector),
      REG_FN(M3D_set_3D_orientation),
      REG_FN(M3D_3D_position),
      REG_FN(M3D_3D_velocity),
      REG_FN(M3D_3D_orientation),
      REG_FN(M3D_3D_update_position),
      REG_FN(M3D_3D_auto_update_position),
      };

   static HPROVIDER self;

   switch (fdwReason)
      {
      case DLL_PROCESS_ATTACH:

         DisableThreadLibraryCalls( hinstDll );
         self = RIB_provider_library_handle();

         RIB_register(self,
                     "MSS 3D audio services",
                      M3D);

         RIB_register(self,
                     "MSS 3D sample services",
                      M3DSAMPLE);

         break;

      case DLL_PROCESS_DETACH:

         RIB_unregister_all(self);
         break;
      }

   return TRUE;
}
