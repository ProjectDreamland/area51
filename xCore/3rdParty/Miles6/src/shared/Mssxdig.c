//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  MSSXDIG.C: Virtual wavetable synthesizer API for XMIDI-triggered      ##
//##              digital sound effects                                     ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 9.0                  ##
//##                                                                        ##
//##  Version 1.00 of 15-Feb-95: Derived from AILXDIG.C V1.01               ##
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

#include <limits.h>

#include "mss.h"
#include "imssapi.h"

#undef min
#define min(a,b) ((a) < (b) ? (a) : (b))

#undef max
#define max(a,b) ((a) >= (b) ? (a) : (b))

#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## Locked code                                                            ##
//##                                                                        ##
//############################################################################

#define LOCK(x)   AIL_vmm_lock  (&(x),sizeof(x))
#define UNLOCK(x) AIL_vmm_unlock(&(x),sizeof(x))

static S32 locked = 0;

void AILXDIG_end(void);

void AILXDIG_start(void)
{
   if (!locked)
      {
      AIL_vmm_lock_range(AILXDIG_start, AILXDIG_end);

      locked = 1;
      }
}

#define DOLOCK() AILXDIG_start()

#else

#define DOLOCK()

#endif

//############################################################################
//##                                                                        ##
//## Write control log value                                                ##
//##                                                                        ##
//############################################################################

static void XMI_write_log(CTRL_LOG FAR *log, S32 status, S32 data_1,   //)
                                S32 data_2)
{
   S32 st;
   S32 ch;

   st = status & 0xf0;
   ch = status & 0x0f;

   switch (st)
      {
      case EV_PROGRAM:
         log->program[ch] = (S8) data_1;
         break;

      case EV_PITCH:
         log->pitch_l[ch] = (S8) data_1;
         log->pitch_h[ch] = (S8) data_2;
         break;

      case EV_CONTROL:

         switch (data_1)
            {
            case CHAN_LOCK:
               log->c_lock[ch]     = (S8) data_2;
               break;

            case CHAN_PROTECT:
               log->c_prot[ch]     = (S8) data_2;
               break;

            case VOICE_PROTECT:
               log->c_v_prot[ch]   = (S8) data_2;
               break;

            case PATCH_BANK_SEL:
               log->bank[ch]       = (S8) data_2;
               break;

            case INDIRECT_C_PFX:
               log->indirect[ch]   = (S8) data_2;
               break;

            case CALLBACK_TRIG:
               log->callback[ch]   = (S8) data_2;
               break;

            case MODULATION:
               log->mod[ch]        = (S8) data_2;
               break;

            case PART_VOLUME:
               log->vol[ch]        = (S8) data_2;
               break;

            case PANPOT:
               log->pan[ch]        = (S8) data_2;
               break;

            case EXPRESSION:
               log->exp[ch]        = (S8) data_2;
               break;

            case SUSTAIN:
               log->sus[ch]        = (S8) data_2;
               break;

            case REVERB:
               log->reverb[ch]     = (S8) data_2;
               break;

            case CHORUS:
               log->chorus[ch]     = (S8) data_2;
               break;

            case DATA_MSB:
               log->bend_range[ch] = (S8) data_2;
               break;
            }
      }
}

//############################################################################
//##                                                                        ##
//## Locate wave library entry by bank/patch value                          ##
//##                                                                        ##
//## Returns pointer to entry or NULL if entry not found                    ##
//##                                                                        ##
//############################################################################

static WAVE_ENTRY FAR *WVL_search(void FAR *wave_lib, S32 bank, S32 patch)
{
   WAVE_ENTRY FAR *E;
   S32        i;

   E = (WAVE_ENTRY FAR *) wave_lib;

   for (i=0; i < MAX_W_ENTRIES; i++,E++)
      {
      if (E->bank == -1)
         {
         return NULL;
         }

      if ((E->bank  == bank) &&
          (E->patch == patch))
         {
         return E;
         }
      }

   return NULL;
}

//############################################################################
//##                                                                        ##
//## Calculate volume value for sample based on channel volume, expression, ##
//## and note velocity                                                      ##
//##                                                                        ##
//## Also update panpot value                                               ##
//##                                                                        ##
//############################################################################

static void XDIG_set_volume(HWAVESYNTH W, S32 voice)
{
   int c,v;

   c = (int)W->chan[(U16)voice];

   v = (int)((W->controls.vol[c] * W->controls.exp[c]) / 127);

   v = (int)((v * W->vel[voice]) / 127);

   AIL_set_sample_volume(W->S[voice],v);

   AIL_set_sample_pan(W->S[voice], W->controls.pan[c]);
}

//############################################################################
//##                                                                        ##
//## Calculate playback rate for sample based on note number and channel    ##
//## pitch wheel / bender range values, as well as sample rate and root     ##
//## note number                                                            ##
//##                                                                        ##
//############################################################################

static void XDIG_set_pitch(HWAVESYNTH W, S32 voice)
{
   S32    ch,root,note,lim,rate;
   S32    base_freq,bent_freq,lim_freq,root_freq;
   S32    pitch_sign,pitch_val;
   HSAMPLE S;

   static const S32 note_frequency[128] =
      {
         8,                                                                               //  0
        17,   18,   19,   20,   21,   23,   24,   26,   27,   29,   30,   32,  //  1-12
        34,   36,   38,   41,   43,   46,   49,   51,   55,   58,   61,   65,  // 13-24
        69,   73,   77,   82,   87,   92,   98,  103,  110,  116,  123,  130,  // 25-36
       138,  146,  155,  164,  174,  185,  196,  207,  220,  233,  247,  261,  // 37-48
       277,  293,  311,  329,  349,  370,  392,  415,  440,  466,  493,  523,  // 49-60
       554,  587,  622,  659,  698,  740,  784,  830,  880,  932,  987, 1046,  // 61-72
      1108, 1174, 1244, 1318, 1396, 1480, 1568, 1661, 1760, 1864, 1975, 2093,  // 73-84
      2217, 2349, 2488, 2636, 2793, 2960, 3136, 3322, 3520, 3729, 3951, 4186,  // 85-96
      4435, 4699, 4977, 5273, 5587, 5920, 6272, 6644, 7040, 7459, 7902, 8372,  // 97-108
      8870, 9398, 9955,10547,11174,11840,12544,13289,14080,14918,15804,16745,  // 109-120
     17740,18796,19910,21094,22348,23680,25088                                      // 121-127
      };


   //
   // Set up variables
   //

   S = W->S[voice];

   ch = W->chan[voice];

   root = W->root[voice];

   rate = W->rate[voice];

   note = W->note[voice];

   //
   // Get pitch value -8192 - 0 - +8192
   //
   // Derive pitch value 0-8192, with sign -1, 0, or +1
   //

   pitch_val = (W->controls.pitch_h[ch] << 7) | W->controls.pitch_l[ch];

   if (pitch_val > 8192)
      {
      pitch_val -= 8191;
      pitch_sign = 1;
      }
   else if (pitch_val < 8192)
      {
      pitch_val  = 8192-pitch_val;
      pitch_sign = -1;
      }
   else
      {
      pitch_val  = 0;
      pitch_sign = 0;
      }

   //
   // Determine new note frequency for melodic or percussive voice
   //

   lim = note + (W->controls.bend_range[ch] * pitch_sign);

   if (lim < 0)
      {
      lim = 0;
      }

   if (lim > 127)
      {
      lim = 127;
      }

   root_freq = note_frequency[root];
   base_freq = note_frequency[note];
   lim_freq  = note_frequency[lim];

   bent_freq = base_freq + ((lim_freq - base_freq) * pitch_val / 8192);

   if (ch != PERCUSS_CHAN)
      {
      //
      // Perform key shift and pitch shift
      //

      rate = rate * base_freq / root_freq;

      rate = rate * bent_freq / base_freq;
      }
   else
      {
      //
      // Perform pitch shift only
      //

      rate = rate * bent_freq / base_freq;
      }

   AIL_set_sample_playback_rate(W->S[voice], rate);
}

//############################################################################
//##                                                                        ##
//## Timbre-installation intercept function                                 ##
//##                                                                        ##
//## Inhibit XMIDI driver from installing timbres which will be handled     ##
//## by XDIG                                                                ##
//##                                                                        ##
//############################################################################

static S32 AILLIBCALLBACK XDIG_TIMB_trap(HMDIDRIVER mdi, //)
                          S32        bank,
                          S32        patch)
{
   HWAVESYNTH W;

   //
   // Get HWAVESYNTH handle for driver
   //

   W = (HWAVESYNTH) (U32) mdi->system_data[0];

   //
   // Search wave library for given bank/patch entry
   //
   // If found, return 1 to inhibit driver installation of timbre
   // If not found, return 0 to allow driver to install timbre
   //

   if (WVL_search(W->library, bank, patch) == NULL)
      {
      return 0;
      }
   else
      {
      return 1;
      }
}

//############################################################################
//##                                                                        ##
//## MIDI event interpreter callback function                               ##
//##                                                                        ##
//## Returns 1 if event handled by wave interpreter, or 0 to pass event     ##
//## to XMIDI driver                                                        ##
//##                                                                        ##
//############################################################################

static S32 AILLIBCALLBACK XDIG_MIDI_trap(HMDIDRIVER mdi,   //)
                          HSEQUENCE   S,
                          S32        status,
                          S32        data_1,
                          S32        data_2)
{
   S32        st,ch;
   S32        i,voice;
   U32       t;
   HWAVESYNTH       W;

   //
   // Get HWAVESYNTH handle for driver
   //

   S = S;

   W = (HWAVESYNTH) (U32) mdi->system_data[0];

   //
   // Get physical MIDI channel and status byte for event
   //

   st = status & 0xf0;
   ch = status & 0x0f;

   //
   // Update MIDI status log
   //

   if ((st == EV_CONTROL) ||
       (st == EV_PROGRAM) ||
       (st == EV_PITCH  ))
      {
      XMI_write_log(&W->controls, status, data_1, data_2);
      }

   //
   // Process event
   //

   if ((st == EV_NOTE_ON) && (data_2 == 0))
      {
      st = EV_NOTE_OFF;
      }

   switch (st)
      {
      //
      // Process MIDI Control Change events
      //

      case EV_CONTROL:

         switch (data_1)
            {
            //
            // Volume / panpot / expression controllers
            //

            case PART_VOLUME:
            case EXPRESSION:
            case PANPOT:

               for (voice=0; voice < W->n_voices; voice++)
                  {
                  if ((W->chan[voice] == ch)
                       &&
                      (AIL_sample_status(W->S[voice]) == SMP_PLAYING))
                     {
                     XDIG_set_volume(W,voice);
                     }
                  }
               break;

            //
            // Adjust pitch bend range
            //

            case DATA_MSB:

               for (voice=0; voice < W->n_voices; voice++)
                  {
                  if ((W->chan[voice] == ch)
                       &&
                      (AIL_sample_status(W->S[voice]) == SMP_PLAYING))
                     {
                     XDIG_set_pitch(W,voice);
                     }
                  }
               break;

            //
            // All Notes Off message
            //

            case ALL_NOTES_OFF:

               for (voice=0; voice < W->n_voices; voice++)
                  {
                  if ((W->chan[voice] == ch)
                       &&
                      (AIL_sample_status(W->S[voice]) == SMP_PLAYING))
                     {
                     AIL_end_sample(W->S[voice]);
                     }
                  }
               break;
            }

         break;

      //
      // Process MIDI Program Change events
      //

      case EV_PROGRAM:

         //
         // Get pointer to wave library entry for this patch #
         //

         W->wave[ch] = WVL_search(W->library,
                                  W->controls.bank[ch],
                                  data_1);

         break;

      //
      // Process MIDI Pitch Bend events
      //

      case EV_PITCH:

         for (voice=0; voice < W->n_voices; voice++)
            {
            if ((W->chan[voice] == ch)
                 &&
                (AIL_sample_status(W->S[voice]) == SMP_PLAYING))
               {
               XDIG_set_pitch(W,voice);
               }
            }
         break;

      //
      // Process MIDI Note On events
      //

      case EV_NOTE_ON:

         //
         // If percussion channel (10), select wave library entry based on
         // key #, rather than patch #
         //

         if (ch == PERCUSS_CHAN)
            {
            W->wave[ch] = WVL_search(W->library,
                                     127,
                                     data_1);
            }

         //
         // Pass event to XMIDI driver if wave library entry not found
         //

         if (W->wave[ch] == NULL)
            {
            break;
            }

         //
         // Find an available sample to play note
         //

         for (voice=0; voice < W->n_voices; voice++)
            {
            if (AIL_sample_status(W->S[voice]) == SMP_DONE)
               {
               break;
               }
            }

         //
         // If no samples available, steal least-recently-triggered
         // sample for this note
         //

         if (voice == W->n_voices)
            {
            t = ULONG_MAX;

            for (i=0; i < W->n_voices; i++)
               {
               if (W->time[i] <= t)
                  {
                  t     = W->time[i];
                  voice = i;
                  }
               }

            AIL_end_sample(W->S[voice]);
            }

         //
         // Record channel, note number, velocity, and LRU timestamp
         //

         W->chan[voice] = ch;
         W->note[voice] = data_1;
         W->root[voice] = W->wave[ch]->root_key;
         W->rate[voice] = W->wave[ch]->playback_rate;
         W->vel [voice] = data_2;
         W->time[voice] = W->event++;

         //
         // Initialize sample
         //

         AIL_init_sample(W->S[voice]);

         AIL_set_sample_type(W->S[voice],
                             W->wave[ch]->format,
                             W->wave[ch]->flags);

         AIL_set_sample_address(W->S[voice],
                                AIL_ptr_add(W->library, W->wave[ch]->file_offset),
                                W->wave[ch]->size);

         XDIG_set_pitch(W,voice);
         XDIG_set_volume(W,voice);

         //
         // Finally, activate sample
         //

         AIL_start_sample(W->S[voice]);

         //
         // Return without passing note event to XMIDI driver
         //

         return 1;

      //
      // Process MIDI Note Off events (melodic voices only)
      //

      case EV_NOTE_OFF:

         if (ch != PERCUSS_CHAN)
            {
            for (voice=0; voice < W->n_voices; voice++)
               {
               if ((W->chan[voice] == ch)
                    &&
                   (W->note[voice] == data_1)
                    &&
                   (AIL_sample_status(W->S[voice]) == SMP_PLAYING))
                  {
                  AIL_end_sample(W->S[voice]);
                  return 1;
                  }
               }
            }

         break;
      }

   //
   // Pass all but note on/note off messages through to driver, to keep
   // device's MIDI state up to date
   //
   return 0;
}

//############################################################################
//##                                                                        ##
//## Install a MIDI wave library and enable digital MIDI services           ##
//##                                                                        ##
//############################################################################

HWAVESYNTH AILCALL AIL_API_create_wave_synthesizer(HDIGDRIVER dig, //)
                                                  HMDIDRIVER mdi,
                                                  void const FAR *wave_lib,
                                                  S32        polyphony)
{
   HWAVESYNTH W;
   S32  i;

   //
   // Ensure that all AILXDIG code and data is locked into memory
   //

   DOLOCK();


   //
   // Allocate HWAVESYNTH structure
   //

   W = (HWAVESYNTH)AIL_mem_alloc_lock(sizeof(WAVE_SYNTH));

   if (W==NULL)
      {
      AIL_set_error("Out of memory.");

      return NULL;
      }

   //
   // Copy parameters to descriptor
   //

   W->dig = dig;
   W->mdi = mdi;

   W->library = (WAVE_ENTRY FAR *) wave_lib;

   mdi->system_data[0] = (S32) (U32) W;

   //
   // Initialize controller log for compatibility with XMIDI initialization
   // state
   //

   for (i=0; i < NUM_CHANS; i++)
      {
      W->controls.program   [i] = 0;
      W->controls.pitch_l   [i] = 0;
      W->controls.pitch_h   [i] = 0x40;
      W->controls.bank      [i] = 0;
      W->controls.vol       [i] = 127;
      W->controls.pan       [i] = 64;
      W->controls.exp       [i] = 127;
      W->controls.bend_range[i] = AIL_preference[MDI_DEFAULT_BEND_RANGE];
      }

   //
   // Allocate as many sample handles possible, up to the polyphony
   // limit specified by the user or the maximum number of wave synthesizer
   // voices, whichever is lower
   //

   for (W->n_voices = 0;
        W->n_voices < min(MAX_W_VOICES, polyphony);
        W->n_voices++)
      {
      W->S[W->n_voices] = AIL_allocate_sample_handle(W->dig);

      if (W->S[W->n_voices] == NULL)
         {
         AIL_set_error("Could not allocate enough SAMPLE handles.");
         for(W->n_voices--;W->n_voices>=0;W->n_voices--)
           AIL_release_sample_handle(W->S[W->n_voices]);
         AIL_mem_free_lock(W);
         return(0);
         }
      }

   //
   // Register callback functions to trap MIDI and timbre-installation 
   // events
   //

   W->prev_event_fn = AIL_register_event_callback  (mdi, XDIG_MIDI_trap);
   W->prev_timb_fn  = AIL_register_timbre_callback (mdi, XDIG_TIMB_trap);

   //
   // Initialize channel library entry list
   //

   for (i=0; i < NUM_CHANS; i++)
      {
      W->wave[i] = NULL;
      }

   //
   // Initialize note table
   //

   for (i=0; i < W->n_voices; i++)
      {
      W->chan[i] = -1;
      }

   //
   // Initialize event counter for LRU voice assignment
   //

   W->event = 0;

   //
   // Return handle to virtual wave synthesizer
   //

   return W;
}

//############################################################################
//##                                                                        ##
//## Disable MIDI wave services                                             ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_destroy_wave_synthesizer(HWAVESYNTH W)
{
   S32 i;

   if (W==NULL)
     return;

   //
   // Release allocated sample handles
   // 

   for (i=0; i < W->n_voices; i++)
      {
      AIL_release_sample_handle(W->S[i]);
      }

   //
   // Restore previous trap callback functions
   //

   AIL_register_event_callback  (W->mdi,W->prev_event_fn);
   AIL_register_timbre_callback (W->mdi,W->prev_timb_fn);

   //
   // Free descriptor and return
   // 

   AIL_mem_free_lock(W);
}

#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## End of locked code                                                     ##
//##                                                                        ##
//############################################################################

void AILXDIG_end(void)
{
   if (locked)
      {
      AIL_vmm_unlock_range(AILXDIG_start, AILXDIG_end);

      locked = 0;
      }
}

#endif
