//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: M3D module for DirectSound 3D, Aureal A3D, and EAX providers ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 07-Sep-98: Initial                                    ##
//##          1.01 of 21-Oct-98: Added sample attributes, sample rate calls ##
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
#include <conio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <process.h>

#include <dsound.h>

#ifdef AUREAL
#include "ia3d.h"
#endif

#ifdef EAX3D

#ifdef EAX2

#include "eax2.h"
#define EAXPROP DSPROPSETID_EAX_ListenerProperties
#define EAXBUFPROP DSPROPSETID_EAX_BufferProperties

#else

#include "eax.h"
#define EAXPROP DSPROPSETID_EAX_ReverbProperties
#define EAXBUFPROP DSPROPSETID_EAXBUFFER_ReverbProperties

#endif

#define PSET_SETGET (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)
#define CREVERB_INVALID_VALUE -1.0E+30F
#define CREVERBBUFFER_SETGET (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET)
#define CREVERBBUFFER_INVALID_VALUE -1.0E+30F

#endif

#include "mss.h"

#include <objbase.h>


#define DSBCAPS_MUTE3DATMAXDISTANCE 0x00020000

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

#ifdef EAX3D
   EAX_ENVIRONMENT,
   EAX_EFFECT_VOLUME,
   EAX_DECAY_TIME,
   EAX_DAMPING,

#ifdef EAX2
   EAX_ENVIRONMENT_SIZE,
   EAX_ENVIRONMENT_DIFFUSION,
   EAX_ROOM,
   EAX_ROOM_HF,
   EAX_DECAY_HF_RATIO,
   EAX_REFLECTIONS,
   EAX_REFLECTIONS_DELAY,
   EAX_REVERB,
   EAX_REVERB_DELAY,
   EAX_ROOM_ROLLOFF,
   EAX_AIR_ABSORPTION,
   EAX_FLAGS,
#endif
#endif

   //
   // Voice attribs for "MSS 3D sample services"
   //

#ifdef EAX3D
   EAX_EFFECT_REVERBMIX,

#ifdef EAX2
   EAX_SAMPLE_DIRECT,
   EAX_SAMPLE_DIRECT_HF,
   EAX_SAMPLE_ROOM,
   EAX_SAMPLE_ROOM_HF,
   EAX_SAMPLE_OBSTRUCTION,
   EAX_SAMPLE_OBSTRUCTION_LF_RATIO,
   EAX_SAMPLE_OCCLUSION,
   EAX_SAMPLE_OCCLUSION_LF_RATIO,
   EAX_SAMPLE_OCCLUSION_ROOM_RATIO,
   EAX_SAMPLE_ROOM_ROLLOFF,
   EAX_SAMPLE_AIR_ABSORPTION,
   EAX_SAMPLE_OUTSIDE_VOLUME_HF,
   EAX_SAMPLE_FLAGS,
#endif

#endif
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

struct DS3VECTOR3D
{
   D3DVALUE x;
   D3DVALUE y;
   D3DVALUE z;
};

//
// Sample descriptor
//

struct SAMPLE3D
{
   OBJTYPE  type;                // Runtime type of object

   U32      index;               // which sample is this?

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

   LPDIRECTSOUNDBUFFER   lpdsb;        // Base DirectSound buffer
   LPDIRECTSOUND3DBUFFER lpds3db;      // Extended DirectSound 3D buffer

   U32 previous_position;              // Previous play cursor position

   U32 lastblockdone;                  // estimated time when last mix will be done

   F32 obstruction;
   F32 occlusion;

   F32 inner_angle;
   F32 outer_angle;
   S32 outer_volume;

   volatile S32 noints;        // no interrupt servicing please

#ifdef EAX3D
   LPKSPROPERTYSET      lpPropertySet; // Property set interface for secondary buffer
#endif
   
   AIL3DSAMPLECB eos;
   H3DSAMPLE clientH3D;
};

//
// define provider name

#ifdef DX7SN
  #define PROVIDER_NAME_STR "DirectSound3D 7+ Software - Pan and Volume"
#else
#ifdef DX7SL
  #define PROVIDER_NAME_STR "DirectSound3D 7+ Software - Light HRTF"
#else
#ifdef DX7SH
  #define PROVIDER_NAME_STR "DirectSound3D 7+ Software - Full HRTF"
#else
#ifdef AUREAL
  #define PROVIDER_NAME_STR "Aureal A3D Interactive (TM)"
#else
#ifdef EAX2
  #define PROVIDER_NAME_STR "Creative Labs EAX 2 (TM)"
#else
#ifdef EAX3D
  #define PROVIDER_NAME_STR "Creative Labs EAX (TM)"
#else
#ifdef HWARE
  #define PROVIDER_NAME_STR "DirectSound3D Hardware Support"
#else
  #define PROVIDER_NAME_STR "DirectSound3D Software Emulation"
#endif
#endif
#endif
#endif
#endif
#endif
#endif


//
// Support up to 8 samples (arbitrary limit) with default DS3D software
// provider
//
// With Aureal, we allocate 64 but later set the limit to the actual # of
// streaming buffers available in hardware
//

#ifdef AUREAL
   //
   // Aureal: limit to 96 voices, or max supported, whichever is lower
   //

   #define N_SAMPLES 96
#else
#ifdef DX7SN
   // fast software
   #define N_SAMPLES 64
#else
#ifdef DX7SL
   // light software
   #define N_SAMPLES 32
#else
#ifdef DX7SH
   // slower software
   #define N_SAMPLES 32
#else
#ifdef SWARE
   //
   // Old Software: limit to 16 voices
   //

   #define N_SAMPLES 16
#else
   //
   // Hardware: limit to 96 voices, or max supported, whichever is lower
   //

   #define N_SAMPLES 96

#endif
#endif
#endif
#endif
#endif

SAMPLE3D samples[N_SAMPLES];
S32 avail_samples = 0;

//
// Globals
//

S32 active = 0;

LPDIRECTSOUND           lpDS;
LPDIRECTSOUNDBUFFER     lpDSPRIM;
LPDIRECTSOUND3DLISTENER lp3DLISTENER;
DSCAPS                  DSCaps;

DS3VECTOR3D listen_position;
DS3VECTOR3D listen_face;
DS3VECTOR3D listen_up;
DS3VECTOR3D listen_velocity;
S32         listen_auto_update = 0;

HTIMER      service_timer;
HTIMER      buffer_timer;

S32         speaker_type;

#ifdef EAX3D
S32         room_type;                // values defined by EAX.H

LPDIRECTSOUNDBUFFER   lpSecondaryBuffer = NULL; // Secondary buffer
LPDIRECTSOUND3DBUFFER lpDs3dBuffer      = NULL; // 3D interface for secondary buffer
LPKSPROPERTYSET       lpPropertySet     = NULL; // Property set interface for secondary buffer

#endif

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


//############################################################################
//##                                                                        ##
//## Convert linear voltage level to 20log10 (dB) level for volume and pan  ##
//## functions                                                              ##
//##                                                                        ##
//## Returns negative dB * 100 down from linear_max                         ##
//##                                                                        ##
//############################################################################

static F32 DS_calc_dB(F32 linear_min, F32 linear_max, F32 linear_level)
{
   double mn,mx,lv,ratio;
   F32    result;

   //
   // Ensure extreme values return max/min results
   //

   if (linear_level <= (linear_min+EPSILON))
      {
      return -10000.0F;
      }

   if (linear_level >= (linear_max-EPSILON))
      {
      return 0.0F;
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
      ratio = (mn + mx) / mx;
      }

   result = (F32) (-2000.0 * log10(ratio));

   return result;
}

//############################################################################
//##                                                                        ##
//## Set volume level of secondary buffer                                   ##
//##                                                                        ##
//############################################################################

static void DS_set_volume(SAMPLE3D FAR *S)
{
   S32 v;

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

   v = S->volume;

   //
   // Set secondary buffer volume
   //

   if (S->lpdsb != NULL)
      {
      F32 net_volume = DS_calc_dB(0,127,
                       (((F32)v)
#ifndef EAX2
                       *(1.0F-S->occlusion)
                       *(1.0F-S->obstruction)
#endif
                       ));

      API_lock();
      S->lpdsb->SetVolume((S32)net_volume);
      API_unlock();
      }
}


//############################################################################
//##                                                                        ##
//## Set cone settings of secondary buffer                                  ##
//##                                                                        ##
//############################################################################

static void DS_set_cone(SAMPLE3D FAR *S)
{
   S32 v;

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

   v = S->outer_volume;

   //
   // Set secondary buffer volume
   //

   if (S->lpdsb != NULL)
      {
      F32 net_volume = DS_calc_dB(0,127,(F32)v);

      API_lock();

      S->lpds3db->SetConeAngles((S32)S->inner_angle,(S32)S->outer_angle,DS3D_DEFERRED );
      S->lpds3db->SetConeOutsideVolume((S32)net_volume,DS3D_DEFERRED );

      lp3DLISTENER->CommitDeferredSettings();

      API_unlock();
      }
}


//############################################################################
//##                                                                        ##
//## Set playback rate of secondary buffer                                  ##
//##                                                                        ##
//############################################################################

static void DS_set_frequency(SAMPLE3D FAR *S)
{
   //
   // Set new frequency
   //

   if (S->lpdsb != NULL)
      {
      API_lock();
      HRESULT result= S->lpdsb->SetFrequency(S->playback_rate);
      API_unlock();
      }
}

//############################################################################
//##                                                                        ##
//## Lock region of secondary buffer, returning write cursor information    ##
//##                                                                        ##
//############################################################################

static S32 DS_lock_secondary_region(SAMPLE3D FAR *S, //)
                                    S32           offset,
                                    S32           size,
                                    void        **p1,
                                    U32          *l1,
                                    void        **p2,
                                    U32          *l2)
{
   HRESULT result;

   if (S->lpdsb == NULL)
      {
      return 0;
      }

   //
   // Lock the buffer, returning 0 on failure
   //

   do
      {
      API_lock();

      result = S->lpdsb->Lock(offset,
                              size,
                              p1,
                              l1,
                              p2,
                              l2,
                              0);

      API_unlock();

      if (result == DSERR_BUFFERLOST)
         {
         API_lock();
         S->lpdsb->Restore();
         API_unlock();
         }
      }
   while (result == DSERR_BUFFERLOST);

   if (result != DS_OK)
      {
      return 0;
      }

   if ((*l1) + (*l2) != (U32) size)
      {
      return 0;
      }

   return 1;
}

//############################################################################
//##                                                                        ##
//## Unlock region of secondary buffer                                      ##
//##                                                                        ##
//############################################################################

static void DS_unlock_secondary_region(SAMPLE3D FAR *S, //)
                                       void         *p1,
                                       U32           l1,
                                       void         *p2,
                                       U32           l2)
{
   HRESULT result;

   if (S->lpdsb == NULL)
      {
      return;
      }

   //
   // Unlock the buffer, returning 0 on failure
   //

   do
      {
      API_lock();

      result = S->lpdsb->Unlock(p1,
                                l1,
                                p2,
                                l2);

      API_unlock();

      if (result == DSERR_BUFFERLOST)
         {
         API_lock();
         S->lpdsb->Restore();
         API_unlock();
         }
      }
   while (result == DSERR_BUFFERLOST);
}

//############################################################################
//##                                                                        ##
//## Start playback of secondary buffer at beginning                        ##
//##                                                                        ##
//############################################################################

static void DS_start_secondary(SAMPLE3D FAR *S)
{
   HRESULT result;

   if (S->lpdsb == NULL)
      {
      return;
      }

   do
      {
      API_lock();

      S->previous_position = -1;
      S->buffers_past_end = 0;

      result = S->lpdsb->Play(0,
                              0,
                              DSBPLAY_LOOPING);

      API_unlock();

      if (result == DSERR_BUFFERLOST)
         {
         API_lock();
         S->lpdsb->Restore();
         API_unlock();
         }
      }
   while (result == DSERR_BUFFERLOST);
}

//############################################################################
//##                                                                        ##
//## Stop playback of secondary buffer                                      ##
//##                                                                        ##
//############################################################################

static void DS_stop_secondary(SAMPLE3D FAR *S)
{
   HRESULT result;

   if (S->lpdsb == NULL)
      {
      return;
      }

   do
      {
      API_lock();

      result = S->lpdsb->Stop();

      API_unlock();

      if (result == DSERR_BUFFERLOST)
         {
         API_lock();
         S->lpdsb->Restore();
         API_unlock();
         }
      }
   while (result == DSERR_BUFFERLOST);

}

//############################################################################
//##                                                                        ##
//## Rewind secondary buffer to beginning                                   ##
//##                                                                        ##
//############################################################################

static void DS_rewind_secondary(SAMPLE3D FAR *S)
{
   HRESULT result;

   if (S->lpdsb == NULL)
      {
      return;
      }

   do
      {
      API_lock();

      result = S->lpdsb->SetCurrentPosition(0);

      API_unlock();

      if (result == DSERR_BUFFERLOST)
         {
         API_lock();
         S->lpdsb->Restore();
         API_unlock();
         }
      }
   while (result == DSERR_BUFFERLOST);
}

//############################################################################
//##                                                                        ##
//## Flush sample's secondary buffer with silence                           ##
//##                                                                        ##
//############################################################################

static void DS_flush_secondary(SAMPLE3D FAR *S)
{
   U32     dwDummy;
   void   *lpDummy;
   U32     cnt;
   void   *dest;
   U32     silence;
   HRESULT result;

   if (S->lpdsb == NULL)
      {
      return;
      }

   //
   // Request lock on entire buffer
   //

   do
      {
      API_lock();

      result = S->lpdsb->Lock(0,
                              BUFF_SIZE * 2,
                             &dest,
                             &cnt,
                             &lpDummy,
                             &dwDummy,
                              0);

      if (result == DSERR_BUFFERLOST)
         {
         S->lpdsb->Restore();
         }
      }
   while (result == DSERR_BUFFERLOST);

   if (result != DS_OK)
      {
      API_unlock();
      return;
      }

   if (cnt != (U32) BUFF_SIZE * 2)
      {
      API_unlock();
      return;
      }

   //
   // Flush with silence
   //

   silence = (S->bytes_per_sample == 2) ? 0 : 0x80808080;

   memset(dest,
              silence,
              cnt);

   //
   // Release lock
   //

   S->lpdsb->Unlock(dest,
                    cnt,
                    lpDummy,
                    dwDummy);

   API_unlock();
}

static void incnoints(SAMPLE3D FAR *S)
{
   volatile S32 FAR* noi=&S->noints;
   __asm
   {
     mov eax,[noi]
     lock inc dword ptr [eax]
   }
}

static void decnoints(SAMPLE3D FAR *S)
{
   volatile S32 FAR* noi=&S->noints;
   __asm
   {
     mov eax,[noi]
     lock dec dword ptr [eax]
   }
}

//############################################################################
//#                                                                          #
//# Re-enable event processing                                               #
//#                                                                          #
//############################################################################

static void wake_sample(SAMPLE3D FAR *S)
{
  decnoints(S);
}

//############################################################################
//#                                                                          #
//# Flush sample buffers                                                     #
//#                                                                          #
//############################################################################

void flush_sample(SAMPLE3D FAR *S)
{
   DS_stop_secondary(S);
   DS_flush_secondary(S);
   DS_rewind_secondary(S);

   S->previous_position = -1;
   S->buffers_past_end = 0;
}

//############################################################################
//##                                                                        ##
//## Copy data from source sample to target secondary buffer                ##
//##                                                                        ##
//## Backfill target secondary buffer with silence to end of source data    ##
//##                                                                        ##
//############################################################################

static S32 DS3D_stream_to_buffer(SAMPLE3D FAR *S, S32 half, S32 len)
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
   S32   doclear=0;
   S32   docb=0;

   //
   // Lock segment to fill
   //

   if (!DS_lock_secondary_region(S,
                                 half * len,
                                 len,
                                 &dest1,
                                 &len1,
                                 &dest2,
                                 &len2))
      {
      return 0;
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

            diag_printf("%X DONE\n",S);

            S->status = SMP_DONE;
            doclear=1;

            if (oldstatus==SMP_PLAYING)
              docb=1;
            break;
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
   if (amtmixed)
   {
     U32 timer=timeGetTime();

     // if not enough time has past for the last block assume this block will start after it
     if (S->lastblockdone>timer)
       timer=S->lastblockdone;

     S->lastblockdone=timer+((amtmixed*1000)/(S->playback_rate*S->bytes_per_sample));
   }

   //
   // Unlock the previously-locked buffer segment
   //

   DS_unlock_secondary_region(S,
                              dest1,
                              len1,
                              dest2,
                              len2);

   if (doclear)
     flush_sample(S);
   
   return(docb);
}


#ifdef EAX3D

//----------------------------------------------------------
// void SetEnvironment(unsigned long envId)
//
// DESCRIPTION: Selects a new environment, from the list
//               in EAX.H.  e.g. EAX_ENVIRONMENT_CAVE
//
// PARAMETERS: Environment ID
//
// RETURNS: no return value.
//
//----------------------------------------------------------

void EAX_SetEnvironment(unsigned long envId)
{
   API_lock();

   lpPropertySet->Set( EAXPROP,
#ifdef EAX2
                      DSPROPERTY_EAXLISTENER_ENVIRONMENT,
#else
                      DSPROPERTY_EAX_ENVIRONMENT,
#endif
                     NULL,0,
                     &envId,sizeof(unsigned long));

   API_unlock();
}

#ifdef EAX2

static void EAX2_Set_int(DSPROPERTY_EAX_LISTENERPROPERTY prop,U32 val)
{
   API_lock();

   lpPropertySet->Set( EAXPROP,
                       prop,
                       NULL,0,
                      &val,sizeof(U32));

   API_unlock();
}

static void EAX2_Set_float(DSPROPERTY_EAX_LISTENERPROPERTY prop,F32 val)
{
   API_lock();

   lpPropertySet->Set( EAXPROP,
                       prop,
                       NULL,0,
                      &val,sizeof(F32));

   API_unlock();
}

static U32 EAX2_Get_int(DSPROPERTY_EAX_LISTENERPROPERTY prop)
{
   U32 val=0xffffffff;
   unsigned long r;

   API_lock();

   lpPropertySet->Get(EAXPROP,
                      prop,
                      NULL,0,
                     &val,sizeof(U32),&r);

   API_unlock();

   return val;
}

static F32 EAX2_Get_float(DSPROPERTY_EAX_LISTENERPROPERTY prop)
{
   F32 val=-1.0;
   unsigned long r;

   API_lock();

   lpPropertySet->Get(EAXPROP,
                      prop,
                      NULL,0,
                     &val,sizeof(F32),&r);

   API_unlock();

   return val;
}


static U32 EAX2_GetSet_int(DSPROPERTY_EAX_LISTENERPROPERTY prop,U32 val)
{
  U32 ret=EAX2_Get_int(prop);
  EAX2_Set_int(prop,val);
  return(ret);
}


static F32 EAX2_GetSet_float(DSPROPERTY_EAX_LISTENERPROPERTY prop,F32 val)
{
  F32 ret=EAX2_Get_float(prop);
  EAX2_Set_float(prop,val);
  return(ret);
}


static void EAX2_Sample_Set_int(SAMPLE3D FAR *S,DSPROPERTY_EAX_BUFFERPROPERTY prop,U32 val)
{
   API_lock();

   S->lpPropertySet->Set(EAXBUFPROP,
                         prop,
                         NULL,
                         0,
                        &val,
                         sizeof(U32));

   API_unlock();
}

static void EAX2_Sample_Set_float(SAMPLE3D FAR *S,DSPROPERTY_EAX_BUFFERPROPERTY prop,F32 val)
{
   API_lock();

   S->lpPropertySet->Set(EAXBUFPROP,
                         prop,
                         NULL,
                         0,
                        &val,
                         sizeof(F32));

   API_unlock();
}

static U32 EAX2_Sample_Get_int(SAMPLE3D FAR *S,DSPROPERTY_EAX_BUFFERPROPERTY prop)
{
   U32 val=0xffffffff;
   unsigned long r;

   API_lock();

   S->lpPropertySet->Get(EAXBUFPROP,
                         prop,
                         NULL,
                         0,
                        &val,
                         sizeof(U32),
                         &r);

   API_unlock();

   return val;
}

static F32 EAX2_Sample_Get_float(SAMPLE3D FAR *S,DSPROPERTY_EAX_BUFFERPROPERTY prop)
{
   F32 val=-1.0f;
   unsigned long r;

   API_lock();

   S->lpPropertySet->Get(EAXBUFPROP,
                         prop,
                         NULL,
                         0,
                        &val,
                         sizeof(F32),
                         &r);

   API_unlock();

   return val;
}


static U32 EAX2_Sample_GetSet_int(SAMPLE3D FAR *S,DSPROPERTY_EAX_BUFFERPROPERTY prop,U32 val)
{
  U32 ret=EAX2_Sample_Get_int(S,prop);
  EAX2_Sample_Set_int(S,prop,val);
  return(ret);
}

static F32 EAX2_Sample_GetSet_float(SAMPLE3D FAR *S,DSPROPERTY_EAX_BUFFERPROPERTY prop,F32 val)
{
  F32 ret=EAX2_Sample_Get_float(S,prop);
  EAX2_Sample_Set_float(S,prop,val);
  return(ret);
}


#define linear_to_dB(val) DS_calc_dB(0.0F,1.0F,val)

static F32 dB_to_linear(F32 dB)
   {
   if ((dB+EPSILON) > 0.0F)
      return 1.0F;
   else
   if ((dB-EPSILON) < -10000.0F)
      return 0.0F;
   else
      return (F32) pow (10.0, (dB/2000.F));
   }

#endif

//----------------------------------------------------------
// void SetVolume(float volume)
//
// DESCRIPTION: Changes the reverb volume
//
// PARAMETERS: volume value
//
// RETURNS: no return value.
//
//----------------------------------------------------------

void EAX_SetVolume(float volume)
{
   API_lock();

#ifdef EAX2
   LONG vol=(LONG)linear_to_dB(volume);
   lpPropertySet->Set(EAXPROP,
                      DSPROPERTY_EAXLISTENER_ROOM,
                      NULL,0,
                     &vol,sizeof(LONG));
   lpPropertySet->Set(EAXPROP,
                      DSPROPERTY_EAXLISTENER_ROOMHF,
                      NULL,0,
                     &vol,sizeof(LONG));
#else
   lpPropertySet->Set(EAXPROP,
                      DSPROPERTY_EAX_VOLUME,
                      NULL,0,
                     &volume,sizeof(float));
#endif
   API_unlock();
}

//----------------------------------------------------------
// void SetDecayTime(float time)
//
// DESCRIPTION: Changes the reverb decay time
//
// PARAMETERS: decay time value
//
// RETURNS: no return value.
//
//----------------------------------------------------------

void EAX_SetDecayTime(float time)
{
   API_lock();

   lpPropertySet->Set(EAXPROP,
#ifdef EAX2
                      DSPROPERTY_EAXLISTENER_DECAYTIME,
#else
                      DSPROPERTY_EAX_DECAYTIME,
#endif
                      NULL,0,
                     &time,sizeof(float));

   API_unlock();
}

//----------------------------------------------------------
// void SetDamping(float damping)
//
// DESCRIPTION: Changes the reverb damping
//
// PARAMETERS: damping value
//
// RETURNS: no return value.
//
//----------------------------------------------------------

void EAX_SetDamping(float damping)
{
   API_lock();

   lpPropertySet->Set(EAXPROP,
#ifdef EAX2
                      DSPROPERTY_EAXLISTENER_DECAYHFRATIO,
#else
                      DSPROPERTY_EAX_DAMPING,
#endif
                      NULL,0,
                     &damping,sizeof(float));

   API_unlock();
}

//----------------------------------------------------------
// unsigned long GetEnvironment()
//
// DESCRIPTION: Gets the current reverb environment
//
// PARAMETERS: none
//
// RETURNS: Environment ID
//
//----------------------------------------------------------

unsigned long EAX_GetEnvironment()
{
   unsigned long envId=EAX_ENVIRONMENT_GENERIC;
   unsigned long r;

   API_lock();

   lpPropertySet->Get(EAXPROP,
#ifdef EAX2
                      DSPROPERTY_EAXLISTENER_ENVIRONMENT,
#else
                      DSPROPERTY_EAX_ENVIRONMENT,
#endif
                      NULL,0,
                     &envId,sizeof(unsigned long),&r);

   API_unlock();

   return envId;
}

//----------------------------------------------------------
// float GetVolume()
//
// DESCRIPTION: Gets the current reverb volume
//
// PARAMETERS: none
//
// RETURNS: volume value
//
//----------------------------------------------------------

float EAX_GetVolume()
{
   float volume=CREVERB_INVALID_VALUE;
   unsigned long r;

   API_lock();

#ifdef EAX2
   LONG room=20000;
   lpPropertySet->Get(EAXPROP,
                      DSPROPERTY_EAXLISTENER_ROOM,
                      NULL,0,
                     &room,sizeof(LONG),&r);

   if (room!=20000)
     volume=dB_to_linear((F32)room);
#else
   lpPropertySet->Get(EAXPROP,
                      DSPROPERTY_EAX_VOLUME,
                      NULL,0,
                     &volume,sizeof(float),&r);
#endif

   API_unlock();

   return volume;
}

//----------------------------------------------------------
// float GetDecayTime()
//
// DESCRIPTION: Gets the current reverb decay time
//
// PARAMETERS: none
//
// RETURNS: decay time value
//
//----------------------------------------------------------

float EAX_GetDecayTime()
{
   float time=CREVERB_INVALID_VALUE;
   unsigned long r;

   API_lock();

   lpPropertySet->Get(EAXPROP,
#ifdef EAX2
                      DSPROPERTY_EAXLISTENER_DECAYTIME,
#else
                      DSPROPERTY_EAX_DECAYTIME,
#endif
                      NULL,0,
                     &time,sizeof(float),&r);

   API_unlock();

   return time;
}

//----------------------------------------------------------
// float GetDamping()
//
// DESCRIPTION: Gets the current reverb damping
//
// PARAMETERS: none
//
// RETURNS: damping value
//
//----------------------------------------------------------

float EAX_GetDamping()
{
   float damping=CREVERB_INVALID_VALUE;
   unsigned long r;

   API_lock();

   lpPropertySet->Get(EAXPROP,
#ifdef EAX2
                      DSPROPERTY_EAXLISTENER_DECAYHFRATIO,
#else
                      DSPROPERTY_EAX_DAMPING,
#endif
                      NULL,0,
                     &damping,sizeof(float),&r);

   API_unlock();

   return damping;
}

//----------------------------------------------------------
// void SetReverbMix(float mix)
//
// DESCRIPTION: Changes the reverb mix value
//
// PARAMETERS: mix value
//
// RETURNS: no return value.
//
//----------------------------------------------------------

void EAX_SetReverbMix(SAMPLE3D FAR *S, float mix)
{
   API_lock();

#ifdef EAX2
   if (fabs(mix+1.0F)<EPSILON)
     mix=1.0F;

   LONG vol=(LONG)linear_to_dB(mix);
   S->lpPropertySet->Set(EAXBUFPROP,
                         DSPROPERTY_EAXBUFFER_ROOMHF,
                         NULL,
                         0,
                        &vol,
                         sizeof(LONG));

   S->lpPropertySet->Set(EAXBUFPROP,
                         DSPROPERTY_EAXBUFFER_ROOM,
                         NULL,
                         0,
                        &vol,
                         sizeof(LONG));
#else
   S->lpPropertySet->Set(EAXBUFPROP,
                         DSPROPERTY_EAXBUFFER_REVERBMIX,
                         NULL,
                         0,
                        &mix,
                         sizeof(float));
#endif

   API_unlock();
}

//----------------------------------------------------------
// float GetReverbMix()
//
// DESCRIPTION: Gets the current reverb mix
//
// PARAMETERS: none
//
// RETURNS: mix value
//
//----------------------------------------------------------

float EAX_GetReverbMix(SAMPLE3D FAR *S)
{
   float mix=CREVERBBUFFER_INVALID_VALUE;
   unsigned long r;

   API_lock();

#ifdef EAX2
   LONG room=20000;
   S->lpPropertySet->Get(EAXBUFPROP,
                         DSPROPERTY_EAXBUFFER_ROOM,
                         NULL,
                         0,
                        &room,
                         sizeof(LONG),
                         &r);
   if (room!=20000)
     mix=dB_to_linear((F32)room);

#else
   S->lpPropertySet->Get(EAXBUFPROP,
                         DSPROPERTY_EAXBUFFER_REVERBMIX,
                         NULL,
                         0,
                        &mix,
                         sizeof(float),
                         &r);
#endif

   API_unlock();

   return mix;
}

//############################################################################
//#                                                                          #
//# Destroy EAX-specific objects                                             #
//#                                                                          #
//############################################################################

void EAX_destroy(void)
{
   API_lock();

   if (lpSecondaryBuffer != NULL)
      {
      lpSecondaryBuffer->Release();
      lpSecondaryBuffer = NULL;
      }

   if (lpDs3dBuffer != NULL)
      {
      lpDs3dBuffer->Release();
      lpDs3dBuffer = NULL;
      }

   if (lpPropertySet != NULL)
      {
      lpPropertySet->Release();
      lpPropertySet = NULL;
      }

   API_unlock();
}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_room_type                                                       #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_room_type (S32 EAX_room_type)
{
   room_type=EAX_room_type;
   if (lpPropertySet)
     EAX_SetEnvironment(EAX_room_type);
}

//##############################################################################
//#                                                                            #
//# M3D_3D_room_type                                                           #
//#                                                                            #
//##############################################################################

S32       AILEXPORT M3D_3D_room_type (void)
{
   return room_type;
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

   return EAX_GetReverbMix(S);
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

   //
   // Assume the argument is zero or positive, and in decibels.
   // Convert it to a ratio suitable for passing
   //

   if (S->lpds3db != NULL)
   {
     EAX_SetReverbMix(S,effects_level);
   }
}

#endif

static DWORD RAD_to_DS(S32 st)
{
  switch (st)
  {
    case AIL_3D_HEADPHONE:
      return(DSSPEAKER_HEADPHONE);
    case AIL_3D_SURROUND:
      return(DSSPEAKER_SURROUND);
    case AIL_3D_4_SPEAKER:
      return(DSSPEAKER_QUAD);
    case AIL_3D_2_SPEAKER:
    default:
      return(DSSPEAKER_STEREO);
  }

}

//##############################################################################
//#                                                                            #
//# M3D_set_3D_speaker_type                                                    #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_speaker_type (S32 spk_type)
{
  API_lock();

  speaker_type=spk_type;

  lpDS->SetSpeakerConfig( RAD_to_DS(spk_type) );

  API_unlock();
}

//##############################################################################
//#                                                                            #
//# M3D_3D_speaker_type                                                        #
//#                                                                            #
//##############################################################################

S32       AILEXPORT M3D_3D_speaker_type (void)
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

  if (lp3DLISTENER)
  {
    lp3DLISTENER->SetRolloffFactor( factor, DS3D_IMMEDIATE);
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

   if (lp3DLISTENER)
   {
     lp3DLISTENER->GetRolloffFactor( &result );
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

  if (lp3DLISTENER)
  {
    lp3DLISTENER->SetDopplerFactor( factor, DS3D_IMMEDIATE);
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

   if (lp3DLISTENER)
   {
     lp3DLISTENER->GetDopplerFactor( &result );
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

  if (lp3DLISTENER)
  {
    lp3DLISTENER->SetDistanceFactor( factor, DS3D_IMMEDIATE);
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

   if (lp3DLISTENER)
   {
     lp3DLISTENER->GetDistanceFactor( &result );
   }

   API_unlock();

   return( result );
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
//# M3D_3D_sample_cone                                                         #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_3D_sample_cone      (H3DSAMPLE samp,
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
//#                                                                            #
//# M3D_set_3D_sample_cone                                                     #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_set_3D_sample_cone     (H3DSAMPLE samp, //)
                                                 F32       inner_angle,
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

   if (S->lpds3db != NULL)
   {
      DS_set_cone(S);
   }
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

   if (S->lpds3db != NULL)
   {
#ifdef EAX2
     EAX2_Sample_Set_int(S,DSPROPERTY_EAXBUFFER_OBSTRUCTION,(S32)linear_to_dB(1.0F-obstruction));
#else
     DS_set_volume(S);
#endif
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

   if (S->lpds3db != NULL)
   {
#ifdef EAX2
     EAX2_Sample_Set_int(S,DSPROPERTY_EAXBUFFER_OCCLUSION,(S32)linear_to_dB(1.0F-occlusion));
#else
     DS_set_volume(S);
#endif
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

//##############################################################################
//#                                                                            #
//# M3D_set_3D_EOS                                                             #
//#                                                                            #
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
//#                                                                          #
//# Destroy system-level emitter object(s) associated with sample            #
//#                                                                          #
//############################################################################

void destroy_sample_emitter(SAMPLE3D FAR *S)
{
   API_lock();

   if (S->lpds3db != NULL)
      {
      S->lpds3db->Release();
      S->lpds3db = NULL;
      }

   if (S->lpdsb != NULL)
      {
      S->lpdsb->Release();
      S->lpdsb = NULL;
      }

#ifdef EAX3D
   if (S->lpPropertySet != NULL)
      {
      S->lpPropertySet->Release();
      S->lpPropertySet = NULL;
      }
#endif

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

   S->previous_position = (U32) -1;
}

//############################################################################
//#                                                                          #
//# Cancel all pending events and return as soon as no events are being      #
//# processed                                                                #
//#                                                                          #
//############################################################################

void sleep_sample(SAMPLE3D FAR *S)
{
   incnoints(S);

   AIL_unlock_mutex();

   while(S->noints!=1)
   {
     Sleep(1);
   }
   
   AIL_lock_mutex();

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
//# Callback function to simulate IDirectSoundNotify interface (broken on    #
//# some drivers / DirectSound3D providers)                                  #
//#                                                                          #
//############################################################################

void AILCALLBACK notify_timer(U32 user)
{
   S32 i;
   S32 docb;

   for (i=0; i < avail_samples; i++)
   {
      SAMPLE3D FAR *S = &samples[i];
      
      incnoints(S);

      if (S->noints!=1)
      {
        decnoints(S);
        continue;
      }

      if (((S->status&255) != SMP_PLAYING) || (S->lpdsb == NULL))
      {
         S->previous_position = (U32) -1;
            decnoints(S);
         continue;
      }

      docb=0;

      API_lock();

      U32 p,w;

      if (!(SUCCEEDED (S->lpdsb->GetCurrentPosition(&p,&w))))
      {
         API_unlock();

         S->previous_position = (U32) -1;
         decnoints(S);
         continue;
      }

      API_unlock();

      if (S->previous_position != (U32) -1)
      {
         if (p < S->previous_position)
         {
            //
            // Cursor has wrapped past end of buffer
            //

            diag_printf("%X BUFFER 2\n",S);

            if ((S->status&255) == SMP_PLAYING)
            {
               docb|=DS3D_stream_to_buffer(S, 1, BUFF_SIZE);
            }
         }
         else if ((p > BUFF_SIZE) && (S->previous_position <= BUFF_SIZE))
         {
            //
            // Cursor has moved into second half of buffer since previous
            // call
            //

            diag_printf("%X BUFFER 1\n",S);

            if ((S->status&255) == SMP_PLAYING)
            {
               docb|=DS3D_stream_to_buffer(S, 0, BUFF_SIZE);
            }
         }
      }

      S->previous_position = p;

      decnoints(S);

      if (S->eos)
      {
        if (docb)
          S->eos(S->clientH3D);
        else
        {
          if ((S->status==SMP_PLAYING) && (S->pos) && (S->pos == S->len))
            if (timeGetTime()>=S->lastblockdone)
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

U32 AILEXPORT PROVIDER_query_attribute (HATTRIB index)
{
   switch ((ATTRIB) index)
      {
      case PROVIDER_NAME:    return (U32) PROVIDER_NAME_STR;
      case PROVIDER_VERSION:  return 0x102;

      case MAX_SUPPORTED_SAMPLES: return avail_samples;

#ifdef EAX3D

      case EAX_ENVIRONMENT:   if (!lpPropertySet) return 0; else return (U32) EAX_GetEnvironment();
      case EAX_EFFECT_VOLUME: if (!lpPropertySet) return 0; else return (U32) float_as_long(EAX_GetVolume());
      case EAX_DECAY_TIME:    if (!lpPropertySet) return 0; else return (U32) float_as_long(EAX_GetDecayTime());
      case EAX_DAMPING:       if (!lpPropertySet) return 0; else return (U32) float_as_long(EAX_GetDamping());

#ifdef EAX2
      case EAX_ENVIRONMENT_SIZE:      return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_Get_float(DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE)));
      case EAX_ENVIRONMENT_DIFFUSION: return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_Get_float(DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION)));
      case EAX_ROOM:                  return( (!lpPropertySet)? 0: EAX2_Get_int(DSPROPERTY_EAXLISTENER_ROOM));
      case EAX_ROOM_HF:               return( (!lpPropertySet)? 0: EAX2_Get_int(DSPROPERTY_EAXLISTENER_ROOMHF));
      case EAX_DECAY_HF_RATIO:        return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_Get_float(DSPROPERTY_EAXLISTENER_DECAYHFRATIO)));
      case EAX_REFLECTIONS:           return( (!lpPropertySet)? 0: EAX2_Get_int(DSPROPERTY_EAXLISTENER_REFLECTIONS));
      case EAX_REFLECTIONS_DELAY:     return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_Get_float(DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY)));
      case EAX_REVERB:                return( (!lpPropertySet)? 0: EAX2_Get_int(DSPROPERTY_EAXLISTENER_REVERB));
      case EAX_REVERB_DELAY:          return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_Get_float(DSPROPERTY_EAXLISTENER_REVERBDELAY)));
      case EAX_ROOM_ROLLOFF:          return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_Get_float(DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR)));
      case EAX_AIR_ABSORPTION:        return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_Get_float(DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF)));
      case EAX_FLAGS:                 return( (!lpPropertySet)? 0: EAX2_Get_int(DSPROPERTY_EAXLISTENER_FLAGS));
#endif
#endif
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

#ifdef EAX3D

   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //

      case EAX_ENVIRONMENT:

         if (!lpPropertySet)
            {
            return 0;
            }
         else
            {
            prev = (S32) EAX_GetEnvironment();
            EAX_SetEnvironment(*(U32 FAR*)value);
            }
         break;

      case EAX_EFFECT_VOLUME:

         if (!lpPropertySet)
            {
            return 0;
            }
         else
            {
            prev = (S32) float_as_long(EAX_GetVolume());
            EAX_SetVolume(*(F32 FAR*)value);
            }
         break;

      case EAX_DECAY_TIME:

         if (!lpPropertySet)
            {
            return 0;
            }
         else
            {
            prev = (S32) float_as_long(EAX_GetDecayTime());
            EAX_SetDecayTime(*(F32 FAR*)value);
            }
         break;

      case EAX_DAMPING:

         if (!lpPropertySet)
            {
            return 0;
            }
         else
            {
            prev = (S32) float_as_long(EAX_GetDamping());
            EAX_SetDamping(*(F32 FAR*)value);
            }
         break;

#ifdef EAX2
      case EAX_ENVIRONMENT_SIZE:      return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_GetSet_float(DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE,*(F32 FAR*)value)));
      case EAX_ENVIRONMENT_DIFFUSION: return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_GetSet_float(DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION,*(F32 FAR*)value)));
      case EAX_ROOM:                  return( (!lpPropertySet)? 0: EAX2_GetSet_int(DSPROPERTY_EAXLISTENER_ROOM,*(U32 FAR*)value));
      case EAX_ROOM_HF:               return( (!lpPropertySet)? 0: EAX2_GetSet_int(DSPROPERTY_EAXLISTENER_ROOMHF,*(U32 FAR*)value));
      case EAX_DECAY_HF_RATIO:        return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_GetSet_float(DSPROPERTY_EAXLISTENER_DECAYHFRATIO,*(F32 FAR*)value)));
      case EAX_REFLECTIONS:           return( (!lpPropertySet)? 0: EAX2_GetSet_int(DSPROPERTY_EAXLISTENER_REFLECTIONS,*(U32 FAR*)value));
      case EAX_REFLECTIONS_DELAY:     return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_GetSet_float(DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY,*(F32 FAR*)value)));
      case EAX_REVERB:                return( (!lpPropertySet)? 0: EAX2_GetSet_int(DSPROPERTY_EAXLISTENER_REVERB,*(U32 FAR*)value));
      case EAX_REVERB_DELAY:          return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_GetSet_float(DSPROPERTY_EAXLISTENER_REVERBDELAY,*(F32 FAR*)value)));
      case EAX_ROOM_ROLLOFF:          return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_GetSet_float(DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR,*(F32 FAR*)value)));
      case EAX_AIR_ABSORPTION:        return( (!lpPropertySet)? 0: (U32) float_as_long(EAX2_GetSet_float(DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF,*(F32 FAR*)value)));
      case EAX_FLAGS:                 return( (!lpPropertySet)? 0: EAX2_GetSet_int(DSPROPERTY_EAXLISTENER_FLAGS,*(U32 FAR*)value));
#endif
      }

#endif

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

   sleep_sample(S);

   DS3D_stream_to_buffer(S, 0, BUFF_SIZE);
   DS3D_stream_to_buffer(S, 1, BUFF_SIZE);

   DS_start_secondary(S);

   S->previous_position = -1;

   S->status = SMP_PLAYING;

   wake_sample(S);
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

   sleep_sample(S);

   S->status = SMP_STOPPED;

   flush_sample(S);

   wake_sample(S);

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
   
   sleep_sample(S);

   DS3D_stream_to_buffer(S, 0, BUFF_SIZE);
   DS3D_stream_to_buffer(S, 1, BUFF_SIZE);

   DS_start_secondary(S);

   S->previous_position = -1;

   S->status = SMP_PLAYING;

   wake_sample(S);
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

   DS_set_volume(S);
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
   DS_set_frequency(S);
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

   if (S->lpds3db != NULL)
      {
      API_lock();

      S->lpds3db->SetMaxDistance( max_dist, DS3D_DEFERRED);
      S->lpds3db->SetMinDistance( min_dist, DS3D_DEFERRED);

      lp3DLISTENER->CommitDeferredSettings();

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

#ifdef EAX3D

   switch ((ATTRIB) index)
      {
      case EAX_EFFECT_REVERBMIX: if (!lpPropertySet) return 0; else return (U32) float_as_long(EAX_GetReverbMix(S));

#ifdef EAX2
      case EAX_SAMPLE_DIRECT:               return( (!lpPropertySet) ? 0: EAX2_Sample_Get_int(S,DSPROPERTY_EAXBUFFER_DIRECT));
      case EAX_SAMPLE_DIRECT_HF:            return( (!lpPropertySet) ? 0: EAX2_Sample_Get_int(S,DSPROPERTY_EAXBUFFER_DIRECTHF));
      case EAX_SAMPLE_ROOM:                 return( (!lpPropertySet) ? 0: EAX2_Sample_Get_int(S,DSPROPERTY_EAXBUFFER_ROOM));
      case EAX_SAMPLE_ROOM_HF:              return( (!lpPropertySet) ? 0: EAX2_Sample_Get_int(S,DSPROPERTY_EAXBUFFER_ROOMHF));
      case EAX_SAMPLE_OBSTRUCTION:          return( (!lpPropertySet) ? 0: EAX2_Sample_Get_int(S,DSPROPERTY_EAXBUFFER_OBSTRUCTION));
      case EAX_SAMPLE_OBSTRUCTION_LF_RATIO: return( (!lpPropertySet) ? 0: (U32) float_as_long(EAX2_Sample_Get_float(S,DSPROPERTY_EAXBUFFER_OBSTRUCTIONLFRATIO)));
      case EAX_SAMPLE_OCCLUSION:            return( (!lpPropertySet) ? 0: EAX2_Sample_Get_int(S,DSPROPERTY_EAXBUFFER_OCCLUSION));
      case EAX_SAMPLE_OCCLUSION_LF_RATIO:   return( (!lpPropertySet) ? 0: (U32) float_as_long(EAX2_Sample_Get_float(S,DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO)));
      case EAX_SAMPLE_OCCLUSION_ROOM_RATIO: return( (!lpPropertySet) ? 0: (U32) float_as_long(EAX2_Sample_Get_float(S,DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO)));
      case EAX_SAMPLE_ROOM_ROLLOFF:         return( (!lpPropertySet) ? 0: (U32) float_as_long(EAX2_Sample_Get_float(S,DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR)));
      case EAX_SAMPLE_AIR_ABSORPTION:       return( (!lpPropertySet) ? 0: (U32) float_as_long(EAX2_Sample_Get_float(S,DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR)));
      case EAX_SAMPLE_OUTSIDE_VOLUME_HF:    return( (!lpPropertySet) ? 0: EAX2_Sample_Get_int(S,DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF));
      case EAX_SAMPLE_FLAGS:                return( (!lpPropertySet) ? 0: EAX2_Sample_Get_int(S,DSPROPERTY_EAXBUFFER_FLAGS));
#endif

      }

#endif

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

#ifdef EAX3D

   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //

      case EAX_EFFECT_REVERBMIX:

         if (!S->lpPropertySet)
            {
            return 0;
            }
         else
            {
            prev = (S32) float_as_long(EAX_GetReverbMix(S));
            EAX_SetReverbMix(S,*(F32 FAR*)value);
            }
         break;

#ifdef EAX2
      case EAX_SAMPLE_DIRECT:               return( (!lpPropertySet) ? 0: EAX2_Sample_GetSet_int(S,DSPROPERTY_EAXBUFFER_DIRECT,*(U32 FAR*)value));
      case EAX_SAMPLE_DIRECT_HF:            return( (!lpPropertySet) ? 0: EAX2_Sample_GetSet_int(S,DSPROPERTY_EAXBUFFER_DIRECTHF,*(U32 FAR*)value));
      case EAX_SAMPLE_ROOM:                 return( (!lpPropertySet) ? 0: EAX2_Sample_GetSet_int(S,DSPROPERTY_EAXBUFFER_ROOM,*(U32 FAR*)value));
      case EAX_SAMPLE_ROOM_HF:              return( (!lpPropertySet) ? 0: EAX2_Sample_GetSet_int(S,DSPROPERTY_EAXBUFFER_ROOMHF,*(U32 FAR*)value));
      case EAX_SAMPLE_OBSTRUCTION:          return( (!lpPropertySet) ? 0: EAX2_Sample_GetSet_int(S,DSPROPERTY_EAXBUFFER_OBSTRUCTION,*(U32 FAR*)value));
      case EAX_SAMPLE_OBSTRUCTION_LF_RATIO: return( (!lpPropertySet) ? 0: (U32) float_as_long(EAX2_Sample_GetSet_float(S,DSPROPERTY_EAXBUFFER_OBSTRUCTIONLFRATIO,*(F32 FAR*)value)));
      case EAX_SAMPLE_OCCLUSION:            return( (!lpPropertySet) ? 0: EAX2_Sample_GetSet_int(S,DSPROPERTY_EAXBUFFER_OCCLUSION,*(U32 FAR*)value));
      case EAX_SAMPLE_OCCLUSION_LF_RATIO:   return( (!lpPropertySet) ? 0: (U32) float_as_long(EAX2_Sample_GetSet_float(S,DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO,*(F32 FAR*)value)));
      case EAX_SAMPLE_OCCLUSION_ROOM_RATIO: return( (!lpPropertySet) ? 0: (U32) float_as_long(EAX2_Sample_GetSet_float(S,DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO,*(F32 FAR*)value)));
      case EAX_SAMPLE_ROOM_ROLLOFF:         return( (!lpPropertySet) ? 0: (U32) float_as_long(EAX2_Sample_GetSet_float(S,DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR,*(F32 FAR*)value)));
      case EAX_SAMPLE_AIR_ABSORPTION:       return( (!lpPropertySet) ? 0: (U32) float_as_long(EAX2_Sample_GetSet_float(S,DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR,*(F32 FAR*)value)));
      case EAX_SAMPLE_OUTSIDE_VOLUME_HF:    return( (!lpPropertySet) ? 0: EAX2_Sample_GetSet_int(S,DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF,*(U32 FAR*)value));
      case EAX_SAMPLE_FLAGS:                return( (!lpPropertySet) ? 0: EAX2_Sample_GetSet_int(S,DSPROPERTY_EAXBUFFER_FLAGS,*(U32 FAR*)value));
#endif

      }

#endif

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

      if (S->lpds3db != NULL)
         {
         API_lock();
         S->lpds3db->SetPosition(X,Y,Z,DS3D_IMMEDIATE);
         API_unlock();
         }
      }
   else if (*t == IS_LISTENER)
      {
      listen_position.x = X;
      listen_position.y = Y;
      listen_position.z = Z;

      API_lock();
      lp3DLISTENER->SetPosition(X,Y,Z,DS3D_IMMEDIATE);
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

      API_lock();
      if (S->lpds3db != NULL)
         {
         S->lpds3db->SetVelocity(dX_per_ms * 1000.0F,
                                 dY_per_ms * 1000.0F,
                                 dZ_per_ms * 1000.0F,
                                 DS3D_IMMEDIATE);
         }
      API_unlock();
      }
   else if (*t == IS_LISTENER)
      {
      listen_velocity.x = dX_per_ms;
      listen_velocity.y = dY_per_ms;
      listen_velocity.z = dZ_per_ms;

      API_lock();
      lp3DLISTENER->SetVelocity(dX_per_ms * 1000.0F,
                                dY_per_ms * 1000.0F,
                                dZ_per_ms * 1000.0F,
                                DS3D_IMMEDIATE);
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

      if (S->lpds3db)
      {
        API_lock();
        S->lpds3db->SetConeOrientation(X_face,
                                       Y_face,
                                       Z_face,
                                       DS3D_IMMEDIATE);
        API_unlock();
      }
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
      lp3DLISTENER->SetOrientation(X_face,
                                   Y_face,
                                   Z_face,
                                   X_up,
                                   Y_up,
                                   Z_up,
                                   DS3D_IMMEDIATE);
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
   // Set up DSBUFFERDESC structure
   //

   DSBUFFERDESC dsbdesc;

   memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));

   dsbdesc.dwSize           =  sizeof(DSBUFFERDESC);

   dsbdesc.dwFlags          =  DSBCAPS_GLOBALFOCUS   |
                               DSBCAPS_CTRL3D |
                               DSBCAPS_GETCURRENTPOSITION2 |
                               DSBCAPS_MUTE3DATMAXDISTANCE |
#ifdef SWARE
                               DSBCAPS_LOCSOFTWARE |
#else
#ifdef HWARE
                               DSBCAPS_LOCHARDWARE |
#else
#ifdef EAX3D
                               DSBCAPS_LOCHARDWARE |
#endif
#endif
#endif
                               DSBCAPS_CTRLVOLUME |
                               DSBCAPS_CTRLFREQUENCY;

#ifdef DX7SN
   dsbdesc.guid3DAlgorithm =DS3DALG_NO_VIRTUALIZATION;
#else
#ifdef DX7SL
   dsbdesc.guid3DAlgorithm =DS3DALG_HRTF_LIGHT;
#else
#ifdef DX7SH
   dsbdesc.guid3DAlgorithm =DS3DALG_HRTF_FULL;
#endif
#endif
#endif

   dsbdesc.dwBufferBytes    =  (BUFF_SIZE * 2);
   dsbdesc.lpwfxFormat      = &wf;

   //
   // Create DirectSound buffer (streaming emitter) object
   //

   S->lpdsb = NULL;

   API_lock();

   if (!(SUCCEEDED (lpDS->CreateSoundBuffer(&dsbdesc,
                                            &S->lpdsb,
                                             NULL))))

      {
      AIL_set_error("Could not create secondary DS buffer");

      API_unlock();
      wake_sample(S);
      return 0;
      }

   API_unlock();

   //
   // Get the DirectSound3D interface for this buffer
   //

   S->lpds3db = NULL;

   API_lock();

   HRESULT result = S->lpdsb->QueryInterface(IID_IDirectSound3DBuffer,
                                 (LPVOID *) &S->lpds3db);

   API_unlock();

   if (!(SUCCEEDED(result)))
      {
      AIL_set_error("Could not obtain IDirectSound3DBuffer interface");
      wake_sample(S);
      return 0;
      }

#ifdef EAX3D

   //
   // Get the sound-source property set for this buffer, and make sure
   // it supports the reverb-mix property
   //

   S->lpPropertySet = NULL;

   API_lock();

   if (FAILED(S->lpds3db->QueryInterface(IID_IKsPropertySet,
                              (void **) &S->lpPropertySet)))
      {
      S->lpPropertySet = NULL;
      }
   else
      {
      U32 support = 0;

#ifdef EAX2
  #define BUFFERPROPSNEEDED DSPROPERTY_EAXBUFFER_ALLPARAMETERS
#else
  #define BUFFERPROPSNEEDED DSPROPERTY_EAXBUFFER_REVERBMIX
#endif

      if (FAILED(S->lpPropertySet->QuerySupport(EAXBUFPROP,
                                                BUFFERPROPSNEEDED,
                                               &support))
                 ||
                 ((support & CREVERBBUFFER_SETGET) != CREVERBBUFFER_SETGET))
         {
         S->lpPropertySet->Release();
         S->lpPropertySet = NULL;
         }
      }

   API_unlock();

#endif

   //
   // Set emitter distances
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

   //
   // Clear all special attenuation effects
   //
   M3D_set_3D_sample_obstruction  (samp, 0.0F);
   M3D_set_3D_sample_occlusion    (samp, 0.0F);

#ifdef EAX3D
   M3D_set_3D_sample_effects_level(samp, 0.0F);
#endif

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

#ifdef AUREAL
  static HMODULE a3dlib=0;
  static LPVOID CREATEADDRESS=0;
  static S32 saved=0;
  static DWORD oldscreen=0;
  static DWORD oldaudio=0;
#else
  #define CREATEADDRESS 0
#endif

//##############################################################################
//#                                                                            #
//# M3D_activate                                                               #
//#                                                                            #
//##############################################################################

M3DRESULT AILEXPORT M3D_activate                 (S32 enable)
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
      // Restart DirectSound with 3D audio enabled
      //

#ifdef AUREAL

      AIL_unlock_mutex();
      if (a3dlib==0)
        a3dlib=LoadLibrary("a3d");
      AIL_lock_mutex();
      if (((U32)a3dlib)>=32)
        CREATEADDRESS=GetProcAddress(a3dlib,"_A3dCreate@12");
      if (CREATEADDRESS==0) {
        AIL_set_error("The A3D library could not be loaded");
        return M3D_INTERNAL_ERR;
      }

      //
      // Are we running under DX5 or greater (use DS3D hardware if so)
      //

      {
        //
        // Get DirectSound object and primary buffer pointers from MSS
        //

        lpDS     = NULL;

        AIL_get_DirectSound_info(          NULL,
           (AILLPDIRECTSOUND FAR *)       &lpDS,
           0);

        if ((lpDS) && (AIL_get_preference(DIG_DS_CREATION_HANDLER)==0))
        {
          DSCaps.dwSize = sizeof(DSCaps);

          API_lock();

          if (SUCCEEDED(lpDS->GetCaps(&DSCaps)))
          {
            if (DSCaps.dwFreeHw3DStreamingBuffers)
            {
              // hardware buffers are available without A3D
              FreeLibrary(a3dlib);

              a3dlib=0;
              CREATEADDRESS=0;
            }
          }

          API_unlock();
        }
      }

      if (CREATEADDRESS)
      {
        // A3D registry key
        #define REG_SETTINGS_KEY           TEXT("Software\\Aureal\\A3D")
        // A3D splash registry values
        #define REG_SETTING_SPLASH_SCREEN  TEXT("SplashScreen")
        #define REG_SETTING_SPLASH_AUDIO   TEXT("SplashAudio")

        // turn off the stupid splash sound and graphic

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
      }

#endif

      if ((AIL_get_preference(DIG_DS_CREATION_HANDLER)!=(S32)CREATEADDRESS) ||
        (!AIL_get_preference(DIG_DS_DSBCAPS_CTRL3D))) {

        AIL_digital_handle_release(NULL);

        AIL_set_preference(DIG_DS_CREATION_HANDLER, (U32)CREATEADDRESS );

        AIL_set_preference(DIG_DS_DSBCAPS_CTRL3D, TRUE);

        AIL_digital_handle_reacquire(NULL);
      }

      //
      // Get DirectSound object and primary buffer pointers from MSS
      //
      // Fail if MSS is not using DirectSound
      //

      lpDS     = NULL;
      lpDSPRIM = NULL;

      AIL_get_DirectSound_info(          NULL,
         (AILLPDIRECTSOUND FAR *)       &lpDS,
         (AILLPDIRECTSOUNDBUFFER FAR *) &lpDSPRIM);

      if (lpDS == NULL)
         {
         AIL_set_error("M3D provider requires AIL_USE_WAVEOUT==NO");
         return M3D_INTERNAL_ERR;
         }

#ifdef HWARE
      //
      // Get # of buffers available in hardware, and set our sample-
      // allocation limit to this value
      //

      DSCaps.dwSize = sizeof(DSCaps);

      API_lock();

      if (!(SUCCEEDED(lpDS->GetCaps(&DSCaps))))
         {
         API_unlock();
         AIL_set_error("lpDS->GetCaps() failed");
         return M3D_INTERNAL_ERR;
         }

      API_unlock();

      avail_samples = max( DSCaps.dwMaxHw3DAllBuffers , DSCaps.dwFreeHw3DAllBuffers);

      avail_samples = min(N_SAMPLES, avail_samples);

#ifdef EAX3D
      //
      // EAX: Subtract one secondary buffer for our internal use
      //

      if (avail_samples)
         {
         --avail_samples;
         }
#endif

#else
      avail_samples = N_SAMPLES;
#endif

      //
      // Bomb out if no samples available
      //

      if (!avail_samples)
         {
         AIL_set_error("Provider could not be initialized -- no 3D voices available");
         return M3D_NOT_INIT;
         }

      //
      // Create listener object
      //

      API_lock();

      if (FAILED(lpDSPRIM->QueryInterface(IID_IDirectSound3DListener,
                                          (LPVOID *) &lp3DLISTENER)))
         {
         AIL_set_error("Failed to create DS3D listener object");

         API_unlock();
         return M3D_INTERNAL_ERR;
         }

      API_unlock();

#ifdef DX7
      API_lock();

      LPDIRECTSOUNDBUFFER lpSecondaryBuffer = NULL;

      WAVEFORMATEX   pcmOut;           // Format of the wave for secondary buffer if we need to make one.
      DSBUFFERDESC   dsbdSecondary;    // description for creating secondary buffer if we need to make one.

      // we don't have a secondary to work with so we will create one.
      ZeroMemory( &dsbdSecondary, sizeof(DSBUFFERDESC));
      ZeroMemory( &pcmOut, sizeof(WAVEFORMATEX));
      // any format should do I just say 11kHz 16 bit mono
      pcmOut.wFormatTag = WAVE_FORMAT_PCM;
      pcmOut.nChannels = 1;
      pcmOut.nSamplesPerSec = 11025;
      pcmOut.nAvgBytesPerSec = 22050;
      pcmOut.nBlockAlign = 2;
      pcmOut.wBitsPerSample = 16;
      pcmOut.cbSize = 0;
      // size is just arbitary but not too small as I have seen problems with single sample buffers...
      dsbdSecondary.dwSize = sizeof(DSBUFFERDESC);
      dsbdSecondary.dwBufferBytes = 1024;
      dsbdSecondary.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_STATIC;
      dsbdSecondary.lpwfxFormat = &pcmOut;
#ifdef DX7SN
      dsbdSecondary.guid3DAlgorithm =DS3DALG_NO_VIRTUALIZATION;
#else
#ifdef DX7SL
      dsbdSecondary.guid3DAlgorithm =DS3DALG_HRTF_LIGHT;
#else
#ifdef DX7SH
      dsbdSecondary.guid3DAlgorithm =DS3DALG_HRTF_FULL;
#endif
#endif
#endif

      if (lpDS->CreateSoundBuffer(&dsbdSecondary, &lpSecondaryBuffer, NULL) != DS_OK)
      {
         if (lp3DLISTENER)
            {
            lp3DLISTENER->Release();
            lp3DLISTENER = NULL;
            }

         API_unlock();

         AIL_set_error("Unable to initialize DirectSound 3D 7+ support");
         return M3D_NOT_INIT;
      }

      // now just release it
      lpSecondaryBuffer->Release();
      lpSecondaryBuffer = NULL;

      API_unlock();
#endif

#ifdef EAX3D

      //
      // EAX: Create dummy secondary buffer to support global reverb object,
      // and get property set
      //
      // (code from Creative's EAXTEST.CPP and .PDF file)
      //

      API_lock();

      lpSecondaryBuffer = NULL;
      lpDs3dBuffer      = NULL;
      lpPropertySet     = NULL;

      WAVEFORMATEX   pcmOut;           // Format of the wave for secondary buffer if we need to make one.
      DSBUFFERDESC   dsbdSecondary;    // description for creating secondary buffer if we need to make one.

      // we don't have a secondary to work with so we will create one.
      ZeroMemory( &dsbdSecondary, sizeof(DSBUFFERDESC));
      ZeroMemory( &pcmOut, sizeof(WAVEFORMATEX));
      // any format should do I just say 11kHz 16 bit mono
      pcmOut.wFormatTag = WAVE_FORMAT_PCM;
      pcmOut.nChannels = 1;
      pcmOut.nSamplesPerSec = 11025;
      pcmOut.nAvgBytesPerSec = 22050;
      pcmOut.nBlockAlign = 2;
      pcmOut.wBitsPerSample = 16;
      pcmOut.cbSize = 0;
      // size is just arbitary but not too small as I have seen problems with single sample buffers...
      dsbdSecondary.dwSize = sizeof(DSBUFFERDESC);
      dsbdSecondary.dwBufferBytes = 1024;
      dsbdSecondary.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_STATIC;
      dsbdSecondary.lpwfxFormat = &pcmOut;

      U32 support = 0;

      if ((lpDS->CreateSoundBuffer(&dsbdSecondary, &lpSecondaryBuffer, NULL) != DS_OK)
          ||
          (lpSecondaryBuffer->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID *) &lpDs3dBuffer) != DS_OK)
          ||
          (lpDs3dBuffer->QueryInterface(IID_IKsPropertySet, (LPVOID *) &lpPropertySet) != DS_OK)
          ||
          (lpPropertySet->QuerySupport(EAXPROP,
#ifdef EAX2
            DSPROPERTY_EAXLISTENER_ALLPARAMETERS,
#else
            DSPROPERTY_EAX_ALL,
#endif
            &support) != DS_OK)
          ||
          ((support & PSET_SETGET) != PSET_SETGET))
         {
         EAX_destroy();

         if (lp3DLISTENER)
            {
            lp3DLISTENER->Release();
            lp3DLISTENER = NULL;
            }

         API_unlock();

#ifdef EAX2
         AIL_set_error("Unable to initialize EAX 2 support");
#else
         AIL_set_error("Unable to initialize EAX support");
#endif
         return M3D_NOT_INIT;
         }

      API_unlock();

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

      S32 i;

      for (i=0; i < avail_samples; i++)
         {
         SAMPLE3D FAR *S = &samples[i];

         memset(S,
                    0,
                    sizeof(struct SAMPLE3D));

         S->index  = i;
         S->status = SMP_FREE;

         S->noints = 0;

         //
         // Clear sample emitter
         //

         S->lpdsb   = NULL;
         S->lpds3db = NULL;

#ifdef EAX3D
         S->lpPropertySet = NULL;
#endif
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


#ifdef EAX3D
      M3D_set_3D_room_type( EAX_ENVIRONMENT_GENERIC );
#endif
      M3D_set_3D_speaker_type( AIL_3D_2_SPEAKER );

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

         reset_sample_voice(S);

         //
         // Mark handle free
         //

         S->status = SMP_FREE;

         //
         // Tell the sample's thread to kill itself, and wait for it to
         // happen
         //

         //
         // Destroy emitter associated with sample
         //

         destroy_sample_emitter(S);
         }

       //
       // Release EAX objects
       //

#ifdef EAX3D
      EAX_destroy();
#endif

      //
      // Release the listener
      //

      if (lp3DLISTENER)
         {
         API_lock();
         lp3DLISTENER->Release();
         lp3DLISTENER = NULL;
         API_unlock();
         }

      active = 0;

#ifdef AUREAL
      if (AIL_get_preference(DIG_DS_CREATION_HANDLER))
      {

        AIL_digital_handle_release(NULL);

        // turn off A3D, but leave 3D mode on
        AIL_set_preference(DIG_DS_CREATION_HANDLER, 0 );

        AIL_set_preference(DIG_DS_DSBCAPS_CTRL3D, TRUE);

        AIL_digital_handle_reacquire(NULL);
      }

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

      if (a3dlib)
      {
        AIL_lock();
        AIL_unlock_mutex();
        FreeLibrary(a3dlib);
        AIL_lock_mutex();
        AIL_unlock();
        a3dlib=0;
      }

#endif

   }

   return M3D_NOERR;
}

//############################################################################
//#                                                                          #
//# DLLMain registers M3D API interface at load time                         #
//#                                                                          #
//############################################################################

#ifdef IS_WIN32

BOOL WINAPI DllMain(HINSTANCE hinstDll, //)
                          DWORD     fdwReason,
                          LPVOID    plvReserved)

#else

#ifdef IS_WIN16

S32 RSX_register(S32 fdwReason)
#define DLL_PROCESS_ATTACH TRUE
#define DLL_PROCESS_DETACH FALSE

#else

//
// Manual registration for DOS platform
//
// Application must supply the provider handle explicitly
//

S32 AILCALL RSX_register(S32 fdwReason, HPROVIDER provider_handle)
#define DLL_PROCESS_ATTACH TRUE
#define DLL_PROCESS_DETACH FALSE

#endif
#endif
{
   const RIB_INTERFACE_ENTRY M3DSAMPLE[] =
      {
#ifdef EAX3D
      REG_AT("EAX sample reverb mix",      EAX_EFFECT_REVERBMIX,  RIB_FLOAT),
      REG_PR("EAX sample reverb mix",      EAX_EFFECT_REVERBMIX,  RIB_FLOAT),

#ifdef EAX2
      REG_AT("EAX2 sample direct",               EAX_SAMPLE_DIRECT,               RIB_DEC  ),
      REG_AT("EAX2 sample direct HF",            EAX_SAMPLE_DIRECT_HF,            RIB_DEC  ),
      REG_AT("EAX2 sample room",                 EAX_SAMPLE_ROOM,                 RIB_DEC  ),
      REG_AT("EAX2 sample room HF",              EAX_SAMPLE_ROOM_HF,              RIB_DEC  ),
      REG_AT("EAX2 sample obstruction",          EAX_SAMPLE_OBSTRUCTION,          RIB_DEC  ),
      REG_AT("EAX2 sample obstruction LF ratio", EAX_SAMPLE_OBSTRUCTION_LF_RATIO, RIB_FLOAT),
      REG_AT("EAX2 sample occlusion",            EAX_SAMPLE_OCCLUSION,            RIB_DEC  ),
      REG_AT("EAX2 sample occlusion LF ratio",   EAX_SAMPLE_OCCLUSION_LF_RATIO,   RIB_FLOAT),
      REG_AT("EAX2 sample occlusion room ratio", EAX_SAMPLE_OCCLUSION_ROOM_RATIO, RIB_FLOAT),
      REG_AT("EAX2 sample room rolloff",         EAX_SAMPLE_ROOM_ROLLOFF,         RIB_FLOAT),
      REG_AT("EAX2 sample air absorption",       EAX_SAMPLE_AIR_ABSORPTION,       RIB_FLOAT),
      REG_AT("EAX2 sample outside volume HF",    EAX_SAMPLE_OUTSIDE_VOLUME_HF,    RIB_DEC  ),
      REG_AT("EAX2 sample flags",                EAX_SAMPLE_FLAGS,                RIB_DEC  ),

      REG_PR("EAX2 sample direct",               EAX_SAMPLE_DIRECT,               RIB_DEC  ),
      REG_PR("EAX2 sample direct HF",            EAX_SAMPLE_DIRECT_HF,            RIB_DEC  ),
      REG_PR("EAX2 sample room",                 EAX_SAMPLE_ROOM,                 RIB_DEC  ),
      REG_PR("EAX2 sample room HF",              EAX_SAMPLE_ROOM_HF,              RIB_DEC  ),
      REG_PR("EAX2 sample obstruction",          EAX_SAMPLE_OBSTRUCTION,          RIB_DEC  ),
      REG_PR("EAX2 sample obstruction LF ratio", EAX_SAMPLE_OBSTRUCTION_LF_RATIO, RIB_FLOAT),
      REG_PR("EAX2 sample occlusion",            EAX_SAMPLE_OCCLUSION,            RIB_DEC  ),
      REG_PR("EAX2 sample occlusion LF ratio",   EAX_SAMPLE_OCCLUSION_LF_RATIO,   RIB_FLOAT),
      REG_PR("EAX2 sample occlusion room ratio", EAX_SAMPLE_OCCLUSION_ROOM_RATIO, RIB_FLOAT),
      REG_PR("EAX2 sample room rolloff",         EAX_SAMPLE_ROOM_ROLLOFF,         RIB_FLOAT),
      REG_PR("EAX2 sample air absorption",       EAX_SAMPLE_AIR_ABSORPTION,       RIB_FLOAT),
      REG_PR("EAX2 sample outside volume HF",    EAX_SAMPLE_OUTSIDE_VOLUME_HF,    RIB_DEC  ),
      REG_PR("EAX2 sample flags",                EAX_SAMPLE_FLAGS,                RIB_DEC  ),
#endif

      REG_FN(M3D_set_3D_sample_effects_level),
      REG_FN(M3D_3D_sample_effects_level),
      REG_FN(M3D_set_3D_room_type),
      REG_FN(M3D_3D_room_type),

#endif

      REG_FN(M3D_set_3D_EOS),
      REG_FN(M3D_3D_sample_query_attribute),
      REG_FN(M3D_3D_set_sample_preference),

      REG_FN(M3D_set_3D_speaker_type),
      REG_FN(M3D_3D_speaker_type),

      REG_FN(M3D_set_3D_sample_obstruction),
      REG_FN(M3D_3D_sample_obstruction),
      REG_FN(M3D_set_3D_sample_occlusion),
      REG_FN(M3D_3D_sample_occlusion),

      REG_FN(M3D_set_3D_sample_cone),
      REG_FN(M3D_3D_sample_cone),

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

#ifdef EAX3D
      REG_AT("EAX environment selection",  EAX_ENVIRONMENT,       RIB_DEC),
      REG_AT("EAX effect volume",          EAX_EFFECT_VOLUME,     RIB_FLOAT),
      REG_AT("EAX decay time",             EAX_DECAY_TIME,        RIB_FLOAT),
      REG_AT("EAX damping",                EAX_DAMPING,           RIB_FLOAT),

      REG_PR("EAX environment selection",  EAX_ENVIRONMENT,       RIB_DEC),
      REG_PR("EAX effect volume",          EAX_EFFECT_VOLUME,     RIB_FLOAT),
      REG_PR("EAX decay time",             EAX_DECAY_TIME,        RIB_FLOAT),
      REG_PR("EAX damping",                EAX_DAMPING,           RIB_FLOAT),

#ifdef EAX2
      REG_AT("EAX2 environment size",       EAX_ENVIRONMENT_SIZE,       RIB_FLOAT),
      REG_AT("EAX2 environment diffusion",  EAX_ENVIRONMENT_DIFFUSION,  RIB_FLOAT),
      REG_AT("EAX2 room",                   EAX_ROOM,                   RIB_DEC),
      REG_AT("EAX2 room HF",                EAX_ROOM_HF,                RIB_DEC),
      REG_AT("EAX2 decay HF ratio",         EAX_DECAY_HF_RATIO,         RIB_FLOAT),
      REG_AT("EAX2 reflections",            EAX_REFLECTIONS,            RIB_DEC),
      REG_AT("EAX2 reflections delay",      EAX_REFLECTIONS_DELAY,      RIB_FLOAT),
      REG_AT("EAX2 reverb",                 EAX_REVERB,                 RIB_DEC),
      REG_AT("EAX2 reverb delay",           EAX_REVERB_DELAY,           RIB_FLOAT),
      REG_AT("EAX2 room rolloff",           EAX_ROOM_ROLLOFF,           RIB_FLOAT),
      REG_AT("EAX2 air absorption",         EAX_AIR_ABSORPTION,         RIB_FLOAT),
      REG_AT("EAX2 flags",                  EAX_FLAGS,                  RIB_DEC),

      REG_PR("EAX2 environment size",       EAX_ENVIRONMENT_SIZE,       RIB_FLOAT),
      REG_PR("EAX2 environment diffusion",  EAX_ENVIRONMENT_DIFFUSION,  RIB_FLOAT),
      REG_PR("EAX2 room",                   EAX_ROOM,                   RIB_DEC),
      REG_PR("EAX2 room HF",                EAX_ROOM_HF,                RIB_DEC),
      REG_PR("EAX2 decay HF ratio",         EAX_DECAY_HF_RATIO,         RIB_FLOAT),
      REG_PR("EAX2 reflections",            EAX_REFLECTIONS,            RIB_DEC),
      REG_PR("EAX2 reflections delay",      EAX_REFLECTIONS_DELAY,      RIB_FLOAT),
      REG_PR("EAX2 reverb",                 EAX_REVERB,                 RIB_DEC),
      REG_PR("EAX2 reverb delay",           EAX_REVERB_DELAY,           RIB_FLOAT),
      REG_PR("EAX2 room rolloff",           EAX_ROOM_ROLLOFF,           RIB_FLOAT),
      REG_PR("EAX2 air absorption",         EAX_AIR_ABSORPTION,         RIB_FLOAT),
      REG_PR("EAX2 flags",                  EAX_FLAGS,                  RIB_DEC),
#endif

      REG_FN(M3D_set_3D_sample_effects_level),
      REG_FN(M3D_3D_sample_effects_level),
      REG_FN(M3D_set_3D_room_type),
      REG_FN(M3D_3D_room_type),
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

   switch (fdwReason)
      {
      case DLL_PROCESS_ATTACH:

         DisableThreadLibraryCalls( hinstDll );
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

         break;

      case DLL_PROCESS_DETACH:

         RIB_unregister_all(self);
         break;
      }

   return TRUE;
}
