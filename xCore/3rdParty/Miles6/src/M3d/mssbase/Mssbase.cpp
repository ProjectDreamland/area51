//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: M3D module for Dolby Surround and 2D providers               ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.01 of 23-Nov-98: Initial                                    ##
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

#define INITGUID

#define diag_printf // AIL_debug_printf

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "mss.h"
#include "imssapi.h"

S32 M3D_started = 0;

C8 M3D_error_text[256];

#ifdef USE_REAR

#define DEFAULT_REAR_ADVANCE 32     // 32 samples

#endif

#define MUTE_AT_MAX_DIST 1      // 0 for compatibility with DS3D drivers
                                // that ignore the DSBCAPS_MUTE3DATMAXDIST flag

#define LN_2 0.69314718F        // = ln(2)

#define SPEED_OF_SOUND 0.355F   // Speed of sound in meters per millisecond
#define DOPPLER_THETA  1.000F   // cos(theta) assuming 0-degree angular displacement
#define PI (3.141592654f)

#define TBLRES 2048
static U8 dot_to_pan[TBLRES+1];

//
// Epsilon value used for FP comparisons with 0
//

#define EPSILON 0.0001F

//
// Additional attributes and preferences
//

enum ATTRIB
{
   //
   // Provider attribs
   //

   MAX_SUPPORTED_SAMPLES,

#ifdef USE_REAR
   REAR_ADVANCE,
#endif

   //
   // Voice attribs for "MSS 3D sample services"
   //

   REVERB_TIME,
   REVERB_PREDELAY_TIME,
   REVERB_DAMPING,
   REVERB_ENVIRONMENT,
   REVERB_EFFECT_VOLUME,
   REVERB_QUALITY
};

//
// Use 32K half-buffers by default
//

#define BUFF_SIZE 32768

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

struct DS3VECTOR3D
{
   F32 x;
   F32 y;
   F32 z;
};

//
// Sample descriptor
//

struct SAMPLE3D
{
   OBJTYPE  type;                // Runtime type of object

   U32      status;              // SMP_ flags: _FREE, _DONE, _PLAYING

   void const FAR *start;        // Sample buffer address (W)
   U32       len  ;              // Sample buffer size in bytes (W)
   U32       pos;                // Index to next byte (R/W)
   U32       done ;              // Nonzero if buffer with len=0 sent by app

   S32      loop_count;          // # of cycles-1 (1=one-shot, 0=indefinite)
   S32      loop_start;          // Starting offset of loop block (0=SOF)
   S32      loop_end;            // End offset of loop block (-1=EOF)

   S32      volume;              // Sample volume 0-127
   S32      playback_rate;       // Playback rate in samples/sec

   S32      bytes_per_sample;    // 1 or 2 for 8- or 16-bit samples

   S32      buffers_past_end;    // # of silent buffers appended

   S32      activated;           // Used to wait for start of playback

   F32      lowpass_value;

   S32      setfilter;

   //
   // Positioning
   //

   DS3VECTOR3D position;         // 3D position
   DS3VECTOR3D face;             // 3D orientation
   DS3VECTOR3D up;               // 3D up-vector
   DS3VECTOR3D velocity;         // 3D velocity
   S32         auto_update;      // TRUE to automatically update in background

   F32         max_dist;         // Sample distances
   F32         min_dist;

   //
   // Provider-specific fields
   //

   S32     is_valid;                   // TRUE if OK to calculate Dolby parms
   S32     doppler_valid;              // TRUE if OK to calculate Doppler shift

   HSAMPLE LR;                         // HSAMPLE representing left-right axis

#ifdef USE_REAR
   HSAMPLE FB;                         // HSAMPLE representing front-back axis
#endif

   U8 FAR *LR_buffer[2];               // Audio buffers for L/R channel
#ifdef USE_REAR
   U8 FAR *FB_buffer[2];               // Audio buffers for F/S channel
#endif
   S32     half_buffer_size;           // Size of each half-buffer

   U32 lastblockdone;                  // estimated time when last mix will be done

   F32 obstruction;
   F32 occlusion;
   F32 baselevel;

   AIL3DSAMPLECB eos;
   H3DSAMPLE clientH3D;
};

//
// Support up to 64 samples (arbitrary limit)
//

#define MAX_SAMPLES 64

static SAMPLE3D samples[MAX_SAMPLES];
static S32 avail_samples = -1;
static S32 n_samples = 16;

//
// Globals
//

static S32 active = 0;

static HDIGDRIVER  dig;

static DS3VECTOR3D listen_position;
static DS3VECTOR3D listen_face;
static DS3VECTOR3D listen_up;
static DS3VECTOR3D listen_cross;
static DS3VECTOR3D listen_velocity;
static S32         listen_auto_update = 0;

static HTIMER      service_timer;
static HTIMER      buffer_timer;

static HPROVIDER   reverb;

static S32         last_room_type=0;

#ifdef USE_REAR
static S32         rear_advance = DEFAULT_REAR_ADVANCE;
#endif

static F32 rolloff_factor = 1.0f;
static F32 doppler_factor = 1.0f;
static F32 distance_factor = 1.0f;

static void API_lock(void)
{
   AIL_lock();
}


static void API_unlock(void)
{
   AIL_unlock();
}


void __inline RAD_vector_cross_product(DS3VECTOR3D *c,
                                       const DS3VECTOR3D *v1,
                                       const DS3VECTOR3D *v2)
{
  c->x = v1->z * v2->y - v1->y * v2->z;
  c->y = v1->x * v2->z - v1->z * v2->x;
  c->z = v1->y * v2->x - v1->x * v2->y;
}


void __inline RAD_vector_subtract(DS3VECTOR3D *d,
                                  const DS3VECTOR3D *v1,
                                  const DS3VECTOR3D *v2)
{
  d->x = v1->x - v2->x;
  d->y = v1->y - v2->y;
  d->z = v1->z - v2->z;
}


void __inline RAD_vector_normalize(DS3VECTOR3D *v)
{
  F32 len = (F32) sqrt((v->x * v->x) +
                       (v->y * v->y) +
                       (v->z * v->z));
  if (len==0.0)
  {
    v->x=1.0;
    v->y=0.0;
    v->z=0.0;
  }
  else
  {
    v->x /= len;
    v->y /= len;
    v->z /= len;
  }
}


F32 __inline RAD_vector_dot_product(const DS3VECTOR3D *v1,
                                    const DS3VECTOR3D *v2)
{
  F32 dot;

  dot  = v1->x * v2->x;
  dot += v1->y * v2->y;
  dot += v1->z * v2->z;

  return( dot );
}

static void checkfilter(SAMPLE3D FAR *S)
{
  if (S->setfilter==0)
  {
    S->setfilter=1;
    AIL_set_sample_processor(S->LR,DP_FILTER,reverb);
#ifdef USE_REAR
    AIL_set_sample_processor(S->FB,DP_FILTER,reverb);
#endif

  }
}

//############################################################################
//##                                                                        ##
//## Calculate pan, volume, and frequency for 2D HSAMPLE pair based on 3D   ##
//## space                                                                  ##
//##                                                                        ##
//############################################################################

static void DOLBY_update_sample(SAMPLE3D FAR *S)
{
   //
   // Return if sample uninitialized
   //

   if (!S->is_valid)
      {
      return;
      }

   //
   // Lock to ensure that changes made to both samples are applied
   // synchronously
   //

   API_lock();

   //
   // Get absolute 3D distance between listener and sample
   //

   F32 dx = S->position.x - listen_position.x;
   F32 dy = S->position.y - listen_position.y;
   F32 dz = S->position.z - listen_position.z;

   F32 distance = (F32) sqrt((dx*dx) + (dy*dy) + (dz*dz));

#if MUTE_AT_MAX_DIST

   //
   // Pause the sound if the distance is greater than the maximum distance
   //

   if (S->status == SMP_PLAYING)
      {
      if (distance > S->max_dist)
         {
         if (AIL_sample_status(S->LR) != SMP_STOPPED)
            {
            AIL_set_sample_volume(S->LR,0);
#ifdef USE_REAR
            AIL_set_sample_volume(S->FB,0);
#endif
            }

         API_unlock();
         return;
         }
      }

#else

   //
   // Inhibit further attenuation at max dist
   //

   if (distance > S->max_dist)
      {
      distance = S->max_dist;
      }

#endif

   //
   // Calculate base volume attenuation
   //

   F32 vol= ((F32)S->volume)
             *(1.0F-S->obstruction);

   if ((S->occlusion<0.50F) || (reverb==0))
     vol=vol*(1.0F-S->occlusion);
   else
     vol=vol*(1.50F-((S->occlusion<0.50F)?0.50F:S->occlusion));

   if (vol<EPSILON)
     vol=0.0;

   F32 volume_atten = -20.0F * (F32) log10( vol / 127.0F);

   //
   // Calculate distance attenuation by inverse-square law
   //

   F32 min            = S->min_dist;
   F32 distance_atten = 0.0F;

   if (min < EPSILON)
      {
      min = EPSILON;
      }

   if (distance < min)
      {
      //
      // Distance is less than the minimum -- no attenuation applied
      //

      distance_atten = 0.0F;
      }
   else
      {
      //
      // The volume of a sound is halved at twice the minimum distance
      // from the listener, halved again at 4X the minimum distance, and
      // so on
      //

      distance_atten = (F32) (log(distance / min) / LN_2) * 6.0F;
      
      distance_atten *= rolloff_factor;
      }

   //
   // Final attenuation = sum of distance and base volume attenuations
   //

   F32 atten = distance_atten + volume_atten;

   S32 scale = ((S32)(127.0F / pow(10.0F, atten / 20.0F)));

   if (scale < 0)   scale = 0;
   if (scale > 127) scale = 127;

   //
   // Determine whether source is in front of, or behind, the listener
   //
   // We plug the listener position and the source position into the
   // plane equation defined by the listener orientation vector, to obtain
   // the relative distances from the origin for the two points.
   //

   F32 listen_fb = (listen_face.x * listen_position.x) +
                   (listen_face.y * listen_position.y) +
                   (listen_face.z * listen_position.z);

   F32 source_fb = (listen_face.x * S->position.x) +
                   (listen_face.y * S->position.y) +
                   (listen_face.z * S->position.z);

#ifdef USE_REAR

   //
   // Apply scale to appropriate axis (LR or FB)
   //

   if (rear_advance)
      {
      //
      // The F-B pan magnitude is multiplied by 4 to get the sound out of the
      // center space more quickly.  Without this factor, the distance
      // attenuation tends to cause the sound to fade away rapidly as soon
      // as it moves off-center.
      //

      F32 mag = (source_fb - listen_fb) / (S->min_dist*128.0F); // JKR - use min for fall off

      S32 lr_vol = scale;
      S32 fb_vol = scale;

      if (mag > 0.0F)
         {
         fb_vol -= (S32)(512.0F * mag);
         }
      else
         {
         lr_vol += (S32)(512.0F * mag);
         }

      if (fb_vol < 0) fb_vol = 0;
      if (lr_vol < 0) lr_vol = 0;

      AIL_set_sample_volume(S->FB,fb_vol);
      AIL_set_sample_volume(S->LR,lr_vol);
      }
   else

#endif
      {
      //
      // If we aren't advancing the rear channel, we cannot allow both
      // front-back and left-right channels to be audible -- one must be
      // muted
      //

#ifdef USE_REAR

      if (source_fb < listen_fb)
         {
         //
         // Source is behind the listener
         //

         AIL_set_sample_volume(S->FB,scale);
         AIL_set_sample_volume(S->LR,0);
         }
      else
         {
         //
         // Source is in front of the listener
         //

         AIL_set_sample_volume(S->FB,0);
         AIL_set_sample_volume(S->LR,scale);
         }

#else

      //
      // In 2D-only mode, back the amplitude off by 3 dB if the sound is to
      // the rear of the listener
      //

      if (source_fb < listen_fb)
         {
         scale = (scale*71) / 100;
         }

      AIL_set_sample_volume(S->LR,scale);
#endif
      }

   //
   // Determine whether source is to the left or right of the listener
   //
   // New version by JKR
   //

   if (distance<=EPSILON)
   {
      AIL_set_sample_pan(S->LR, 64);
   }
   else
   {
     DS3VECTOR3D diff;

     //
     // (face cross up) dot (the diff in position as a unit vector) gives
     //   an angle from 180 (all left) to 0 (all right).  we just scale
     //   the angle to our pan setting.
     //

     RAD_vector_subtract(&diff,&S->position,&listen_position);
     RAD_vector_normalize(&diff);
     F32 dot=RAD_vector_dot_product(&listen_cross,&diff);

     AIL_set_sample_pan(S->LR,dot_to_pan[ (S32)((dot+1.0F)*((F32)TBLRES/2.0F))]);
   }


   //
   // Apply Doppler frequency-shifting if enabled for this sample
   //

   if ((fabs(listen_velocity.x) > EPSILON) ||
       (fabs(listen_velocity.y) > EPSILON) ||
       (fabs(listen_velocity.z) > EPSILON))
      {
      S->doppler_valid = 1;
      }

   if (S->doppler_valid)
      {
      F32 dx = S->velocity.x - listen_velocity.x;
      F32 dy = S->velocity.y - listen_velocity.y;
      F32 dz = S->velocity.z - listen_velocity.z;

      F32 velocity = (F32) sqrt((dx*dx) + (dy*dy) + (dz*dz));

      //
      // Sign of velocity determined by whether object is approaching
      // source or receding from it
      //

      dx = (S->position.x + S->velocity.x) - (listen_position.x + listen_velocity.x);
      dy = (S->position.y + S->velocity.y) - (listen_position.y + listen_velocity.y);
      dz = (S->position.z + S->velocity.z) - (listen_position.z + listen_velocity.z);

      F32 new_distance = (F32) sqrt((dx*dx) + (dy*dy) + (dz*dz));

      if (new_distance < distance)
         {
         //
         // Source and listener approaching each other -- flip sign of
         // velocity to create shift towards higher frequencies
         //

         velocity = -velocity;
         }

      //
      // Apply Doppler shift to playback rate
      //

      F32 f_obs = ((F32) S->playback_rate) *
         (SPEED_OF_SOUND / (SPEED_OF_SOUND + (velocity * distance_factor * doppler_factor * DOPPLER_THETA)));

      AIL_set_sample_playback_rate(S->LR, (S32) f_obs);
#ifdef USE_REAR
      AIL_set_sample_playback_rate(S->FB, (S32) f_obs);
#endif
      }

   //scale from 100 Hz to Nyquist
   if (S->occlusion<=EPSILON)
   {
     if (S->setfilter)
     {
       F32 value=100000.0F;

       if (fabs(value-S->lowpass_value)>EPSILON)
       {
         checkfilter(S);
         AIL_set_filter_sample_preference(S->LR,"Lowpass Cutoff",&value);
#ifdef USE_REAR
         AIL_set_filter_sample_preference(S->FB,"Lowpass Cutoff",&value);
#endif
         S->lowpass_value=value;
       }
     }
   }
   else
   {
     F32 value=(((F32)((AIL_sample_playback_rate(S->LR)/2)-100))*(1.0F-S->occlusion))+100.0F;

     if (fabs(value-S->lowpass_value)>EPSILON)
     {
       checkfilter(S);
       AIL_set_filter_sample_preference(S->LR,"Lowpass Cutoff",&value);
#ifdef USE_REAR
       AIL_set_filter_sample_preference(S->FB,"Lowpass Cutoff",&value);
#endif
       S->lowpass_value=value;
     }
   }

   API_unlock();
}

//##############################################################################
//#																									 #
//# M3D_set_3D_room_type																		 #
//#																									 #
//##############################################################################

static void		  AILEXPORT M3D_set_3D_room_type (S32 EAX_room_type)
{
   S32 i;

   F32 value=(F32)EAX_room_type;
   last_room_type=EAX_room_type;

   for (i=0; i < avail_samples; i++)
   {
     SAMPLE3D FAR *S = &samples[i];

     if (EAX_room_type!=ENVIRONMENT_GENERIC)
       checkfilter(S);

     if (S->setfilter)
     {
       AIL_set_filter_sample_preference(S->LR,"Reverb EAX Environment",&value);
#ifdef USE_REAR
       AIL_set_filter_sample_preference(S->FB,"Reverb EAX Environment",&value);
#endif
       F32 effects_level=0.0F;

       AIL_filter_sample_attribute(S->LR,"Reverb Mix",&effects_level);
       S->baselevel=effects_level;
       if (S->baselevel<=EPSILON)
         S->baselevel=1.0F;
     }
   }
}

//##############################################################################
//#																									 #
//# M3D_3D_room_type																				 #
//#																									 #
//##############################################################################

static S32		 AILEXPORT M3D_3D_room_type (void)
{
	return last_room_type;
}

//##############################################################################
//#																									 #
//# M3D_3D_sample_effects_level																 #
//#																									 #
//##############################################################################

static F32		  AILEXPORT M3D_3D_sample_effects_level (H3DSAMPLE samp)
{
	SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

	if (S->status == SMP_FREE)
		{
		return 0;
		}

   F32 effects_level=0.0F;

   if (S->setfilter)
     AIL_filter_sample_attribute(S->LR,"Reverb Mix",&effects_level);

   effects_level=effects_level/S->baselevel;

   return effects_level;
}


//##############################################################################
//#                                                                            #
//# M3D_set_3D_rolloff_factor                                                  #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_rolloff_factor (F32 factor)
{
  rolloff_factor = factor;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_rolloff_factor                                                      #
//#                                                                            #
//##############################################################################

F32       AILEXPORT M3D_3D_rolloff_factor (void)
{
   return( rolloff_factor );
}


//##############################################################################
//#                                                                            #
//# M3D_set_3D_doppler_factor                                                  #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_doppler_factor (F32 factor)
{
  doppler_factor = factor;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_doppler_factor                                                      #
//#                                                                            #
//##############################################################################

F32       AILEXPORT M3D_3D_doppler_factor (void)
{
   return( doppler_factor );
}


//##############################################################################
//#                                                                            #
//# M3D_set_3D_distance_factor                                                 #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_distance_factor (F32 factor)
{
  distance_factor = factor;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_distance_factor                                                     #
//#                                                                            #
//##############################################################################

F32       AILEXPORT M3D_3D_distance_factor (void)
{
   return( distance_factor );
}


//##############################################################################
//#																									 #
//# M3D_set_3D_EOS                                                             #
//#																									 #
//##############################################################################

static AIL3DSAMPLECB AILEXPORT M3D_set_3D_EOS     (H3DSAMPLE client,
                                            H3DSAMPLE samp,
														  AIL3DSAMPLECB eos)
{
  SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

  AIL3DSAMPLECB prev=S->eos;

  S->eos=eos;

  S->clientH3D=client;

  return prev;
}

//##############################################################################
//#																									 #
//# M3D_set_3D_sample_effects_level															 #
//#																									 #
//##############################################################################

static void		  AILEXPORT M3D_set_3D_sample_effects_level(H3DSAMPLE samp, //)
																	  F32			effects_level)
{
	SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

	if (S->status == SMP_FREE)
		{
		return;
		}

   if (S->setfilter)
   {
     // if negative -1, then force full effects
     if (fabs(effects_level+1.0F)<EPSILON)
       effects_level=S->baselevel;
     else
       effects_level=effects_level*S->baselevel;

     AIL_set_filter_sample_preference(S->LR,"Reverb Mix",&effects_level);
#ifdef USE_REAR
     AIL_set_filter_sample_preference(S->FB,"Reverb Mix",&effects_level);
#endif
   }
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_obstruction                                                  #
//#                                                                            #
//##############################################################################

static F32        AILEXPORT M3D_3D_sample_obstruction (H3DSAMPLE samp)
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

static F32        AILEXPORT M3D_3D_sample_occlusion (H3DSAMPLE samp)
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
//# M3D_set_3D_sample_obstruction                                              #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_set_3D_sample_obstruction(H3DSAMPLE samp, //)
                                                   F32       obstruction)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S->obstruction = obstruction;

   DOLBY_update_sample(S);
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_occlusion                                                #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_set_3D_sample_occlusion(H3DSAMPLE samp, //)
                                                 F32       occlusion)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S->occlusion = occlusion;

   DOLBY_update_sample(S);
}

//############################################################################
//##                                                                        ##
//## Recalculate pan, volume, and frequency for all 2D HSAMPLEs based on    ##
//## listener attributes                                                    ##
//##                                                                        ##
//############################################################################

static void DOLBY_update_listener(void)
{
   S32 i;

   for (i=0; i < avail_samples; i++)
      {
      SAMPLE3D FAR *S = &samples[i];

      if (S->status == SMP_FREE)
         {
         continue;
         }

      DOLBY_update_sample(S);
      }
}

//############################################################################
//##                                                                        ##
//## Copy data from source sample to target secondary buffer                ##
//##                                                                        ##
//## Backfill target secondary buffer with silence to end of source data    ##
//##                                                                        ##
//############################################################################

static S32 DOLBY_stream_to_buffer(SAMPLE3D FAR *S, S32 half)
{
   void FAR *in;
   U32       in_len;
   U32       copy_len;
   U32       out_len;
   U32       amtmixed=0;
   S32       docb=0;

   out_len = S->half_buffer_size;

   void FAR *out_lr;
   out_lr  = S->LR_buffer[half];

#ifdef USE_REAR
   void FAR *out_fb;
   out_fb  = S->FB_buffer[half];
#endif

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

         if (S->buffers_past_end++ > 2*2)
         {
            U32 oldstatus=S->status;

            S->status = SMP_DONE;

            if (oldstatus==SMP_PLAYING)
              docb=1;
         }

         break;
         }

      //
      // Copy block of data directly from source sample to buffer
      //
      // Size of block to copy is determined by smaller amount
      // of available or needed data
      //
      // copy_len = # of destination bytes to write
      // We multiply in_len by 2 because of expansion from mono source
      // to stereo dest
      //

      copy_len = min(out_len, in_len*2);

      //
      // Do Dolby L/R channel (both stereo channels in phase)
      //

      if (S->bytes_per_sample == 1)
         {
#ifdef USE_REAR
         if (!rear_advance)
            {
            U8 FAR *lr  = (U8 FAR *) out_lr;
            U8 FAR *fb  = (U8 FAR *) out_fb;
            U8 FAR *src = (U8 FAR *) in;

            U8 FAR *fb_end = (U8 FAR *) AIL_ptr_add(fb, copy_len);

            while (fb < fb_end)
               {
               lr[0] = lr[1] = *src;
               lr += 2;

               fb[0] = *src;
               fb[1] = (256 - ((*src++) ^ 256)) ^ 256;
               fb += 2;
               }
            }
         else
            {
            U8 FAR *lr  = (U8 FAR *) out_lr;
            U8 FAR *fb  = (U8 FAR *) out_fb;
            U8 FAR *src = (U8 FAR *) in;

            U8 FAR *lr_end = (U8 FAR *) AIL_ptr_add(lr, copy_len);

            while (lr < lr_end)
               {
               lr[0] = lr[1] = *src++;
               lr += 2;
               }

            src = (U8 FAR *) AIL_ptr_add(in, rear_advance * 2);
            U8 FAR *fb_end = (U8 FAR *) AIL_ptr_add(fb, copy_len);

            S32 pad = AIL_ptr_dif(AIL_ptr_add(src, (copy_len / 2)),
                                  AIL_ptr_add(S->start, S->len));

            if (pad > 0)
               {
               fb_end = (U8 FAR *) AIL_ptr_add(fb_end, -(2 * pad));
               }

            while (fb < fb_end)
               {
               fb[0] = *src;
               fb[1] = (256 - ((*src++) ^ 256)) ^ 256; 
               fb += 2;
               }

            while (pad > 0)
               {
               fb[0] = fb[1] = 128;
               fb += 2;
               pad -= 2;
               }
            }
#else
         U8 FAR *lr  = (U8 FAR *) out_lr;
         U8 FAR *src = (U8 FAR *) in;

         U8 FAR *lr_end = (U8 FAR *) AIL_ptr_add(lr, copy_len);

         while (lr < lr_end)
            {
            lr[0] = lr[1] = *src++;
            lr += 2;
            }
#endif
         }
      else
         {
#ifdef USE_REAR
         if (!rear_advance)
            {
            U16 FAR *lr  = (U16 FAR *) out_lr;
            U16 FAR *fb  = (U16 FAR *) out_fb;
            U16 FAR *src = (U16 FAR *) in;

            U16 FAR *fb_end = (U16 FAR *) AIL_ptr_add(fb, copy_len);

            while (fb < fb_end)
               {
               lr[0] = lr[1] = *src;
               lr += 2;

               fb[0] = *src;
               fb[1] = 65535L - (U16)LE_SWAP16(src);
               MEM_LE_SWAP16(fb+1);
               ++src;
               fb += 2;
               }
            }
         else
            {
            U16 FAR *lr  = (U16 FAR *) out_lr;
            U16 FAR *fb  = (U16 FAR *) out_fb;
            U16 FAR *src = (U16 FAR *) in;

            U16 FAR *lr_end = (U16 FAR *) AIL_ptr_add(lr, copy_len);

            while (lr < lr_end)
               {
               lr[0] = lr[1] = *src++;
               lr += 2;
               }

            src = (U16 FAR *) AIL_ptr_add(in, rear_advance * 2);
            U16 FAR *fb_end = (U16 FAR *) AIL_ptr_add(fb, copy_len);

            S32 pad = AIL_ptr_dif(AIL_ptr_add(src, (copy_len / 2)),
                                  AIL_ptr_add(S->start, S->len));

            if (pad > 0)
               {
               fb_end = (U16 FAR *) AIL_ptr_add(fb_end, -(2 * pad));
               }

            while (fb < fb_end)
               {
               fb[0] = *src;
               fb[1] = 65535L - (U16)LE_SWAP16(src);
               MEM_LE_SWAP16(fb+1);
               ++src;
               fb += 2;
               }

            while (pad > 0)
               {
               fb[0] = fb[1] = 0;
               fb += 2;
               pad -= 2;
               }
            }
#else
         U16 FAR *lr  = (U16 FAR *) out_lr;
         U16 FAR *src = (U16 FAR *) in;

         U16 FAR *lr_end = (U16 FAR *) AIL_ptr_add(lr, copy_len);

         while (lr < lr_end)
            {
            lr[0] = lr[1] = *src++;
            lr += 2;
            }
#endif
         }

      //
      // Update source sample position index
      //

      S->pos += (copy_len / 2);

      amtmixed += (copy_len / 2);

      out_len -= copy_len;
      out_lr   = (U8 *) out_lr + copy_len;
#ifdef USE_REAR
      out_fb   = (U8 *) out_fb + copy_len;
#endif
      }

   //
   // If insufficient source data was available to fill output buffer,
   // flush remainder of buffer with silence
   //

   if (out_len > 0)
      {
      memset(out_lr,
            (S->bytes_per_sample == 2) ? 0 : 0x80808080,
             out_len);

#ifdef USE_REAR
      memset(out_fb,
            (S->bytes_per_sample == 2) ? 0 : 0x80808080,
             out_len);
#endif

      out_len = 0;
      }


   // setup to monitor the "SMP_DONE" time
   if (amtmixed) 
   {
     U32 timer=AIL_ms_count();

     // if not enough time has past for the last block assume this block will start after it
     if (S->lastblockdone>timer)
       timer=S->lastblockdone;

     S->lastblockdone=timer+((amtmixed*1000)/(S->playback_rate*S->bytes_per_sample));
   }

   return(docb);
}

//############################################################################
//#                                                                          #
//# Return floating-point type as unsigned long DWORD (without actually      #
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
//# Return signed long DWORD as single-precision float (without actually     #
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
//# Destroy system-level emitter object(s) associated with sample            #
//#                                                                          #
//############################################################################

static void destroy_sample_emitter(SAMPLE3D FAR *S)
{
   API_lock();

   if (S->LR_buffer[0] != NULL)
      {
      AIL_mem_free_lock((void FAR *) S->LR_buffer[0]);
      S->LR_buffer[0] = NULL;
      }

#ifdef USE_REAR
   if (S->FB_buffer[0] != NULL)
      {
      AIL_mem_free_lock((void FAR *) S->FB_buffer[0]);
      S->FB_buffer[0] = NULL;
      }
#endif

   if (S->LR_buffer[1] != NULL)
      {
      AIL_mem_free_lock((void FAR *) S->LR_buffer[1]);
      S->LR_buffer[1] = NULL;
      }

#ifdef USE_REAR
   if (S->FB_buffer[1] != NULL)
      {
      AIL_mem_free_lock((void FAR *) S->FB_buffer[1]);
      S->FB_buffer[1] = NULL;
      }
#endif

   if (S->LR != NULL)
      {
      AIL_release_sample_handle(S->LR);
      }

#ifdef USE_REAR
   if (S->FB != NULL)
      {
      AIL_release_sample_handle(S->FB);
      }
#endif

   API_unlock();
}

//############################################################################
//#                                                                          #
//# Initialize sample                                                        #
//#                                                                          #
//############################################################################

static void init_sample(SAMPLE3D FAR *S)
{
   S->type            =  IS_SAMPLE;

   S->is_valid        = 0;
   S->doppler_valid   = 0;

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

   if (last_room_type!=ENVIRONMENT_GENERIC)
     checkfilter(S);

   if (S->setfilter)
   {
     F32 re=(F32)last_room_type;
     AIL_set_filter_sample_preference(S->LR,"Reverb EAX Environment",&re);
#ifdef USE_REAR
     AIL_set_filter_sample_preference(S->FB,"Reverb EAX Environment",&re);
#endif
   }
}


// setup the low level HSAMPLE
static void init_HSAMPLE(SAMPLE3D FAR *S)
{
   //
   // Set HSAMPLE properties
   //

   F32 rt,rp,rm,rd,re;
   if (S->setfilter)
   {
     AIL_filter_sample_attribute(S->LR,"Reverb Time",&rt);
     AIL_filter_sample_attribute(S->LR,"Reverb PreDelay",&rp);
     AIL_filter_sample_attribute(S->LR,"Reverb Mix",&rm);
     AIL_filter_sample_attribute(S->LR,"Reverb Damping",&rd);
     AIL_filter_sample_attribute(S->LR,"Reverb EAX Environment",&re);
   }

   S->lowpass_value=100000.0F;

   AIL_init_sample(S->LR);
#ifdef USE_REAR
   AIL_init_sample(S->FB);
#endif

   S32 word_mask   = (S->bytes_per_sample == 2) ? DIG_F_16BITS_MASK : 0;
   S32 sample_type = (S->bytes_per_sample == 2) ? DIG_PCM_SIGN      : 0;

   AIL_set_sample_type(S->LR,
                       DIG_F_STEREO_MASK | word_mask,
                       sample_type);

#ifdef USE_REAR
   AIL_set_sample_type(S->FB,
                       DIG_F_STEREO_MASK | word_mask,
                       sample_type);
#endif

   AIL_set_sample_playback_rate(S->LR, S->playback_rate);

#ifdef USE_REAR
   AIL_set_sample_playback_rate(S->FB, S->playback_rate);
#endif

   if (S->setfilter)
   {
     S->setfilter=0;  // reset setfilter flag because we AIL_init_sampled above
     checkfilter(S);

     AIL_set_filter_sample_preference(S->LR,"Reverb EAX Environment",&re);
     AIL_set_filter_sample_preference(S->LR,"Reverb Time",&rt);
     AIL_set_filter_sample_preference(S->LR,"Reverb PreDelay",&rp);
     AIL_set_filter_sample_preference(S->LR,"Reverb Mix",&rm);
     AIL_set_filter_sample_preference(S->LR,"Reverb Damping",&rd);

#ifdef USE_REAR
     AIL_set_filter_sample_preference(S->FB,"Reverb EAX Environment",&re);
     AIL_set_filter_sample_preference(S->FB,"Reverb Time",&rt);
     AIL_set_filter_sample_preference(S->FB,"Reverb PreDelay",&rp);
     AIL_set_filter_sample_preference(S->FB,"Reverb Mix",&rm);
     AIL_set_filter_sample_preference(S->FB,"Reverb Damping",&rd);
#endif
   }

}

//############################################################################
//#                                                                          #
//# Terminate playing sample immediately, aborting all pending events        #
//#                                                                          #
//# Do NOT call from within background thread -- will cause deadlocks due    #
//# to sleep_sample() call!                                                  #
//#                                                                          #
//############################################################################

static void reset_sample_voice(SAMPLE3D FAR *S)
{
   API_lock();

   S->buffers_past_end=0;

   S->status = SMP_DONE;

   AIL_stop_sample(S->LR);
#ifdef USE_REAR
   AIL_stop_sample(S->FB);
#endif

   API_unlock();

   init_HSAMPLE(S);
}

//############################################################################
//#                                                                          #
//# Buffer-service function, called from AIL timer with foreground thread    #
//# halted                                                                   #
//#                                                                          #
//############################################################################

static void AILCALLBACK notify_timer(U32 user)
{
   S32 i;

   for (i=0; i < avail_samples; i++)
   {
      SAMPLE3D FAR *S = &samples[i];

      if ((S->status&255) != SMP_PLAYING)
         {
         continue;
         }

      //
      // See if R/L buffer needs servicing
      //

      S32 lr = AIL_sample_buffer_ready(S->LR);
      
      if (lr == -1)
         {
         continue;
         }

#ifdef USE_REAR

      S32 fb = AIL_sample_buffer_ready(S->FB);

      if (fb == -1)
         {
         continue;
         }

#endif

      //
      // Refresh requested half-buffer in both F/B and L/R channels
      //
      // F/B channel = steered 100% to the surround (rear) channel by
      //               out-of-phase relationship between stereo channels
      //
      //               May be attenuated for volume
      //
      // L/R channel = steered 100% to center channel by in-phase
      //               relationship between stereo channels
      //
      //               May be panned for L-R steering
      //

      S32 docb=DOLBY_stream_to_buffer(S, lr);

#ifdef USE_REAR
      AIL_load_sample_buffer(S->FB,
                             fb,
                             S->FB_buffer[fb],
                             S->half_buffer_size);
#endif

      AIL_load_sample_buffer(S->LR,
                             lr,
                             S->LR_buffer[lr],
                             S->half_buffer_size);

      if (S->eos)
      {
        if (docb)
          S->eos(S->clientH3D);
        else
        {
          if ((S->status==SMP_PLAYING) && (S->pos) && (S->pos == S->len))
            if (AIL_ms_count()>=S->lastblockdone)
            {
              S->status=SMP_PLAYING|256;
              S->eos(S->clientH3D);
            }
        }
      }

   }
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
#ifdef USE_REAR
      case PROVIDER_NAME:     return (U32) "Dolby Surround";
      case REAR_ADVANCE:      return rear_advance;
#else
      case PROVIDER_NAME:     return (U32) "Miles Fast 2D Positional Audio";
#endif
      case PROVIDER_VERSION:  return 0x101;

      case MAX_SUPPORTED_SAMPLES: return( (avail_samples!=-1)?avail_samples:n_samples );
      }

   return 0;
}

//############################################################################
//#                                                                          #
//# Return M3D error text                                                    #
//#                                                                          #
//############################################################################

static C8 FAR *       AILEXPORT M3D_error       (void)
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

static M3DRESULT AILEXPORT M3D_startup     (void)
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

static M3DRESULT      AILEXPORT M3D_shutdown    (void)
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

static S32 AILEXPORT M3D_set_provider_preference (HATTRIB    preference, //)
                                           void FAR*  value)
{
   S32 prev = -1;

   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //

      case MAX_SUPPORTED_SAMPLES:
         n_samples=*(U32*)value;
         if (n_samples>MAX_SAMPLES)
           n_samples=MAX_SAMPLES;
         break;

#ifdef USE_REAR
      case REAR_ADVANCE:

         prev = rear_advance;
         rear_advance = *(S32 FAR*)value;
         break;
#endif
      }

   return prev;
}

//##############################################################################
//#                                                                            #
//# M3D_allocate_3D_sample_handle                                              #
//#                                                                            #
//##############################################################################

static H3DSAMPLE  AILEXPORT M3D_allocate_3D_sample_handle (void)
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

   return (H3DSAMPLE) S;
}

//##############################################################################
//#                                                                            #
//# M3D_release_3D_sample_handle                                               #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_release_3D_sample_handle (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S->eos=0;

   reset_sample_voice(S);

   S->lowpass_value=100000.0F;
   
   AIL_init_sample(S->LR);
#ifdef USE_REAR
   AIL_init_sample(S->FB);
#endif
   S->setfilter=0;

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

static void       AILEXPORT M3D_start_3D_sample         (H3DSAMPLE samp)
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

   DOLBY_update_sample(S);

   S->status = SMP_PLAYING;
}

//##############################################################################
//#                                                                            #
//# M3D_stop_3D_sample                                                         #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_stop_3D_sample          (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status != SMP_PLAYING)
      {
      return;
      }

   S->status = SMP_STOPPED;

   API_lock();

   AIL_stop_sample(S->LR);
#ifdef USE_REAR
   AIL_stop_sample(S->FB);
#endif

   API_unlock();
}

//##############################################################################
//#                                                                            #
//# M3D_resume_3D_sample                                                       #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_resume_3D_sample        (H3DSAMPLE samp)
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

   S->status = SMP_PLAYING;

   API_lock();

   AIL_resume_sample(S->LR);
#ifdef USE_REAR
   AIL_resume_sample(S->FB);
#endif

   API_unlock();
}

//##############################################################################
//#                                                                            #
//# M3D_end_3D_sample                                                          #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_end_3D_sample        (H3DSAMPLE samp)
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

static void       AILEXPORT M3D_set_3D_sample_volume    (H3DSAMPLE samp, //)
                                                  S32       volume)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S->volume = volume;

   DOLBY_update_sample(S);
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_playback_rate                                            #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_set_3D_sample_playback_rate    (H3DSAMPLE samp, //)
                                                         S32       playback_rate)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return;
      }

   S->playback_rate = playback_rate;

   API_lock();

   AIL_set_sample_playback_rate(S->LR, playback_rate);
#ifdef USE_REAR
   AIL_set_sample_playback_rate(S->FB, playback_rate);
#endif

   API_unlock();

   DOLBY_update_sample(S);
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_sample_offset                                                   #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_set_3D_sample_offset    (H3DSAMPLE samp, //)
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

static void       AILEXPORT M3D_set_3D_sample_loop_count(H3DSAMPLE samp, //)
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

static void       AILEXPORT M3D_set_3D_sample_loop_block(H3DSAMPLE samp, //)
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
//# M3D_3D_sample_volume                                                       #
//#                                                                            #
//##############################################################################

static S32        AILEXPORT M3D_3D_sample_volume        (H3DSAMPLE samp)
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

static S32        AILEXPORT M3D_3D_sample_playback_rate        (H3DSAMPLE samp)
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

static U32        AILEXPORT M3D_3D_sample_status        (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (((S->status&255)==SMP_PLAYING) && (S->pos) && (S->pos == S->len))
     if (AIL_ms_count()>=S->lastblockdone)
       return SMP_DONE;

   return S->status;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_offset                                                       #
//#                                                                            #
//##############################################################################

static U32        AILEXPORT M3D_3D_sample_offset        (H3DSAMPLE     samp)
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

static U32        AILEXPORT M3D_3D_sample_length        (H3DSAMPLE     samp)
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

static U32        AILEXPORT M3D_3D_sample_loop_count    (H3DSAMPLE samp)
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
//# M3D_set_3D_sample_distances                                                #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_set_3D_sample_distances (H3DSAMPLE samp, //)
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

   DOLBY_update_sample(S);
}

//##############################################################################
//#                                                                            #
//# M3D_3D_sample_distances                                                    #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_3D_sample_distances     (H3DSAMPLE samp, //)
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

static U32 AILEXPORT M3D_3D_sample_query_attribute (H3DSAMPLE samp, //)
                                             HATTRIB   index)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   F32 value=-1.0F;

   switch ((ATTRIB) index)
      {
      case REVERB_TIME:
        if (!S->setfilter)
          value=0.0F;
        else
          AIL_filter_sample_attribute(S->LR,"Reverb Time",&value);
        return (U32) float_as_long(value);

      case REVERB_PREDELAY_TIME:
        if (!S->setfilter)
          value=0.0F;
        else
          AIL_filter_sample_attribute(S->LR,"Reverb PreDelay",&value);
        return (U32) float_as_long(value);

      case REVERB_EFFECT_VOLUME:
        if (!S->setfilter)
          value=0.0F;
        else
          AIL_filter_sample_attribute(S->LR,"Reverb Mix",&value);
        return (U32) float_as_long(value);

      case REVERB_DAMPING:
        if (!S->setfilter)
          value=0.0F;
        else
          AIL_filter_sample_attribute(S->LR,"Reverb Damping",&value);
        return (U32) float_as_long(value);

      case REVERB_ENVIRONMENT:
        if (!S->setfilter)
          return(0);
        AIL_filter_sample_attribute(S->LR,"Reverb EAX Environment",&value);
        return (U32)value;

      case REVERB_QUALITY:
        if (!S->setfilter)
          return(0);
        AIL_filter_sample_attribute(S->LR,"Reverb Quality",&value);
        return( (value>EPSILON)?1:0 );
      }

   return 0;
}

//############################################################################
//#                                                                          #
//# Set provider preference value, returning previous setting                #
//#                                                                          #
//############################################################################

static S32 AILEXPORT M3D_3D_set_sample_preference (H3DSAMPLE  samp, //)
                                            HATTRIB    preference,
                                            void FAR*  value)
{
   S32 prev = -1;

   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return -1;
      }

   F32 prevf=1.0F;

   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //

      case REVERB_TIME:
         
         checkfilter(S);
         AIL_filter_sample_attribute(S->LR,"Reverb Time",&prevf);
         prev = (S32) float_as_long(prevf);
         AIL_set_filter_sample_preference(S->LR,"Reverb Time",(F32 FAR*)value);
#ifdef USE_REAR
         AIL_set_filter_sample_preference(S->FB,"Reverb Time",(F32 FAR*)value);
#endif
         break;

      case REVERB_PREDELAY_TIME:

         checkfilter(S);
         AIL_filter_sample_attribute(S->LR,"Reverb PreDelay",&prevf);
         prev = (S32) float_as_long(prevf);
         AIL_set_filter_sample_preference(S->LR,"Reverb PreDelay",(F32 FAR*)value);
#ifdef USE_REAR
         AIL_set_filter_sample_preference(S->FB,"Reverb PreDelay",(F32 FAR*)value);
#endif
         break;

      case REVERB_DAMPING:

         checkfilter(S);
         AIL_filter_sample_attribute(S->LR,"Reverb Damping",&prevf);
         prev = (S32) float_as_long(prevf);
         AIL_set_filter_sample_preference(S->LR,"Reverb Damping",(F32 FAR*)value);
#ifdef USE_REAR
         AIL_set_filter_sample_preference(S->FB,"Reverb Damping",(F32 FAR*)value);
#endif
         break;

      case REVERB_EFFECT_VOLUME:

         if ((*(F32 FAR*)value)>EPSILON)
           checkfilter(S);

         if (S->setfilter)
         {
           AIL_filter_sample_attribute(S->LR,"Reverb Mix",&prevf);
           prev = (S32) float_as_long(prevf);
           AIL_set_filter_sample_preference(S->LR,"Reverb Mix",(F32 FAR*)value);
#ifdef USE_REAR
           AIL_set_filter_sample_preference(S->FB,"Reverb Mix",(F32 FAR*)value);
#endif
         }
         else
         {
           prev=(S32) float_as_long(0.0F);
         }
         break;

      case REVERB_ENVIRONMENT:

         if ((*(S32 FAR*)value)!=ENVIRONMENT_GENERIC)
           checkfilter(S);

         if (S->setfilter)
         {
           F32 p,v=(F32)*(S32 FAR*)value;
           AIL_filter_sample_attribute(S->LR,"Reverb EAX Environment",&p);
           AIL_set_filter_sample_preference(S->LR,"Reverb EAX Environment",&v);
#ifdef USE_REAR
           AIL_set_filter_sample_preference(S->FB,"Reverb EAX Environment",&v);
#endif
           prev=(S32)p;
         }
         else
         {
           prev=0;
         }
         break;

      case REVERB_QUALITY:

         if ((*(S32 FAR*)value)!=0)
           checkfilter(S);

         if (S->setfilter)
         {
           F32 p,v=(F32)*(S32 FAR*)value;
           AIL_filter_sample_attribute(S->LR,"Reverb Quality",&p);
           AIL_set_filter_sample_preference(S->LR,"Reverb Quality",&v);
#ifdef USE_REAR
           AIL_set_filter_sample_preference(S->FB,"Reverb Quality",&v);
#endif
           prev=(S32)p;
         }
         else
         {
           prev=0;
         }
         break;

      }

   return prev;
}


//##############################################################################
//#                                                                            #
//# M3D_active_3D_sample_count                                                 #
//#                                                                            #
//##############################################################################

static S32      AILEXPORT M3D_active_3D_sample_count   (void)
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

static H3DPOBJECT AILEXPORT M3D_3D_open_listener        (void)
{
   static OBJTYPE listener = IS_LISTENER;

   return (H3DPOBJECT) &listener;
}

//##############################################################################
//#                                                                            #
//# M3D_3D_close_listener                                                      #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_3D_close_listener       (H3DPOBJECT listener)
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

static H3DPOBJECT AILEXPORT M3D_3D_open_object          (void)
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

static void       AILEXPORT M3D_3D_close_object         (H3DPOBJECT obj)
{
   //
   // Not supported
   //
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_position                                                        #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_set_3D_position         (H3DPOBJECT obj, //)
                                                  F32     X,
                                                  F32     Y,
                                                  F32     Z)
{
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;

   if (t == NULL)
      {
      return;
      }

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      S->position.x = X;
      S->position.y = Y;
      S->position.z = Z;

      DOLBY_update_sample(S);
      }
   else if (*t == IS_LISTENER)
      {
      listen_position.x = X;
      listen_position.y = Y;
      listen_position.z = Z;

      DOLBY_update_listener();
      }
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_velocity_vector                                                 #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_set_3D_velocity_vector  (H3DPOBJECT obj, //)
                                                  F32     dX_per_ms,
                                                  F32     dY_per_ms,
                                                  F32     dZ_per_ms)
{
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;

   if (t == NULL)
      {
      return;
      }

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      S->velocity.x = dX_per_ms;
      S->velocity.y = dY_per_ms;
      S->velocity.z = dZ_per_ms;

      //
      // Enable Doppler calculations if a non-zero velocity value is set
      //

      if ((fabs(S->velocity.x) > EPSILON) ||
          (fabs(S->velocity.y) > EPSILON) ||
          (fabs(S->velocity.z) > EPSILON))
         {
         S->doppler_valid = 1;
         }

      DOLBY_update_sample(S);
      }
   else if (*t == IS_LISTENER)
      {
      listen_velocity.x = dX_per_ms;
      listen_velocity.y = dY_per_ms;
      listen_velocity.z = dZ_per_ms;

      DOLBY_update_listener();
      }
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_velocity                                                        #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_set_3D_velocity         (H3DPOBJECT obj, //)
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

static void       AILEXPORT M3D_set_3D_orientation      (H3DPOBJECT obj, //)
                                                  F32     X_face,
                                                  F32     Y_face,
                                                  F32     Z_face,
                                                  F32     X_up,
                                                  F32     Y_up,
                                                  F32     Z_up)
{
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;

   DS3VECTOR3D f,u;

   if (t == NULL)
      {
      return;
      }

   //
   // copy the vectors
   //

   f.x=X_face;
   f.y=Y_face;
   f.z=Z_face;

   u.x=X_up;
   u.y=Y_up;
   u.z=Z_up;

   //
   // Assign orientation to specified object
   //

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      S->face = f;

      S->up = u;
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

      listen_face = f;

      listen_up = u;

      RAD_vector_cross_product(&listen_cross, &listen_face, &listen_up);

      DOLBY_update_listener();
      }
}

//##############################################################################
//#                                                                            #
//# M3D_3D_position                                                            #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_3D_position             (H3DPOBJECT  obj, //)
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

      if (X) *X = S->position.x;
      if (Y) *Y = S->position.y;
      if (Z) *Z = S->position.z;
      }
   else if (*t == IS_LISTENER)
      {
      if (X) *X = listen_position.x;
      if (Y) *Y = listen_position.y;
      if (Z) *Z = listen_position.z;
      }
}

//##############################################################################
//#                                                                            #
//# M3D_3D_velocity                                                            #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_3D_velocity             (H3DPOBJECT  obj, //)
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

      if (dX_per_ms) *dX_per_ms = S->velocity.x;
      if (dY_per_ms) *dY_per_ms = S->velocity.y;
      if (dZ_per_ms) *dZ_per_ms = S->velocity.z;
      }
   else if (*t == IS_LISTENER)
      {
      if (dX_per_ms) *dX_per_ms = listen_velocity.x;
      if (dY_per_ms) *dY_per_ms = listen_velocity.y;
      if (dZ_per_ms) *dZ_per_ms = listen_velocity.z;
      }
}

//##############################################################################
//#                                                                            #
//# M3D_3D_orientation                                                         #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_3D_orientation          (H3DPOBJECT  obj, //)
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

      if (X_face) *X_face = S->face.x;
      if (Y_face) *Y_face = S->face.y;
      if (Z_face) *Z_face = S->face.z;
      if (X_up)   *X_up   = S->up.x;
      if (Y_up)   *Y_up   = S->up.y;
      if (Z_up)   *Z_up   = S->up.z;
      }
   else if (*t == IS_LISTENER)
      {
      if (X_face) *X_face = listen_face.x;
      if (Y_face) *Y_face = listen_face.y;
      if (Z_face) *Z_face = listen_face.z;
      if (X_up)   *X_up   = listen_up.x;
      if (X_up)   *Y_up   = listen_up.y;
      if (X_up)   *Z_up   = listen_up.z;
      }
}

//##############################################################################
//#                                                                            #
//# M3D_3D_update_position                                                     #
//#                                                                            #
//##############################################################################

static void       AILEXPORT M3D_3D_update_position      (H3DPOBJECT obj, //)
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

static void       AILEXPORT M3D_3D_auto_update_position (H3DPOBJECT obj, //)
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

static S32        AILEXPORT M3D_set_3D_sample_data      (H3DSAMPLE         samp, //)
                                                  AILSOUNDINFO FAR *info)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

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

   //
   // End and re-initialize samples assigned to this voice
   //

   AIL_stop_sample(S->LR);
#ifdef USE_REAR
   AIL_stop_sample(S->FB);
#endif

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
   S->playback_rate =  info->rate;

   S->status        =  SMP_DONE;

   S->eos=0;

   init_HSAMPLE(S);

   //
   // Set default emitter distances
   //

   M3D_set_3D_sample_distances(samp,
                               200.0F,
                               1.0F);
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

   S->baselevel=1.0F;
   M3D_set_3D_sample_effects_level(samp, 0.0F);

   M3D_set_3D_sample_obstruction  (samp, 0.0F);
   M3D_set_3D_sample_occlusion    (samp, 0.0F);

   //
   // Mark sample valid and force calculation of initial conditions
   //

   S->is_valid = 1;

   DOLBY_update_sample(S);

   return 1;
}

//############################################################################
//#                                                                          #
//# AIL timer callback for automatic position updates                        #
//#                                                                          #
//############################################################################

static void AILCALLBACK M3D_serve(U32 user)
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

//##############################################################################
//#                                                                            #
//# M3D_activate                                                               #
//#                                                                            #
//##############################################################################

static M3DRESULT AILEXPORT M3D_activate                 (S32 enable)
{
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
      // Get handle to primary driver (by default, first driver allocated)
      //
      // If this is NULL, it means the digital subsystem hasn't been
      // initialized yet, or the obsolete MSSDS subsystem is in use
      //

      dig = AIL_primary_digital_driver(NULL);

      if (dig == NULL)
         {
         AIL_set_error("2D subsystem invalid or uninitialized");
         return M3D_NOT_INIT;
         }

      //
      // Allocate as many sample handle pairs as possible, up to N_SAMPLES
      // pairs
      //

      HSAMPLE L[MAX_SAMPLES];
#ifdef USE_REAR
      HSAMPLE F[MAX_SAMPLES];
#endif

      avail_samples = 0;

      S32 i;   

      for (i=0; i < n_samples; i++)
         {
         L[i] = AIL_allocate_sample_handle(dig);

         if (L[i] == NULL)
            {
            break;
            }

#ifdef USE_REAR
         F[i] = AIL_allocate_sample_handle(dig);

         if (F[i] == NULL)
            {
            AIL_release_sample_handle(L[i]);
            break;
            }
#endif

         ++avail_samples;
         }

      //
      // Set default rear advance in samples
      //

#ifdef USE_REAR
      rear_advance = DEFAULT_REAR_ADVANCE;
#endif

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
      // Initialize samples 
      //

      for (i=0; i < avail_samples; i++)
         {
         SAMPLE3D FAR *S = &samples[i];

         memset(S,
                0,
                sizeof(struct SAMPLE3D));

         S->status = SMP_FREE;

         //
         // Assign 2D sample handles
         //

         S->LR = L[i];
#ifdef USE_REAR
         S->FB = F[i];
#endif

         //
         // Allocate buffers
         //

         S->half_buffer_size = BUFF_SIZE;

         for (S32 i=0; i < 2; i++)
            {
            S->LR_buffer[i] = (U8 FAR *)
               AIL_mem_alloc_lock(S->half_buffer_size);

#ifdef USE_REAR
            S->FB_buffer[i] = (U8 FAR *)
               AIL_mem_alloc_lock(S->half_buffer_size);
#endif

            if ((S->LR_buffer[i] == NULL)
#ifdef USE_REAR
                ||
                (S->FB_buffer[i] == NULL)
#endif
               )
               {
               AIL_set_error("Could not allocate sample streaming buffers");
               return M3D_OUT_OF_MEM;
               }
            }
         }

      HPROENUM next=HPROENUM_FIRST;
      HPROVIDER dest;
      char* str;

      reverb=0;

      while ((AIL_enumerate_filters(&next, &dest,&str)) && (reverb==0))
      {
        if (AIL_strcmp(str,"Reverb3 Filter")==0)
          reverb=dest;
      }

		M3D_set_3D_room_type( ENVIRONMENT_GENERIC );

      //
      // Register and start service timer
      //

      service_timer = AIL_register_timer(M3D_serve);

      AIL_set_timer_period(service_timer, SERVICE_MSECS * 1000);

      AIL_start_timer(service_timer);

      //
      // Register and start buffer-notification timer at 30 Hz
      //

      buffer_timer = AIL_register_timer(notify_timer);

      AIL_set_timer_frequency(buffer_timer, 30);

      AIL_start_timer(buffer_timer);

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

      //
      // Deallocate resources associated with samples
      //

      S32 i;

      for (i=0; i < avail_samples; i++)
         {
         SAMPLE3D FAR *S = &samples[i];

         //
         // Mark handle free
         //

         S->status = SMP_FREE;

         //
         // Destroy emitter associated with sample
         //

         destroy_sample_emitter(S);
         }

      active = 0;
      }

   return M3D_NOERR;
}


static void dotables()
{
  S32 i;
  
  for(i=0;i<=TBLRES;i++)
  {
    S32 pan=127-(S32)ceil((acos(  ( ((F32)i)/((F32)TBLRES/2.0F))-1.0F)  *127.0F)/PI);
    if (pan>127) pan=127; else if (pan<0) pan=0;

    dot_to_pan[ i ] = (U8)pan;
  }
}

//
// Application must supply the provider handle explicitly
//

extern "C" S32 MSS_RIB_Main( HPROVIDER provider_handle, U32 up_down );
extern "C" S32 MSS_RIB_Main( HPROVIDER provider_handle, U32 up_down )
{
   const RIB_INTERFACE_ENTRY M3DSAMPLE[] =
      {
      REG_AT("Reverb time",               REVERB_TIME,            RIB_FLOAT),
      REG_AT("Predelay time",             REVERB_PREDELAY_TIME,   RIB_FLOAT),
      REG_AT("Damping",                   REVERB_DAMPING,         RIB_FLOAT),
      REG_AT("Effect volume",             REVERB_EFFECT_VOLUME,   RIB_FLOAT),
      REG_AT("Environment selection",     REVERB_ENVIRONMENT,     RIB_DEC),

      REG_AT("Reverb Quality",            REVERB_QUALITY,         RIB_DEC),
      REG_PR("Reverb Quality",            REVERB_QUALITY,         RIB_DEC),

      REG_PR("Reverb time",               REVERB_TIME,            RIB_FLOAT),
      REG_PR("Predelay time",             REVERB_PREDELAY_TIME,   RIB_FLOAT),
      REG_PR("Damping",                   REVERB_DAMPING,         RIB_FLOAT),
      REG_PR("Effect volume",             REVERB_EFFECT_VOLUME,   RIB_FLOAT),
      REG_PR("Environment selection",     REVERB_ENVIRONMENT,     RIB_DEC),

      REG_FN(M3D_set_3D_sample_obstruction),
      REG_FN(M3D_3D_sample_obstruction),
      REG_FN(M3D_set_3D_sample_occlusion),
      REG_FN(M3D_3D_sample_occlusion),

      REG_FN(M3D_set_3D_EOS),
      REG_FN(M3D_set_3D_sample_effects_level),
  		REG_FN(M3D_3D_sample_effects_level),
	  	REG_FN(M3D_set_3D_room_type),
  		REG_FN(M3D_3D_room_type),

      REG_FN(M3D_3D_sample_query_attribute),
      REG_FN(M3D_3D_set_sample_preference),

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
      REG_PR("Maximum supported samples",  MAX_SUPPORTED_SAMPLES, RIB_DEC),

#ifdef USE_REAR
      REG_AT("Rear advance samples",       REAR_ADVANCE,          RIB_DEC),
      REG_PR("Rear advance samples",       REAR_ADVANCE,          RIB_DEC),
#endif

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
      REG_FN(M3D_3D_auto_update_position)
      };

   static HPROVIDER self;

   if (up_down)
      {
         dotables();

#ifdef IS_WINDOWS
         self = RIB_provider_library_handle();
#else
         self = provider_handle;
#endif

         RIB_register(self,
                     "MSS 3D audio services",
                      M3D);

         RIB_register(self,
                     "MSS 3D sample services",
                      M3DSAMPLE);
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
