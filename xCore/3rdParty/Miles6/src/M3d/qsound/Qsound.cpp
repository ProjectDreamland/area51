//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: QSound M3D provider                                          ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 10-Apr-99: Initial                                    ##
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

#define  diag_printf //AIL_debug_printf

#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <process.h>
#include <objbase.h>

//
// Support both QSound APIs
//
#include "qapi.h"

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

   MAX_SUPPORTED_SAMPLES,
   TECHNOLOGY_VERSION,           // Version of the mixer DLL
   SPEAKER_PLACEMENT,            // QMIX_SPEAKERS_xxxx
   CROSSOVER_FILTER,             // QMIX_OPTIONS_CROSSOVER
   LISTENER_DOPPLER,             // QMIX_OPTIONS_DOPPLER
   FLIP_SPEAKERS,                // QMIX_OPTIONS_FLIPOUTPUT
   REAR_EFFECTS,                 // QMIX_OPTIONS_REAR
   ROOM_SIZE,                    // Room size in meters
   SPEED_OF_SOUND,               // Speed of sound in meters per second

   LICENSED_NAME,

   //
   // Per-sample attribs
   //

   PROCESSING_MODE,              // QMIX_CHANNEL_xxxx flags
};

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

   QSVECTOR    position;         // 3D position
   QSVECTOR    face;             // 3D orientation
   QSVECTOR    up;               // 3D up-vector
   QSVECTOR    velocity;         // 3D velocity
   S32         auto_update;      // TRUE to automatically update in background

   F32         max_dist;         // Sample distances
   F32         min_dist;

   volatile S32 cancel_pending;      // we took an early exit from sleep_sample(), don't do anything else

   LPMIXWAVE lpWave;                   // QSound wave buffer
   int iChannel;                       // QSound channel
#define  QSOUND_INVALID_CHANNEL  (-1)

   U32 lastblockdone;                  // estimated time when last mix will be done

   F32 obstruction;
   F32 occlusion;

   F32 inner_angle;
   F32 outer_angle;
   S32 outer_volume;

   AIL3DSAMPLECB eos;
   H3DSAMPLE clientH3D;
   S32           docb;
};


//
// Limit to 64 voices
//

#define N_SAMPLES 64

char M3DNAME[256]=QSOUND_PROVIDER_NAME " (Evaluation)";

SAMPLE3D samples[N_SAMPLES];

S32 avail_samples = 0;

S32			speaker_type;

S32         first_play=1;

//
// Globals
//

S32 active = 0;

HQMIXER hQMixer;

QSVECTOR    listen_position;
QSVECTOR    listen_face;
QSVECTOR    listen_up;
QSVECTOR    listen_velocity;
S32         listen_auto_update = 0;

HTIMER      service_timer;
HTIMER		buffer_timer;


void API_lock(void)
{
	if (AIL_get_preference(AIL_LOCK_PROTECTION))
     AIL_lock();
}

void API_unlock(void)
{
	if (AIL_get_preference(AIL_LOCK_PROTECTION))
  	AIL_unlock();
}


//##############################################################################
//#																									 #
//# M3D_set_3D_speaker_type																	 #
//#																									 #
//##############################################################################

void		  AILEXPORT M3D_set_3D_speaker_type (S32 spk_type)
{
  API_lock();

  speaker_type=spk_type;

  switch (spk_type)
  {
	 case AIL_3D_HEADPHONE:
      QSoundSetSpeakerPlacement (hQMixer, QMIX_SPEAKERS_HEADPHONES);
      break;
	 case AIL_3D_4_SPEAKER:
      QSoundSetSpeakerPlacement (hQMixer, QMIX_SPEAKERS_QUAD);
      break;
	 case AIL_3D_SURROUND:
	 case AIL_3D_2_SPEAKER:
	 default:
      QSoundSetSpeakerPlacement (hQMixer, QMIX_SPEAKERS_STEREO);
      break;
  }

  API_unlock();
}

//##############################################################################
//#																									 #
//# M3D_3D_speaker_type																			 #
//#																									 #
//##############################################################################

S32		 AILEXPORT M3D_3D_speaker_type (void)
{
	return speaker_type;
}


//############################################################################
//##                                                                        ##
//## Convert linear voltage level to 20log10 (dB) level for volume and pan  ##
//## functions                                                              ##
//##                                                                        ##
//## Returns dB down from linear_max                                        ##
//##                                                                        ##
//############################################################################

static F32 calc_dB(S32 linear_min, S32 linear_max, S32 linear_level)
{
   double mn,mx,lv,ratio;
   F32    result;

   //
   // Ensure extreme values return max/min results
   //

   if (linear_level == linear_min)
      {
      return 100;
      }

   if (linear_level == linear_max)
      {
      return 0;
      }

   mn = (double) linear_min;
   mx = (double) linear_max;
   lv = (double) linear_level;

   if (lv != 0)
      {
      ratio = (mn + mx) / lv;
      }
   else
      {
      ratio = (mn + mx) / 1;
      }

   result = (F32) (20.0 * log10(ratio));

   return result;
}


static void QS_set_error (void)
{
   char errmsg[256];

   QSoundGetErrorText (QSoundGetLastError(), errmsg, sizeof(errmsg));
   diag_printf (errmsg);
   AIL_set_error (errmsg);
}

//############################################################################
//##                                                                        ##
//## Set volume level of secondary buffer                                   ##
//##                                                                        ##
//############################################################################

static void QS_set_volume(SAMPLE3D FAR *S)
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

   if (S->iChannel != QSOUND_INVALID_CHANNEL)
      {
      S32 qsound_volume;

      //
      // Set secondary buffer volume.  QSound uses a linear volume scale of
      // 0 (minimum) to 32767 (maximum), which maps conveniently to ours
      // with a multiplication by 258 (32767/127).
      //
      // We check for full volume and set it explicitly to 32767, in
      // case full volume is special internally to QSound.
      //
      F32 vol= ((F32)S->volume) *
                     (1.0F-S->occlusion) *
                     (1.0F-S->obstruction);

      if (vol<EPSILON)
        vol=0.0;

      if (vol >= 126.5F)
         qsound_volume = 32767;
      else
         qsound_volume = (S32)(vol*258.0F);

      API_lock();

      MMRESULT result = QSoundSetVolume (hQMixer, S->iChannel,
         QMIX_IMMEDIATE, qsound_volume);
      CHECK_MMRESULT (result);

      API_unlock();
      }
}


//############################################################################
//##                                                                        ##
//## Set cone of secondary buffer                                           ##
//##                                                                        ##
//############################################################################

static void QS_set_cone(SAMPLE3D FAR *S)
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

   if (S->iChannel != QSOUND_INVALID_CHANNEL)
      {

      //
      //
      //

      F32 vol= calc_dB(0,127,S->outer_volume);

      API_lock();

      MMRESULT result = QSoundSetSourceCone2 (hQMixer, S->iChannel,
         QMIX_IMMEDIATE,
         &S->face,
         S->inner_angle,
         S->outer_angle,
         vol);

      API_unlock();

      CHECK_MMRESULT (result);

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
	S->outer_angle = inner_angle;
	S->outer_volume = outer_volume;

	QS_set_cone(S);
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

   QS_set_volume(S);
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

   QS_set_volume(S);
}

//############################################################################
//##                                                                        ##
//## Set playback rate of secondary buffer                                  ##
//##                                                                        ##
//############################################################################

static void QS_set_frequency(SAMPLE3D FAR *S)
{
   //
   // Set new frequency
   //

   if (S->iChannel != QSOUND_INVALID_CHANNEL)
      {
      API_lock();

      MMRESULT result = QSoundSetFrequency (hQMixer, S->iChannel,
         QMIX_IMMEDIATE, S->playback_rate);
      CHECK_MMRESULT (result);

      API_unlock();
      }
}

//############################################################################
//##                                                                        ##
//## Print debugging information for a channel                              ##
//##                                                                        ##
//############################################################################

static void QS_dump_channel(SAMPLE3D FAR *S)
{
   QMIX_CHANNEL_PARAMS params;
   DWORD volume;
   DWORD status;

   API_lock();
   status = QSoundGetChannelStatus (hQMixer, S->iChannel);
   QSoundGetVolume (hQMixer, S->iChannel, &volume);
   memset (&params, 0, sizeof(params));
   params.dwSize = sizeof(params);
   QSoundGetChannelParams (hQMixer, S->iChannel, &params);

   diag_printf ("\r\n"
      "%08lX: type %02x status 0x%02x modes %02x\r\n"
      "\t  dist %f (min %f, max %f, rolloff %f);\r\n"
      "\t  vol %lu/%f * rolloff scale %f = applied vol %f\r\n",
      S, params.dwChannelType, status, params.dwModes,
      params.flDistance, params.flMinDistance, params.flMaxDistance, params.flRolloff,
      volume, params.flVolume, params.flRolloffScale, params.flAppliedVolume);

   API_unlock();
}

//############################################################################
//##                                                                        ##
//## Start playback of secondary buffer at beginning                        ##
//##                                                                        ##
//############################################################################

static void QS_start_secondary(SAMPLE3D FAR *S)
{
   if (S->iChannel == QSOUND_INVALID_CHANNEL)
      return;

   if (S->lpWave == NULL)
      return;

   API_lock();

   S->buffers_past_end = 0;

   //
   // Clear any leftover data in the channel.  If the channel had
   // been paused, it will stay paused.
   //
   MMRESULT result = QSoundFlushChannel (hQMixer, S->iChannel, 0);
   CHECK_MMRESULT (result);

   //
   // Start the channel, looping indefinitely.  We will stream data
   // into it as needed.
   //
   result = QSoundPlayEx (hQMixer, S->iChannel,
      QMIX_CLEARQUEUE | QMIX_IMMEDIATE, S->lpWave, -1, NULL);
   CHECK_MMRESULT (result);

   //
   // Restart the channel in case it was previously paused.
   //
   result = QSoundRestartChannel (hQMixer, S->iChannel, 0);
   CHECK_MMRESULT (result);

   API_unlock();
}

//############################################################################
//##                                                                        ##
//## Stop playback of secondary buffer (not resumable)                      ##
//##                                                                        ##
//############################################################################

static void QS_stop_secondary(SAMPLE3D FAR *S)
{
   if (S->iChannel == QSOUND_INVALID_CHANNEL)
      return;

   API_lock();

   MMRESULT result = QSoundFlushChannel (hQMixer, S->iChannel, 0);
   CHECK_MMRESULT (result);

   API_unlock();
}

//############################################################################
//##                                                                        ##
//## Pause playback of secondary buffer                                     ##
//##                                                                        ##
//############################################################################

static void QS_pause_secondary(SAMPLE3D FAR *S)
{
   if (S->iChannel == QSOUND_INVALID_CHANNEL)
      return;

   API_lock();

   MMRESULT result = QSoundPauseChannel (hQMixer, S->iChannel, 0);
   CHECK_MMRESULT (result);

   API_unlock();
}

//############################################################################
//##                                                                        ##
//## Restart playback of secondary buffer                                   ##
//##                                                                        ##
//############################################################################

static void QS_restart_secondary(SAMPLE3D FAR *S)
{
   if (S->iChannel == QSOUND_INVALID_CHANNEL)
      return;

   API_lock();

   MMRESULT result = QSoundRestartChannel (hQMixer, S->iChannel, 0);
   CHECK_MMRESULT (result);

   API_unlock();
}

void flush_sample(SAMPLE3D FAR *S);

//############################################################################
//##                                                                        ##
//## Copy data from source sample to target secondary buffer                ##
//##                                                                        ##
//## Backfill target secondary buffer with silence to end of source data    ##
//##                                                                        ##
//############################################################################

BOOL CALLBACK QS_stream_callback (void *out, U32 out_len, void *user)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) user;

   //QS_dump_channel (S);              // for debugging

   void *in;                           // pointer to sample data to mix
   U32   in_len;                       // bytes left in sample
   U32   copy_len;
   U32   amtmixed=0;

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
   // Return TRUE until we are supposed to stop streaming, which is after
   // we've played two buffers of silence.
   //
   return (S->buffers_past_end <= 2);
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
   MMRESULT result;

   API_lock();

   if (S->iChannel != QSOUND_INVALID_CHANNEL)
      {
      result = QSoundCloseChannel (hQMixer, S->iChannel, 0);
      CHECK_MMRESULT (result);
      S->iChannel = QSOUND_INVALID_CHANNEL;
      }

   if (S->lpWave != NULL)
      {
      result = QSoundFreeWave (hQMixer, S->lpWave);
      CHECK_MMRESULT (result);
      S->lpWave = NULL;
      }

   API_unlock();
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
//# Cancel all pending events and return as soon as no events are being      #
//# processed                                                                #
//#                                                                          #
//############################################################################

void sleep_sample(SAMPLE3D FAR *S)
{
   S->cancel_pending = 1;
}

//############################################################################
//#                                                                          #
//# Re-enable event processing                                               #
//#                                                                          #
//############################################################################

void wake_sample(SAMPLE3D FAR *S)
{
   S->cancel_pending = 0;
}

//############################################################################
//#                                                                          #
//# Flush sample buffers                                                     #
//#                                                                          #
//############################################################################

void flush_sample(SAMPLE3D FAR *S)
{
   QS_stop_secondary(S);

   S->buffers_past_end = 0;
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
//# Retrieve a standard RIB provider attribute by index                      #
//#                                                                          #
//############################################################################

U32 AILEXPORT PROVIDER_query_attribute (HATTRIB index)
{
   MMRESULT result;

   switch ((ATTRIB) index)
      {
      case PROVIDER_NAME:
         return (U32) M3DNAME;
      case PROVIDER_VERSION:
         return 0x100;
      case MAX_SUPPORTED_SAMPLES:
         return avail_samples;
      case TECHNOLOGY_VERSION:
         {
         QMIXINFO info;
         memset (&info, 0, sizeof(info));
         info.dwSize = sizeof(info);
         QSoundGetInfoEx (&info);
         return (info.dwMajorVersion << 8) | (info.dwMinorVersion & 0xFF);
         }

      case SPEAKER_PLACEMENT:
         {
         U32 placement;
         QSoundGetSpeakerPlacement (hQMixer, &placement);
         return placement;             // QMIX_SPEAKERS_xxxx flags
         }
      case CROSSOVER_FILTER:
         {
         U32 flags;
         result = QSoundGetOptions (hQMixer, &flags);
         CHECK_MMRESULT(result);
         return ((flags & QMIX_OPTIONS_CROSSOVER) != 0);
         }
      case LISTENER_DOPPLER:
         {
         U32 flags;
         result = QSoundGetOptions (hQMixer, &flags);
         CHECK_MMRESULT(result);
         return ((flags & QMIX_OPTIONS_DOPPLER) != 0);
         }
      case FLIP_SPEAKERS:
         {
         U32 flags;
         result = QSoundGetOptions (hQMixer, &flags);
         CHECK_MMRESULT(result);
         return ((flags & QMIX_OPTIONS_FLIPOUTPUT) != 0);
         }
      case REAR_EFFECTS:
         {
         U32 flags;
         result = QSoundGetOptions (hQMixer, &flags);
         CHECK_MMRESULT(result);
         return ((flags & QMIX_OPTIONS_REAR) != 0);
         }
      case ROOM_SIZE:
         {
         F32 room_size;
         QSoundGetRoomSize (hQMixer, &room_size);
         return float_as_long (room_size);
         }
      case SPEED_OF_SOUND:
         {
         F32 speed;                 // in meters per second
         QSoundGetSpeedOfSound (hQMixer, &speed);
         return float_as_long (speed);
         }
      }

   return 0;
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
   S32 prev = -1;

   switch ((ATTRIB) preference)
      {
      case LICENSED_NAME:
         first_play=0;
         wsprintf( M3DNAME,"%s (%s)",QSOUND_PROVIDER_NAME,(char FAR*)value);
         break;

      case SPEAKER_PLACEMENT:
         QSoundGetSpeakerPlacement (hQMixer, (U32 *) &prev);
         QSoundSetSpeakerPlacement (hQMixer, *(S32 FAR *)value);
         break;

      case CROSSOVER_FILTER:
         QSoundGetOptions (hQMixer, (U32 *) &prev);
         prev = ((prev & QMIX_OPTIONS_CROSSOVER) != 0);
         QSoundSetOptions (hQMixer,
            *(BOOL FAR *)value ? QMIX_OPTIONS_CROSSOVER : 0,
            QMIX_OPTIONS_CROSSOVER);
         break;

      case LISTENER_DOPPLER:
         QSoundGetOptions (hQMixer, (U32 *) &prev);
         prev = ((prev & QMIX_OPTIONS_DOPPLER) != 0);
         QSoundSetOptions (hQMixer,
            *(BOOL FAR *)value ? QMIX_OPTIONS_DOPPLER : 0,
            QMIX_OPTIONS_DOPPLER);
         break;

      case FLIP_SPEAKERS:
         QSoundGetOptions (hQMixer, (U32 *) &prev);
         prev = ((prev & QMIX_OPTIONS_FLIPOUTPUT) != 0);
         QSoundSetOptions (hQMixer,
            *(BOOL FAR *)value ? QMIX_OPTIONS_FLIPOUTPUT : 0,
            QMIX_OPTIONS_FLIPOUTPUT);
         break;

      case REAR_EFFECTS:
         QSoundGetOptions (hQMixer, (U32 *) &prev);
         prev = ((prev & QMIX_OPTIONS_REAR) != 0);
         QSoundSetOptions (hQMixer,
            *(BOOL FAR *)value ? QMIX_OPTIONS_REAR : 0,
            QMIX_OPTIONS_REAR);
         break;

      case ROOM_SIZE:
         QSoundGetRoomSize (hQMixer, (F32 *)&prev);
         QSoundSetRoomSize (hQMixer, *(F32 FAR *)value, 0);
         break;

      case SPEED_OF_SOUND:
         QSoundGetSpeedOfSound (hQMixer, (F32 *)&prev);
         QSoundSetSpeedOfSound (hQMixer, *(F32 FAR *)value, 0);
         break;
      }

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

   diag_printf("%X START\r\n",S);

   QS_start_secondary(S);

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

   diag_printf("%X STOP\r\n",S);

   S->status = SMP_STOPPED;

   QS_pause_secondary(S);
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
   // Activate sample and wait for command to take effect
   //

   diag_printf("%X RESUME\r\n",S);

   QS_restart_secondary(S);

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

   QS_set_volume(S);
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

   S->playback_rate = playback_rate;
   QS_set_frequency(S);
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

   if (S->iChannel != QSOUND_INVALID_CHANNEL)
      {
      QMIX_DISTANCES distances;

      memset (&distances, 0, sizeof(distances));
      distances.cbSize = sizeof(distances);
      distances.minDistance = min_dist;
      distances.maxDistance = max_dist;
      distances.scale = 1.0F;

      API_lock();

      MMRESULT result = QSoundSetDistanceMapping (hQMixer, S->iChannel,
         QMIX_IMMEDIATE, &distances);
      CHECK_MMRESULT (result);

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

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   switch ((ATTRIB) index)
      {
      //
      // The sample-specific processing mode flags are all lumped together
      // in a single preference, because they have interactions with each
      // other.  This should help if QSound decides to add more flags.
      //
      case PROCESSING_MODE:
         {
         QMIX_CHANNEL_PARAMS params;

         memset (&params, 0, sizeof(params));
         params.dwSize = sizeof(params);
         QSoundGetChannelParams (hQMixer, S->iChannel, &params);
         return params.dwModes;
         }
      }

   return 0;
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
   S32 prev = -1;

   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return -1;
      }

   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //

      case PROCESSING_MODE:
         {
         QMIX_CHANNEL_PARAMS params;

         memset (&params, 0, sizeof(params));
         params.dwSize = sizeof(params);
         QSoundGetChannelParams (hQMixer, S->iChannel, &params);
         prev = (S32)params.dwModes;

         params.dwModes = *((U32*)value);
         QSoundSetChannelParams (hQMixer, S->iChannel, 0, &params);
         break;
         }
      }

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
//# M3D_set_3D_position                                                        #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_position         (H3DPOBJECT obj, //)
                                                  F32     X,
                                                  F32     Y,
                                                  F32     Z)
{
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;
   MMRESULT result;

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

      if (S->iChannel != QSOUND_INVALID_CHANNEL)
         {
//       diag_printf ("Source   at %f %f %f\r\n", X, Y, Z);
         API_lock();

         result = QSoundSetSourcePosition (hQMixer, S->iChannel,
            QMIX_IMMEDIATE, &S->position);
         CHECK_MMRESULT (result);

         API_unlock();
         }
      }
   else if (*t == IS_LISTENER)
      {
      listen_position.x = X;
      listen_position.y = Y;
      listen_position.z = Z;

//    diag_printf ("Listener at %f %f %f\r\n", X, Y, Z);
      API_lock();

      result = QSoundSetListenerPosition (hQMixer,
         &listen_position, QMIX_IMMEDIATE);
      CHECK_MMRESULT (result);

      API_unlock();
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
   QSVECTOR per_second;
   MMRESULT result;

   if (t == NULL)
      {
      return;
      }

   per_second.x = dX_per_ms * 1000.0F;
   per_second.y = dY_per_ms * 1000.0F;
   per_second.z = dZ_per_ms * 1000.0F;

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      S->velocity.x = dX_per_ms;
      S->velocity.y = dY_per_ms;
      S->velocity.z = dZ_per_ms;

      if (S->iChannel != QSOUND_INVALID_CHANNEL)
         {
         API_lock();

         result = QSoundSetSourceVelocity (hQMixer, S->iChannel,
            QMIX_IMMEDIATE, &per_second);
         CHECK_MMRESULT (result);

         API_unlock();
         }
      }
   else if (*t == IS_LISTENER)
      {
      listen_velocity.x = dX_per_ms;
      listen_velocity.y = dY_per_ms;
      listen_velocity.z = dZ_per_ms;

      API_lock();

      result = QSoundSetListenerVelocity (hQMixer,
         &per_second, QMIX_IMMEDIATE);
      CHECK_MMRESULT (result);

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

      S->face.x = X_face;
      S->face.y = Y_face;
      S->face.z = Z_face;

      S->up.x = X_up;
      S->up.y = Y_up;
      S->up.z = Z_up;
      
      QS_set_cone(S);
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

      listen_face.x = X_face;
      listen_face.y = Y_face;
      listen_face.z = Z_face;

      listen_up.x = X_up;
      listen_up.y = Y_up;
      listen_up.z = Z_up;

      API_lock();

      MMRESULT result = QSoundSetListenerOrientation (hQMixer,
         &listen_face, &listen_up, QMIX_IMMEDIATE);
      CHECK_MMRESULT (result);

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

   S->lpWave        = NULL;
   S->iChannel      = QSOUND_INVALID_CHANNEL;

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
   // Prepare the sample for the QSound mixer
   //
   QMIXWAVEPARAMS params;
   memset (&params, 0, sizeof(params));
   params.Stream.Format = &wf;
   params.Stream.BlockBytes = BUFF_SIZE;
   params.Stream.Callback = QS_stream_callback;
   params.Stream.User = S;             // pass S to callback function
   params.Stream.Tag = S - samples;    // sample number

   API_lock ();

   S->lpWave = QSoundOpenWaveEx (hQMixer, &params, QMIX_INPUTSTREAM);

   if (! S->lpWave)
      {
      API_unlock ();

      AIL_set_error("Could not prepare sample");
      return 0;
      }

   //
   // Assign a mixer channel to the sample
   //
   S->iChannel = QSoundOpenChannel (hQMixer, 0, QMIX_OPENAVAILABLE);
   if (S->iChannel == QSOUND_INVALID_CHANNEL)
      {
      API_unlock ();

      AIL_set_error("Could not create channel");
      return 0;
      }

   API_unlock ();

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
   // Release the background thread from its sleep state
   //

   wake_sample(S);

   //
   // Clear all special attenuation effects
   //
   M3D_set_3D_sample_obstruction  (samp, 0.0F);
   M3D_set_3D_sample_occlusion    (samp, 0.0F);

	//
	// Set cone
	//

	M3D_set_3D_sample_cone    (samp,
										360.0F,
										360.0F,
                              127);

   if (first_play)
   {
     first_play=0;
     MessageBox(0,"This is an evaluation version of " QSOUND_PROVIDER_NAME "\n\nYou cannot distribute it with your application until you obtain a license from QSound (contact RAD for details).","For Evaluation Only!",MB_OK|MB_ICONINFORMATION);
   }
   return 1;
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

      if (S->eos)
      {
        if ((S->status==SMP_PLAYING) && (S->pos) && (S->pos == S->len))
          if (timeGetTime()>=S->lastblockdone)
          {
            S->status=SMP_PLAYING|256;
            S->eos(S->clientH3D);
          }
      }

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
//# Shut down QSound after cleaning up all resources.                        #
//#                                                                          #
//############################################################################

void shut_down (void)
{
   if (hQMixer)
      {
      QSoundCloseSession (hQMixer);
      hQMixer = NULL;
      }
}

//##############################################################################
//#                                                                            #
//# M3D_activate                                                               #
//#                                                                            #
//##############################################################################

M3DRESULT AILEXPORT M3D_activate                 (S32 enable)
{
   MMRESULT result;
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
      // Get DirectSound pointer from MSS; if MSS is already
      // using DirectSound, we must temporarily disable it
      // across the calls to initialize and activate QSound.  This
      // is presumably because QSound initializes DirectSound using
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
            AIL_set_error("Unable to release DirectSound object");
            return M3D_NOT_INIT;
            }
         prev_lpds = (AILLPDIRECTSOUNDBUFFER) ~NULL;
         }

      //
      // Initialize QSound (QMixer or QMDX).
      //
      QMIXCONFIG QMConfig;
      memset (&QMConfig, 0, sizeof(QMConfig));
      QMConfig.dwSize = sizeof(QMConfig);

      //
      // Play silence even when there are no waves to be mixed, to
      // reduce startup latency, and use DirectX's left-handed coordinate
      // system.  Use the primary buffer only if the AIL preference
      // is set.
      //
      QMConfig.dwFlags = QMIX_CONFIG_PLAYSILENCE | QMIX_CONFIG_LEFTCOORDS;
      if (AIL_get_preference (DIG_DS_USE_PRIMARY))
         QMConfig.dwFlags |= QMIX_CONFIG_USEPRIMARY;

      //
      // Use the default output sampling rate, which can be overridden
      // through the QSound configuration file.
      //
      QMConfig.dwSamplingRate = 0;

      //
      // Identify the main window.
      //
      QMConfig.hwnd = AIL_HWND ();

      //
      // Configure for the desired number of channels, according to
      // the AIL preference (default = 16).  We use QSound's resource
      // manager, so some number of these may be accelerated in hardware.
      //
      avail_samples = AIL_get_preference (DIG_MIXER_CHANNELS);
      if (avail_samples > N_SAMPLES)
         avail_samples = N_SAMPLES;
      QMConfig.iChannels = avail_samples;

      //
      // Force QSound to use its software mixer for all channels.
      //
      QMConfig.iChannels3dQS = -1;
      QMConfig.iChannels2dQS = -1;

      //
      // If MSS is using WaveOut, instruct QSound to use WaveOut --
      // this will not work if someone else (including MSS) has
      // DirectSound open.
      //
      // If MSS is using DirectSound, and has already created a
      // DirectSound object, reopen the object with the 3D capability
      // enabled and pass it to QSound.  (QSound will hang at init
      // time if another application has DirectSound open.)
      //

      if (AIL_get_preference (DIG_USE_WAVEOUT))
         {
         QMConfig.iOutput = QMIX_CONFIG_OUTPUT_WAVEOUT;
         QMConfig.iLatency = 100;
         }
      else
         {
         QMConfig.iOutput = QMIX_CONFIG_OUTPUT_DIRECTSOUND;
         QMConfig.iLatency = 50;
         }

      AIL_unlock_mutex();
      hQMixer = QSoundInitEx (&QMConfig);
      AIL_lock_mutex();
      if (! hQMixer)
         {
         QS_set_error ();
         return M3D_NOT_INIT;
         }

      AIL_unlock_mutex();
      result = QSoundActivate (hQMixer, TRUE);
      AIL_lock_mutex();
      if (result != 0)
         {
         QS_set_error ();
         shut_down ();
         return M3D_NOT_INIT;
         }

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

      //
      // Set the pan rate for all channels to zero (although it should
      // not matter because we use QMIX_IMMEDIATE everywhere.)
      //
      QSoundSetPanRate (hQMixer, 0, QMIX_ALL, 0);

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

      S32 i;

      for (i=0; i < avail_samples; i++)
         {
         SAMPLE3D FAR *S = &samples[i];

         memset(S, 0, sizeof(struct SAMPLE3D));

         S->status = SMP_FREE;

         S->cancel_pending = 0;

         //
         // Clear sample emitter
         //

         S->lpWave = NULL;
         S->iChannel = QSOUND_INVALID_CHANNEL;

         }

      //
      // Register and start service timer
      //

      service_timer = AIL_register_timer(M3D_serve);

      AIL_set_timer_period(service_timer, SERVICE_MSECS * 1000 );

      AIL_start_timer(service_timer);

		//
		// Register and start buffer-notification timer at 20 Hz
		//

		buffer_timer = AIL_register_timer(notify_timer);

		AIL_set_timer_frequency(buffer_timer, 20);

		AIL_start_timer(buffer_timer);


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

      //
      // Go through the same gyrations we did when starting up, to
      // prevent QSound from hanging because of cooperative level
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
      // Deactivate QSound.
      //

      AIL_unlock_mutex();
      result = QSoundActivate (hQMixer, FALSE);
      AIL_lock_mutex();
      if (result != 0)
         {
         QS_set_error ();
         shut_down ();
         return M3D_CLOSE_ERR;
         }

      AIL_unlock_mutex();
      result = QSoundCloseSession (hQMixer);
      AIL_lock_mutex();
      if (result != 0)
         {
         QS_set_error ();
         return M3D_CLOSE_ERR;
         }
      hQMixer = NULL;

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
            return M3D_CLOSE_ERR;
            }
         }

      active = 0;
      diag_printf ("Successfully closed QSound.\r\n");
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
      REG_AT("Processing mode flags",     PROCESSING_MODE,        RIB_HEX),

      REG_FN(M3D_set_3D_EOS),
      REG_FN(M3D_set_3D_sample_obstruction),
      REG_FN(M3D_3D_sample_obstruction),
      REG_FN(M3D_set_3D_sample_occlusion),
      REG_FN(M3D_3D_sample_occlusion),

//		REG_FN(M3D_set_3D_sample_cone),
//		REG_FN(M3D_3D_sample_cone),

      REG_FN(M3D_3D_sample_query_attribute),
      REG_FN(M3D_3D_set_sample_preference),
      };

   const RIB_INTERFACE_ENTRY M3D[] =
      {
      REG_AT("Name",                       PROVIDER_NAME,         RIB_STRING),
      REG_AT("Version",                    PROVIDER_VERSION,      RIB_HEX),
      REG_AT("Maximum supported samples",  MAX_SUPPORTED_SAMPLES, RIB_DEC),
      REG_AT("QSound technology version",  TECHNOLOGY_VERSION,    RIB_HEX),
      REG_AT("Speaker placement",          SPEAKER_PLACEMENT,     RIB_DEC),
      REG_AT("Crossover filter",           CROSSOVER_FILTER,      RIB_BOOL),
      REG_AT("Factor listener velocity",   LISTENER_DOPPLER,      RIB_BOOL),
      REG_AT("Swap output channels",       FLIP_SPEAKERS,         RIB_BOOL),
      REG_AT("Rear of listener effects",   REAR_EFFECTS,          RIB_BOOL),
      REG_AT("Room size",                  ROOM_SIZE,             RIB_FLOAT),
      REG_AT("Speed of sound",             SPEED_OF_SOUND,        RIB_FLOAT),

      REG_PR("Speaker placement",          SPEAKER_PLACEMENT,     RIB_DEC),
      REG_PR("Crossover filter",           CROSSOVER_FILTER,      RIB_BOOL),
      REG_PR("Factor listener velocity",   LISTENER_DOPPLER,      RIB_BOOL),
      REG_PR("Swap output channels",       FLIP_SPEAKERS,         RIB_BOOL),
      REG_PR("Rear of listener effects",   REAR_EFFECTS,          RIB_BOOL),
      REG_PR("Room size",                  ROOM_SIZE,             RIB_FLOAT),
      REG_PR("Speed of sound",             SPEED_OF_SOUND,        RIB_FLOAT),
      REG_PR("Licensed name",              LICENSED_NAME,         RIB_STRING),

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
