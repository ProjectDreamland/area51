//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: M3D module for Intel RSX 3D audio                            ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 25-Aug-98: Initial                                    ##
//##          1.01 of 21-Oct-98: Added sample rate calls                    ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//##  Portions taken from Intel's STREAMEM.CPP example                      ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#define diag_printf //AIL_debug_printf("at %i ",timeGetTime()); AIL_debug_printf

#ifdef STANDALONE
#else
#define INITGUID
#endif

#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <process.h>

#include <objbase.h>
#include "rsx.h"
#include "mss.h"

S32 M3D_started = 0;

C8 M3D_error_text[256];

#ifdef STANDALONE

BOOL WINAPI RSXEntryPoint(HINSTANCE hInstance, ULONG ulReason, LPVOID pv);
STDAPI CreateRSX(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID FAR *ppvObj);
void WINAPI DestroyRSX(void);
LONG WINAPI PrivateRegSet(LPCTSTR lpValueName, LPBYTE lpData);
LONG WINAPI PrivateRegQueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);

#define PrivateRegGet(name,dest) PrivateRegQueryValueEx(NULL, name, NULL, NULL, ((LPBYTE) (dest)), NULL);

HANDLE hRegistryChanged;

#endif

//
// Additional attributes and preferences
//

enum ATTRIB
{
   //
   // Provider attribs
   //

   MAX_SUPPORTED_SAMPLES,

#ifdef STANDALONE

   //
   // RSX preferences
   //

   DEVICETYPE_VALUE,
   DEBUGLEVEL_VALUE,
   NUMWAVEBUFFERS_VALUE,
   PERIPHERAL_VALUE,
   BUFFERSIZE_VALUE,
   TOOLPATH_VALUE,
   MIDIID_VALUE,
   DISABLEMMX_VALUE,
   CACHE_TIME_VALUE,
   LISTENER_FORMAT_VALUE,
   TRAYON_VALUE,
   DATADLL_VALUE,
   FLOAT_CLIP_VAL,
   LOCALIZE_VALUE,
   SPEAKERHELP_VALUE,
   SOFTWARE_VALUE,
   CPU_OVERRIDE,
   DSOUND_VALUE

#endif
};

#ifdef STANDALONE

//
// RSX preference names (from PRIVATE.H)
//

#define RSX20_DEVICETYPE_VALUE	   "Device Type"        // directsound
#define RSX20_DEBUGLEVEL_VALUE	   "Debug Level"        // 0
#define RSX20_NUMWAVEBUFFERS_VALUE "NumWaveBuffers"     // 3
#define RSX20_PERIPHERAL_VALUE	   "Peripheral"         // 0
#define RSX20_BUFFERSIZE_VALUE	   "Buffersize"         // 0x78
#define RSX20_TOOLPATH_VALUE	   "RSXTOOLPATH"        // c:\windows\system\rsxtool.exe
#define RSX20_MIDIID_VALUE	   "MidiID"             // -1
#define RSX20_DISABLEMMX_VALUE	   "NOMMX"              // 1
#define RSX20_CACHE_TIME_VALUE      "Cache Time"         // 3
#define RSX20_LISTENER_FORMAT_VALUE "Device Format"      // 0x0b
#define RSX20_TRAYON_VALUE          "Tray Enabled"       // 0x01
#define RSX20_DATADLL_VALUE	    "Data"               // rsxdata.dll
#define RSX20_FLOAT_CLIP_VAL	    "Clip"               // 1
#define RSX20_LOCALIZE_VALUE        "True 3D"            // 1
#define RSX20_SPEAKERHELP_VALUE     "Speaker Help"       // 0
#define RSX20_SOFTWARE_VALUE	    "ForceSoftware3D"    // 1
#define RSX20_CPU_OVERRIDE	    "Force CPU"          // optional, normally 0
#define RSX20_DSOUND_VALUE          "ForceDirectSound"   // optional, normally 0

#endif

//
// Event IDs for thread communication
//

#define BUFFER1_EVENT (0)
#define BUFFER2_EVENT (1)
#define DIE_EVENT     (2)

#define NUM_EVENTS    (3)

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
   S32      orig_playback_rate;  // Original rate set when sample created
   S32      playback_rate;       // Playback rate in samples/sec

   S32      bytes_per_sample;    // 1 or 2 for 8- or 16-bit samples

   S32      buffers_past_end;    // # of silent buffers appended

   S32      activated;           // Used to wait for start of playback

   //
   // Positioning
   //

   RSXVECTOR3D position;         // 3D position
   RSXVECTOR3D face;             // 3D orientation
   RSXVECTOR3D up;               // 3D up-vector
   RSXVECTOR3D velocity;         // 3D velocity
   S32         auto_update;      // TRUE to automatically update in background

   //
   // Thread synchronization
   //

   volatile S32 is_safe;             // Thread is waiting for next event
   volatile S32 cancel_pending;      // we took an early exit from sleep_sample(), don't do anything else

   //
   // RSX fields
   //

   U8* m_Data[2];                      // RSX output buffers, 16K each by default

   IRSXStreamingEmitter *m_lpSE;	      // RSX streaming sound emitter
   HANDLE m_lpThread;

   HANDLE m_hBufferEvents[NUM_EVENTS]; // Buffer events for this sample

   RSXBUFFERHDR m_Buffer1;             // RSX buffer headers that are submitted to
   RSXBUFFERHDR m_Buffer2;             // the streaming emitter

   RSXEMITTERMODEL rsxEModel;          // Environment model

   U32 lastblockdone;                  // estimated time when last mix will be done

   F32 fMinFront;
   F32 fMinBack;
   F32 fMaxFront;
   F32 fMaxBack;

   F32 obstruction;
   F32 occlusion;

   AIL3DSAMPLECB eos;
   H3DSAMPLE clientH3D;
   S32           docb;
};

//
// RSX supports unlimited samples, but choose a reasonable max and default
//

#define MAX_SAMPLES 64

static SAMPLE3D samples[MAX_SAMPLES];
static S32 n_samples=16;

//
// Globals
//

S32 active = 0;

IRSX                  *m_lpRSX=0;         // RSX COM provider
IRSXDirectListener    *m_lpDL = NULL;   // Singleton RSX listener object
IRSXStreamingListener *m_lpSL = NULL;   // Singleton RSX listener object

RSXVECTOR3D listen_position;
RSXVECTOR3D listen_face;
RSXVECTOR3D listen_up;
RSXVECTOR3D listen_velocity;
S32         listen_auto_update = 0;

HTIMER      service_timer;
HTIMER		buffer_timer;

S32			speaker_type;

HDIGDRIVER  dig;
HSAMPLE     stream;
S32         stream_buffer_size;
S32         stream_active;
U8         *stream_buffer[2];

static F32 rolloff_factor = 1.0f;
static F32 doppler_factor = 1.0f;
static F32 distance_factor = 1.0f;

static void OurSetEvent(SAMPLE3D FAR* S, U32 eventnum);

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
//#                                                                            #
//# M3D_set_3D_speaker_type                                                    #
//#                                                                            #
//##############################################################################

void		  AILEXPORT M3D_set_3D_speaker_type (S32 spk_type)
{
  S32 spk;

  API_lock();

  speaker_type=spk_type;

  switch (spk_type)
  {
	 case AIL_3D_HEADPHONE:
      spk=1;
      PrivateRegSet(RSX20_PERIPHERAL_VALUE, (LPBYTE) &spk);
      break;
	 case AIL_3D_4_SPEAKER:
	 case AIL_3D_SURROUND:
	 case AIL_3D_2_SPEAKER:
	 default:
      spk=0;
      PrivateRegSet(RSX20_PERIPHERAL_VALUE, (LPBYTE) &spk);
      break;
  }
  SetEvent(hRegistryChanged);

  API_unlock();
}

//##############################################################################
//#                                                                            #
//# M3D_3D_speaker_type                                                        #
//#                                                                            #
//##############################################################################

S32		 AILEXPORT M3D_3D_speaker_type (void)
{
	return speaker_type;
}


static void set_volume(SAMPLE3D FAR* S)
{
   S->rsxEModel.fIntensity = (F32(S->volume) / 127.0F)
                              *(1.0F-S->occlusion)
                              *(1.0F-S->obstruction);

   S->rsxEModel.cbSize = sizeof(RSXEMITTERMODEL);

   if (S->m_lpSE != NULL)
      {
      API_lock();
      S->m_lpSE->SetModel(&S->rsxEModel);
      API_unlock();
      }
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

   set_volume(S);
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

   set_volume(S);
}

//############################################################################
//#                                                                          #
//# Abort any pending sample events                                          #
//#                                                                          #
//############################################################################

void reset_sample_events(SAMPLE3D FAR *S)
{
   S32 j;

   for (j = 0; j < NUM_EVENTS; j++)
      {
      ResetEvent(S->m_hBufferEvents[j]);
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

   while ((S->m_lpThread) && (!S->is_safe))
      {
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
   diag_printf("flush\n");

   API_lock();

   if (S->m_lpSE != NULL)
      {
      S->m_lpSE->Flush();
      }

   API_unlock();

   // give RSX sometime to flush the buffers
   Sleep(10);

   //
   // Abort events caused by ->Flush()'s returned buffers
   //

   reset_sample_events(S);

   S->buffers_past_end=0;
   diag_printf("end flush\n");
}

//############################################################################
//##                                                                        ##
//## Copy data from source sample to target secondary buffer                ##
//##                                                                        ##
//## Backfill target secondary buffer with silence to end of source data    ##
//##                                                                        ##
//############################################################################

static void RSX_stream_to_buffer(SAMPLE3D FAR *S, void FAR *dest, S32 len)
{
   void *out;
   U32   out_len;
   void *in;
   U32   in_len;
   U32   copy_len;
   U32   amtmixed=0;

   diag_printf("stream\n");
   //
   // Init output pointer to beginning of secondary buffer segment
   //

   out     = dest;
   out_len = len;

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
      diag_printf("silence fill\n");
      memset(out,
            (S->bytes_per_sample == 2) ? 0 : 0x80808080,
             out_len);

      out_len = 0;
      }


   // setup to monitor the "SMP_DONE" time
   if (amtmixed) {
     U32 timer=timeGetTime();

     // if not enough time has past for the last block assume this block will start after it
     if (S->lastblockdone>timer)
       timer=S->lastblockdone;

     S->lastblockdone=timer+((amtmixed*1000)/(S->playback_rate*S->bytes_per_sample));
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

   S->volume             =  AIL_get_preference(DIG_DEFAULT_VOLUME);
   S->playback_rate      =  22050;
   S->orig_playback_rate =  22050;

   S->status          =  SMP_DONE;

   S->auto_update     =  0;
}

//############################################################################
//#                                                                          #
//# Terminate playing sample immediately, aborting all pending events        #
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

   AIL_unlock_mutex();
   flush_sample(S);
   AIL_lock_mutex();

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
   SAMPLE3D FAR *S       = (SAMPLE3D FAR *) pParam;
   
   while (1)
      {
      S32 j = (S32) &S->is_safe;

      _asm
         {
         mov eax,[j]
         lock inc dword ptr [eax]
         }

      dwReturnedEvent = WaitForMultipleObjects(NUM_EVENTS,
                                               S->m_hBufferEvents,
                                               FALSE,
                                               INFINITE);

      _asm
         {
         mov eax,[j]
         lock dec dword ptr [eax]
         }

      if (S->cancel_pending)
         {
         continue;
         }

      //
      // DIE event may be issued for any sample, with or without an
      // assigned emitter
      //

      if (dwReturnedEvent == DIE_EVENT)
         {
         diag_printf("%X DIE\n",S);

         S->status = SMP_DONE;

         if (S->m_lpSE)
            {
            flush_sample(S);
            }

         return 0;
         }

      //
      // All others require an assigned emitter -- if no emitter assigned,
      // no operations can be executed
      //

      if (S->m_lpSE == NULL)
         {
         continue;
         }

      //
      // Process event on sample with valid assigned emitter
      //

      switch (dwReturnedEvent)
         {

         case BUFFER1_EVENT:

            diag_printf("%X BUFFER 1\n",S);

            if ((S->status&255) == SMP_PLAYING)
               {
               RSX_stream_to_buffer(S, S->m_Data[0], BUFF_SIZE);

               API_lock();
               S->m_lpSE->SubmitBuffer(&S->m_Buffer1);
               API_unlock();
               }
            break;

         case BUFFER2_EVENT:

            diag_printf("%X BUFFER 2\n",S);

            if ((S->status&255) == SMP_PLAYING)
               {
               RSX_stream_to_buffer(S, S->m_Data[1], BUFF_SIZE);

               API_lock();
               S->m_lpSE->SubmitBuffer(&S->m_Buffer2);
               API_unlock();
               }
            break;
         }
      }
}


//
// SetEvent that checks whether the thread is running first
//

static void OurSetEvent(SAMPLE3D FAR* S, U32 eventnum)
{
  //
  // Create a thread to feed the streaming emitter
  //
  DWORD stupId;

  if (S->m_lpThread==0) {
    S->m_lpThread=(HANDLE)-1;
    S->m_lpThread=CreateThread(0,0,pfnThreadProc,(LPVOID) S,0,&stupId);
  }

  SetEvent(S->m_hBufferEvents[eventnum]);
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
#ifdef STANDALONE
      U32        value;
      static C8  string[MAX_PATH];

      case PROVIDER_NAME:    return (U32) "RAD Game Tools RSX 3D Audio";
#else
      case PROVIDER_NAME:    return (U32) "Intel Realistic Sound Experience(TM)";
#endif
      case PROVIDER_VERSION: return 0x101;

      case MAX_SUPPORTED_SAMPLES: return n_samples;

#ifdef STANDALONE

      case DEBUGLEVEL_VALUE:

         PrivateRegGet(RSX20_DEBUGLEVEL_VALUE, &value);
         return value;

      case NUMWAVEBUFFERS_VALUE:

         PrivateRegGet(RSX20_NUMWAVEBUFFERS_VALUE, &value);
         return value;

      case PERIPHERAL_VALUE:

         PrivateRegGet(RSX20_PERIPHERAL_VALUE, &value);
         return value;

      case BUFFERSIZE_VALUE:

         PrivateRegGet(RSX20_BUFFERSIZE_VALUE, &value);
         return value;

      case MIDIID_VALUE:

         PrivateRegGet(RSX20_MIDIID_VALUE, &value);
         return value;

      case DISABLEMMX_VALUE:

         PrivateRegGet(RSX20_DISABLEMMX_VALUE, &value);
         return value;

      case CACHE_TIME_VALUE:

         PrivateRegGet(RSX20_CACHE_TIME_VALUE, &value);
         return value;

      case LISTENER_FORMAT_VALUE:

         PrivateRegGet(RSX20_LISTENER_FORMAT_VALUE, &value);
         return value;

      case TRAYON_VALUE:

         PrivateRegGet(RSX20_TRAYON_VALUE, &value);
         return value;

      case FLOAT_CLIP_VAL:

         PrivateRegGet(RSX20_FLOAT_CLIP_VAL, &value);
         return value;

      case LOCALIZE_VALUE:

         PrivateRegGet(RSX20_LOCALIZE_VALUE, &value);
         return value;

      case SPEAKERHELP_VALUE:

         PrivateRegGet(RSX20_SPEAKERHELP_VALUE, &value);
         return value;

      case SOFTWARE_VALUE:

         PrivateRegGet(RSX20_SOFTWARE_VALUE, &value);
         return value;

      case CPU_OVERRIDE:

         PrivateRegGet(RSX20_CPU_OVERRIDE, &value);
         return value;

      case DSOUND_VALUE:

         PrivateRegGet(RSX20_DSOUND_VALUE, &value);
         return value;

      case DEVICETYPE_VALUE:

         PrivateRegGet(RSX20_DEVICETYPE_VALUE, string);
         return (U32) string;

      case TOOLPATH_VALUE:

         PrivateRegGet(RSX20_TOOLPATH_VALUE, string);
         return (U32) string;

      case DATADLL_VALUE:

         PrivateRegGet(RSX20_DATADLL_VALUE, string);
         return (U32) string;
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
                                           void FAR*  value)
{
   S32 prev = -1;

#ifdef STANDALONE

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

      case DEBUGLEVEL_VALUE:

         PrivateRegGet(RSX20_DEBUGLEVEL_VALUE, &prev);
         PrivateRegSet(RSX20_DEBUGLEVEL_VALUE, (LPBYTE) value);
         break;

      case NUMWAVEBUFFERS_VALUE:

         PrivateRegGet(RSX20_NUMWAVEBUFFERS_VALUE, &prev);
         PrivateRegSet(RSX20_NUMWAVEBUFFERS_VALUE, (LPBYTE) value);
         break;

      case PERIPHERAL_VALUE:

         PrivateRegGet(RSX20_PERIPHERAL_VALUE, &prev);
         PrivateRegSet(RSX20_PERIPHERAL_VALUE, (LPBYTE) value);
         break;

      case BUFFERSIZE_VALUE:

         PrivateRegGet(RSX20_BUFFERSIZE_VALUE, &prev);
         PrivateRegSet(RSX20_BUFFERSIZE_VALUE, (LPBYTE) value);
         break;

      case MIDIID_VALUE:

         PrivateRegGet(RSX20_MIDIID_VALUE, &prev);
         PrivateRegSet(RSX20_MIDIID_VALUE, (LPBYTE) value);
         break;

      case DISABLEMMX_VALUE:

         PrivateRegGet(RSX20_DISABLEMMX_VALUE, &prev);
         PrivateRegSet(RSX20_DISABLEMMX_VALUE, (LPBYTE) value);
         break;

      case CACHE_TIME_VALUE:

         PrivateRegGet(RSX20_CACHE_TIME_VALUE, &prev);
         PrivateRegSet(RSX20_CACHE_TIME_VALUE, (LPBYTE) value);
         break;

      case LISTENER_FORMAT_VALUE:

         PrivateRegGet(RSX20_LISTENER_FORMAT_VALUE, &prev);
         PrivateRegSet(RSX20_LISTENER_FORMAT_VALUE, (LPBYTE) value);
         break;

      case TRAYON_VALUE:

         PrivateRegGet(RSX20_TRAYON_VALUE, &prev);
         PrivateRegSet(RSX20_TRAYON_VALUE, (LPBYTE) value);
         break;

      case FLOAT_CLIP_VAL:

         PrivateRegGet(RSX20_FLOAT_CLIP_VAL, &prev);
         PrivateRegSet(RSX20_FLOAT_CLIP_VAL, (LPBYTE) value);
         break;

      case LOCALIZE_VALUE:

         PrivateRegGet(RSX20_LOCALIZE_VALUE, &prev);
         PrivateRegSet(RSX20_LOCALIZE_VALUE, (LPBYTE) value);
         break;

      case SPEAKERHELP_VALUE:

         PrivateRegGet(RSX20_SPEAKERHELP_VALUE, &prev);
         PrivateRegSet(RSX20_SPEAKERHELP_VALUE, (LPBYTE) value);
         break;

      case SOFTWARE_VALUE:

         PrivateRegGet(RSX20_SOFTWARE_VALUE, &prev);
         PrivateRegSet(RSX20_SOFTWARE_VALUE, (LPBYTE) value);
         break;

      case CPU_OVERRIDE:

         PrivateRegGet(RSX20_CPU_OVERRIDE, &prev);
         PrivateRegSet(RSX20_CPU_OVERRIDE, (LPBYTE) value);
         break;

      case DSOUND_VALUE:

         PrivateRegGet(RSX20_DSOUND_VALUE, &prev);
         PrivateRegSet(RSX20_DSOUND_VALUE, (LPBYTE) value);
         break;

      case DEVICETYPE_VALUE:

         PrivateRegGet(RSX20_DEVICETYPE_VALUE, &prev);
         PrivateRegSet(RSX20_DEVICETYPE_VALUE, (LPBYTE) value);
         break;

      case TOOLPATH_VALUE:

         PrivateRegGet(RSX20_TOOLPATH_VALUE, &prev);
         PrivateRegSet(RSX20_TOOLPATH_VALUE, (LPBYTE) value);
         break;

      case DATADLL_VALUE:

         PrivateRegGet(RSX20_DSOUND_VALUE, &prev);
         PrivateRegSet(RSX20_DSOUND_VALUE, (LPBYTE) value);
         break;
      }

   //
   // Tell RSX lib to invalidate internal copies of registry values
   //

   SetEvent(hRegistryChanged);

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

   for (i=0; i < n_samples; i++)
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

   if (i == n_samples)
      {
      return NULL;
      }

   SAMPLE3D FAR *S = &samples[i];

   if (S->m_Data[0]==0)
   {
     S->m_Data[0]=(U8*)AIL_mem_alloc_lock(BUFF_SIZE*2);
     if (S->m_Data[0]==0)
       return(0);
   }

   S->m_Data[1]=S->m_Data[0]+BUFF_SIZE;

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
   // start the thread
   //

   DWORD stupId;
   
   if (S->m_lpThread==0) {
     S->m_lpThread=(HANDLE)-1;
     S->m_lpThread=CreateThread(0,0,pfnThreadProc,(LPVOID) S,0,&stupId);
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

   S->docb=0;
   S->status = SMP_PLAYING;
   S->activated = 1;

   diag_printf("%X START\n",S);

   S->buffers_past_end=0;

   AIL_unlock_mutex();
   flush_sample(S);
   AIL_lock_mutex();

   RSX_stream_to_buffer(S, S->m_Data[0], BUFF_SIZE);
   RSX_stream_to_buffer(S, S->m_Data[1], BUFF_SIZE);

   S->cancel_pending=1;

   API_lock();
   S->m_lpSE->SubmitBuffer(&S->m_Buffer1);
   S->m_lpSE->SubmitBuffer(&S->m_Buffer2);
   API_unlock();

   S->cancel_pending=0;
}

//##############################################################################
//#                                                                            #
//# M3D_stop_3D_sample                                                         #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_stop_3D_sample          (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if ((S->status&255) != SMP_PLAYING)
      {
      return;
      }

   diag_printf("%X STOP\n",S);

   S->status = SMP_STOPPED;

   AIL_unlock_mutex();
   flush_sample(S);
   AIL_lock_mutex();

   S->activated = 0;

   S->status = SMP_STOPPED;
}

//##############################################################################
//#                                                                            #
//# M3D_resume_3D_sample                                                       #
//#                                                                            #
//##############################################################################

void       AILEXPORT M3D_resume_3D_sample        (H3DSAMPLE samp)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if ((S->status == SMP_FREE) || ((S->status&255) == SMP_PLAYING))
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
   // start the thread
   //

   DWORD stupId;

   if (S->m_lpThread==0) {
     S->m_lpThread=(HANDLE)-1;
     S->m_lpThread=CreateThread(0,0,pfnThreadProc,(LPVOID) S,0,&stupId);
   }

   //
   // Initialize sample voice
   //

   reset_sample_voice(S);

   //
   // Activate sample and wait for command to take effect
   //

   S->docb=0;
   S->status = SMP_PLAYING;
   S->activated = 1;

   // start playback

   diag_printf("%X START\n",S);

   S->buffers_past_end=0;

   AIL_unlock_mutex();
   flush_sample(S);
   AIL_lock_mutex();

   RSX_stream_to_buffer(S, S->m_Data[0], BUFF_SIZE);
   RSX_stream_to_buffer(S, S->m_Data[1], BUFF_SIZE);

   S->cancel_pending=1;

   API_lock();
   S->m_lpSE->SubmitBuffer(&S->m_Buffer1);
   S->m_lpSE->SubmitBuffer(&S->m_Buffer2);
   API_unlock();

   S->cancel_pending=0;

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

   //
   // The intensity (volume) of the emitter
   // Generally between 0.00f and 1.00f
   //

   S->volume = volume;

   set_volume(S);

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

   //
   // Calculate the pitch of the emitter, represented as a decimal percentage
   // of the original playback rate where 1.00 = 100% of the original rate
   //

   F32 pitch = F32(playback_rate) / F32(S->orig_playback_rate);

   //
   // Clamp to [0.25, 4.0] in accordance with RSX spec
   //

   if (pitch < 0.25F)
      {
      pitch = 0.25F;
      }

   if (pitch > 4.0F)
      {
      pitch = 4.0F;
      }

   S->playback_rate = S32(pitch * F32(S->orig_playback_rate));

   //
   // Set RSX emitter pitch
   //

   if (S->m_lpSE != NULL)
      {
      API_lock();
      S->m_lpSE->SetPitch(pitch);
      API_unlock();
      }
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

   S->fMinFront = min_dist;
   S->fMinBack  = min_dist;
   S->fMaxFront = max_dist;
   S->fMaxBack  = max_dist;

   //
   // This defines the inner ellipsoid of the
   // emitter.  Sound is ambient in this region
   //

   if (rolloff_factor < EPSILON )
   {
     // if no roll_off, use max dist
     S->rsxEModel.fMinFront = max_dist;
     S->rsxEModel.fMinBack  = max_dist;
   }
   else
   {
     S->rsxEModel.fMinFront = min_dist * rolloff_factor;
     S->rsxEModel.fMinBack  = min_dist * rolloff_factor;
   }

   //
   // This defines the outer ellipsoid of the emitter
   // Sound is localized between the edge of the inner
   // ellipsoid and the outer ellipse. If the listener
   // position is outside of this ellipsoid the emitter
   // can not be heard.
   //

   S->rsxEModel.fMaxFront = min_dist*32.0F;  //JKR - force the RSX volume fall off
   S->rsxEModel.fMaxBack  = min_dist*32.0F;  //      to match DS3D-style (roughly)

   //
   // The intensity (volume) of the emitter
   // Generally between 0.00f and 1.00f
   //

   S->rsxEModel.fIntensity = F32(S->volume) / 127.0F;

   S->rsxEModel.cbSize = sizeof(RSXEMITTERMODEL);

   if (S->m_lpSE != NULL)
      {
      API_lock();
      S->m_lpSE->SetModel(&S->rsxEModel);
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

   if (max_dist) *max_dist = S->fMaxFront;
   if (min_dist) *min_dist = S->fMinFront;
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

   for (i=0; i < n_samples; i++)
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

      if (S->m_lpSE != NULL)
         {
         API_lock();
         S->m_lpSE->SetPosition(&S->position);
         API_unlock();
         }
      }
   else if (*t == IS_LISTENER)
      {
      listen_position.x = X;
      listen_position.y = Y;
      listen_position.z = Z;

      API_lock();

      if (m_lpDL == NULL)
         {
         m_lpSL->SetPosition(&listen_position);
         }
      else
         {
         m_lpDL->SetPosition(&listen_position);
         }

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

   F32 magnitude = (F32) sqrt((dX_per_ms * dX_per_ms) +
                              (dY_per_ms * dY_per_ms) +
                              (dZ_per_ms * dZ_per_ms));

   if ( magnitude > EPSILON )
   {
     dX_per_ms /= magnitude;
     dY_per_ms /= magnitude;
     dZ_per_ms /= magnitude;
   }

   magnitude = magnitude * ( doppler_factor * distance_factor );

   dX_per_ms *= magnitude;
   dY_per_ms *= magnitude;
   dZ_per_ms *= magnitude;

   if (t == NULL)
      {
      return;
      }

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      S->velocity.x = dX_per_ms*1000.0F;
      S->velocity.y = dY_per_ms*1000.0F;
      S->velocity.z = dZ_per_ms*1000.0F;

      if (S->m_lpSE)
      {
        API_lock();
        S->m_lpSE->SetVelocity(&S->velocity);
        API_unlock();
      }

      }
   else if (*t == IS_LISTENER)
      {
      listen_velocity.x = dX_per_ms*1000.0F;
      listen_velocity.y = dY_per_ms*1000.0F;
      listen_velocity.z = dZ_per_ms*1000.0F;

      API_lock();

      if (m_lpDL == NULL)
         {
         m_lpSL->SetPosition(&listen_velocity);
         }
      else
         {
         m_lpDL->SetPosition(&listen_velocity);
         }

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
   OBJTYPE FAR *t = (OBJTYPE FAR *) obj;
   
   magnitude = magnitude * distance_factor * doppler_factor;

   dX_per_ms *= magnitude;
   dY_per_ms *= magnitude;
   dZ_per_ms *= magnitude;

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      S->velocity.x = dX_per_ms*1000.0F;
      S->velocity.y = dY_per_ms*1000.0F;
      S->velocity.z = dZ_per_ms*1000.0F;

      if (S->m_lpSE)
      {
        API_lock();
        S->m_lpSE->SetVelocity(&S->velocity);
        API_unlock();
      }

      }
   else if (*t == IS_LISTENER)
      {
      listen_velocity.x = dX_per_ms*1000.0F;
      listen_velocity.y = dY_per_ms*1000.0F;
      listen_velocity.z = dZ_per_ms*1000.0F;

      API_lock();

      if (m_lpDL == NULL)
         {
         m_lpSL->SetPosition(&listen_velocity);
         }
      else
         {
         m_lpDL->SetPosition(&listen_velocity);
         }

      API_unlock();
      }
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

      if (S->m_lpSE != NULL)
         {
         API_lock();
         S->m_lpSE->SetOrientation(&S->face);
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

      if (m_lpDL == NULL)
         {
         m_lpSL->SetOrientation(&listen_face, &listen_up);
         }
      else
         {
         m_lpDL->SetOrientation(&listen_face, &listen_up);
         }

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
   
   F32 x,y,z,magnitude;

   if (t == NULL)
      {
      return;
      }

   if (*t == IS_SAMPLE)
      {
      SAMPLE3D FAR *S = (SAMPLE3D FAR *) obj;

      x=S->velocity.x/1000.0F;
      y=S->velocity.y/1000.0F;
      z=S->velocity.z/1000.0F;
      }
   else if (*t == IS_LISTENER)
      {
      x=listen_velocity.x/1000.0F;
      y=listen_velocity.y/1000.0F;
      z=listen_velocity.z/1000.0F;
      }

   magnitude = (F32) sqrt((x * x) +
                          (y * y) +
                          (z * z));

   if ( magnitude > EPSILON )
   {
     x /= magnitude;
     y /= magnitude;
     z /= magnitude;
   }

   if ( ( doppler_factor < EPSILON ) || ( distance_factor < EPSILON ) )
     magnitude = 0.0f;
   else
     magnitude = magnitude / ( doppler_factor * distance_factor );

   x *= magnitude;
   y *= magnitude;
   z *= magnitude;
   
   if (dX_per_ms) *dX_per_ms = x;
   if (dY_per_ms) *dY_per_ms = y;
   if (dZ_per_ms) *dZ_per_ms = z;
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

S32        AILEXPORT M3D_set_3D_sample_data      (H3DSAMPLE samp, //)
                                                  AILSOUNDINFO FAR *info)
{
   SAMPLE3D FAR *S = (SAMPLE3D FAR *) samp;

   if (S->status == SMP_FREE)
      {
      return 0;
      }

   //
   // Parse .WAV file
   //

   if (info == NULL)
      {
      static S32 silence;

      AILSOUNDINFO dummy_info;

      dummy_info.format = WAVE_FORMAT_PCM;
      dummy_info.data_ptr = &silence;
      dummy_info.data_len = 4;
      dummy_info.rate = 22050;
      dummy_info.bits = 16;
      dummy_info.channels = 1;
      dummy_info.samples = 2;
      dummy_info.block_size = 2;

      info = &dummy_info;
      }
   else
      {
      if (info->format != WAVE_FORMAT_PCM)
         {
         AIL_set_error("Not a PCM .WAV file");
         return 0;
         }

      if (info->channels != 1)
         {
         AIL_set_error("Not a mono .WAV file");
         return 0;
         }
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

   if (S->m_lpSE != NULL)
      {
      API_lock();
      S->m_lpSE->Release();
      API_unlock();

      S->m_lpSE = NULL;
      }

   //
   // Initialize sample buffer headers
   //

   ZeroMemory(&S->m_Buffer1, sizeof(RSXBUFFERHDR));
   S->m_Buffer1.cbSize = sizeof(RSXBUFFERHDR);
   S->m_Buffer1.dwSize = BUFF_SIZE;
   S->m_Buffer1.lpData = (PCHAR) S->m_Data[0];

   ZeroMemory(&S->m_Buffer2, sizeof(RSXBUFFERHDR));
   S->m_Buffer2.cbSize = sizeof(RSXBUFFERHDR);
   S->m_Buffer2.dwSize = BUFF_SIZE;
   S->m_Buffer2.lpData = (PCHAR) S->m_Data[1];

   //
   // Assign the events to use for each buffer
   // When these events are signalled we know that
   // RSX is finished with the buffer and we can
   // reuse it
   //

   S->m_Buffer1.hEventSignal  = S->m_hBufferEvents[BUFFER1_EVENT];
   S->m_Buffer2.hEventSignal  = S->m_hBufferEvents[BUFFER2_EVENT];

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

   S->volume             =  AIL_get_preference(DIG_DEFAULT_VOLUME);
   S->playback_rate      =  info->rate;
   S->orig_playback_rate =  info->rate;

   S->status        =  SMP_DONE;

   //
   // Create streaming emitter object
   //

   RSXSTREAMINGEMITTERDESC seDesc;
   WAVEFORMATEX            wf;

   memset(&seDesc,
              0,
              sizeof(RSXSTREAMINGEMITTERDESC));

   seDesc.cbSize = sizeof(RSXSTREAMINGEMITTERDESC);

   seDesc.lpwf = &wf;

   seDesc.lpwf->wFormatTag      = WAVE_FORMAT_PCM;
   seDesc.lpwf->nChannels       = (S16) info->channels;
   seDesc.lpwf->nSamplesPerSec  = info->rate;
   seDesc.lpwf->nAvgBytesPerSec = (info->rate * info->channels * info->bits) / 8;
   seDesc.lpwf->nBlockAlign     = (info->channels * info->bits) / 8;
   seDesc.lpwf->wBitsPerSample  = (S16) info->bits;
   seDesc.lpwf->cbSize          = 0;

   API_lock();

   HRESULT hr = m_lpRSX->CreateStreamingEmitter(&seDesc,
                                                &S->m_lpSE, 
                                                 NULL);

   API_unlock();

   if (FAILED(hr)) 
      {
      AIL_set_error("Could not create streaming emitter for .WAV");
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

   M3D_set_3D_sample_obstruction  (samp, 0.0F);
   M3D_set_3D_sample_occlusion    (samp, 0.0F);

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

   return 1;
}

#define RSX20_REG_KEY "SOFTWARE\\Intel\\Realistic Sound Experience\\2.0"
#define RSX20_DISABLEMMX_VALUE "NOMMX"

static void disableMMX()
{
  HKEY  hKey;
  DWORD dwDisposition;
  DWORD disable = 1;
  HRESULT hr;

  hr = RegCreateKeyEx(HKEY_LOCAL_MACHINE, RSX20_REG_KEY, 0, 0,
                      REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                      NULL, &hKey, &dwDisposition);
  if (hr == ERROR_SUCCESS)
  {
    hr = RegSetValueEx(hKey, RSX20_DISABLEMMX_VALUE, 0,
                       REG_DWORD, (LPBYTE)&disable,
                       sizeof(DWORD));
  }
  RegCloseKey(hKey);
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

   for (i=0; i < n_samples; i++)
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

   for (i=0; i < n_samples; i++)
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

   //
   // Feed output streaming buffer, if appropriate
   //

   if ((m_lpSL != NULL) && 
       (stream_active))
      {
      S32 ready = AIL_sample_buffer_ready(stream);

      if (ready != -1)
         {
         m_lpSL->RequestBuffer((C8 *) stream_buffer[ready],
                                      NULL,
                                      0);

         AIL_load_sample_buffer(stream,
                                ready,
                                stream_buffer[ready],
                                stream_buffer_size);
         }
      }
}

//##############################################################################
//#                                                                            #
//# M3D_activate                                                               #
//#                                                                            #
//##############################################################################

HINSTANCE DLL_hInstance;

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
      // disable MMX because it is buggy and noisy
      //

      disableMMX();

      //
      // Start COM
      //

      AIL_unlock_mutex();
      CoInitialize(NULL);
      AIL_lock_mutex();

      //
      // Create listener
      //

      API_lock();
      AIL_unlock_mutex();

      HRESULT hr;

#ifdef STANDALONE

	   hRegistryChanged = CreateEvent(NULL, FALSE, FALSE, "RSXRegChange");

      m_lpRSX = NULL;

      hr = CreateRSX(NULL, IID_IRSX20, (void **) &m_lpRSX);

#else

      hr = CoCreateInstance(CLSID_RSX20,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IRSX20,
                 (void **) &m_lpRSX);

#endif
      
      AIL_lock_mutex();
      API_unlock();

      //
      // If it wasn't created either the registry doesn't contain the
      // CLSID for RSX or the directory mentioned in the registry
      // doesn't contain rsx.dll.  The easy fix for this problem
      // is to run the RSX setup again.
      //

      if ((FAILED(hr)) || (!m_lpRSX))
         {
         AIL_set_error("Failed to create RSX object");
         return M3D_INTERNAL_ERR;
         }

      //
      // Create default environment for listener
      //

      RSXENVIRONMENT rsxEnv;

      rsxEnv.cbSize  = sizeof(RSXENVIRONMENT);
      rsxEnv.dwFlags = RSXENVIRONMENT_SPEEDOFSOUND;
      rsxEnv.fSpeedOfSound = 331.359985F;

      API_lock();

      m_lpRSX->SetEnvironment(&rsxEnv);

      API_unlock();

      //
      // Set default reverb configuration
      //

      RSXREVERBMODEL rsxRvb;

      rsxRvb.cbSize     = sizeof(RSXREVERBMODEL);
      rsxRvb.bUseReverb = FALSE;
      rsxRvb.fDecayTime = 1.5f;
      rsxRvb.fIntensity = 0.1f;

      API_lock();

      m_lpRSX->SetReverb(&rsxRvb);

      API_unlock();

      //
      // Create listener -- either streaming or direct, depending on
      // whether WAVEOUT is in use
      //

      m_lpDL        = NULL;
      m_lpSL        = NULL;
      dig           = NULL;
      stream        = NULL;
      stream_active = NULL;

      if (AIL_get_preference(DIG_USE_WAVEOUT) == YES)
         {
         //
         // MSS is using waveOut, so create streaming listener
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
         // Allocate a sample handle for streaming buffer
         //

         stream = AIL_allocate_sample_handle(dig);

         if (stream == NULL)
            {
            AIL_set_error("No 2D sample handles available for output stream");
            return M3D_NOT_INIT;
            }

         //
         // Streaming listener format = whatever the hardware is configured
         // to use
         //

         S32 rate;
         S32 format;

         AIL_digital_configuration(dig,
                                  &rate,
                                  &format,
                                   NULL);

         AIL_init_sample(stream);

         AIL_set_sample_volume        (stream, 127);
         AIL_set_sample_pan           (stream, 64);
         AIL_set_sample_type          (stream, format, 0);
         AIL_set_sample_playback_rate (stream, rate);

         //
         // Get minimum sample buffer size for requested output format
         //

         U32 min_buffer_size = AIL_minimum_sample_buffer_size(dig,
                                                              rate,
                                                              format);

         //
         // Configure streaming listener creation descriptor
         //

         WAVEFORMATEX m_wfx;

         m_wfx.wFormatTag      = WAVE_FORMAT_PCM;
         m_wfx.nChannels       = 2;
         m_wfx.nSamplesPerSec  = rate;
         m_wfx.wBitsPerSample  = (format & DIG_F_16BITS_MASK) ? 16 : 8;
         m_wfx.nAvgBytesPerSec = rate * (m_wfx.wBitsPerSample / 8) * 2;
         m_wfx.nBlockAlign     = (m_wfx.wBitsPerSample / 8) * 2;
         m_wfx.cbSize          = 0;

         //
         // Loop until buffer of sufficient size is returned from
         // CreateStreamingListener()
         //

         DWORD dwListenerBufferSizeInMS = 50;

         RSXSTREAMINGLISTENERDESC slDesc;

         while (1)
            {
            memset(&slDesc,
                  0,
                  sizeof(slDesc));

            slDesc.cbSize                = sizeof(RSXSTREAMINGLISTENERDESC);
            slDesc.dwRequestedBufferSize = (dwListenerBufferSizeInMS * m_wfx.nAvgBytesPerSec) / 1000;
            slDesc.lpwf                  = &m_wfx;
            slDesc.dwUser                = 0;

            //
            // Allocate streaming listener object
            //

            API_lock();

            if (FAILED(m_lpRSX->CreateStreamingListener(&slDesc,
                                                        &m_lpSL,
                                                         NULL)) || (!m_lpSL) )
               {
               API_unlock();

               AIL_set_error("Error creating RSX listener");
               return M3D_INTERNAL_ERR;
               }

            API_unlock();

            //
            // If actual half-buffer size is less than minimum sample buffer
            // size, destroy the streaming listener and re-create it with
            // a longer buffer time
            //
            // Otherwise, break successfully out of loop
            //

            if (slDesc.dwActualBufferSize >= min_buffer_size)
               {
               break;
               }

            m_lpSL->Release();
            m_lpSL = NULL;

            //
            // Reallocation of the streaming listener is a time-consuming
            // operation, so accelerate it by geometrically approaching
            // the desired buffer size/time
            //

            if ((slDesc.dwActualBufferSize * 2) < min_buffer_size)
               {
               dwListenerBufferSizeInMS *= 2;
               }
            else
               {
               dwListenerBufferSizeInMS += 25;
               }
            }

         //
         // Once the size is known, allocate the stream buffers
         //

         stream_buffer_size = slDesc.dwActualBufferSize;

         for (S32 i=0; i < 2; i++)
            {
            stream_buffer[i] = (U8 FAR *) AIL_mem_alloc_lock(stream_buffer_size);

            if (stream_buffer[i] == NULL)
               {
               AIL_set_error("Could not allocate sample streaming buffers");
               return M3D_OUT_OF_MEM;
               }
            }
         }
      else
         {
         RSXDIRECTLISTENERDESC rsxDL;

         rsxDL.cbSize   = sizeof(RSXDIRECTLISTENERDESC);
         rsxDL.hMainWnd = AIL_HWND();

         rsxDL.dwUser   = 0;
         rsxDL.lpwf     = NULL;

         API_lock();

         AIL_unlock_mutex();
         if (FAILED(m_lpRSX->CreateDirectListener(&rsxDL,
                                                  &m_lpDL,
                                                   NULL)) || (!m_lpDL) )
            {
            AIL_lock_mutex();
            API_unlock();

            AIL_set_error("Error creating RSX listener -- audio device missing or in use?");
            return M3D_INTERNAL_ERR;
            }

         AIL_lock_mutex();
         API_unlock();
         }

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

      SAMPLE3D FAR *S;

      for (i=0; i < n_samples; i++)
         {
         S = &samples[i];

         memset(S,
                0,
                sizeof(struct SAMPLE3D));

         S->status = SMP_FREE;

         S->is_safe        = 0;
         S->cancel_pending = 0;

         //
         // Create Win32 signal events for sample
         //

         S32 j;

         for (j = 0; j < NUM_EVENTS; j++)
            {
            //
            // Events are autoreset and initially non-signaled
            //

            S->m_hBufferEvents[j] = CreateEvent(NULL,
                                                FALSE,
                                                FALSE,
                                                NULL);
            if (!S->m_hBufferEvents[j])
               {
               AIL_set_error("Error creating events");
               return M3D_INTERNAL_ERR;
               }
            }

         //
         // Initialize sample buffer headers
         //

         ZeroMemory(&S->m_Buffer1, sizeof(RSXBUFFERHDR));
         S->m_Buffer1.cbSize = sizeof(RSXBUFFERHDR);
         S->m_Buffer1.dwSize = BUFF_SIZE;
         S->m_Buffer1.lpData = (PCHAR) S->m_Data[0];

         ZeroMemory(&S->m_Buffer2, sizeof(RSXBUFFERHDR));
         S->m_Buffer2.cbSize = sizeof(RSXBUFFERHDR);
         S->m_Buffer2.dwSize = BUFF_SIZE;
         S->m_Buffer2.lpData = (PCHAR) S->m_Data[1];

         //
         // Assign the events to use for each buffer
         // When these events are signalled we know that
         // RSX is finished with the buffer and we can
         // reuse it
         //

         S->m_Buffer1.hEventSignal  = S->m_hBufferEvents[BUFFER1_EVENT];
         S->m_Buffer2.hEventSignal  = S->m_hBufferEvents[BUFFER2_EVENT];

         //
         // Clear sample emitter
         //

         S->m_lpSE = NULL;

         //
         // Clear thread variable
         //

         S->m_lpThread=0;
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


      if (m_lpSL != NULL)
         {
         stream_active = 1;
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

#ifdef STANDALONE
	   CloseHandle(hRegistryChanged);
#endif

      //
      // Stop service timer
      //

      if (m_lpSL != NULL)
         {
         stream_active = 0;
         }

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

      for (i=0; i < n_samples; i++)
         {
         SAMPLE3D FAR *S = &samples[i];

         //
         // Mark handle free
         //

         S->status = SMP_FREE;

         //
         // Tell the sample's thread to kill itself, and wait for it to
         // happen
         //

         if (S->m_lpThread) 
         {

           OurSetEvent(S,DIE_EVENT);

           WaitForSingleObject(S->m_lpThread,
                               INFINITE);

           CloseHandle(S->m_lpThread);

	        S->m_lpThread=0;
         }

         //
         // Destroy emitter associated with sample
         //

         if (S->m_lpSE != NULL)
            {
            API_lock();
            S->m_lpSE->Release();
            API_unlock();

            S->m_lpSE = NULL;
            }

         //
         // Destroy event handles associated with sample
         //

         S32 j;

         for (j = 0; j < NUM_EVENTS; j++)
            {
            CloseHandle(S->m_hBufferEvents[j]);
            }

         if (S->m_Data[0])
           AIL_mem_free_lock(S->m_Data[0]);

         }

      //
      // Release the listener
      //

      if (m_lpDL)
         {
         API_lock();
         m_lpDL->Release();
         API_unlock();

         m_lpDL = NULL;
         }

      if (m_lpSL)
         {
         API_lock();
         m_lpSL->Release();
         API_unlock();

         m_lpSL = NULL;
         }

      //
      // Release RSX
      //

#ifdef STANDALONE
      AIL_unlock_mutex();
      DestroyRSX();
      AIL_lock_mutex();
#endif

      if (m_lpRSX)
         {
         API_lock();
         AIL_unlock_mutex();
         m_lpRSX->Release();
         AIL_lock_mutex();
         API_unlock();

         m_lpRSX = NULL;
         }

      AIL_release_sample_handle( stream );
      stream = 0;

      if (stream_buffer[0] != NULL)
         {
         AIL_mem_free_lock((void FAR *) stream_buffer[0]);
         stream_buffer[0] = NULL;
         }

      if (stream_buffer[1] != NULL)
         {
         AIL_mem_free_lock((void FAR *) stream_buffer[1]);
         stream_buffer[1] = NULL;
         }

      //
      // Shut down COM
      //

      AIL_unlock_mutex();
      CoUninitialize();
      AIL_lock_mutex();

      active = 0;
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
		REG_FN(M3D_set_3D_speaker_type),
		REG_FN(M3D_3D_speaker_type),

      REG_FN(M3D_set_3D_EOS),
      REG_FN(M3D_set_3D_sample_obstruction),
      REG_FN(M3D_3D_sample_obstruction),
      REG_FN(M3D_set_3D_sample_occlusion),
      REG_FN(M3D_3D_sample_occlusion),
      
      REG_FN(M3D_set_3D_rolloff_factor),
      REG_FN(M3D_3D_rolloff_factor),

      REG_FN(M3D_set_3D_doppler_factor),
      REG_FN(M3D_3D_doppler_factor),

      REG_FN(M3D_set_3D_distance_factor),
      REG_FN(M3D_3D_distance_factor),
      };

   const RIB_INTERFACE_ENTRY M3D[] =
      {
      REG_AT("Name",                      PROVIDER_NAME,         RIB_STRING),
      REG_AT("Version",                   PROVIDER_VERSION,      RIB_HEX),
      REG_AT("Maximum supported samples", MAX_SUPPORTED_SAMPLES, RIB_DEC),
      REG_PR("Maximum supported samples", MAX_SUPPORTED_SAMPLES, RIB_DEC),

#ifdef STANDALONE

      REG_AT(RSX20_DEVICETYPE_VALUE,	   DEVICETYPE_VALUE,       RIB_STRING),
      REG_AT(RSX20_DEBUGLEVEL_VALUE,	   DEBUGLEVEL_VALUE,       RIB_DEC),
      REG_AT(RSX20_NUMWAVEBUFFERS_VALUE,	NUMWAVEBUFFERS_VALUE,   RIB_DEC),
      REG_AT(RSX20_PERIPHERAL_VALUE,	   PERIPHERAL_VALUE,       RIB_DEC),
      REG_AT(RSX20_BUFFERSIZE_VALUE,	   BUFFERSIZE_VALUE,       RIB_DEC),
      REG_AT(RSX20_LISTENER_FORMAT_VALUE, LISTENER_FORMAT_VALUE,  RIB_DEC),
      REG_AT(RSX20_FLOAT_CLIP_VAL,	      FLOAT_CLIP_VAL,         RIB_DEC),
      REG_AT(RSX20_LOCALIZE_VALUE,        LOCALIZE_VALUE,         RIB_DEC),

      REG_PR(RSX20_DEVICETYPE_VALUE,	   DEVICETYPE_VALUE,       RIB_STRING),
      REG_PR(RSX20_DEBUGLEVEL_VALUE,	   DEBUGLEVEL_VALUE,       RIB_DEC),
      REG_PR(RSX20_NUMWAVEBUFFERS_VALUE,	NUMWAVEBUFFERS_VALUE,   RIB_DEC),
      REG_PR(RSX20_PERIPHERAL_VALUE,	   PERIPHERAL_VALUE,       RIB_DEC),
      REG_PR(RSX20_BUFFERSIZE_VALUE,	   BUFFERSIZE_VALUE,       RIB_DEC),
      REG_PR(RSX20_LISTENER_FORMAT_VALUE, LISTENER_FORMAT_VALUE,  RIB_DEC),
      REG_PR(RSX20_FLOAT_CLIP_VAL,	      FLOAT_CLIP_VAL,         RIB_DEC),
      REG_PR(RSX20_LOCALIZE_VALUE,        LOCALIZE_VALUE,         RIB_DEC),

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

         DLL_hInstance = hinstDll;

         DisableThreadLibraryCalls( hinstDll );

#ifdef STANDALONE
	 RSXEntryPoint(DLL_hInstance, DLL_PROCESS_ATTACH, plvReserved);
#endif

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

#ifdef STANDALONE
         RSXEntryPoint(DLL_hInstance, DLL_PROCESS_DETACH, plvReserved);
#endif

         break;
      }

   return TRUE;
}

