//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  DLS1SYN.CPP: DLS1 MIDI synthesizer                                    ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 12-Oct-97: Initial                                    ##
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

#include <stdlib.h>
#include <stdio.h>
#include "math.h"

#include "mss.h"
#include "imssapi.h"

#include "mssdls.h"
#include "dls1.h"
#include "synth.h"

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

#define LOCK(x)    AIL_vmm_lock  ((void*)&(x),sizeof(x))
#define LOCKPTR(x) AIL_vmm_lock  ((void*)(x),sizeof(x))
#define UNLOCK(x)  AIL_vmm_unlock((void*)&(x),sizeof(x))

static S32 locked = 0;

static void AIL_lock_end(void);

static void AIL_lock_start(void)
{
   if (!locked)
      {
      AIL_vmm_lock_range(AIL_lock_start, AIL_lock_end);

      locked = 1;
      }
}

#define DOLOCK() AIL_lock_start()

#else

#define DOLOCK()

#endif

//############################################################################
//#                                                                          #
//#  Assign voice to specified MIDI note                                     #
//#                                                                          #
//############################################################################

static void DLS_VOICE_note_on(VOICE FAR *V, MIDI_MESSAGE FAR *EV, S32 key, REGION FAR *RGN)
{
   //
   // *V = VOICE structure to initialize with this note
   //

   INSTRUMENT FAR*INS  = &instrument_list->array [EV->instrument_index];
   DLS_FILE   FAR*file = &file_list->array       [INS->DLS_file];

   V->key             =  key;
   V->wave            = &file->WAVE_list[RGN->wave.ulTableIndex];
   V->active          =  TRUE;
   V->release_request =  FALSE;
   V->play_cursor     =  0;
   V->src_fract       =  0;
   V->left_val        =  0;
   V->right_val       =  0;
   V->trigger         = *EV;
   V->region          =  RGN;
   V->BPS             = (V->wave->format & DIG_F_16BITS_MASK) ? 2 : 1;

   V->LFO_phase_accumulator = 0.0F;

   V->mixer_operation = (M_VOL_SCALING | M_RESAMPLE)
                          |
                        ((V->wave->format & DIG_F_16BITS_MASK) ? M_SRC_16 : 0)
                          |
                        ((DLS->output_format & DIG_F_STEREO_MASK) ? M_DEST_STEREO : 0);

   //
   // Get static pitch (based on non-variant parameters) for this note
   //
   // Value expressed in pitch cents (1/100 semitone)
   //

   V->static_pitch =
      F32((RGN->connection->lScale[KEY_NUMBER_TO_PITCH] * V->key) >> 7)
      +
      F32(RGN->sample.sFineTune)
      -
      F32((RGN->connection->lScale[KEY_NUMBER_TO_PITCH] * RGN->sample.usUnityNote) >> 7);

   //
   // Get static attenuation (based on non-variant parameters) for this note
   //

   V->static_atten = 
      (20.0F * F32(log10(16129.0 / F32(EV->data_2 * EV->data_2))));

   if (RGN->sample.lAttenuation != ABSOLUTE_ZERO)
      {
      V->static_atten -= (((F32) (RGN->sample.lAttenuation)) / 655360.0F);
      }

   //
   // Get default pan and convert to relative MIDI units, scaling from 
   // (-50,+50) to (-64,+64)
   // 

   S32 default_pan = (S32) (((F32) RGN->connection->lScale[DEFAULT_PAN]) / 655360.0F);

   default_pan = (default_pan * 128) / 100;

   V->default_pan = default_pan;

   //
   // Get EG1 scalar (normally implicitly defined at -96 dB) and flip sign
   // for attenuation
   //

   S32 EG1_scale = RGN->connection->lScale[EG1_TO_ATTENUATION];

   if (EG1_scale == ABSOLUTE_ZERO)
      {
      V->EG1_active = FALSE;
      }
   else
      {
      V->EG1_active = TRUE;
      V->EG1_scale  = ((F32)(EG1_scale)) / -655360.0F;
      V->EG1_atten  = V->EG1_scale;
      V->EG1_phase  = ATTACK_PHASE;

      //
      // Get EG1 attack time and convert from 32-bit time cents to
      // interval count
      //

      S32 EG1_attack = RGN->connection->lScale[VOL_EG_ATTACK_TIME];

      if (EG1_attack != ABSOLUTE_ZERO)
         {
         //
         // Scale EG1 attack time by velocity-to-attack-time connection
         //

         S32 s = RGN->connection->lScale[VOL_EG_VELOCITY_TO_ATTACK];

         if (s != ABSOLUTE_ZERO)
            {
            EG1_attack += ((s / 128) * EV->data_2);
            }

         F32 usec = (F32)(1000000.0F * pow(2.0F, F32(EG1_attack) /   (F32)(65536*1200)));

         V->EG1_attack_intervals = ((S32)(usec / DLS->service_rate_uS));

         if (V->EG1_attack_intervals)
            {
            V->EG1_attack_dB_per_interval = V->EG1_scale / ((F32)(V->EG1_attack_intervals));
            }
         }
      else
         {
         V->EG1_attack_intervals = 0;
         }

      //
      // Get EG1 sustain level and convert from percentage of peak to
      // decibels of attenuation
      //

      F32 sustain_percent = RGN->connection->lScale[VOL_EG_SUSTAIN_LEVEL] / 655360.0F;

      V->EG1_sustain_atten = V->EG1_scale - ((V->EG1_scale * sustain_percent) / 100.0F);

      //
      // Get EG1 decay time and convert from 32-bit time cents to
      // interval count
      //

      S32 EG1_decay = RGN->connection->lScale[VOL_EG_DECAY_TIME];

      if (EG1_decay != ABSOLUTE_ZERO)
         {
         //
         // Scale EG1 decay time by key #-to-decay-time connection
         //

         S32 s = RGN->connection->lScale[VOL_EG_KEY_TO_DECAY];

         if (s != ABSOLUTE_ZERO)
            {
            EG1_decay += ((s / 128) * V->key);
            }

         F32 usec = ((F32)(1000000.0F * pow(2.0F, ((F32)(EG1_decay)) / ((F32)(65536*1200)))));

         V->EG1_decay_dB_per_interval = V->EG1_scale / (usec / DLS->service_rate_uS);
         }
      else
         {
         V->EG1_decay_dB_per_interval = V->EG1_sustain_atten;
         }

      //
      // Get EG1 release time and convert from 32-bit time cents to
      // interval count
      //

      S32 EG1_release = RGN->connection->lScale[VOL_EG_RELEASE_TIME];

      if (EG1_release != ABSOLUTE_ZERO)
         {
         F32 usec = ((F32)(1000000.0F * pow(2.0F, ((F32)(EG1_release)) / ((F32)(65536*1200)))));

         V->EG1_release_intervals = ((S32)(usec / DLS->service_rate_uS));

         if (V->EG1_release_intervals)
            {
            V->EG1_release_dB_per_interval = V->EG1_scale / ((F32)(V->EG1_release_intervals));
            }
         }
      else
         {
         V->EG1_release_intervals = 0;
         }
      }

   //
   // Get EG2 scalar (normally ABSOLUTE_ZERO)
   //

   S32 EG2_scale = RGN->connection->lScale[EG2_TO_PITCH];

   if (EG2_scale == ABSOLUTE_ZERO)
      {
      V->EG2_active = FALSE;
      }
   else
      {
      V->EG2_active = TRUE;
      V->EG2_scale  = ((F32)(EG2_scale)) / 65536.0F;
      V->EG2_pitch  = V->EG2_scale;
      V->EG2_phase  = ATTACK_PHASE;

      //
      // Get EG2 attack time and convert from 32-bit time cents to
      // interval count
      //

      S32 EG2_attack = RGN->connection->lScale[PITCH_EG_ATTACK_TIME];

      if (EG2_attack != ABSOLUTE_ZERO)
         {
         //
         // Scale EG2 attack time by velocity-to-attack-time connection
         //

         S32 s = RGN->connection->lScale[PITCH_EG_VELOCITY_TO_ATTACK];

         if (s != ABSOLUTE_ZERO)
            {
            EG2_attack += ((s / 128) * EV->data_2);
            }

         F32 usec = ((F32)(1000000.0F * pow(2.0F, ((F32)(EG2_attack)) / ((F32)(65536*1200)))));

         V->EG2_attack_intervals = ((S32)(usec / DLS->service_rate_uS));

         if (V->EG2_attack_intervals)
            {
            V->EG2_attack_per_interval = V->EG2_scale / ((F32)(V->EG2_attack_intervals));
            }
         }
      else
         {
         V->EG2_attack_intervals = 0;
         }

      //
      // Get EG2 sustain level and convert from percentage of peak to
      // decibels of attenuation
      //

      F32 sustain_percent = RGN->connection->lScale[PITCH_EG_SUSTAIN_LEVEL] / 655360.0F;

      V->EG2_sustain_pitch = V->EG2_scale - ((V->EG2_scale * sustain_percent) / 100.0F);

      //
      // Get EG2 decay time and convert from 32-bit time cents to 
      // interval count
      //

      S32 EG2_decay = RGN->connection->lScale[PITCH_EG_DECAY_TIME];

      if (EG2_decay != ABSOLUTE_ZERO)
         {
         //
         // Scale EG2 decay time by key #-to-decay-time connection
         //

         S32 s = RGN->connection->lScale[PITCH_EG_KEY_TO_DECAY];

         if (s != ABSOLUTE_ZERO)
            {
            EG2_decay += ((s / 128) * V->key);
            }

         F32 usec = ((F32)(1000000.0F * pow(2.0F, ((F32)(EG2_decay)) / ((F32)(65536*1200)))));

         V->EG2_decay_per_interval = V->EG2_scale / (usec / DLS->service_rate_uS);
         }
      else
         {
         V->EG2_decay_per_interval = V->EG2_sustain_pitch;
         }

      //
      // Get EG2 release time and convert from 32-bit time cents to
      // interval count
      //

      S32 EG2_release = RGN->connection->lScale[PITCH_EG_RELEASE_TIME];

      if (EG2_release != ABSOLUTE_ZERO)
         {
         F32 usec = F32(1000000.0F * pow(2.0F, F32(EG2_release) / F32(65536*1200)));

         V->EG2_release_intervals = ((S32)(usec / DLS->service_rate_uS));

         if (V->EG2_release_intervals)
            {
            V->EG2_release_per_interval = V->EG2_scale / F32(V->EG2_release_intervals);
            }
         }
      else
         {
         V->EG2_release_intervals = 0;
         }
      }

   //
   // Initialize LFO phase accumulator
   //

   V->LFO_phase_accumulator = 0.0F;

   //
   // Get LFO start delay time and convert from 32-bit time cents to
   // interval count
   //

   S32 LFO_delay = RGN->connection->lScale[LFO_START_DELAY];

   if (LFO_delay == ABSOLUTE_ZERO)
      {
      V->LFO_holdoff = 0;
      }
   else
      {
      F32 usec = F32(1000000.0F * pow(2.0F, F32(LFO_delay) / F32(65536*1200)));

      V->LFO_holdoff = ((S32)(usec / DLS->service_rate_uS));
      }

   //
   // Get LFO frequency and convert from 32-bit absolute pitch to radians per
   // interval
   //

   F32 LFO_frequency =
      ((F32)(pow(2.0F, (((((F32)(RGN->connection->lScale[LFO_FREQUENCY])) /
         65536.0F) - 6900.0F) / 1200.0F))) * 440.0F);

   V->LFO_radians_per_interval = 
      (LFO_frequency * DLS->service_rate_uS * F_2PI) / 1000000.0F;

   //
   // Precalculate other LFO constants
   //

   S32 S = RGN->connection->lScale[LFO_ATTENUATION_SCALE];
   S32 M = RGN->connection->lScale[LFO_MODW_TO_ATTENUATION];

   V->LFO_atten_scale  = (S == ABSOLUTE_ZERO) ? 0.0F : F32(S) / 655360.0F;
   V->LFO_CC1_to_atten = (M == ABSOLUTE_ZERO) ? 0.0F : F32(M) / 655360.0F;

   S = RGN->connection->lScale[LFO_PITCH_SCALE];
   M = RGN->connection->lScale[LFO_MODW_TO_PITCH];

   V->LFO_pitch_scale  = (S == ABSOLUTE_ZERO) ? 0.0F : F32(S) / 65536.0F;
   V->LFO_CC1_to_pitch = (M == ABSOLUTE_ZERO) ? 0.0F : F32(M) / 65536.0F;

   //
   // Set loop boundaries, if wave sample is looped
   //
   // Otherwise loop boundaries are set to -1 to indicate single-shot sample
   //

   if (RGN->sample.cSampleLoops == 0)
      {
      V->loop_start = -1;
      V->loop_size  =  0;
      V->loop_end   = -1;
      }
   else
      {
      V->loop_start = (RGN->loop.ulStart * V->BPS);
      V->loop_size  = (RGN->loop.ulLength * V->BPS);
      V->loop_end   = V->loop_start + V->loop_size;
      }
}

//############################################################################
//#                                                                          #
//#  Turn off MIDI note associated with specified voice, either by sending   #
//#  the voice into its release phase (immediate=0), or by deallocating the  #
//#  voice structure itself (immediate=1)                                    #
//#                                                                          #
//############################################################################

static void DLS_VOICE_note_off(VOICE FAR *V, S32 immediate)
{
   if (immediate)
      {
      V->active = FALSE;
      return;
      }

   V->release_request = TRUE;
}

//############################################################################
//#                                                                          #
//#  Search instrument list, returning index of entry with matching bank/    #
//#  patch #, or -1 if no match found                                        #
//#                                                                          #
//############################################################################

static S32 DLS_instrument_lookup(S32 bank, S32 patch, S32 is_drum)
{
   //
   // Set flag bit in bank comparison value for drums
   //

   if (is_drum)
      {
      bank |= F_INSTRUMENT_DRUMS;
      }

   //
   // First try to find instrument in specified bank
   //

   INSTRUMENT FAR* INS;
   for (INS = instrument_list->used; INS; INS=INS->next)
      {
      if ((INS->header.Locale.ulBank       == (U32) bank) &&
          (INS->header.Locale.ulInstrument == (U32) patch))
          {
          return INS->entry_num;
          }
      }

   //
   // If this is a drum kit instrument, fall back to instrument
   // in bank 0 if it isn't found in the specified bank
   //

   if (is_drum)
      {
      for (INS = instrument_list->used; INS; INS=INS->next)
         {
         if ((INS->header.Locale.ulBank       == F_INSTRUMENT_DRUMS) &&
             (INS->header.Locale.ulInstrument == (U32) patch))
            {
            return INS->entry_num;
            }
         }
      }

   //
   // Failing that, return -1 to indicate instrument not found
   //

   return -1;
}

//############################################################################
//#                                                                          #
//#  Turn all active notes off                                               #
//#                                                                          #
//############################################################################

static void DLS_all_notes_off(S32 c)
{
   c=c;
   for (S32 vn=0; vn < DLS->n_voices; vn++)
      {
      VOICE FAR *V = &DLS->voice_list[vn];

      if (!V->active)
         {
         continue;
         }

      DLS_VOICE_note_off(V, FALSE);
      }
}

//############################################################################
//#                                                                          #
//#  Reset all controllers (option == 127)                                   #
//#                                                                          #
//############################################################################

static void DLS_reset_all_controllers(S32 c, S32 option, S32 program)
{
   DLS->channel[c].pitch            = 0.0F;
   DLS->channel[c].pitch_bend_range = F32(AIL_get_preference(MDI_DEFAULT_BEND_RANGE)) * 100.0F;
   DLS->channel[c].coarse_tuning    = 0;
   DLS->channel[c].fine_tuning      = 0.0F;

   DLS->channel[c].control[MODULATION ] = 0;
   DLS->channel[c].control[DATA_MSB   ] = 0;
   DLS->channel[c].control[DATA_LSB   ] = 0;
   DLS->channel[c].control[SUSTAIN    ] = 0;
   DLS->channel[c].control[REVERB     ] = 40;
   DLS->channel[c].control[CHORUS     ] = 0;
   DLS->channel[c].control[RPN_LSB    ] = 0;
   DLS->channel[c].control[RPN_MSB    ] = 0;
   DLS->channel[c].control[GM_BANK_MSB] = 0;
   DLS->channel[c].control[GM_BANK_LSB] = 0;

   if (option == 127)
      {
      DLS->channel[c].control[PART_VOLUME] = 100;
      DLS->channel[c].control[PANPOT     ] = 64;
      DLS->channel[c].control[EXPRESSION ] = 127;
      }

   if (program)
      {
      DLS->channel[c].bank_LSB = 0;
      DLS->channel[c].bank_MSB = 0;
      DLS->channel[c].patch    = 0;
      DLS->channel[c].instrument_index = DLS_instrument_lookup(0,
                                                               0,
                                                               c == PERCUSS_CHAN);
      }
}

//############################################################################
//#                                                                          #
//#  Set a desired RPN MSB or LSB                                            #
//#                                                                          #
//############################################################################

static void DLS_set_RPN(S32 c, S32 data_MSB, S32 data_LSB)
{
   if (data_MSB == -1)
      {
      data_MSB = DLS->channel[c].control[DATA_MSB];
      }

   if (data_LSB == -1)
      {
      data_LSB = DLS->channel[c].control[DATA_LSB];
      }

   S32 value = (data_MSB << 7) | data_LSB;

   S32 current = (DLS->channel[c].control[RPN_MSB] << 7) |
                  DLS->channel[c].control[RPN_LSB];

   switch (current)
      {
      //
      // Convert pitch bend range from signed 14-bit to cents (1/100-semitone
      // units)
      //

      case 0: 
      
         DLS->channel[c].pitch_bend_range = F32(value) / 1.28F;
         break;

      //
      // Convert fine tuning from unsigned 14-bit to cents (-1 - +1 semitones
      // expressed in 1/100-semitone units)
      //

      case 1: 
      
         DLS->channel[c].fine_tuning = F32(value - 8192) / 81.92F;
         break;

      //
      // Convert coarse tuning from unsigned 7-bit to signed 7-bit (-64 - +63
      // semitones)
      //

      case 2:

         DLS->channel[c].coarse_tuning = data_MSB - 64;
         break;
      }
}

//############################################################################
//#                                                                          #
//#  Handle MIDI note-on event                                               #
//#                                                                          #
//############################################################################

static void DLS_event_note_on(MIDI_MESSAGE FAR *EV)
{
   //
   // Calculate effective key # (= note # + coarse tuning if not channel 10)
   //

   S32 key = EV->data_1;

   if (EV->channel != PERCUSS_CHAN)
      {
      key += DLS->channel[EV->channel].coarse_tuning;

      if (key > 127)
         {
         key = 127;
         }

      if (key < 0)
         {
         key = 0;
         }
      }

   //
   // Identify valid instrument region associated with this note
   //

   INSTRUMENT FAR*INS = &instrument_list->array[EV->instrument_index];
   REGION FAR *RGN = INS->region_list;

   U32 i;
   for (i=0; i < INS->header.cRegions; i++,RGN++)
      {
      if ((key >= (S32) RGN->header.RangeKey.usLow) &&
          (key <= (S32) RGN->header.RangeKey.usHigh))
          {
          break;
          }
      }

   //
   // If no valid entry for this key region, return without
   // allocating a voice slot
   //

   if (i == INS->header.cRegions)
      {
      return;
      }

   //
   // If no valid WAVE data available for this region, return without
   // allocating a voice slot
   //

   if (!file_list->array[INS->DLS_file].WAVE_list[RGN->wave.ulTableIndex].valid)
      {
      return;
      }

   //
   // Find free voice slot to trigger note
   //

   VOICE FAR *V;

   S32 vn;
   for (vn=0; vn < DLS->n_voices; vn++)
      {
      V = &DLS->voice_list[vn];

      if (!V->active)
         {
         break;
         }
      }

   if (vn == DLS->n_voices)
      {
      //
      // No free voices -- we must steal one from an existing note
      //
      // The voice to be stolen must be playing on a higher MIDI channel
      // (other than 10), with higher channel #s robbed before lower ones.
      // If no voices exist on any lower-priority channel, the new note is
      // not played.
      //
      // Channel 10 has the highest priority of all -- a drum note cannot
      // be stolen under any circumstances.
      //

      S32 end_search = (EV->channel == PERCUSS_CHAN) ? MIN_CHAN : EV->channel;
      S32 found = 0;

      for (S32 ch=MAX_CHAN; (ch >= end_search) && (!found); ch--)
         {
         if (ch == PERCUSS_CHAN)
            {
            continue;
            }

         for (vn=0; vn < DLS->n_voices; vn++)
            {
            V = &DLS->voice_list[vn];

            if (V->trigger.channel == ch)
               {
               //
               // Steal this voice
               //

               V->active = FALSE;

               found = 1;
               break;
               }
            }
         }

      if (!found)
         {
         //
         // No voices eligible for reuse by this note, exit
         //

         return;
         }
      }

   //
   // Release any notes coinciding with other instances of this note
   // in the channel, unless non-self-exclusive flag is set
   //

   if (!(RGN->header.fusOptions & F_RGN_OPTION_SELFNONEXCLUSIVE))
      {
      for (vn=0; vn < DLS->n_voices; vn++)
         {
         VOICE FAR *vt = &DLS->voice_list[vn];

         if (!vt->active)
            {
            continue;
            }

         if ((vt->trigger.instrument_index == EV->instrument_index) && 
             (vt->trigger.channel          == EV->channel) &&
             (vt->trigger.data_1           == EV->data_1))
             {
             DLS_VOICE_note_off(vt, FALSE);
             }
         }
      }

   //
   // If instrument belongs to a key group, release any voices used
   // by other regions in the same key group
   //

   if (RGN->header.usKeyGroup)
      {
      for (vn=0; vn < DLS->n_voices; vn++)
         {
         VOICE FAR *vt = &DLS->voice_list[vn];

         if (!vt->active)
            {
            continue;
            }

         if ((vt->trigger.instrument_index  == EV->instrument_index) &&
             (vt->region->header.usKeyGroup == RGN->header.usKeyGroup))
             {
             DLS_VOICE_note_off(vt, FALSE);
             }
         }
      }

   //
   // Initialize new voice for this note and region
   //

   DLS_VOICE_note_on(V, EV, key, RGN);
}

#ifdef IS_16
   #pragma optimize( "l", off ) // Disable loop optimizations (bug in 16-bit)
#endif

//############################################################################
//#                                                                          #
//#  Handle MIDI note-off event                                              #
//#                                                                          #
//############################################################################

static void DLS_event_note_off(MIDI_MESSAGE FAR *EV)
{
   //
   // Set any voices associated with this channel/note combination into
   // their release phase
   //

   for (S32 vn=0; vn < DLS->n_voices; vn++)
      {
      VOICE FAR *vt = &DLS->voice_list[vn];

      if (!vt->active)
         {
         continue;
         }

      if ((vt->trigger.channel == EV->channel) &&
          (vt->trigger.data_1  == EV->data_1))
          {
          DLS_VOICE_note_off(vt, FALSE);
          }
      }
}

#ifdef IS_16
   #pragma optimize( "l", on )  // Re-enable loop optimizations (bug in 16-bit)
#endif

//############################################################################
//#                                                                          #
//#  Handle MIDI pitch-wheel event                                           #
//#                                                                          #
//############################################################################

static void AILCALL DLS_event_pitch_wheel(S32 c, S32 data_1, S32 data_2)
{
   MIDI_CHANNEL FAR *CH = &DLS->channel[c];

   F32 p = F32((((data_2 << 7) + data_1) - 8192));

   CH->pitch = CH->pitch_bend_range * (p / 8192.0F);
}

//############################################################################
//#                                                                          #
//#  Periodic timer service callback handler                                 #
//#                                                                          #
//############################################################################


static void AILLIBCALLBACK DLS_serve(U32 user)
{
   SYNTHDEVICE FAR *DLS = (SYNTHDEVICE FAR*) user;

   //
   // Get # of milliseconds elapsed since last tick
   //

   U32 start_us = AIL_us_count();

   S32 elapsed_us = start_us - DLS->last_interval_us_count;

   if (elapsed_us < 0)
   {
     elapsed_us = start_us + (0xffffffff - DLS->last_interval_us_count);
   }

   S32 elapsed_ms = elapsed_us / 1000;

   //
   // Get # of bytes in build buffer to write this period
   //

   S32 samples_this_period = ((S32)((DLS->output_sample_rate * elapsed_ms) / 1000.0F));

   S32 bytes_this_period = (samples_this_period * DLS->channels_per_sample * sizeof(S32))-
                           DLS->last_total;

   DLS->last_total+=bytes_this_period;

   if (samples_this_period)  // only advance the time if we generate data!!
   {
     if (elapsed_ms>1000)
     {
       DLS->last_interval_us_count = start_us;
       DLS->last_total=0;
     }
   }

   //
   // Add to cumulative total of bytes to write
   //

   DLS->bytes_to_write += bytes_this_period;

   //
   // Process MIDI traffic received since last interval
   //

   while (DLS->MIDI_queue_tail != DLS->MIDI_queue_head)
      {
      //
      // Get current message
      //

      MIDI_MESSAGE FAR *EV = &DLS->MIDI_queue[DLS->MIDI_queue_tail];

      //
      // Process only traffic associated with valid instruments
      //

      if (instrument_list->array[EV->instrument_index].entry_num != -1)
         {
         //
         // Handle note-related messages
         //
         // (Program Change and Control messages handled by MIDI trap)
         //

         switch (EV->status)
            {
            case EV_CONTROL:

               switch (EV->data_1)
                  {
                  case ALL_NOTES_OFF:

                     DLS_all_notes_off(EV->channel);
                     break;
                  }

               break;

            case EV_NOTE_ON:

               DLS_event_note_on(EV);
               break;

            case EV_NOTE_OFF:

               DLS_event_note_off(EV);
               break;
            }
         }

      //
      // Advance queue tail pointer
      //

      if (DLS->MIDI_queue_tail == MQ_SIZE-1)
         {
         DLS->MIDI_queue_tail = 0;
         }
      else
         {
         DLS->MIDI_queue_tail++;
         }
      }

   //
   // Get build buffer and segment to write this interval
   //
   // If none available (i.e., output buffers stalled for some reason, or data
   // being written by synthesizer at faster rate than DMA playback), skip
   // voice processing this interval
   //

   BUILD_BUFFER FAR *buffer = &DLS->build[DLS->current_build_buffer];

   if (buffer->status == BUILD_FULL)
      {
      //
      // No buffers available, dump any surplus data we've built up to try to
      // compensate
      //

      DLS->bytes_to_write = 0;
      }
   else
      {
      //
      // Flush build buffer if we are about to fill the first block
      //

      if (buffer->status == BUILD_EMPTY)
         {
#ifdef OLD_DLS_REVERB_PREFERENCES
         if (AIL_get_preference(DLS_ENABLE_GLOBAL_REVERB))
            {
            DLS->mixer_flush(buffer->block,
                             DLS->bytes_per_buffer,
                             DLS->reverb_buffers[DLS->n_reverb_buffers-1],
                             AIL_get_preference(DLS_GLOBAL_REVERB_LEVEL),
                             DLS->use_MMX);
            }
         else
#endif
            {
            DLS->mixer_flush(buffer->block,
                             DLS->bytes_per_buffer,
                             NULL,
                             0
#ifdef IS_X86
                             ,DLS->use_MMX
#endif
                             );
            }
         }

      //
      // Get segment boundaries to fill this period
      //

      S32 FAR *build = (S32 FAR *) AIL_ptr_add(buffer->block,
                                           buffer->status);

      S32 bytes_remaining = DLS->bytes_per_buffer - buffer->status;

      S32 build_len = min(bytes_remaining, DLS->bytes_to_write);

      //
      // For all active voices
      //
      //    Merge data from voice source into 32-bit signed build buffer, with
      //    optional scaling and resampling
      //

      for (S32 vn=0; vn < DLS->n_voices; vn++)
         {
         VOICE FAR *V = &DLS->voice_list[vn];

         if (!V->active)
            {
            continue;
            }

         //
         // Get pointer to structure representing voice's MIDI channel
         //

         MIDI_CHANNEL FAR *CH = &DLS->channel[V->trigger.channel];

         //
         // If note-off message has been sent to turn off this voice, and
         // sustain pedal is NOT down, send note into its release phase
         //
         // If the sustain pedal is down, we'll ignore the release request
         // and check again next interval
         //

         if ((V->release_request) && (CH->control[SUSTAIN] < 64))
            {
            V->release_request = FALSE;

            V->EG1_phase = RELEASE_PHASE;
            V->EG2_phase = RELEASE_PHASE;
            }

         //
         // Update EG1 state
         //

         F32 EG1_atten = 0.0F;

         if (V->EG1_active)
            {
            switch (V->EG1_phase)
               {
               case ATTACK_PHASE:

                  if (V->EG1_attack_intervals > 0)
                     {
                     V->EG1_atten -= V->EG1_attack_dB_per_interval;

                     if (V->EG1_atten < 0.0F)
                        {
                        V->EG1_atten = 0.0F;
                        }

                     --V->EG1_attack_intervals;
                     }
                  else
                     {
                     V->EG1_atten = 0.0F;
                     V->EG1_phase = DECAY_PHASE;
                     }

                  break;

               case DECAY_PHASE:

                  if (V->EG1_atten < V->EG1_sustain_atten)
                     {
                     V->EG1_atten += V->EG1_decay_dB_per_interval;
                     }
                  else
                     {
                     V->EG1_atten = V->EG1_sustain_atten;
                     V->EG1_phase = SUSTAIN_PHASE;
                     }
                  break;

               case SUSTAIN_PHASE:

                  break;

               case RELEASE_PHASE:

                  //
                  // Free voice if release time expires
                  //

                  if (V->EG1_release_intervals > 0)
                     {
                     V->EG1_atten += V->EG1_release_dB_per_interval;

                     --V->EG1_release_intervals;
                     }
                  else
                     {
                     V->active = FALSE;
                     }

                  break;
               }

            //
            // Free voice if EG1_atten hits full-scale attenuation
            //

            if (V->EG1_atten >= 96.0F)
               {
               V->active = FALSE;
               }

            //
            // If voice was freed up as a result of EG1 termination, continue
            // with next voice
            //

            if (!V->active)
               {
               continue;
               }
            else
               {
               EG1_atten = V->EG1_atten;
               }
            }

         //
         // Update EG2 state
         //

         F32 EG2_pitch = 0.0F;

         if (V->EG2_active) 
            {
            S32 sustain_cmp;
            S32 release_cmp;

            switch (V->EG2_phase)
               {
               case ATTACK_PHASE:

                  if (V->EG2_attack_intervals > 0)
                     {
                     V->EG2_pitch -= V->EG2_attack_per_interval;

                     if (V->EG2_pitch < 0.0F)
                        {
                        V->EG2_pitch = 0.0F;
                        }

                     --V->EG2_attack_intervals;
                     }
                  else
                     {
                     V->EG2_pitch = 0.0F;
                     V->EG2_phase = DECAY_PHASE;
                     }

                  break;

               case DECAY_PHASE:

                  if (V->EG2_scale > 0.0F)
                     {
                     sustain_cmp = (V->EG2_pitch < V->EG2_sustain_pitch);
                     }
                  else
                     {
                     sustain_cmp = (V->EG2_pitch > V->EG2_sustain_pitch);
                     }

                  if (sustain_cmp)
                     {
                     V->EG2_pitch += V->EG2_decay_per_interval;
                     }
                  else
                     {
                     V->EG2_pitch = V->EG2_sustain_pitch;
                     V->EG2_phase = SUSTAIN_PHASE;
                     }
                  break;

               case SUSTAIN_PHASE:

                  break;

               case RELEASE_PHASE:

                  //
                  // Stop updating EG2 if release time expires
                  //
                  // (Active flag stays set, though -- we want the final
                  // scale value to continue to be applied for the rest of
                  // the note)
                  //

                  if (V->EG2_scale > 0.0F)
                     {
                     release_cmp = (V->EG2_pitch < V->EG2_scale);
                     }
                  else
                     {
                     release_cmp = (V->EG2_pitch > V->EG2_scale);
                     }

                  if (V->EG2_release_intervals > 0)
                     {
                     if (release_cmp)
                        {
                        V->EG2_pitch += V->EG2_release_per_interval;
                        }

                     --V->EG2_release_intervals;
                     }
                  else
                     {
                     V->EG2_pitch = V->EG2_scale;
                     }

                  break;
               }

            //
            // Invert sign to get working pitch value
            //

            EG2_pitch = V->EG2_scale - V->EG2_pitch;
            }

         //
         // Update LFO state
         //

         F32 LFO_atten;
         F32 LFO_pitch;

         if (V->LFO_holdoff > 0)
            {
            --V->LFO_holdoff;

            LFO_atten = 0.0F;
            LFO_pitch = 0.0F;
            }
         else
            {
            //
            // Get instantaneous LFO amplitude, then advance LFO phase for
            // this interval
            //

            F32 LFO_output = F32(sin(V->LFO_phase_accumulator));

            V->LFO_phase_accumulator += V->LFO_radians_per_interval;

            if (V->LFO_phase_accumulator >= F_2PI)
               {
               V->LFO_phase_accumulator -= F_2PI;
               }

            //
            // Calculate LFO volume attenuation
            //

            S32 CC1 = CH->control[MODULATION];

            F32 LFO_atten_CC1 = (V->LFO_CC1_to_atten * F32(CC1)) / 128.0F;

            LFO_atten = LFO_output * (V->LFO_atten_scale + LFO_atten_CC1);

            //
            // Calculate LFO pitch control
            //

            F32 LFO_pitch_CC1 = (V->LFO_CC1_to_pitch * F32(CC1)) / 128.0F;

            LFO_pitch = LFO_output * (V->LFO_pitch_scale + LFO_pitch_CC1);
            }

         //
         // Accumulated pitch calculation
         //

         F32 accum_pitch = V->static_pitch +
                              LFO_pitch       +
                              CH->pitch       +
                              CH->fine_tuning +
                              EG2_pitch;

         F32 relative_pitch = (F32) pow(2.0F, (accum_pitch / 1200.0F));

         S32 playback_ratio =
            ((S32)((V->wave->rate * relative_pitch * 65536.0F) / DLS->output_sample_rate));

         //
         // Accumulated attenuation calculation
         //

         S32 CC7  = CH->control[PART_VOLUME];
         S32 CC11 = CH->control[EXPRESSION];
         S32 CC10 = CH->control[PANPOT];

         CC10 += V->default_pan;

         if (CC10 < 0) CC10 = 0;
         if (CC10 > 127) CC10 = 127;

         F32 v_atten = CC7  ? (20.0F * F32(log10(16129.0 / F32(CC7  * CC7))))  : 999.0F;
         F32 e_atten = CC11 ? (20.0F * F32(log10(16129.0 / F32(CC11 * CC11)))) : 999.0F;

         F32 mono_atten = V->static_atten +
                             v_atten         + 
                             e_atten         +
                             LFO_atten       +
                             EG1_atten;

         mono_atten -= F32(AIL_get_preference(DLS_VOLUME_BOOST));

         if (mono_atten < 0.0F)
            {
            mono_atten = 0.0F;
            }

         //
         // Calculate amplitude scalars
         //
         // If scalars hit 0 in any EG1 phase other than attack, we assume
         // it's safe to turn off the voice (not part of DLS spec, but
         // potentially capable of saving CPU time)
         //

         S32 left_scale;
         S32 right_scale;

         if (V->mixer_operation & M_DEST_STEREO)
            {
            F32 p_left  = (CC10 != 127) ? (20.0F * F32(log10(sqrt(127.0F / F32(127-CC10))))) : 999.0F;
            F32 p_right = (CC10)        ? (20.0F * F32(log10(sqrt(127.0F / F32(    CC10))))) : 999.0F;

            F32 left_atten  = mono_atten + p_left;
            F32 right_atten = mono_atten + p_right;

            if (left_atten > 0.0F)
               {
               left_scale = ((S32)(2047.0F / pow(10.0F, left_atten  / 20.0F)));
               }
            else
               {
               left_scale = 2047;
               }

            if (right_atten > 0.0F)
               {
               right_scale = ((S32)(2047.0F / pow(10.0F, right_atten / 20.0F)));
               }
            else
               {
               right_scale = 2047;
               }

            if ((V->EG1_phase != ATTACK_PHASE) &&
                (left_scale   == 0)            &&
                (right_scale  == 0))
               {
               V->active = 0;
               continue;
               }
            }
         else
            {
            right_scale = 0;

            if (mono_atten > 0.0F)
               {
               left_scale = ((S32)(2047.0F / pow(10.0F, mono_atten / 20.0F)));
               }
            else
               {
               left_scale = 2047;
               }

            if ((V->EG1_phase != ATTACK_PHASE) &&
                (left_scale == 0))
               {
               V->active = 0;
               continue;
               }
            }

         //
         // Enable/disable filtering based on user preference
         //

         if (AIL_get_preference(DLS_ENABLE_FILTERING))
            {
            V->mixer_operation |= M_FILTER;
            }
         else
            {
            V->mixer_operation &= ~M_FILTER;
            }

         //
         // Mix source wave data into destination build buffer until source
         // data exhausted or destination buffer full
         //

         if (build_len > 0)
            {
            S32 FAR *dest = build;
            void FAR *dest_end = AIL_ptr_add(build, build_len);

            while (1)
               {
               const void FAR *src = AIL_ptr_add(V->wave->data, V->play_cursor);
               S32       src_len;

               void FAR *loop_end = NULL;

               if (V->play_cursor < V->loop_end)
                  {
                  src_len = V->loop_end - V->play_cursor;

                  loop_end = AIL_ptr_add(src, src_len);
                  }
               else
                  {
                  src_len = V->wave->len - V->play_cursor;
                  }

               void FAR *src_end = AIL_ptr_add(src, src_len);

               //
               // Call mixer merge routine
               //

         #ifdef IS_32

              DLS->mixer_merge(&src,
                                &V->src_fract,
                                src_end,
                                &dest,
                                dest_end,
                                &V->left_val,
                                &V->right_val,
                                playback_ratio,
                                left_scale,
                                right_scale,
                                V->mixer_operation
#ifdef IS_X86
                                ,DLS->use_MMX
#endif
                                );
         #else

               U32 src_offset  = LOWORD(src);
               U32 dest_offset = LOWORD(dest);

               U32 src_end_offset  = src_offset  + AIL_ptr_dif(src_end, src);
               U32 dest_end_offset = dest_offset + AIL_ptr_dif(dest_end,dest);

               DLS->mixer_merge(HIWORD(src),
                                HIWORD(dest),
                                &V->src_fract,
                                &src_offset,
                                &dest_offset,
                                src_end_offset,
                                dest_end_offset,
                                &V->left_val,
                                &V->right_val,
                                playback_ratio,
                                (left_scale << 16) | right_scale,
                                V->mixer_operation );

               src  =             AIL_ptr_add(src,  src_offset  - LOWORD(src));
               dest = (S32 FAR *) AIL_ptr_add(dest, dest_offset - LOWORD(dest));

         #endif

               V->play_cursor = AIL_ptr_dif(src, V->wave->data);

               //
               // If end of loop reached with room left in the build buffer,
               // seek back to beginning of loop and continue mixing
               //

               if ((loop_end != NULL) && (AIL_ptr_ge(src,loop_end) || AIL_ptr_lt(dest,dest_end)))
                  {
                  V->play_cursor = V->loop_start;

                  //
                  // If room left in build buffer, keep mixing
                  //

                  if (AIL_ptr_lt(dest,dest_end))
                     {
                     continue;
                     }
                  }

               break;
               }
            }

         //
         // If play cursor at end of wave data, release this voice
         //

         if (V->play_cursor >= V->wave->len)
            {
            V->active = 0;
            }
         }

      //
      // Advance build-buffer position
      //

      buffer->status += build_len;

      DLS->bytes_to_write -= build_len;

      //
      // If we just wrote to the last segment in the build buffer, mark
      // the buffer BUILD_FULL and add it to the output FIFO
      //

      if (buffer->status >= DLS->bytes_per_buffer)
         {
         buffer->status = BUILD_FULL;

         DLS->build_queue[DLS->build_queue_head] = buffer;

#ifdef OLD_DLS_REVERB_PREFERENCES

         //
         // If reverb enabled, copy contents of this build buffer to
         // first reverb buffer and then propagate the buffers
         //

         if (AIL_get_preference(DLS_ENABLE_GLOBAL_REVERB))
            {
            AIL_memcpy(DLS->reverb_buffers[0],
                    buffer->block,
                    DLS->bytes_per_buffer);

            for (S32 i=DLS->n_reverb_buffers-2; i >= 0; i--)
               {
               AIL_memcpy(DLS->reverb_buffers[i+1],
                       DLS->reverb_buffers[i],
                       DLS->bytes_per_buffer);
               }
            }
#endif

         //
         // Advance output FIFO head pointer
         //

          if (DLS->build_queue_head == ARYSIZE(DLS->build_queue)-1)
            {
            DLS->build_queue_head = 0;
            }
         else
            {
            DLS->build_queue_head++;
            }

         //
         // Increment count of buffers filled since startup
         //

         ++DLS->buffers_filled;

         //
         // Advance pointer to fill next buffer
         //

         if (DLS->current_build_buffer == N_BUILD_BUFFERS-1)
            {
            DLS->current_build_buffer = 0;
            }
         else
            {
            DLS->current_build_buffer++;
            }
         }
      }

   //
   // If DLS_STREAM_BOOTSTRAP preference is set, don't pass first
   // buffer to output stream until at least two buffers' worth of
   // data are ready
   //

   if (DLS->buffers_filled < N_BUILD_BUFFERS)
      {
      if (AIL_get_preference(DLS_STREAM_BOOTSTRAP))
         {
         return;
         }
      }

   //
   // If buffer output list contains at least one entry, copy it to
   // a stream buffer if one is available
   //

//     AIL_debug_printf("tot: %i %i %i\n",(DLS->build_queue_head-DLS->build_queue_tail+16)%16,DLS->build_queue_head,DLS->build_queue_tail);

   while (DLS->build_queue_tail != DLS->build_queue_head)
      {
      //
      // Look for a free stream buffer to receive queued build buffer
      //

      S32 stream_buff = DLS->stream_poll_CB(DLS->user);

      if (stream_buff == -1)
         {
         //
         // No stream buffer available yet, exit
         //

         break;
         }

      //
      // Get source build pointer and destination stream pointer
      //

      BUILD_BUFFER FAR *buffer = DLS->build_queue[DLS->build_queue_tail];

      void FAR *src  = buffer->block;
      void FAR *dest = DLS->stream_lock_CB(DLS->user, stream_buff);

      //
      // Copy build buffer to output stream
      //

      DLS->mixer_copy(src,
                      DLS->bytes_per_buffer,
                      dest,
                      DLS->output_format,
#ifdef IS_X86
                      DLS->use_MMX
#else
                      0
#endif
                      );

      //
      // Submit filled stream buffer
      //

      DLS->stream_unlock_CB(DLS->user, stream_buff);

      //
      // Mark submitted buffer EMPTY to make it available for reuse
      //

      buffer->status = BUILD_EMPTY;

      //
      // Advance queue tail pointer
      //

      if (DLS->build_queue_tail == ARYSIZE(DLS->build_queue)-1)
         {
         DLS->build_queue_tail = 0;
         }
      else
         {
         DLS->build_queue_tail++;
         }
      }

   //
   // keep the profiling information
   //

   U32 end_us=AIL_us_count();

   start_us=(end_us<start_us)?(end_us+(0xffffffff-start_us)):(end_us-start_us);

   DLS->us_count+=start_us;
   if (DLS->us_count>10000000) {
     DLS->ms_count+=(DLS->us_count/1000);
     DLS->us_count=DLS->us_count%1000;
   }
}

//############################################################################
//#                                                                          #
//#  MIDI event interpreter callback function                                #
//#                                                                          #
//#  Returns 1 if event handled by DLS interpreter, or 0 to pass event       #
//#  to XMIDI driver                                                         #
//#                                                                          #
//############################################################################

static S32 AILLIBCALLBACK DLS_MIDI_trap(HMDIDRIVER mdi,   //)
                              HSEQUENCE  S,
                              S32        status,
                              S32        data_1,
                              S32        data_2)
{
   //
   // Get physical MIDI channel and status byte for event
   //

   S32 st = status & 0xf0;
   S32 ch = status & 0x0f;

   //
   // Process system exclusive traffic
   //

   if ((st == EV_CONTROL) && (data_1 == SYSEX_BYTE))
      {
      static const S32 DLS_sysex[2][7] =
         {
         {0xf0, 0x7e, 0x7f, 0x0a, 0x01, 0xf7, -1},   // DLS System On
         {0xf0, 0x7e, 0x7f, 0x0a, 0x02, 0xf7, -1}    // DLS System Off
         };

      static S32 sysex_state[2] = {0, 0};

      //
      // See if we've received a complete message that we recognize
      //

      data_2 |= (ch << 7);

      S32 recognized = -1;

      for (S32 i=0; i < 2; i++)
         {
         if (DLS_sysex[i][sysex_state[i]] == data_2)
            {
            //
            // Match found, advance recognition state for this message
            //

            ++sysex_state[i];

            //
            // If end of string found, accept this message and reset
            // recognizer state
            //

            if (DLS_sysex[i][sysex_state[i]] == -1)
               {
               sysex_state[i] = 0;
               recognized = i;
               break;
               }
            }
         else
            {
            //
            // Mismatch found, reset recognition state to 0
            //

            sysex_state[i] = 0;
            }
         }

      //
      // Act on recognized sysex message, if any
      //

      switch (recognized)
         {
         case 0:
            
            //
            // Turn DLS Level 1 support on
            //

            DLS->enabled = 1;
            break;

         case 1:

            //
            // Turn DLS Level 1 support off
            //

            DLS->enabled = 0;
            break;
         }

      // 
      // Pass all sysex messages (even recognized ones) on to driver
      //

      return !AIL_get_preference(DLS_GM_PASSTHROUGH);
      }

   //
   // If DLS Level 1 processing turned off via sysex command, exit
   //

   if (!DLS->enabled)
      {
      return !AIL_get_preference(DLS_GM_PASSTHROUGH);
      }

   //
   // Reject unrecognized messages
   //

   if ((st >= 0xf0) || (st < 0x80) || (st == EV_POLY_PRESS))
      {
      return !AIL_get_preference(DLS_GM_PASSTHROUGH);
      }

   //
   // Convert implicit note-off into explicit event
   //

   if ((st == EV_NOTE_ON) && (data_2 == 0))
      {
      st = EV_NOTE_OFF;
      }

   //
   // Is this a program change message or bank change message?  If so,
   // process it now to determine whether or not the DLS synth should
   // be responding to further events in this channel
   //

   S32 new_patch = 0;

   if (st == EV_PROGRAM)
      {
      DLS->channel[ch].patch = data_1;
      new_patch = 1;
      }
   else if (st == EV_CONTROL)
      {
      if ((data_1 == PATCH_BANK_SEL) 
           && 
          (AIL_get_preference(DLS_BANK_SELECT_ALIAS)))
         {
         //
         // If DLS_BANK_SELECT_ALIAS is set, XMIDI Patch Bank Select
         // (controller 114) is treated identically to General MIDI
         // bank LSB, with MSB = 0
         //

         DLS->channel[ch].bank_MSB = 0;
         DLS->channel[ch].bank_LSB = data_2;

         new_patch = 1;
         }
      else if (data_1 == GM_BANK_LSB)
         {
         DLS->channel[ch].bank_LSB = data_2;

         new_patch = 1;
         }
      else if (data_1 == GM_BANK_MSB)
         {
         DLS->channel[ch].bank_MSB = data_2;

         new_patch = 1;
         }
      }

   //
   // Does this channel already have a valid instrument assignment?  If not,
   // force an attempt to find one each time an event on this channel
   // occurs
   //

   if (DLS->channel[ch].instrument_index == -1)
      {
      new_patch = 1;
      }

   //
   // Look up the instrument structure and log its entry index,
   // or -1 if no matching instrument found
   //

   if (new_patch)
      {
      //
      // Find melodic or drum instrument that matches this bank/patch pair
      //

      U32 bank = (DLS->channel[ch].bank_MSB << 7) + 
                  DLS->channel[ch].bank_LSB;

      U32 patch = DLS->channel[ch].patch;

      DLS->channel[ch].instrument_index = DLS_instrument_lookup(bank, 
                                                                patch,
                                                                ch == PERCUSS_CHAN);
      }

   //
   // Process data-only messages (control/pitch traffic) for all channels,
   // including those that don't (yet) have a valid DLS instrument assignment
   //

   switch (st)
      {
      case EV_CONTROL:

         DLS->channel[ch].control[data_1] = (S8) data_2;

         switch (data_1)
            {
            case RESET_ALL_CTRLS:

               DLS_reset_all_controllers(ch, data_2, 0);
               break;

            case DATA_MSB:
               
               DLS_set_RPN(ch, data_2, -1);
               break;

            case DATA_LSB:

               DLS_set_RPN(ch, -1, data_2);
               break;
            }

         break;

      case EV_PITCH:

         DLS_event_pitch_wheel(ch, data_1, data_2);
         break;
      }

   //
   // If this MIDI channel has a valid DLS instrument assignment, insert
   // MIDI message and its associated instrument into queue for processing
   // at next service interval
   //
   // Note that we don't increment the queue tail index until the
   // event has been entirely written -- this avoids any possible 
   // synchronization issues between the timer and MIDI interrupt routines
   //

   if (DLS->channel[ch].instrument_index != -1)
      {
      DLS->MIDI_queue[DLS->MIDI_queue_head].status           = (U8) st;
      DLS->MIDI_queue[DLS->MIDI_queue_head].channel          = (U8) ch;
      DLS->MIDI_queue[DLS->MIDI_queue_head].data_1           = (U8) data_1;
      DLS->MIDI_queue[DLS->MIDI_queue_head].data_2           = (U8) data_2;
      DLS->MIDI_queue[DLS->MIDI_queue_head].instrument_index = DLS->channel[ch].instrument_index;

      if (DLS->MIDI_queue_head == MQ_SIZE-1)
         {
         DLS->MIDI_queue_head = 0;
         }
      else
         {
         DLS->MIDI_queue_head++;
         }
      }

   //
   // If this is a note message in a MIDI channel with a valid DLS
   // assignment, return 1 to indicate that the DLS synth has taken
   // responsibility for handling the message
   //
   // Otherwise, forward message on to other driver(s) if DLS_GM_PASSTHROUGH
   // set
   //

   if (((st == EV_NOTE_ON) || (st == EV_NOTE_OFF))
       &&
       (DLS->channel[ch].instrument_index != -1))
      {
      return 1;
      }
   else
      {
      if (AIL_get_preference(DLS_GM_PASSTHROUGH))
         {
         if (DLS->prev_event_fn)
            {
            return DLS->prev_event_fn(mdi, S, status, data_1, data_2);
            }
         else
            {
            return 0;
            }
         }
      else
         {
         return 1;
         }
      }
}

//############################################################################
//#                                                                          #
//#  Timbre-installation intercept function                                  #
//#                                                                          #
//#  Inhibit XMIDI driver from installing timbres which will be handled      #
//#  by XDIG                                                                 #
//#                                                                          #
//############################################################################

static S32 AILLIBCALLBACK DLS_TIMB_trap(HMDIDRIVER mdi, //)
                              S32        bank,
                              S32        patch)
{
   //
   // Search DLS instrument banks for given bank/patch entry
   //
   // If found, return 1 to inhibit driver installation of timbre
   // If not found, return 0 to allow driver to install timbre
   //

   if (DLS_instrument_lookup(bank, patch, 0) == -1)
      {
      if (DLS->prev_timb_fn)
         {
         return DLS->prev_timb_fn(mdi, bank, patch);
         }
      else
         {
         return 0;
         }
      }
   else
      {
      return 1;
      }
}

//############################################################################
//#                                                                          #
//#  Init synthesizer resources                                              #
//#                                                                          #
//############################################################################

S32 DLS_init (S32 FAR   *lpdwHandle, //)
              S32        dwFlags,
              HMDIDRIVER MIDI_driver,        // XMIDI driver handle
              S32        output_format,      // DIG_F format for output
              S32        output_sample_rate, // Output samples/second
              S32        stream_buffer_size, // Size of each buffer segment in bytes
              U32        user,               // User value passed to buffer callbacks
              AILDLSPCB  stream_poll_CB,     // Buffer polling handler
              AILDLSLCB  stream_lock_CB,     // Buffer lock handler
              AILDLSUCB  stream_unlock_CB)   // Buffer unlock handler
{
   dwFlags=dwFlags;

   DOLOCK();

   //
   // Allow only one synthesizer device instance to exist
   //

   if (DLS != NULL)
      {
      return DLS_ALREADY_OPENED;
      }

   DLS = (SYNTHDEVICE FAR *) AIL_mem_alloc_lock(sizeof(SYNTHDEVICE));

   if (DLS == NULL)
      {
      return DLS_OUT_OF_MEM;
      }

   if (lpdwHandle != NULL)
      {
      *lpdwHandle = (S32) DLS;
      }

   //
   // Initialize all fields to 0
   //

   AIL_memset(DLS, 0, sizeof(SYNTHDEVICE));

   if (InstrumentsInit()==0)
     return( DLS_OUT_OF_MEM );

   //
   // Copy parameters to synth structure
   //

   DLS->mdi                = MIDI_driver;
   DLS->output_format      = output_format;
   DLS->output_sample_rate = F32(output_sample_rate);
   DLS->stream_buffer_size = stream_buffer_size;
   DLS->user               = user;
   DLS->stream_poll_CB     = stream_poll_CB;
   DLS->stream_lock_CB     = stream_lock_CB;
   DLS->stream_unlock_CB   = stream_unlock_CB;

   DLS->channels_per_sample = ((output_format & DIG_F_STEREO_MASK) ? 2 : 1);
   DLS->bytes_per_channel   = ((output_format & DIG_F_16BITS_MASK) ? 2 : 1);

   DLS->bytes_per_sample    = DLS->channels_per_sample * DLS->bytes_per_channel;

   //
   // Select default DLS mixer kernel provider
   //

   MIXER_FLUSH MIXER_flush;
   MIXER_MERGE MIXER_merge;
   MIXER_COPY  MIXER_copy;

   RIB_INTERFACE_ENTRY MIXER[] =
      {
      FN(MIXER_flush),
      FN(MIXER_merge),
      FN(MIXER_copy)
      };

   HPROVIDER HP;
   
   RIB_enumerate_providers("MSS mixer services",
                            NULL,
                           &HP);

   RIB_request(HP,"MSS mixer services",MIXER);

   DLS->mixer_merge = MIXER_merge;
   DLS->mixer_flush = MIXER_flush;
   DLS->mixer_copy  = MIXER_copy;

   //
   // Init MIDI channel status
   //

   S32 i;
   for (i=0; i < 16; i++)
      {
      DLS_reset_all_controllers(i, 127, 1);
      }

   //
   // Init message queue
   //

   DLS->MIDI_queue_head = 0;
   DLS->MIDI_queue_tail = 0;

   //
   // Allocate list of VOICE structures and zero all fields
   //

   DLS->n_voices = AIL_get_preference(DLS_VOICE_LIMIT);

   DLS->voice_list = (VOICE FAR *) AIL_mem_alloc_lock(DLS->n_voices * sizeof(VOICE));

   if (DLS->voice_list == NULL)
      {
      DLS_shutdown(*lpdwHandle, 0);
      return DLS_OUT_OF_MEM;
      }

   AIL_memset(DLS->voice_list, 0, DLS->n_voices * sizeof(VOICE));

   //
   // Set service rate = value of DLS_TIMEBASE preference
   //

   DLS->service_rate_uS = 1000000.0F / F32(AIL_get_preference(DLS_TIMEBASE));

   DLS->timer_handle = AIL_register_timer(DLS_serve);

   if (DLS->timer_handle == -1)
      {
      DLS_shutdown(*lpdwHandle, 0);
      return DLS_INTERNAL_ERR;
      }

   AIL_set_timer_user(DLS->timer_handle, (U32) DLS);

//   debug_printf("WAILDLS1: Service rate = %ld Hz, %ld uS\n",
//      AIL_get_preference(DLS_TIMEBASE),
//      ((S32)(DLS->service_rate_uS)));

   AIL_set_timer_period(DLS->timer_handle,
                    ((S32)(DLS->service_rate_uS)));

   //
   // Initialize 32-bit-per-channel stereo or mono intermediate buffers
   //

   DLS->bytes_per_buffer = (DLS->stream_buffer_size / DLS->bytes_per_channel)
                           * sizeof(S32);

   for (i=0; i < N_BUILD_BUFFERS; i++)
      {
      DLS->build[i].status = BUILD_EMPTY;
      DLS->build[i].block = (S32 FAR *) AIL_mem_alloc_lock(DLS->bytes_per_buffer);

      if (DLS->build[i].block == NULL)
         {
         DLS_shutdown(*lpdwHandle, 0);
         return DLS_OUT_OF_MEM;
         }
      }

   //
   // Initialize FIFO queue of output buffer addresses
   //
   // This FIFO ensures that build buffers are passed to the output stream
   // in order of creation
   //

   DLS->build_queue_head = 0;
   DLS->build_queue_tail = 0;
   DLS->buffers_filled   = 0;

   DLS->current_build_buffer = 0;

//   debug_printf("WAILDLS1: Stream size=%ld\n", DLS->stream_buffer_size);

   //
   // Allocate reverb buffers equal in size to stream build buffer
   //

   DLS->n_reverb_buffers = 0;
   DLS->reverb_buffers = NULL;

#ifdef OLD_DLS_REVERB_PREFERENCES

   if (AIL_get_preference(DLS_ENABLE_GLOBAL_REVERB))
      {
      DLS->n_reverb_buffers = max(1,AIL_get_preference(DLS_GLOBAL_REVERB_TIME));

      DLS->reverb_buffers = (S32 FAR * FAR *)
         AIL_mem_alloc_lock(DLS->n_reverb_buffers * sizeof(S32 FAR *));

      if (DLS->reverb_buffers == NULL)
         {
         DLS_shutdown(*lpdwHandle, 0);
         return DLS_OUT_OF_MEM;
         }

      AIL_memset(DLS->reverb_buffers, 0, DLS->n_reverb_buffers * sizeof(S32 FAR *));

      for (i=0; i < DLS->n_reverb_buffers; i++)
         {
         DLS->reverb_buffers[i] = (S32 FAR *) AIL_mem_alloc_lock(DLS->bytes_per_buffer);

         if (DLS->reverb_buffers[i] == NULL)
            {
            DLS_shutdown(*lpdwHandle, 0);
            return DLS_OUT_OF_MEM;
            }

         DLS->mixer_flush(DLS->reverb_buffers[i],
                          DLS->bytes_per_buffer,
                          NULL,
                          0,
                          0);
         }
      }
#endif

   //
   // Register callback functions to trap MIDI and timbre-installation
   // events
   //

   DLS->prev_event_fn = AIL_register_event_callback (DLS->mdi, DLS_MIDI_trap);
   DLS->prev_timb_fn  = AIL_register_timbre_callback(DLS->mdi, DLS_TIMB_trap);

   //
   // Enable MMX processing if supported by CPU and requested by caller
   //

#ifdef IS_X86
   DLS->use_MMX = AIL_MMX_available();
#endif

   //
   // Init buffer timing vars
   //

   DLS->bytes_to_write = 0;
   DLS->last_interval_us_count = AIL_us_count();

   //
   // Enable DLS processing (ON by default, until turned off by DLS sysex
   // message type 02)
   //

   DLS->enabled = 1;

   //
   // Start background timer service and return success
   //

   AIL_start_timer(DLS->timer_handle);

   return DLS_NOERR;
}

//############################################################################
//#                                                                          #
//#  Shut down synthesizer resources                                         #
//#                                                                          #
//############################################################################

S32 DLS_shutdown (S32 dwDLSHandle, //)
                  S32 dwFlags)
{
   dwFlags=dwFlags;

   if (DLS == NULL)
      {
      return DLS_NOT_INIT;
      }

   if (dwDLSHandle != (S32) DLS)
      {
      return DLS_INVALID_HANDLE;
      }

   //
   // First, stop all background processing by restoring previous trap
   // callback functions and killing the service timer
   //

    AIL_register_event_callback(DLS->mdi, DLS->prev_event_fn);
    DLS->prev_event_fn = NULL;

    AIL_register_timbre_callback(DLS->mdi, DLS->prev_timb_fn);
    DLS->prev_timb_fn = NULL;

   if (DLS->timer_handle != -1)
      {
      AIL_release_timer_handle(DLS->timer_handle);
      DLS->timer_handle = -1;
      }

   //
   // Delay 16 milliseconds to ensure background thread has time to finish
   // before destroying resources it depends on
   //

   AIL_delay(1);

   //
   // Unload all files and instruments
   //

   DLSUnloadAll(dwDLSHandle, 0);

   InstrumentsDeinit();

   //
   // Release build buffers
   //

   for (S32 i=0; i < N_BUILD_BUFFERS; i++)
      {
      if (DLS->build[i].block != NULL)
         {
         AIL_mem_free_lock(DLS->build[i].block);
         DLS->build[i].block = NULL;
         }
      }

   //
   // Release reverb buffers
   //

   if (DLS->reverb_buffers != NULL)
      {
      for (S32 i=0; i < DLS->n_reverb_buffers; i++)
         {
         if (DLS->reverb_buffers[i] != NULL)
            {
            AIL_mem_free_lock(DLS->reverb_buffers[i]);
            DLS->reverb_buffers[i] = NULL;
            }
         }

      AIL_mem_free_lock(DLS->reverb_buffers);
      DLS->reverb_buffers = NULL;
      }

   //
   // Release VOICE structures
   //

   if (DLS->voice_list != NULL)
      {
      AIL_mem_free_lock(DLS->voice_list);
      DLS->voice_list = NULL;
      }

   //
   // Release DLS structure
   //

   AIL_mem_free_lock(DLS);
   DLS = NULL;

//   debug_printf("WAILDLS1: Shutdown successful\n");

   return DLS_NOERR;
}

#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## End of locked code                                                     ##
//##                                                                        ##
//############################################################################

static void AIL_lock_end(void)
{
   if (locked)
      {
      AIL_vmm_unlock_range(AIL_lock_start, AIL_lock_end);

      locked = 0;
      }
}

#endif

