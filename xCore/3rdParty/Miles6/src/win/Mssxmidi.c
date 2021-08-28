//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  MSSXMIDI.C: API module and support routines for XMIDI playback        ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 9.0                  ##
//##                                                                        ##
//##  Version 1.00 of 15-Feb-95: Derived from AILXMIDI.C V1.03              ##
//##          1.01 of 19-Jun-95: Do not reset loop count at sequence start  ##
//##                             MIDI master volume added                   ##
//##                             XMIDI Channel Mute controller added        ##
//##                             Beat/bar callbacks added                   ##
//##          1.02 of 10-Aug-95: Exact TIMB chunk size used in copy         ##
//##                             Compiled with -Zp1 for XMIDI branching     ##
//##                             Reset loop count to 1 at end of playback   ##
//##                             Initialize EOS at allocation time          ##
//##                             Don't skip event after branch              ##
//##          1.04 of 15-Feb-96: Fixes for optimization and multiple        ##
//##                             16 bit loads (JKR)                         ##
//##          1.05 of 08-May-96: Fix XMIDI error found by Adeline           ##
//##          1.06 of  2-Nov-96: Changes made to support DLS                ##
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

#include "limits.h"
#include "stdlib.h"

#include "mss.h"

#include "imssapi.h"

//
// Channel lock status
//

#define UNLOCKED  0
#define LOCKED    1
#define PROTECTED 2

//
// Tag setting macro
//

#define SETTAG(var,tag) AIL_memcpy(var,tag,4);


//
// Internal prototypes
//

static void XMI_flush_channel_notes(HSEQUENCE S, S32 channel);

//############################################################################
//##                                                                        ##
//## Locked static data                                                     ##
//##                                                                        ##
//############################################################################

//
// XMI_serve()
//

static U32       entry = 0;
static HSEQUENCE S;
static S32       i,j,n,sequence_done;
static S32       q,t;
static U32       channel,status,type,len;
static U8 const  FAR *ptr;
static U8 const  FAR *event;


//############################################################################
//##                                                                        ##
//## Return size in bytes of MIDI channel voice message, based on type      ##
//##                                                                        ##
//############################################################################

static S32 AILCALL XMI_message_size(S32 status)
{
   switch (status & 0xf0)
      {
      case EV_NOTE_OFF  :
      case EV_NOTE_ON   :
      case EV_POLY_PRESS:
      case EV_CONTROL   :
      case EV_PITCH     : return 3;

      case EV_PROGRAM   :
      case EV_CHAN_PRESS: return 2;
      }

   return 0;
}


//############################################################################
//##                                                                        ##
//## Write channel voice message to MIDI driver buffer                      ##
//##                                                                        ##
//############################################################################

static void XMI_MIDI_message(HMDIDRIVER mdi, //)
                                    S32            status,
                                    S32            d1,
                                    S32            d2)
{
   U32 dwMsg;

   dwMsg = (status & 0xff) | ((d1 & 0x7f) << 8) | ((d2 & 0x7f) << 16);

   if (!mdi->released)
      {
      if (mdi->deviceid != MIDI_NULL_DRIVER)
         {
         midiOutShortMsg(mdi->hMidiOut, dwMsg);
         }

      //
      // If requested, send every MIDI Note Off message twice to help
      // low-end MIDI interfaces like Sound Blasters get the message
      //

      if (AIL_preference[MDI_DOUBLE_NOTE_OFF])
         {
         if ( ((status & 0xf0) == EV_NOTE_OFF)  ||
              (((status & 0xf0) == EV_NOTE_ON) && (d2 == 0)))
               {
               if (mdi->deviceid != MIDI_NULL_DRIVER)
                  {  
                  midiOutShortMsg(mdi->hMidiOut, dwMsg);
                  }
               }
         }
      }
}

//############################################################################
//##                                                                        ##
//## Write system exclusive message to MIDI driver buffer                   ##
//##                                                                        ##
//############################################################################

static void XMI_sysex_message(HMDIDRIVER mdi, //)
                                     U8 const    FAR *message,
                                     S32        size)
{
   U8 const FAR *ptr;
   S32 i;

   //
   // Get # of bytes in VLN length specifier
   //

   ptr = (U8 FAR *) message + 1;

   XMI_read_VLN(&ptr);

   //
   // Get size of message less VLN length
   //

   size = (size - AIL_ptr_dif(ptr, message)) + 1;

   //
   // Copy and transmit message
   //

   *(U8 FAR *) mdi->mhdr->lpData = *message;

   size = min(size, AIL_preference[MDI_SYSEX_BUFFER_SIZE]);

   AIL_memcpy (((U8 FAR *) mdi->mhdr->lpData) + 1,
               ptr,
               size);

   //
   // Send to channel-voice message trap as series of SYSEX_BYTE pseudo-
   // control events
   //

   if (mdi->event_trap != NULL)
      {
      for (i=0; i < size; i++)
         {
         S32 result;

         U8 val = ((U8 FAR *) mdi->mhdr->lpData)[i];
         U8 ch  = 0;

         //
         // Send any data bytes > 0x80 on channel 2 with high bit masked,
         // all others on channel 1
         //

         if (val > 0x80)
            {
            val &= 0x7f;
            ch = 1;
            }

         MSS_do_cb5_with_ret(result, (AILEVENTCB), mdi->event_trap,
            mdi->callingDS,mdi->EVENT_IsWin32s,0,
            mdi,NULL,EV_CONTROL | ch,SYSEX_BYTE,val);
         }
      }

   //
   // Transmit to driver
   //

   mdi->mhdr->dwBufferLength = size;

   if (!mdi->released)
      {
      if (mdi->deviceid != MIDI_NULL_DRIVER)
         {
         midiOutLongMsg(mdi->hMidiOut,
                        mdi->mhdr,
                        sizeof(MIDIHDR));
         }
      }
}

//############################################################################
//##                                                                        ##
//## Read control log value                                                 ##
//##                                                                        ##
//############################################################################

static S32 XMI_read_log(CTRL_LOG FAR *log, S32 status, S32 data_1)
{
   S32 st;
   S32 ch;

   st = status & 0xf0;
   ch = status & 0x0f;

   switch (st)
      {
      case EV_PROGRAM:
         return log->program[ch];

      case EV_PITCH:
         return (log->pitch_h[ch] << 7) | log->pitch_l[ch];

      case EV_CONTROL:

         switch (data_1)
            {
            case CHAN_LOCK:
               return log->c_lock[ch];

            case CHAN_PROTECT:
               return log->c_prot[ch];

            case CHAN_MUTE:
               return log->c_mute[ch];

            case VOICE_PROTECT:
               return log->c_v_prot[ch];

            case PATCH_BANK_SEL:
               return log->bank[ch];

            case GM_BANK_LSB:
               return log->gm_bank_l[ch];

            case GM_BANK_MSB:
               return log->gm_bank_m[ch];

            case INDIRECT_C_PFX:
               return log->indirect[ch];

            case CALLBACK_TRIG:
               return log->callback[ch];

            case MODULATION:
               return log->mod[ch];

            case PART_VOLUME:
               return log->vol[ch];

            case PANPOT:
               return log->pan[ch];

            case EXPRESSION:
               return log->exp[ch];

            case SUSTAIN:
               return log->sus[ch];

            case REVERB:
               return log->reverb[ch];

            case CHORUS:
               return log->chorus[ch];

            case RPN_LSB:
               return log->RPN_L[ch];

            case RPN_MSB:
               return log->RPN_M[ch];

            case PB_RANGE:
               return log->bend_range[ch];

            default:
               return -1;
            }

      default:
         return -1;
      }
}

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

            case CHAN_MUTE:
               log->c_mute[ch]     = (S8) data_2;
               break;

            case VOICE_PROTECT:
               log->c_v_prot[ch]   = (S8) data_2;
               break;

            case PATCH_BANK_SEL:
               log->bank[ch]       = (S8) data_2;
               break;

            case GM_BANK_LSB:
               log->gm_bank_l[ch]  = (S8) data_2;
               break;

            case GM_BANK_MSB:
               log->gm_bank_m[ch]  = (S8) data_2;
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

            case RPN_LSB:
               log->RPN_L[ch]    = (S8) data_2;
               break;

            case RPN_MSB:
               log->RPN_M[ch]    = (S8) data_2;
               break;

            case DATA_MSB:

               //
               // If current RPN is 0 0 (bender range), fall through to
               // log this MSB setting under the PB_RANGE pseudo-control
               //
               // Otherwise ignore it (other RPNs not supported by XMIDI
               // standard, although synthesizers may recognize 
               // them)
               //

               if ((log->RPN_L[ch] != 0) || (log->RPN_M[ch] != 0))
                  {
                  break;
                  }

            case PB_RANGE:
               log->bend_range[ch] = (S8) data_2;
               break;
            }
      }
}

//############################################################################
//##                                                                        ##
//## Send MIDI channel voice message associated with a specific sequence    ##
//##                                                                        ##
//## Includes controller logging and XMIDI extensions                       ##
//##                                                                        ##
//## Warnings: ICA_enable should be 0 when calling outside XMIDI event loop ##
//##           May be recursively called by XMIDI controller handlers       ##
//##                                                                        ##
//############################################################################

static void XMI_send_channel_voice_message(HSEQUENCE S, //)
                                                  S32       status,
                                                  S32       data_1,
                                                  S32       data_2,
                                                  S32       ICA_enable)
{
   S32             st,i;
   S32             phys,log;
   HMDIDRIVER mdi;
   S32             result;

   //
   // Get driver for sequence
   //

   mdi = S->driver;

   //
   // Translate logical to physical channel
   //

   st  = status & 0xf0;
   log = status & 0x0f;

   phys = S->chan_map[log];

   //
   // If indirect controller override active, substitute indirect
   // controller value for data_2, and cancel indirect override
   //

   if ((st == EV_CONTROL) &&
       (ICA_enable)       &&
       (S->shadow.indirect[log] != -1))
      {
      data_2 = S->shadow.indirect[log];

      S->shadow.indirect[log] = -1;
      }

   //
   // Update local MIDI status log
   //

   if ((st == EV_CONTROL) ||
       (st == EV_PROGRAM) ||
       (st == EV_PITCH  ))
      {
      XMI_write_log(&S->shadow,st | log,data_1,data_2);
      }

   //
   // If this is a Control Change event, handle special XMIDI controllers
   // and extended features
   //
   // Controller handlers should 'break' to pass message on to driver, or
   // 'return' if message is not to be transmitted
   //

   if (st == EV_CONTROL)
      {
      switch (data_1)
         {
         //
         // INDIRECT_C_PFX: Override value of next controller event
         //                 with value from nth index in
         //                 application's Indirect Controller Array
         //

         case INDIRECT_C_PFX:

            if (S->ICA)
               {      
               S->shadow.indirect[log] = S->ICA[data_2];
               }
            break;

         //
         // CALLBACK_PFX:   Override value of next controller event
         //                 with value from user callback function
         //

         case CALLBACK_PFX:

            if (S->prefix_callback != NULL)
               {
               MSS_do_cb3_with_ret( S->shadow.indirect[log], (AILPREFIXCB),
                  S->prefix_callback,S->driver->callingDS, S->PREFIX_IsWin32s, data_2,
                  S, log, data_2);
               }
            break;

         //
         // PB_RANGE:       Control bender range by first sending RPN 0 0
         //

         case PB_RANGE:

            XMI_send_channel_voice_message(S,
                                           EV_CONTROL | log,
                                           RPN_LSB,
                                           0,
                                           0);

            XMI_send_channel_voice_message(S,
                                           EV_CONTROL | log,
                                           RPN_MSB,
                                           0,
                                           0);

            XMI_send_channel_voice_message(S,
                                           EV_CONTROL | log,
                                           DATA_LSB,
                                           0,
                                           0);
            break;

         //
         // PART_VOLUME:    Scale volume according to sequence's current
         //                 volume setting and overall driver master volume
         //

         case PART_VOLUME:

            data_2 = (data_2    *
                      S->volume *
                      mdi->master_volume) / (127*127);

            if (data_2 > 127)
               {
               data_2 = 127;
               }

            if (data_2 < 0)
               {
               data_2 = 0;
               }

            break;

         //
         // CLEAR_BEAT_BAR: Reset beat/bar count to 0:0, clear fraction,
         //                 and predecrement to compensate for current
         //                 interval
         //

         case CLEAR_BEAT_BAR:

            S->beat_count     = 0;
            S->measure_count  = 0;

            S->beat_fraction  = 0;
            S->beat_fraction -= S->time_fraction;

            //
            // If beat/bar callback function active, trigger it
            //

            if (S->beat_callback != NULL)
               {
               MSS_do_cb4( (AILBEATCB),
                 S->beat_callback, S->driver->callingDS, S->BEAT_IsWin32s,
                   mdi, S, 0, 0);
               }

            return;

         //
         // CALLBACK_TRIG:  Call XMIDI user function, passing sequence
         //                 handle, channel #, and callback controller value
         //

         case CALLBACK_TRIG:

            if (S->trigger_callback != NULL)
               {
               MSS_do_cb3( (AILTRIGGERCB),
                 S->trigger_callback, S->driver->callingDS, S->TRIGGER_IsWin32s,
                 S, log, data_2);
               }

            return;

         //
         // FOR_LOOP:       Mark the start of an XMIDI FOR...NEXT/BREAK loop
         //
         //                  1-127: Play n iterations
         //                      0: Play indefinitely
         //

         case FOR_LOOP:

            //
            // Find first available FOR loop entry
            //

            for (i=0; i < FOR_NEST; i++)
               {
               if (S->FOR_loop_count[i] == -1)
                  {
                  break;
                  }
               }

            //
            // If none available, ignore controller -- else set loop pointer
            // and count
            //

            if (i == FOR_NEST)
               {
               return;
               }

            S->FOR_loop_count [i] = data_2;
            S->FOR_ptrs       [i] = S->EVNT_ptr;

            return;

         //
         // NEXT_LOOP:      Mark the end of an XMIDI FOR...NEXT/BREAK loop
         //
         //                 64-127: Continue looping until FOR count reached
         //                   0-63: Break from current loop
         //

         case NEXT_LOOP:

            //
            // Otherwise, find innermost (most recent) FOR loop
            //

            for (i=FOR_NEST-1; i >= 0; i--)
               {
               if (S->FOR_loop_count[i] != -1)
                  {
                  break;
                  }
               }

            //
            // Break out of loop if value < 64
            //

            if (data_2 < 64)
               {
               S->FOR_loop_count[i] = -1;
               return;
               }

            //
            // If no FOR loops active, ignore controller
            //

            if (i == -1)
               {
               return;
               }

            //
            // If loop count == 0, loop indefinitely
            //

            if (S->FOR_loop_count[i] == 0)
               {
               S->EVNT_ptr = S->FOR_ptrs[i];
               return;
               }

            //
            // Otherwise, decrement loop count and, if the result is not
            // zero, loop back to FOR controller's location
            //
            // When loop finishes, set loop count to -1 to indicate
            // availability of FOR loop entry
            //

            if (--S->FOR_loop_count[i] != 0)
               {
               S->EVNT_ptr = S->FOR_ptrs[i];
               }
            else
               {
               S->FOR_loop_count[i] = -1;
               }

            return;

         //
         // SEQ_BRANCH:     Branch immediately to specified Sequence Branch
         //                 Index point
         //

         case SEQ_BRANCH:

            AIL_branch_index(S,data_2);
            return;

         //
         // CHAN_PROTECT:   Protect physical channel from being locked by
         //                 API or another sequence
         //
         //                 64-127: Enable lock protection
         //                   0-63: Disable lock protection
         //

         case CHAN_PROTECT:

            //
            // If channel is already locked, it's too late to protect it
            //

            if (mdi->lock[phys] == LOCKED)
               {
               return;
               }

            //
            // Otherwise, set UNLOCKED (by implication, UNPROTECTED)
            // or PROTECTED
            //

            if (data_2 < 64)
               {
               mdi->lock[phys] = UNLOCKED;
               }
            else
               {
               mdi->lock[phys] = PROTECTED;
               }

            return;

         //
         // CHAN_LOCK:      Lock/unlock physical channel for use by this
         //                 sequence's logical channel
         //
         //                 64-127: Search for and lock physical channel
         //                   0-63: Release physical channel to prior user
         //

         case CHAN_LOCK:

            if (data_2 >= 64)
               {
               //
               // Channel cannot be redundantly locked
               //

               if (mdi->lock[phys] == LOCKED)
                  {
                  return;
                  }

               //
               // Lock a physical channel (1-based), if possible
               //

               i = AIL_lock_channel(mdi);

               if (!i)
                  {
                  return;
                  }

               //
               // Map sequence channel (0-based) to locked physical
               // channel (1-based)
               //

               AIL_map_sequence_channel(S,log+1,i);

               //
               // Keep track of which sequence locked the channel, so
               // other sequences can be inhibited from writing to it
               //

               mdi->locker[i-1] = S;
               }
            else
               {
               //
               // Channel must be locked in order to release it
               //

               if (mdi->lock[phys] != LOCKED)
                  {
                  return;
                  }

               //
               // Turn all notes off in channel
               //

               XMI_flush_channel_notes(S,log);

               //
               // Release locked physical channel (1-based)
               //

               AIL_release_channel(mdi,phys+1);

               //
               // Re-establish normal physical channel mapping
               // for logical channel
               //

               AIL_map_sequence_channel(S,log+1,log+1);
               }

            return;
         }
      }

   //
   // If this physical channel is locked by the API or by another
   // sequence, return
   //

   if ((mdi->lock[phys] == LOCKED) && (mdi->locker[phys] != S))
      {
      return;
      }

   //
   // Keep track of overall physical channel note counts
   //

   if (st == EV_NOTE_ON)
      {
      ++mdi->notes[phys];
      }
   else if (st == EV_NOTE_OFF)
      {
      --mdi->notes[phys];
      }

   //
   // Keep track of most recent sequence to use channel
   //

   mdi->user[phys] = S;

   //
   // If logical channel muted with XMIDI Channel Mute controller (107),
   // return without transmitting note-on events
   //

   if ((st == EV_NOTE_ON)
         &&
       (S->shadow.c_mute[log] >= 64))
      {
      return;
      }

   //
   // Allow application a chance to process the event...
   //

   if (mdi->event_trap != NULL)
      {
      MSS_do_cb5_with_ret( result, (AILEVENTCB), mdi->event_trap, mdi->callingDS,mdi->EVENT_IsWin32s, 0,
         mdi,S,st | phys,data_1,data_2);

      if (result)
         {
         return;
         }
      }

   //
   // ...otherwise, transmit message to driver
   //

   XMI_MIDI_message(mdi,st | phys,data_1,data_2);
}

//############################################################################
//##                                                                        ##
//## Flush sequence note queue                                              ##
//##                                                                        ##
//############################################################################

static void  XMI_flush_note_queue(HSEQUENCE S)
{
   S32 i,nmsgs;

   nmsgs = 0;

   for (i=0; i < MAX_NOTES; i++)
      {
      if (S->note_chan[i] == -1)
         {
         continue;
         }

      //
      // Send MIDI Note Off message
      //

      XMI_send_channel_voice_message(S,
                                     S->note_chan[i] | EV_NOTE_OFF,
                                     S->note_num [i],
                                     0,
                                     0);
      //
      // Release queue entry and increment "note off" count
      //

      S->note_chan[i] = -1;

      nmsgs++;
      }

   S->note_count = 0;

   //
   // If any messages were sent, delay before returning to give
   // slower MPU-401 devices enough time to process MIDI data
   //

   if ((nmsgs) && (!AIL_background()))
      {
      AIL_delay(3);
      }
}

//############################################################################
//##                                                                        ##
//## Flush notes in one channel only                                        ##
//##                                                                        ##
//############################################################################

static void XMI_flush_channel_notes(HSEQUENCE S, S32 channel)
{
   S32 i;

   for (i=0; i < MAX_NOTES; i++)
      {
      if (S->note_chan[i] != channel)
         {
         continue;
         }

      //
      // Send MIDI Note Off message
      //

      XMI_send_channel_voice_message(S,
                                     S->note_chan[i] | EV_NOTE_OFF,
                                     S->note_num [i],
                                     0,
                                     0);
      //  
      // Release queue entry
      //

      S->note_chan[i] = -1;
      }
}

//############################################################################
//##                                                                        ##
//## Transmit logged channel controller values                              ##
//##                                                                        ##
//############################################################################

static void XMI_refresh_channel(HSEQUENCE S, S32 ch)
{
   //
   // Set bank and patch values first ...
   //

   if (S->shadow.gm_bank_l[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     GM_BANK_LSB,
                                     S->shadow.gm_bank_l[ch],
                                     0);
      }

   if (S->shadow.gm_bank_m[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     GM_BANK_MSB,
                                     S->shadow.gm_bank_m[ch],
                                     0);
      }

   if (S->shadow.bank[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     PATCH_BANK_SEL,
                                     S->shadow.bank[ch],
                                     0);
      }

   if (S->shadow.program[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_PROGRAM | ch,
                                     S->shadow.program[ch],
                                     0,
                                     0);
      }

   //
   // ... followed by pitch bender ...
   //

   if (S->shadow.pitch_h[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_PITCH | ch,
                                     S->shadow.pitch_l[ch],
                                     S->shadow.pitch_h[ch],
                                     0);
      }

   //
   // ... followed by controller events
   //

   if (S->shadow.c_mute[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     CHAN_MUTE,
                                     S->shadow.c_mute[ch],
                                     0);
      }

   if (S->shadow.c_prot[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     CHAN_PROTECT,
                                     S->shadow.c_prot[ch],
                                     0);
      }

   if (S->shadow.c_v_prot[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     VOICE_PROTECT,
                                     S->shadow.c_v_prot[ch],
                                     0);
      }

   if (S->shadow.mod[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     MODULATION,
                                     S->shadow.mod[ch],
                                     0);
      }

   if (S->shadow.vol[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     PART_VOLUME,
                                     S->shadow.vol[ch],
                                     0);
      }

   if (S->shadow.pan[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     PANPOT,
                                     S->shadow.pan[ch],
                                     0);
      }

   if (S->shadow.exp[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     EXPRESSION,
                                     S->shadow.exp[ch],
                                     0);
      }

   if (S->shadow.sus[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     SUSTAIN,
                                     S->shadow.sus[ch],
                                     0);
      }

   if (S->shadow.reverb[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     REVERB,
                                     S->shadow.reverb[ch],
                                     0);
      }

   if (S->shadow.chorus[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     CHORUS,
                                     S->shadow.chorus[ch],
                                     0);
      }

   if (S->shadow.bend_range[ch] != -1)
      {
      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     PB_RANGE,
                                     S->shadow.bend_range[ch],
                                     0);
      }

   //
   // Disregard callback member -- present only for use with
   // AIL_controller_value()
   //
}

//############################################################################
//##                                                                        ##
//## Update all channels in sequence based on differences between two state ##
//## tables                                                                 ##
//##                                                                        ##
//## Controllers with a different value will be updated; upon exit, the     ##
//## contents of the original state table will be identical to those of the ##
//## updated version                                                        ##
//##                                                                        ##
//############################################################################

static void XMI_update_sequence(HSEQUENCE S, //)
                                CTRL_LOG *original,
                                CTRL_LOG *updated)
{
   S32 ch;

   for (ch = MIN_CHAN; ch <= MAX_CHAN; ch++)
      {
      //
      // Set bank and patch values first ...
      //

      if ((updated->gm_bank_l[ch] != original->gm_bank_l[ch]) &&
          (updated->gm_bank_l[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       GM_BANK_LSB,
                                       updated->gm_bank_l[ch],
                                       0);
         }

      if ((updated->gm_bank_m[ch] != original->gm_bank_m[ch]) &&
          (updated->gm_bank_m[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       GM_BANK_MSB,
                                       updated->gm_bank_m[ch],
                                       0);
         }

      if ((updated->bank[ch] != original->bank[ch]) &&
          (updated->bank[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       PATCH_BANK_SEL,
                                       updated->bank[ch],
                                       0);
         }

      if ((updated->program[ch] != original->program[ch]) &&
          (updated->program[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_PROGRAM | ch,
                                       updated->program[ch],
                                       0,
                                       0);
         }

      //
      // ... followed by pitch bender ...
      //

      if ((updated->pitch_h[ch] != original->pitch_h[ch]) &&
          (updated->pitch_h[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_PITCH | ch,
                                       updated->pitch_l[ch],
                                       updated->pitch_h[ch],
                                       0);
         }

      //
      // ... followed by controller events
      //

      if ((updated->c_mute[ch] != original->c_mute[ch]) &&
          (updated->c_mute[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       CHAN_MUTE,
                                       updated->c_mute[ch],
                                       0);
         }

      if ((updated->c_prot[ch] != original->c_prot[ch]) &&
          (updated->c_prot[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       CHAN_PROTECT,
                                       updated->c_prot[ch],
                                       0);
         }

      if ((updated->c_v_prot[ch] != original->c_v_prot[ch]) &&
          (updated->c_v_prot[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       VOICE_PROTECT,
                                       updated->c_v_prot[ch],
                                       0);
         }

      if ((updated->mod[ch] != original->mod[ch]) &&
          (updated->mod[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       MODULATION,
                                       updated->mod[ch],
                                       0);
         }

      if ((updated->vol[ch] != original->vol[ch]) &&
          (updated->vol[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       PART_VOLUME,
                                       updated->vol[ch],
                                       0);
         }

      if ((updated->pan[ch] != original->pan[ch]) &&
          (updated->pan[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       PANPOT,
                                       updated->pan[ch],
                                       0);
         }

      if ((updated->exp[ch] != original->exp[ch]) &&
          (updated->exp[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       EXPRESSION,
                                       updated->exp[ch],
                                       0);
         }

      if ((updated->sus[ch] != original->sus[ch]) &&
          (updated->sus[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       SUSTAIN,
                                       updated->sus[ch],
                                       0);
         }

      if ((updated->reverb[ch] != original->reverb[ch]) &&
          (updated->reverb[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       REVERB,
                                       updated->reverb[ch],
                                       0);
         }

      if ((updated->chorus[ch] != original->chorus[ch]) &&
          (updated->chorus[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       CHORUS,
                                       updated->chorus[ch],
                                       0);
         }

      if ((updated->bend_range[ch] != original->bend_range[ch]) &&
          (updated->bend_range[ch] != -1))
         {
         XMI_send_channel_voice_message(S,
                                       EV_CONTROL | ch,
                                       PB_RANGE,
                                       updated->bend_range[ch],
                                       0);
         }

      //
      // Create delay if not in background, to keep from overflowing hardware
      // FIFOs
      //

      if ((!(ch & 3)) && (!AIL_background()))
         {
         AIL_delay(1);
         }
      }
}

//############################################################################
//##                                                                        ##
//## Initialize state table entries                                         ##
//##                                                                        ##
//############################################################################

static void XMI_init_sequence_state(HSEQUENCE S)
{
   S32             i;
   static CTRL_LOG temp_log;

   //
   // Initialize logical-physical channel map to identity set
   //

   for (i=0; i < NUM_CHANS; i++)
      {
      S->chan_map[i] = i;
      }

   //
   // Initialize all logged controllers to -1
   //

   AIL_memset(&temp_log, -1, sizeof(CTRL_LOG));

   AIL_memcpy(&S->shadow, &temp_log, sizeof(CTRL_LOG));

   
   //
   // Initialize FOR loop counters
   //

   for (i=0; i < FOR_NEST; i++)
      {
      S->FOR_loop_count[i] = -1;
      }

   //
   // Initialize note queue
   //

   for (i=0; i < MAX_NOTES; i++)
      {
      S->note_chan[i] = -1;
      }

   S->note_count = 0;

   //
   // Initialize timing variables
   //
   // Default to 4/4 time at 120 beats/minute
   //

   S->interval_count =  0;

   S->beat_count     =  0;
   S->measure_count  = -1;

   S->beat_fraction  =  0;
   S->time_fraction  =  0;

   S->time_numerator =  4;

   S->time_per_beat  =  500000*16;

   S->interval_num   =  0;
}

//############################################################################
//##                                                                        ##
//## Reset sequence pointers and initialize state table entries             ##
//##                                                                        ##
//############################################################################

static void XMI_rewind_sequence(HSEQUENCE S)
{
   //
   // Initialize sequence state table
   //

   XMI_init_sequence_state(S);

   //
   // Initialize event pointer to start of XMIDI EVNT chunk data
   //

   S->EVNT_ptr = (U8 FAR *) S->EVNT + 8;
}

//############################################################################
//##                                                                        ##
//## Send updated volume control messages to all channels in sequence       ##
//##                                                                        ##
//############################################################################

static void XMI_update_volume(HSEQUENCE S)
{
   S32 ch;

   for (ch=0; ch < NUM_CHANS; ch++)
      {
      //
      // Skip channels with no volume controller history
      //

      if (S->shadow.vol[ch] == -1)
         {
         continue;
         }

      //
      // Retransmit volume values to permit volume scaling
      //

      XMI_send_channel_voice_message(S,
                                     EV_CONTROL | ch,
                                     PART_VOLUME,
                                     S->shadow.vol[ch],
                                     0);
      }
}

//############################################################################
//##                                                                        ##
//## Timer interrupt routine for XMIDI sequencing                           ##
//##                                                                        ##
//############################################################################

void WINAPI XMI_serve(HMDIDRIVER mdi) // WINAPI because same DS as caller
{
   //
   // Exit at once if service disabled
   //

   if (mdi->disable)
      {
      return;
      }

   //
   // Disallow re-entrant calls (but leave interrupts enabled so that
   // .DIG processing can run in a separate thread)
   //

   MSSLockedIncrementPtr(entry);

   if (entry!=1)
      {
      MSSLockedDecrementPtr(entry);
      return;
      }

   //
   // Process all active sequences
   //

   for (n = mdi->n_sequences,S = &mdi->sequences[0]; n; --n,++S)
      {
      //
      // Skip sequence if stopped, finished, or not allocated
      //

      if (S->status != SEQ_PLAYING)
         {
         continue;
         }

      sequence_done = 0;

      //
      // Bump sequence interval number counter
      //

      ++S->interval_num;

      //
      // Add tempo percent to tempo overflow counter
      //

      S->tempo_error += S->tempo_percent;

      //
      // Execute interval zero, one, or more times, depending on tempo DDA
      // count
      //

      while (S->tempo_error >= 100)
         {
         S->tempo_error -= 100;

         //
         // Decrement note times and turn off any expired notes
         //

         if (S->note_count > 0)
            {
            for (i=0; i < MAX_NOTES; i++)
               {
               if (S->note_chan[i] == -1)
                  {
                  continue;
                  }

               if (--S->note_time[i] > 0)
                  {
                  continue;
                  }

               //
               // Note expired -- send MIDI Note Off message
               //

               XMI_send_channel_voice_message(S,
                                              S->note_chan[i] | EV_NOTE_OFF,
                                              S->note_num [i],
                                              0,
                                              0);
               //
               // Release queue entry, decrement sequence note count,
               // and exit loop if no active sequence notes left
               //

               S->note_chan[i] = -1;

               if (--S->note_count == 0)
                  {
                  break;
                  }
               }
            }

         //
         // Decrement interval delta-time count and process next interval if
         // ready
         //

         if (--S->interval_count <= 0)
            {
            //
            // Fetch events until next interval's delta-time byte (< 0x80)
            //

            while ((!sequence_done) && ((status = *S->EVNT_ptr) >= 0x80))
               {
               switch (status)
                  {
                  //
                  // Process MIDI meta-event
                  //

                  case EV_META:

                     S->EVNT_ptr=AIL_ptr_add(S->EVNT_ptr,1);
                     type = *(S->EVNT_ptr);

                     S->EVNT_ptr=AIL_ptr_add(S->EVNT_ptr,1);
                     len = XMI_read_VLN(&S->EVNT_ptr);

                     switch (type)
                        {
                        //
                        // End-of-track event (XMIDI end-of-sequence)
                        //

                        case META_EOT:

                           //
                           // Set sequence_done to inhibit post-interval
                           // processing

                           sequence_done = 1;

                           //
                           // If loop count == 0, loop indefinitely
                           //
                           // Otherwise, decrement loop count and, if the
                           // result is not zero, return to beginning of
                           // sequence
                           //

                           if ((S->loop_count == 0)
                              ||
                              (--S->loop_count != 0))
                              {
                              S->EVNT_ptr = (U8 FAR *)AIL_ptr_add( S->EVNT , 8);

                              S->beat_count    =  0;
                              S->measure_count = -1;
                              S->beat_fraction =  0;

                              //
                              // If beat/bar callback function active,
                              // trigger it
                              //

                              if (S->beat_callback != NULL)
                                 {
                                 MSS_do_cb4( (AILBEATCB),
                                    S->beat_callback,S->driver->callingDS, S->BEAT_IsWin32s,
                                      S->driver, S, 0, 0);
                                 }

                              break;
                              }

                           //
                           // Otherwise, stop sequence and set status
                           // to SEQ_DONE
                           //
                           // Reset loop count to 1, to enable unlooped replay
                           //

                           S->loop_count = 1;

                           AIL_stop_sequence(S);
                           S->status = SEQ_DONE;

                           //
                           // Invoke end-of-sequence callback function, if any
                           //

                           if (S->EOS != NULL)
                              {
                              MSS_do_cb1( (AILSEQUENCECB),
                                 S->EOS,S->driver->callingDS,S->EOS_IsWin32s,
                                 S);
                              }

                           break;

                        //
                        // Tempo event
                        //

                        case META_TEMPO:

                           //
                           // Calculate tempo as 1/16-uS per MIDI
                           // quarter-note
                           //

                           t = ((S32) *(S->EVNT_ptr  ) << 16) +
                               ((S32) *((U8 FAR*)AIL_ptr_add(S->EVNT_ptr,1)) << 8 ) +
                               ((S32) *((U8 FAR*)AIL_ptr_add(S->EVNT_ptr,2))      );

                           S->time_per_beat = t * 16;

                           break;

                        //
                        // Time signature event
                        //

                        case META_TIME_SIG:

                           //
                           // Fetch time numerator
                           //

                           S->time_numerator = *S->EVNT_ptr;

                           //
                           // Fetch time denominator: 0 = whole note,
                           // 1 = half-note, 2 = quarter-note, 3 = eighth-note...
                           //

                           t = *((U8 FAR*)AIL_ptr_add(S->EVNT_ptr,1)) - 2;

                           //
                           // Calculate beat period in terms of quantization
                           // rate
                           //

                           q = 16000000L / AIL_preference[MDI_SERVICE_RATE];

                           if (t < 0)
                              {
                              t = -t;

                              S->time_fraction = (q >> t);
                              }
                           else
                              {
                              S->time_fraction = (q << t);
                              }

                           //
                           // Predecrement beat fraction for this interval;
                           // signal beginning of new measure
                           //

                           S->beat_fraction  = 0;
                           S->beat_fraction -= S->time_fraction;

                           S->beat_count     = 0;
                           S->measure_count++;

                           //
                           // If beat/bar callback function active,
                           // trigger it
                           //

                           if (S->beat_callback != NULL)
                              {
                              MSS_do_cb4( (AILBEATCB),
                                 S->beat_callback,S->driver->callingDS,S->BEAT_IsWin32s,
                                                   S->driver,
                                                   S,
                                                   S->beat_count,
                                                   S->measure_count);
                              }

                           break;
                        }

                     S->EVNT_ptr =AIL_ptr_add(S->EVNT_ptr,len);
                     break;

                  //
                  // Process MIDI System Exclusive message
                  //

                  case EV_SYSEX:
                  case EV_ESC:

                     //
                     // Read message length and copy data to buffer
                     //

                     ptr = (U8 FAR *) AIL_ptr_add(S->EVNT_ptr , 1);

                     len = XMI_read_VLN(&ptr);

                     len += AIL_ptr_dif(ptr, S->EVNT_ptr);

                     XMI_sysex_message(mdi,S->EVNT_ptr,len);

                     S->EVNT_ptr =AIL_ptr_add(S->EVNT_ptr,len);
                     break;

                  //
                  // Process MIDI channel voice message
                  //

                  default:

                     event   = S->EVNT_ptr;
                     channel = status & 0x0f;
                     status  = status & 0xf0;

                     //
                     // Transmit message with ICA override enabled
                     //

                     XMI_send_channel_voice_message(S,
                                                   *S->EVNT_ptr,
                                                  *((U8 FAR*)AIL_ptr_add(S->EVNT_ptr,1)),
                                                  *((U8 FAR*)AIL_ptr_add(S->EVNT_ptr,2)),
                                                    1);
                     //
                     // Index next event
                     //
                     // Allocate note queue entries for Note On messages
                     //

                     if (status != EV_NOTE_ON)
                        {
                        //
                        // If sequence was terminated by callback then
                        // stop the sequence 
                        //

                        if (S->status != SEQ_PLAYING)
                           {
                           sequence_done = 1;
                           break;
                           }

                        //
                        // If this was a control change event which caused
                        // a branch to take place -- either a FOR/NEXT
                        // controller or a user callback with an API branch
                        // call -- then don't skip the current event
                        //

                        if (event == S->EVNT_ptr)
                           {
                           S->EVNT_ptr=AIL_ptr_add(S->EVNT_ptr, XMI_message_size(*S->EVNT_ptr));
                           }
                        }
                     else
                        {
                        //
                        // Find free note queue entry
                        //

                        for (i=0; i < MAX_NOTES; i++)
                           {
                           if (S->note_chan[i] == -1)
                              {
                              break;
                              }
                           }

                        //
                        // Shut down sequence if note queue overflows
                        //
                        // Should never happen since excessive polyphony is
                        // trapped by MIDIFORM
                        //

                        if (i == MAX_NOTES)
                           {
                           AIL_set_error("Internal note queue overflow.");

                           AIL_stop_sequence(S);
                           S->status = SEQ_DONE;

                           entry = 0;
                           return;
                           }

                        //
                        // Increment sequence-based note counter
                        //
                        // Record note's channel, number, and duration
                        //

                        ++S->note_count;

                        S->note_chan[i] = channel;

                        S->EVNT_ptr=AIL_ptr_add(S->EVNT_ptr,1);

                        S->note_num [i] = *(S->EVNT_ptr);

                        S->EVNT_ptr=AIL_ptr_add(S->EVNT_ptr,2);
                        S->note_time[i] = XMI_read_VLN(&S->EVNT_ptr);
                        }

                     break;
                  }
               }

            //
            // Terminate this interval and set delta-time count to skip
            // next 0-127 intervals
            //

            if (!sequence_done)
               {
               S->interval_count = *S->EVNT_ptr;
               S->EVNT_ptr=AIL_ptr_add(S->EVNT_ptr,1);
               }
            }

         if (!sequence_done)
            {
            //
            // Advance beat/bar counters
            //

            S->beat_fraction += S->time_fraction;

            if (S->beat_fraction >= S->time_per_beat)
               {
               S->beat_fraction -= S->time_per_beat;
               ++S->beat_count;

               if (S->beat_count >= S->time_numerator)
                  {
                  S->beat_count = 0;
                  ++S->measure_count;
                  }

               //
               // If beat/bar callback function active, trigger it
               //

               if (S->beat_callback != NULL)
                  {
                  AIL_sequence_position(S, &i, &j);

                  MSS_do_cb4( (AILBEATCB),
                     S->beat_callback,S->driver->callingDS,S->BEAT_IsWin32s,
                     S->driver, S, i, j);
                  }
               }
            }
         }

      if (!sequence_done)
         {
         //
         // Update volume ramp, if any
         //

         if (S->volume != S->volume_target)
            {
            S->volume_accum += S->driver->interval_time;

            while (S->volume_accum >= S->volume_period)
               {
               S->volume_accum -= S->volume_period;

               if (S->volume_target > S->volume)
                  {
                  ++S->volume;
                  }
               else
                  {
                  --S->volume;
                  }

               if (S->volume == S->volume_target)
                  {
                  break;
                  }
               }

            //
            // Update volume controllers once every 8 intervals
            // to avoid generating excessive MIDI traffic
            //

            if (!(S->interval_num & 0x07))
               {
               XMI_update_volume(S);
               }
            }

         //
         // Update tempo ramp, if any
         //

         if (S->tempo_percent != S->tempo_target)
            {
            S->tempo_accum += S->driver->interval_time;

            while (S->tempo_accum >= S->tempo_period)
               {
               S->tempo_accum -= S->tempo_period;

               if (S->tempo_target > S->tempo_percent)
                  {
                  ++S->tempo_percent;
                  }
               else
                  {
                  --S->tempo_percent;
                  }

               if (S->tempo_percent == S->tempo_target)
                  {
                  break;
                  }
               }
            }
         }
      }

   MSSLockedDecrementPtr(entry);
}

//############################################################################
//##                                                                        ##
//## Initialize XMIDI device                                                ##
//##                                                                        ##
//############################################################################

void init_mdi_defaults(HMDIDRIVER mdi)
{
   S32 i;

   //
   // Initialize synthesizer to General MIDI defaults
   //

   for (i=0; i < NUM_CHANS; i++)
      {
      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        PATCH_BANK_SEL,
                                        0);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        GM_BANK_LSB,
                                        0);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        GM_BANK_MSB,
                                        0);

      XMI_MIDI_message(mdi,EV_PROGRAM | i,
                                        0,0);

      XMI_MIDI_message(mdi,EV_PITCH   | i,
                                        0x00,0x40);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        VOICE_PROTECT,
                                        0);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        MODULATION,
                                        0);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        PART_VOLUME,
                                        AIL_preference[MDI_DEFAULT_VOLUME]);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        PANPOT,
                                        64);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        EXPRESSION,
                                        127);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        SUSTAIN,
                                        0);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        REVERB,
                                        40);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        CHORUS,
                                        0);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        RPN_LSB,
                                        0);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        RPN_MSB,
                                        0);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        DATA_LSB,
                                        0);

      XMI_MIDI_message(mdi,EV_CONTROL | i,
                                        DATA_MSB,
                                        AIL_preference[MDI_DEFAULT_BEND_RANGE]);

      if (!(i & 3))
         {
         AIL_delay(1);
         }
      }
}


//############################################################################
//##                                                                        ##
//## Uninstall XMIDI audio driver, freeing all allocated resources          ##
//##                                                                        ##
//## This function is called via the AIL_DRIVER.destructor vector only      ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_midiOutClose(HMDIDRIVER mdi)
{
   HMDIDRIVER cur,prev;
   S32        i;

   //
   // Stop all playing sequences to avoid hung notes
   //

   for (i=0; i < mdi->n_sequences; i++)
      {
      AIL_end_sequence(&mdi->sequences[i]);
      }

   //
   // Unlink from HMDIDRIVER chain
   //

   if (mdi == MDI_first)
      {
      MDI_first = mdi->next;
      }
   else
      {
      prev = MDI_first;
      cur  = MDI_first->next;

      while (cur != mdi)
         {
         if (cur == NULL)
            {
            return;
            }

         prev = cur;
         cur  = cur->next;
         }

      prev->next = cur->next;
      }

   //
   // Unprepare sysex header
   //

   if (!mdi->released)
      {
      if (mdi->deviceid != MIDI_NULL_DRIVER)
         {
         midiOutUnprepareHeader(mdi->hMidiOut,
                                mdi->mhdr,
                                sizeof(MIDIHDR));
         }
      }

   //
   // Close MIDI device
   //

   if (!mdi->released)
      {
      if (mdi->deviceid != MIDI_NULL_DRIVER)
         {
         midiOutReset(mdi->hMidiOut);
         midiOutClose(mdi->hMidiOut);
         }
      }

   //
   // Stop sequencer timer service
   //

   AIL_release_timer_handle(mdi->timer);

   //
   // Release memory resources
   //

   AIL_mem_free_lock(mdi->sysdata);
   AIL_mem_free_lock(mdi->mhdr);

   AIL_mem_free_lock(mdi->sequences);
   AIL_mem_free_lock(mdi);
}

//############################################################################
//##                                                                        ##
//## Temporarily release the Windows HMIDIOUT device                        ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_MIDI_handle_release(HMDIDRIVER mdi)
{
  if ((mdi==0) || (MDI_first==0))
    return(0);

  if (mdi->released)
    return(1);

  MSSLockedIncrementPtr(entry);

   for (i=0; i < mdi->n_sequences; i++)
      {
      if (mdi->sequences[i].status==SEQ_PLAYING)
        {
        AIL_stop_sequence(&mdi->sequences[i]);
        mdi->sequences[i].status=SEQ_PLAYINGBUTRELEASED;
        }
      }

   //
   // Unprepare sysex header
   //

   if (mdi->deviceid != MIDI_NULL_DRIVER)
      {
      midiOutUnprepareHeader(mdi->hMidiOut,
                             mdi->mhdr,
                             sizeof(MIDIHDR));
      }

   //
   // Close MIDI device
   //

   if (mdi->deviceid != MIDI_NULL_DRIVER)
      {
      midiOutReset(mdi->hMidiOut);
      midiOutClose(mdi->hMidiOut);
      }

   //
   // Set status variables
   //

   mdi->hMidiOut=0;
   mdi->released=1;

  MSSLockedDecrementPtr(entry);

  return(1);
}

//############################################################################
//##                                                                        ##
//## Reacquire the Windows HMIDIOUT device                                  ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_MIDI_handle_reacquire(HMDIDRIVER mdi)
{
   S32 r,i;

   if ((mdi==0) || (MDI_first==0))
     return(0);

   if (!mdi->released)
     return(1);

   MSSLockedIncrementPtr(entry);

   if (mdi->deviceid != MIDI_NULL_DRIVER)
      {
      OutMilesMutex();

      r = midiOutOpen((LPHMIDIOUT) &mdi->hMidiOut,
                                    (unsigned int)mdi->deviceid,
                                    (DWORD) NULL,
                                    0L,
                                    CALLBACK_NULL);
      InMilesMutex();

      if (r)
         {
         --entry;
         return(0);
         }

      midiOutPrepareHeader(mdi->hMidiOut,
                           mdi->mhdr,
                           sizeof(MIDIHDR));
      }

   mdi->released=0;

   MSSLockedDecrementPtr(entry);

   init_mdi_defaults(mdi);

   for (i=0; i < mdi->n_sequences; i++)
      {
      if (mdi->sequences[i].status==SEQ_PLAYINGBUTRELEASED)
        {
        mdi->sequences[i].status=SEQ_STOPPED;
        AIL_resume_sequence(&mdi->sequences[i]);
        }
      }

  return(1);
}


//############################################################################
//##                                                                        ##
//## Install and initialize XMIDI audio driver                              ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_midiOutOpen(HMDIDRIVER FAR *drvr,   //)
                               LPHMIDIOUT FAR *lphMidiOut,
                               U32            dwDeviceID)
{
   S32 i,r;
   HMDIDRIVER mdi;
   static char errbuff[80];

   //
   // Allocate memory for MDI_DRIVER structure
   //

   mdi = AIL_mem_alloc_lock(sizeof(MDI_DRIVER));

   if (mdi == NULL)
      {
      AIL_set_error("Could not allocate memory for driver descriptor.");

      return 1;
      }

   SETTAG(mdi->tag,"HMDI");

   *drvr = mdi;

   mdi->deviceid      = dwDeviceID;

   //
   // Open MIDI driver
   //

   if (mdi->deviceid == MIDI_NULL_DRIVER)
      {
      if (lphMidiOut)
         {
         *lphMidiOut = NULL;
         }
      }
   else
      {
      OutMilesMutex();

      r = midiOutOpen((LPHMIDIOUT) &mdi->hMidiOut,
                                    (unsigned int)mdi->deviceid,
                                    (DWORD) NULL,
                                    0L,
                                    CALLBACK_NULL);
      InMilesMutex();


      if (r)
         {
         AIL_set_error("midiOutOpen() failed.");

         AIL_mem_free_lock(mdi);

         return r;
         }

      if (lphMidiOut)
         {
         *lphMidiOut = &mdi->hMidiOut;
         }
      }

   //
   // Allocate SEQUENCE structures for driver and set params
   //

   #ifndef IS_WIN32
     MSSGetTaskInfo(&mdi->callingCT,&mdi->callingDS);
   #endif

   mdi->n_sequences = AIL_preference[MDI_SEQUENCES];

   mdi->sequences = AIL_mem_alloc_lock(sizeof(struct _SEQUENCE) * mdi->n_sequences);

   if (mdi->sequences == NULL)
      {
      AIL_set_error("Could not allocate SEQUENCE structures.");

      if (mdi->deviceid != MIDI_NULL_DRIVER)
         {
         midiOutClose(mdi->hMidiOut);
         }

      AIL_mem_free_lock(mdi);

      return 1;
      }

   for (i=0; i < mdi->n_sequences; i++)
      {
      SETTAG(mdi->sequences[i].tag,"HSEQ");

      mdi->sequences[i].status = SEQ_FREE;
      mdi->sequences[i].driver = mdi;
      }

   //
   // Initialize miscellaneous MDI_DRIVER variables
   //

   mdi->event_trap    = NULL;
   mdi->timbre_trap   = NULL;

   mdi->interval_time = 1000000L / AIL_preference[MDI_SERVICE_RATE];

   mdi->disable       = 0;

   mdi->released      = 0;

   mdi->master_volume = 127;

   //
   // Initialize channel lock table to NULL (all physical channels
   // available)
   //

   for (i=0; i < NUM_CHANS; i++)
      {
      mdi->lock  [i] = UNLOCKED;
      mdi->locker[i] = NULL;
      mdi->owner [i] = NULL;
      mdi->user  [i] = NULL;
      mdi->state [i] = UNLOCKED;
      mdi->notes [i] = 0;
      }

   //
   // Allocate timer for XMIDI sequencing
   //

   mdi->timer = AIL_register_timer( (AILTIMERCB) XMI_serve);

   if (mdi->timer == -1)
      {
      AIL_set_error("Out of timer handles.");

      AIL_mem_free_lock(mdi->sequences);

      if (mdi->deviceid != MIDI_NULL_DRIVER)
         {
         midiOutClose(mdi->hMidiOut);
         }

      AIL_mem_free_lock(mdi);

      return 1;
      }

   AIL_set_timer_user(mdi->timer,(U32) mdi);

   //
   // Allocate system exclusive header
   //

   mdi->mhdr=AIL_mem_alloc_lock(sizeof(MIDIHDR));

   if (mdi->mhdr==NULL)
      {
      AIL_set_error("Could not allocate MIDIHDR structure.");

      AIL_release_timer_handle(mdi->timer);
      AIL_mem_free_lock(mdi->sequences);

      if (mdi->deviceid != MIDI_NULL_DRIVER)
         {
         midiOutClose(mdi->hMidiOut);
         }

      AIL_mem_free_lock(mdi);

      return 1;
      }

   AIL_memset(mdi->mhdr,0,sizeof(MIDIHDR));

   //
   // Allocate system exclusive buffer
   //

   mdi->sysdata=AIL_mem_alloc_lock(AIL_preference[MDI_SYSEX_BUFFER_SIZE]);

   if (mdi->sysdata == NULL)
      {
      AIL_set_error("Could not allocate system exclusive buffer.");

      AIL_mem_free_lock(mdi->mhdr);
      AIL_release_timer_handle(mdi->timer);
      AIL_mem_free_lock(mdi->sequences);

      if (mdi->deviceid != MIDI_NULL_DRIVER)
         {
         midiOutClose(mdi->hMidiOut);
         }

      AIL_mem_free_lock(mdi);

      return 1;
      }

   AIL_memset(mdi->sysdata,0, AIL_preference[MDI_SYSEX_BUFFER_SIZE]);

   //
   // Prepare sysex header and buffer for use
   //

   mdi->mhdr->lpData         = mdi->sysdata;
   mdi->mhdr->dwBufferLength = AIL_preference[MDI_SYSEX_BUFFER_SIZE];

   if (mdi->deviceid != MIDI_NULL_DRIVER)
      {
      r = midiOutPrepareHeader(mdi->hMidiOut,
                              mdi->mhdr,
                              sizeof(MIDIHDR));

      if (r)
         {
         AIL_sprintf(errbuff,"midiOutPrepareHeader() failed, code $%X\n",r);
         AIL_set_error(errbuff);

         AIL_mem_free_lock(mdi->sysdata);
         AIL_mem_free_lock(mdi->mhdr);
         AIL_release_timer_handle(mdi->timer);
         AIL_mem_free_lock(mdi->sequences);
         midiOutClose(mdi->hMidiOut);
         AIL_mem_free_lock(mdi);

         return r;
         }
      }

   init_mdi_defaults(mdi);

   //
   // Start XMIDI timer service and return MDI_DRIVER descriptor
   //

   AIL_set_timer_frequency(mdi->timer,AIL_preference[MDI_SERVICE_RATE]);
   AIL_start_timer(mdi->timer);

   //
   // Link HMDIDRIVER into chain
   //

   if (MDI_first != NULL)
      {
      mdi->next = MDI_first;
      }
   else
      {
      mdi->next = NULL;
      }

   MDI_first = mdi;

   //
   // Return success
   //

   return 0;
}

//############################################################################
//##                                                                        ##
//## Allocate a SEQUENCE structure for use with a given driver              ##
//##                                                                        ##
//############################################################################

HSEQUENCE AILCALL AIL_API_allocate_sequence_handle(HMDIDRIVER mdi)
{
   S32       i;
   HSEQUENCE S;

   //
   // Lock timer services to prevent reentry
   //

   AIL_lock();

   //
   // Look for an unallocated sequence structure
   //

   for (i=0; i < mdi->n_sequences; i++)
      {
      if (mdi->sequences[i].status == SEQ_FREE)
         break;
      }

   //
   // If all structures in use, return NULL
   //

   if (i == mdi->n_sequences)
      {
      AIL_set_error("Out of sequence handles.");

      AIL_unlock();
      return NULL;
      }

   S = &mdi->sequences[i];

   //
   // Initialize sequence to "done" status
   //

   S->status = SEQ_DONE;

   //
   // Initialize state table values
   //

   XMI_init_sequence_state(S);

   //
   // Initialize end-of-sequence callback to NULL
   //

   S->EOS = NULL;

   //
   // Return sequence handle
   //

   AIL_unlock();
   return S;
}

//############################################################################
//##                                                                        ##
//## Free a SEQUENCE structure for later allocation                         ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_release_sequence_handle(HSEQUENCE S)
{
   if (S == NULL)
      {
      return;
      }

   //
   // Turn off all playing notes in sequence; release all channels
   //

   AIL_stop_sequence(S);

   //
   // Set 'free' flag
   //

   S->status = SEQ_FREE;
}

//############################################################################
//##                                                                        ##
//## Initialize a SEQUENCE structure to prepare for playback of desired     ##
//## XMIDI sequence file image                                              ##
//##                                                                        ##
//## Sequence is allocated (not free), done playing, and stopped            ##
//##                                                                        ##
//## Returns 0 if sequence initialization failed                            ##
//##        -1 if initialized OK but timbre was missing                     ##
//##         1 if initialization and timbre-loading successful              ##
//##                                                                        ##
//## Should not be called from callback function                            ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_init_sequence(HSEQUENCE S, void const FAR *start, S32 sequence_num)
{
   U8 const        FAR *image;
   U8 const        FAR *end;
   U32             i,j,len;
   TIMB_chunk FAR *T;
   static U8       TIMB[1024];
   U32             bank,patch;
   S32             result;

   if (S == NULL)
      {
      return 0;
      }

   //
   // Declare critical section
   //

   AIL_lock();

   //
   // Initialize status
   //

   S->status = SEQ_DONE;

   //
   // Find requested sequence in XMIDI image
   //

   image = XMI_find_sequence(start, sequence_num);

   if (image == NULL)
      {
      AIL_set_error("No valid XMIDI sequences present in file.");

      AIL_unlock();
      return 0;
      }

   //
   // Locate IFF chunks within FORM XMID:
   //
   // TIMB = list of bank/patch pairs needed to play sequence (optional)
   // RBRN = list of branch target points (optional)
   // EVNT = XMIDI event list (mandatory)
   //

   len = 8 + BE_SWAP32( ((U8 FAR *) (image+4)));
   end = image + len;

   image += 12;

   S->TIMB = NULL;
   S->RBRN = NULL;
   S->EVNT = NULL;

   while (image < end)
      {
      if (!AIL_strnicmp(image,"TIMB",4))
         {
         S->TIMB = image;
         }

      if (!AIL_strnicmp(image,"RBRN",4))
         {
         S->RBRN = image;
         }

      if (!AIL_strnicmp(image,"EVNT",4))
         {
         S->EVNT = image;
         }

      image += 8 + BE_SWAP32( ((U8 FAR *) (image+4)));
      }

   //
   // Sequence must contain EVNT chunk
   //

   if (S->EVNT == NULL)
      {
      AIL_set_error("Invalid XMIDI sequence.");

      AIL_unlock();
      return 0;
      }

   //
   // Initialize sequence callback and state data
   //

   S->ICA              = NULL;
   S->prefix_callback  = NULL;
   S->trigger_callback = NULL;
   S->beat_callback    = NULL;
   S->EOS              = NULL;
   S->loop_count       = 1;

   XMI_rewind_sequence(S);

   //
   // Initialize volume and tempo
   //

   S->volume         =  AIL_preference[MDI_DEFAULT_VOLUME];
   S->volume_target  =  AIL_preference[MDI_DEFAULT_VOLUME];
   S->volume_period  =  0;
   S->volume_accum   =  0;

   S->tempo_percent  =  100;
   S->tempo_target   =  100;
   S->tempo_period   =  0;
   S->tempo_accum    =  0;
   S->tempo_error    =  0;

   //
   // If no TIMB chunk present, return success
   //

   if (S->TIMB == NULL)
      {
      AIL_unlock();
      return 1;
      }

   //
   // Make modifiable copy of TIMB chunk
   //

   AIL_memcpy(TIMB,
        S->TIMB,
           min(sizeof(TIMB),
               8 + BE_SWAP32( ((U8 FAR *) ((BYTE FAR *) S->TIMB+4)))));

   T = (TIMB_chunk *) TIMB;

   //
   // If timbre-request callback function registered, pass each bank/patch
   // pair to the function to see if it has to be requested from the driver
   //
   // Remove references to any timbres handled by the callback function
   //

   if (S->driver->timbre_trap != NULL)
      {
      i = 0;

      while (i < (U32) T->n_entries)
         {
         patch  =  ((U32) T->timbre[i]) & 0xff;
         bank   = (((U32) T->timbre[i]) & 0xff00) >> 8;

         MSS_do_cb3_with_ret(result,(AILTIMBRECB),
               S->driver->timbre_trap,S->driver->callingDS,S->driver->TIMBRE_IsWin32s,0,
               S->driver, (S32) bank, (S32) patch);

         if (!result)
            {
            //
            // Timbre request was not handled by callback function --
            // check next timbre
            //

            ++i;
            }
         else
            {
            //
            // Timbre request was handled by callback function --
            // excise from TIMB chunk
            //
            // Scroll all subsequent entries down one slot, and test
            // the next timbre at the current slot
            //

            for (j = i+1; j < (U32) T->n_entries; j++)
               {
               T->timbre[j-1] = T->timbre[j];
               }

            --T->n_entries;

            if (T->lsb < 2)
               {
               T->lsb -= 2;
               T->msb--;
               }
            else
               T->lsb -= 2;
            }
         }
      }

   //
   // End critical section
   //

   AIL_unlock();

   return 1;
}

//############################################################################
//##                                                                        ##
//## Start playback of sequence from beginning                              ##
//##                                                                        ##
//## At a minimum, sequence must first have been initialized with a prior   ##
//## call to AIL_init_sequence()                                            ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_start_sequence(HSEQUENCE S)
{
   if (S == NULL)
      {
      return;
      }

   //
   // Make sure sequence has been allocated
   //

   if (S->status == SEQ_FREE)
      {
      return;
      }

   //
   // Stop sequence if playing
   //

   AIL_stop_sequence(S);

   //
   // Rewind sequence to beginning
   //

   XMI_rewind_sequence(S);

   //
   // Set 'playing' status
   //

   S->status = SEQ_PLAYING;

   if (S->driver->released) {
     AIL_stop_sequence(S);
     S->status = SEQ_PLAYINGBUTRELEASED;
   }
}

//############################################################################
//##                                                                        ##
//## Stop playback of sequence                                              ##
//##                                                                        ##
//## Sequence playback may be resumed with AIL_resume_sequence(), or        ##
//## restarted from the beginning with AIL_start_sequence()                 ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_stop_sequence(HSEQUENCE S)
{
   HMDIDRIVER mdi;
   S32             log,phys;

   if (S == NULL)
      {
      return;
      }

   //
   // Make sure sequence is currently playing
   //

   if ((S->status != SEQ_PLAYING)  && (S->status != SEQ_PLAYINGBUTRELEASED))
      {
      return;
      }

   //
   // Mask 'playing' status
   //

   S->status = SEQ_STOPPED;

   //
   // Turn off any active notes in sequence
   //

   XMI_flush_note_queue(S);

   //
   // Prepare sequence's channels for use with other sequences, leaving
   // shadow array intact for later recovery by AIL_resume_sequence()
   //

   mdi = S->driver;

   for (log=0; log < NUM_CHANS; log++)
      {
      phys = S->chan_map[log];

      //
      // If sustain pedal on, release it
      //

      if (S->shadow.sus[log] >= 64)
         {
         XMI_MIDI_message(mdi,EV_CONTROL | phys,
                                           SUSTAIN,
                                           0);
         }

      //
      // If channel-lock protection active, cancel it
      //

      if (S->shadow.c_prot[log] >= 64)
         {
         mdi->lock[phys] = UNLOCKED;
         }

      //
      // If voice-stealing protection active, cancel it
      //

      if (S->shadow.c_v_prot[log] >= 64)
         {
         XMI_MIDI_message(mdi,EV_CONTROL | phys,
                                           VOICE_PROTECT,
                                           0);
         }

      //
      // Finally, if channel was locked, release it
      //

      if (S->shadow.c_lock[log] >= 64)
         {
         AIL_release_channel(mdi, phys+1);
         }
      }
}

//############################################################################
//##                                                                        ##
//## Resume playback of previously stopped sequence                         ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_resume_sequence(HSEQUENCE S)
{
   HMDIDRIVER mdi;
   S32        log;
   S32        ch;

   if (S == NULL)
      {
      return;
      }

   //
   // Make sure sequence has been previously stopped
   //

   if (S->status == SEQ_STOPPED)
      {

      //
      // Re-establish channel locks
      //

      mdi = S->driver;

      for (log=0; log < NUM_CHANS; log++)
         {
         if (S->shadow.c_lock[log] >= 64)
            {
            ch = AIL_lock_channel(mdi) - 1;

            S->chan_map[log] = (ch == -1) ? log : ch;
            }
         }

      //
      // Re-establish logged controller values (except Channel Lock, which
      // was done above)
      //

      for (log=0; log < NUM_CHANS; log++)
         {
         XMI_refresh_channel(S,log);
         }

     startplaying:

      //
      // Set 'playing' status
      //

      S->status = SEQ_PLAYING;

      if (S->driver->released) {
        AIL_stop_sequence(S);
        S->status = SEQ_PLAYINGBUTRELEASED;
      }
   } else if (S->status == SEQ_DONE)
     goto startplaying;

}

//############################################################################
//##                                                                        ##
//## Terminate playback of sequence, setting sequence status to SEQ_DONE    ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_end_sequence(HSEQUENCE S)
{
   if (S == NULL)
      {
      return;
      }

   //
   // Make sure sequence is currently allocated
   //

   if ((S->status == SEQ_FREE) || (S->status == SEQ_DONE))
      {
      return;
      }

   //
   // Stop sequence and set 'done' status
   //

   AIL_stop_sequence(S);

   S->status = SEQ_DONE;

   //
   // Call EOS handler, if any
   //

   if (S->EOS != NULL)
      {
      MSS_do_cb1( (AILSEQUENCECB),S->EOS,S->driver->callingDS, S->EOS_IsWin32s,
        S);
      }
}

//############################################################################
//##                                                                        ##
//## Set sequence loop count                                                ##
//##                                                                        ##
//##  1: Single iteration, no looping                                       ##
//##  0: Loop indefinitely                                                  ##
//##  n: Play sequence n times                                              ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sequence_loop_count(HSEQUENCE S, S32 loop_count)
{
   if (S == NULL)
      {
      return;
      }

   S->loop_count = loop_count;
}

//############################################################################
//##                                                                        ##
//## Set relative tempo percentage for sequence, 0-100+ %                   ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sequence_tempo(HSEQUENCE S, S32 tempo, S32 milliseconds)
{
   if (S == NULL)
      {
      return;
      }

   //
   // Disable XMIDI service while altering tempo control data
   //

   MSSLockedIncrementPtr(S->driver->disable);

   //
   // Set new tempo target; exit if no change
   //

   S->tempo_target = tempo;

   if (S->tempo_percent == S->tempo_target)
      {
      MSSLockedDecrementPtr(S->driver->disable);

      return;
      }

   //
   // Otherwise, set up tempo ramp
   //

   if (milliseconds == 0)
      {
      S->tempo_percent = S->tempo_target;
      }
   else
      {
      S->tempo_period = (milliseconds * 1000L) /
                    labs(S->tempo_percent - S->tempo_target);

      S->tempo_accum  = 0;
      }

   //
   // Restore XMIDI service and return
   //

   MSSLockedDecrementPtr(S->driver->disable);
}

//############################################################################
//##                                                                        ##
//## Set volume scaling factor for all channels in sequence, 0-127          ##
//##                                                                        ##
//## Values above 127 cause "compression" effect                            ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sequence_volume(HSEQUENCE S, S32 volume, S32 milliseconds)
{
   if (S == NULL)
      {
      return;
      }

   //
   // Disable XMIDI service while altering volume control data
   //

   MSSLockedIncrementPtr(S->driver->disable);

   //
   // Set new volume target; exit if no change
   //

   S->volume_target = volume;

   if (S->volume == S->volume_target)
      {
      MSSLockedDecrementPtr(S->driver->disable);

      return;
      }

   //
   // Otherwise, set up volume ramp
   //

   if (milliseconds == 0)
      {
      S->volume = S->volume_target;
      }
   else
      {
      S->volume_period = (milliseconds * 1000L) /
                    labs(S->volume - S->volume_target);

      S->volume_accum  = 0;
      }

   //
   // Restore interrupt state, update channel volume settings, and exit
   //

   XMI_update_volume(S);

   //
   // Restore XMIDI service and return
   //

   MSSLockedDecrementPtr(S->driver->disable);
}

//############################################################################
//##                                                                        ##
//## Get status of sequence                                                 ##
//##                                                                        ##
//############################################################################

U32 AILCALL AIL_API_sequence_status(HSEQUENCE S)
{
   if (S == NULL)
      {
      return 0;
      }

   return S->status;
}

//############################################################################
//##                                                                        ##
//## Get number of sequence loops remaining                                 ##
//##                                                                        ##
//## 1 indicates that the sequence is on its last iteration                 ##
//## 0 indicates that the sequence is looping indefinitely                  ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_sequence_loop_count(HSEQUENCE S)
{
   if (S == NULL)
      {
      return -1;
      }

   return S->loop_count;
}


//############################################################################
//##                                                                        ##
//## Set master volume for all sequences                                    ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_XMIDI_master_volume(HMDIDRIVER mdi, S32 master_volume)
{
   HSEQUENCE S;
   S32       i;

   //
   // Set new volume; return if redundant setting
   //

   if (mdi->master_volume == master_volume)
      {
      return;
      }

   mdi->master_volume = master_volume;

   //
   // Force all sequences to update their volume controllers
   //

   MSSLockedIncrementPtr(mdi->disable);

   for (i = mdi->n_sequences,S = &mdi->sequences[0]; i; --i,++S)
      {
      if ((S->status != SEQ_PLAYING) && (S->status != SEQ_PLAYINGBUTRELEASED))
         {
         continue;
         }

      XMI_update_volume(S);
      }

   MSSLockedDecrementPtr(mdi->disable);
}

//############################################################################
//##                                                                        ##
//## Return current master MIDI volume setting                              ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_XMIDI_master_volume(HMDIDRIVER mdi)
{
   return mdi->master_volume;
}

//############################################################################
//##                                                                        ##
//## Return relative tempo percentage for sequence, 0-100+ %                ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_sequence_tempo(HSEQUENCE S)
{
   if (S == NULL)
      {
      return 0;
      }

   return S->tempo_percent;
}

//############################################################################
//##                                                                        ##
//## Return volume scaling factor for sequence, 0-127                       ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_sequence_volume(HSEQUENCE S)
{
   if (S == NULL)
      {
      return 0;
      }

   return S->volume;
}

//############################################################################
//##                                                                        ##
//## Return number of actively playing sequences for given driver           ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_active_sequence_count(HMDIDRIVER mdi)
{
   S32 i,n;

   n = 0;

   for (i=0; i < mdi->n_sequences; i++)
      {
      if ((mdi->sequences[i].status == SEQ_PLAYING) || (mdi->sequences[i].status == SEQ_PLAYINGBUTRELEASED))
         {
         ++n;
         }
      }

   return n;
}

//############################################################################
//##                                                                        ##
//## Return current value of desired MIDI controller in sequence            ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_controller_value(HSEQUENCE S, S32 channel, S32 controller_num)
{
   if (S == NULL)
      {
      return -1;
      }

   return XMI_read_log(&S->shadow,
                       EV_CONTROL | (channel-1),
                       controller_num);
}

//############################################################################
//##                                                                        ##
//## Return number of 'on' notes in given channel                           ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_channel_notes(HSEQUENCE S, S32 channel)
{
   S32 i,n;

   if (S == NULL)
      {
      return 0;
      }

   //
   // Count number of notes with desired channel
   //

   n = 0;

   for (i=0; i < NUM_CHANS; i++)
      {
      if (S->note_chan[i] == (channel-1))
         {
         ++n;
         }
      }

   return n;
}

//############################################################################
//##                                                                        ##
//## Report relative beat and measure count for current XMIDI sequence      ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_sequence_position(HSEQUENCE S, S32 FAR *beat, S32 FAR *measure)
{
   S32 b,m,i,f;

   if (S == NULL)
      {
      return;
      }

   //
   // Disable XMIDI service to prevent errors
   //

   MSSLockedIncrementPtr(S->driver->disable);

   b = S->beat_count;
   m = S->measure_count;

   //
   // Advance beat/measure count by AIL_preference[MDI_QUANT_ADVANCE]
   // intervals
   //

   f = S->beat_fraction;

   for (i=0; i < AIL_preference[MDI_QUANT_ADVANCE]; i++)
      {
      f += S->time_fraction;

      if (f >= S->time_per_beat)
         {
         f -= S->time_per_beat;
         ++b;

         if (b >= S->time_numerator)
            {
            b = 0;
            ++m;
            }
         }
      }

   //
   // Avoid negative measure counts prior to sequence start
   //

   if (m < 0)
      {
      m = 0;
      }

   //
   // Return beat and/or measure count, as desired
   //

   if (measure != NULL)
      {
      *measure = m;
      }

   if (beat != NULL)
      {
      *beat = b;
      }

   MSSLockedDecrementPtr(S->driver->disable);

   return;
}

//############################################################################
//##                                                                        ##
//## Branch immediately to specified Sequence Branch Index point            ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_branch_index(HSEQUENCE S, U32 marker)
{
   S32             n,i;
   RBRN_entry FAR *R;

   if (S == NULL)
      {
      return;
      }

   //
   // Make sure RBRN block is present
   //

   if (S->RBRN == NULL)
      {
      return;
      }

   //
   // Get number of RBRN entries
   //

   n = *(S16 FAR *) ((U8 FAR *) S->RBRN + 8);

   //
   // Get pointer to RBRN entry list
   //

   R = (RBRN_entry FAR *) ((U8 FAR *) S->RBRN + 10);

   //
   // Search RBRN list for specified index
   //
   // If not found, return
   //

   for (i=0; i < n; i++)
      {
      if ((U32) (U16) R[i].bnum == marker)
         {
         break;
         }
      }

   if (i == n)
      {
      return;
      }

   //
   // Move sequence pointer to branch point
   //
   // The first event fetched at the new location will be the Sequence Branch
   // Index controller, so this routine may safely be called from within
   // an XMIDI Callback Trigger, Force Branch, or other 3-byte event
   // handler
   //
   // Zero interval count, so next event will be fetched immediately
   //

   S->EVNT_ptr = AIL_ptr_add((U8 FAR *) S->EVNT,8 + R[i].offset);

   S->interval_count = 0;

   //
   // Cancel all FOR...NEXT loops unless application specifies otherwise
   //

   if (AIL_preference[MDI_ALLOW_LOOP_BRANCHING] == NO)
      {
      for (n=0; n < FOR_NEST; n++)
         {
         S->FOR_loop_count[n] = -1;
         }
      }
}

//############################################################################
//##                                                                        ##
//## Install function handler for XMIDI Callback Prefix events              ##
//##                                                                        ##
//############################################################################

AILPREFIXCB AILCALL AIL_API_register_prefix_callback(HSEQUENCE S, AILPREFIXCB callback)
{
   AILPREFIXCB old;

   if (S == NULL)
      {
      return NULL;
      }

   old = S->prefix_callback;

   #ifndef IS_WIN32
     CheckWin32sCB(S->PREFIX_IsWin32s);
   #endif

   S->prefix_callback = callback;

   return old;
}

//############################################################################
//##                                                                        ##
//## Install function handler for XMIDI Callback Trigger events             ##
//##                                                                        ##
//############################################################################

AILTRIGGERCB AILCALL AIL_API_register_trigger_callback(HSEQUENCE S, AILTRIGGERCB callback)
{
   AILTRIGGERCB old;

   if (S == NULL)
      {
      return NULL;
      }

   old = S->trigger_callback;

   #ifndef IS_WIN32
     CheckWin32sCB(S->TRIGGER_IsWin32s);
   #endif

   S->trigger_callback = callback;

   return old;
}

//############################################################################
//##                                                                        ##
//## Install function handler for end-of-sequence callbacks                 ##
//##                                                                        ##
//############################################################################

AILSEQUENCECB AILCALL AIL_API_register_sequence_callback(HSEQUENCE S, AILSEQUENCECB callback)
{
   AILSEQUENCECB old;

   if (S == NULL)
      {
      return NULL;
      }

   old = S->EOS;

   #ifndef IS_WIN32
     CheckWin32sCB(S->EOS_IsWin32s);
   #endif

   S->EOS = callback;

   return old;
}

//############################################################################
//##                                                                        ##
//## Install callback function handler for XMIDI beat/bar change events     ##
//##                                                                        ##
//############################################################################

AILBEATCB AILCALL AIL_API_register_beat_callback(HSEQUENCE S, AILBEATCB callback)
{
   AILBEATCB old;

   if (S == NULL)
      {
      return NULL;
      }

   old = S->beat_callback;

   #ifndef IS_WIN32
     CheckWin32sCB(S->BEAT_IsWin32s);
   #endif

   S->beat_callback = callback;

   return old;
}

//############################################################################
//##                                                                        ##
//## Install callback function handler for MIDI/XMIDI event trap            ##
//##                                                                        ##
//############################################################################

AILEVENTCB AILCALL AIL_API_register_event_callback(HMDIDRIVER mdi, AILEVENTCB callback)
{
   AILEVENTCB old;

   old = mdi->event_trap;

   #ifndef IS_WIN32
     CheckWin32sCB(mdi->EVENT_IsWin32s);
   #endif

   mdi->event_trap = callback;

   return old;
}

//############################################################################
//##                                                                        ##
//## Install callback function handler for MIDI/XMIDI timbre loading        ##
//##                                                                        ##
//## Note: MSS doesn't support GTL access -- this function is primarily     ##
//## for use by WAILXDIG                                                    ##
//##                                                                        ##
//############################################################################

AILTIMBRECB AILCALL AIL_API_register_timbre_callback(HMDIDRIVER mdi, AILTIMBRECB callback)
{
   AILTIMBRECB old;

   old = mdi->timbre_trap;

   #ifndef IS_WIN32
     CheckWin32sCB(mdi->TIMBRE_IsWin32s);
   #endif

   mdi->timbre_trap = callback;

   return old;
}

//############################################################################
//##                                                                        ##
//## Set sequence user data value at specified index                        ##
//##                                                                        ##
//## Any desired 32-bit value may be stored at one of eight user data words ##
//## associated with a given SEQUENCE                                       ##
//##                                                                        ##
//## Callback functions may access the user data array at interrupt time    ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sequence_user_data(HSEQUENCE S, U32 index, S32 value)
{
   if (S == NULL)
      {
      return;
      }

   S->user_data[index] = value;
}

//############################################################################
//##                                                                        ##
//## Get sequence user data value at specified index                        ##
//##                                                                        ##
//## Any desired 32-bit value may be stored at one of eight user data words ##
//## associated with a given SEQUENCE                                       ##
//##                                                                        ##
//## Callback functions may access the user data array at interrupt time    ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_sequence_user_data(HSEQUENCE S, U32 index)
{
   if (S == NULL)
      {
      return 0;
      }

   return S->user_data[index];
}

//############################################################################
//##                                                                        ##
//## Register an Indirect Controller Array for use with specified sequence  ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_register_ICA_array(HSEQUENCE S, U8 FAR *array)
{
   if (S == NULL)
      {
      return;
      }

   S->ICA = array;
}

//############################################################################
//##                                                                        ##
//## Lock an unprotected physical channel                                   ##
//##                                                                        ##
//## Returns 0 if lock attempt failed                                       ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_lock_channel(HMDIDRIVER mdi)
{
   S32       i,j;
   S32       ch,best;
   HSEQUENCE S;

   //
   // Disable XMIDI service while locking channel
   //

   MSSLockedIncrementPtr(mdi->disable);

   //
   // Search for highest channel # with lowest note activity,
   // skipping already-locked and protected physical channels
   //

   ch   = -1;
   best = LONG_MAX;

   for (i=MAX_LOCK_CHAN; i >= MIN_LOCK_CHAN; i--)
      {
      if (i == PERCUSS_CHAN)
         {
         continue;
         }

      if ((mdi->lock[i] == LOCKED) ||
          (mdi->lock[i] == PROTECTED))
         {
         continue;
         }

      if (mdi->notes[i] < best)
         {
         best = mdi->notes[i];
         ch   = i;
         }
      }

   //
   // If no unprotected channels available, ignore lock protection and
   // try again
   //

   if (ch == -1)
      {
      for (i=MAX_LOCK_CHAN; i >= MIN_LOCK_CHAN; i--)
         {
         if (i == PERCUSS_CHAN)
            {
            continue;
            }

         if (mdi->lock[i] == LOCKED)
            {
            continue;
            }

         if (mdi->notes[i] < best)
            {
            best = mdi->notes[i];
            ch   = i;
            }
         }
      }

   //
   // If no unlocked channels available, return failure
   //

   if (ch == -1)
      {
      MSSLockedDecrementPtr(mdi->disable);

      return 0;
      }

   //
   // Otherwise, release sustain pedal and turn off all active notes in
   // physical channel, regardless of sequence
   //

   XMI_MIDI_message(mdi,EV_CONTROL | ch,
                        SUSTAIN,
                        0);

   for (i = mdi->n_sequences,S = &mdi->sequences[0]; i; --i,++S)
      {
      if (S->status == SEQ_FREE)
         {
         continue;
         }

      for (j=0; j < MAX_NOTES; j++)
         {
         if (S->note_chan[j] == -1)
            {
            continue;
            }

         if (S->chan_map[S->note_chan[j]] != ch)
            {
            continue;
            }

         XMI_send_channel_voice_message(S,
                                        S->note_chan[j] | EV_NOTE_OFF,
                                        S->note_num [j],
                                        0,
                                        0);
         S->note_chan[j] = -1;
         }
      }

   //
   // Lock channel
   //
   // By default, API asserts ownership of channel (locker=NULL), and
   // last sequence to use channel is recorded as its original owner
   //

   mdi->state [ch] = mdi->lock[ch];
   mdi->lock  [ch] = LOCKED;
   mdi->locker[ch] = NULL;
   mdi->owner [ch] = mdi->user[ch];

   //
   // Return 1-based channel number to caller
   //

   MSSLockedDecrementPtr(mdi->disable);

   return ch+1;
}

//############################################################################
//##                                                                        ##
//## Release (unlock) a locked physical channel                             ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_release_channel(HMDIDRIVER mdi, S32 channel)
{
   S32       i,j,ch;
   HSEQUENCE S;

   //
   // Convert channel # to 0-based internal notation
   //

   ch = channel-1;

   //
   // If channel is not locked, return
   //

   if (mdi->lock[ch] != LOCKED)
      {
      return;
      }

   //
   // Disable XMIDI service while unlocking channel
   //

   MSSLockedIncrementPtr(mdi->disable);

   //
   // Restore channel's original state and ownership
   //

   mdi->lock[ch] = mdi->state[ch];
   mdi->user[ch] = mdi->owner[ch];

   //
   // Release sustain pedal and turn all notes off in channel,
   // regardless of sequence
   //

   XMI_MIDI_message(mdi,EV_CONTROL | ch,
                        SUSTAIN,
                        0);

   for (i = mdi->n_sequences,S = &mdi->sequences[0]; i; --i,++S)
      {
      if (S->status == SEQ_FREE)
         {
         continue;
         }

      for (j=0; j < MAX_NOTES; j++)
         {
         if (S->note_chan[j] == -1)
            {
            continue;
            }

         if (S->chan_map[S->note_chan[j]] != ch)
            {
            continue;
            }

         XMI_send_channel_voice_message(S,
                                        S->note_chan[j] | EV_NOTE_OFF,
                                        S->note_num [j],
                                        0,
                                        0);
         S->note_chan[j] = -1;
         }
      }

   //
   // Bring channel up to date with owner's current controller values, if
   // owner is valid sequence
   //

   if (mdi->owner[ch] != NULL)
      {
      if (mdi->owner[ch]->status != SEQ_FREE)
         {
         XMI_refresh_channel(mdi->owner[ch],ch);
         }
      }

   MSSLockedDecrementPtr(mdi->disable);
}

//############################################################################
//##                                                                        ##
//## Map sequence's logical channel to desired physical output channel      ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_map_sequence_channel(HSEQUENCE S, S32 seq_channel, S32 new_channel)
{
   if (S == NULL)
      {
      return;
      }

   //
   // Redirect output on this sequence's channel to new channel
   //

   S->chan_map[seq_channel-1] = new_channel-1;

   //
   // If channel is locked by API or other sequence, reassign it to
   // this sequence so it's not inhibited from playing
   //

   if ((S->driver->lock  [new_channel-1] == LOCKED) &&
       (S->driver->locker[new_channel-1] != S))
      {
      S->driver->locker[new_channel-1] = S;
      }
}

//############################################################################
//##                                                                        ##
//## Return physical channel to which sequence's logical channel is mapped  ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_true_sequence_channel(HSEQUENCE S, S32 seq_channel)
{
   if (S == NULL)
      {
      return 0;
      }

   return S->chan_map[seq_channel-1] + 1;
}

//############################################################################
//##                                                                        ##
//## Transmit MIDI channel voice message via desired physical channel       ##
//##                                                                        ##
//## This function disregards channel locking and other XMIDI features      ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_send_channel_voice_message(HMDIDRIVER mdi, HSEQUENCE S, //)
                                                           S32 status,
                                                           S32 data_1,
                                                           S32 data_2)
{
   S32        result;
   HMDIDRIVER drvr;

   //
   // Get driver handle to use (derive from sequence handle if driver NULL)
   //

   drvr = mdi;

   if (drvr == NULL)
      {
      if (S == NULL)
         {
         return;
         }

      drvr = S->driver;
      }

   //
   // Disable XMIDI service while accessing MIDI_data[] buffer
   //

   MSSLockedIncrementPtr(drvr->disable);

   if (S == NULL)
      {
      //
      // If this is a Part Volume (7) controller, scale its value by the
      // driver's master volume setting
      //

      if (((status & 0xf0) == EV_CONTROL) &&
          (data_1         == PART_VOLUME))
         {
         data_2 = (data_2 * drvr->master_volume) / 127;

         if (data_2 > 127)
            {
            data_2 = 127;
            }

         if (data_2 < 0)
            {
            data_2 = 0;
            }
         }

      //
      // If no sequence handle given, transmit message on physical channel
      // without XMIDI logging
      //

      result = 0;

      if (drvr->event_trap != NULL)
         {
         MSS_do_cb5_with_ret(result, (AILEVENTCB), drvr->event_trap,
            drvr->callingDS,drvr->EVENT_IsWin32s,0,
            drvr,NULL,status,data_1,data_2);
         }

      if (!result)
         {
         XMI_MIDI_message(drvr,status,data_1,data_2);
         }
      }
   else
      {
      //
      // Otherwise, perform logical-to-physical translation and XMIDI
      // interpretation based on sequence handle, when transmitting
      // message
      //

      XMI_send_channel_voice_message(S,
                                     status,
                                     data_1,
                                     data_2,
                                     0);
      }

   //
   // Reenable XMIDI service
   //

   MSSLockedDecrementPtr(drvr->disable);
}


//############################################################################
//##                                                                        ##
//## Transmit MIDI System Exclusive message                                 ##
//##                                                                        ##
//## System Exclusive message must be passed in Standard MIDI Files 1.0     ##
//## format, may be of type F0 or F7 (although some drivers may not support ##
//## F7 messages), and must not exceed 512 bytes                            ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_send_sysex_message(HMDIDRIVER mdi, void const FAR *buffer)
{
   U8 const    FAR *ptr;
   U32        len;

   //
   // Disable XMIDI service while accessing MIDI_data[] buffer
   //

   MSSLockedIncrementPtr(mdi->disable);

   //
   // Read message length and copy data to buffer
   //

   ptr = (U8 FAR *) buffer + 1;

   len = XMI_read_VLN(&ptr);

   len += AIL_ptr_dif(ptr, buffer);

   XMI_sysex_message(mdi, buffer, len);

   //
   // Reenable XMIDI service
   //

   MSSLockedDecrementPtr(mdi->disable);
}


//############################################################################
//##                                                                        ##
//## Get size and current position of sequence in milliseconds              ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_sequence_ms_position(HSEQUENCE S, //)
                                  S32 FAR    *total_milliseconds,
                                  S32 FAR   *current_milliseconds)
{
   //
   // Init total interval count = 0
   //

   S32 int_cnt = 0;
   S32 current = -1;

   //
   // Walk through all events in EVNT chunk, keeping track of
   // instrument regions used
   //

   U8 const FAR *ptr = (U8 FAR *) S->EVNT + 8;

   S32 done = 0;
   S32 last_int_stat = 0;

   while (!done)
      {
      S32 status = *ptr;
      S32 channel,type,len;

      //
      // If scan pointer reaches event pointer, mark current location
      // (Current event location is not valid until S->interval_count
      // reaches 0)
      //

      if ((AIL_ptr_dif(ptr,S->EVNT_ptr)>=0) && (current == -1))
         {
         current = int_cnt;

         if (S->interval_count >= 0)
            {
            current -= S->interval_count;
            }
         }

      //
      // Process interval byte
      //

      if (status < 0x80)
         {
         //
         // Accumulate interval count
         //

         last_int_stat = status;
         int_cnt += status;

         //
         // Skip delta time interval byte
         //

         ptr=AIL_ptr_add(ptr,1);
         continue;
         }

      switch (status)
         {
         //
         // Process MIDI meta-event, checking for end of sequence
         //

         case EV_META:

            ptr=AIL_ptr_add(ptr,1);

            type = *ptr;

            ptr=AIL_ptr_add(ptr,1);
            len = XMI_read_VLN(&ptr);

            if (type == META_EOT)
               {
               done = 1;
               }
            else
               {
               ptr=AIL_ptr_add(ptr,len);
               }
            break;

         //
         // Skip MIDI System Exclusive message
         //

         case EV_SYSEX:
         case EV_ESC:

            ptr=AIL_ptr_add(ptr,1);

            len = XMI_read_VLN(&ptr);

            ptr=AIL_ptr_add(ptr,len);
            break;

         //
         // Process MIDI channel voice message
         //

         default:

            channel = status & 0x0f;
            status  = status & 0xf0;

            //
            // Advance past channel-voice message
            //

            ptr=AIL_ptr_add(ptr,XMI_message_size(status));

            //
            // If this was EV_NOTE_ON, advance past duration word
            //

            if (status == EV_NOTE_ON)
               {
               XMI_read_VLN(&ptr);
               }

            break;
         }
      }

   //
   // Return requested values
   //

   if (total_milliseconds != NULL)
      {
      *total_milliseconds = (1000*int_cnt)/AIL_get_preference(MDI_SERVICE_RATE);
      }

   if (current_milliseconds != NULL)
      {
      *current_milliseconds = (1000*current)/AIL_get_preference(MDI_SERVICE_RATE);
      }
}

//############################################################################
//##                                                                        ##
//## Seek to a specified millisecond within a sequence                      ##
//##                                                                        ##
//############################################################################

void     AILCALL AIL_API_set_sequence_ms_position  (HSEQUENCE S, //)
                                            S32       milliseconds)
{
   HMDIDRIVER mdi;
   S32        log,phys;

   //
   // Get pointer to sequence events
   //

   U8 const FAR *ptr = (U8 FAR *) S->EVNT + 8;

   S32 target_interval=(milliseconds*AIL_get_preference(MDI_SERVICE_RATE))/1000;
   S32 done = 0;

   //
   // Init total interval count = 0
   //

   S32 int_cnt = 0;

   //
   // Make copy of control log which reflects current sequence state
   //

   CTRL_LOG original = S->shadow;

   //
   // Turn off any active notes in sequence
   //

   XMI_flush_note_queue(S);

   mdi = S->driver;

   for (log=0; log < NUM_CHANS; log++)
      {
      phys = S->chan_map[log];

      //
      // If sustain pedal on, release it
      //

      if (S->shadow.sus[log] >= 64)
         {
         XMI_MIDI_message(mdi,EV_CONTROL | phys,
                                           SUSTAIN,
                                           0);
         }
      }

   //
   // Walk through all events in EVNT chunk, keeping track of
   // instrument regions used
   //

   while (!done)
      {
      S32 status = *ptr;
      S32 type,len;

      //
      // If target interval reached, set new position and break out of loop
      //

      if (int_cnt >= target_interval)
         {
         S->EVNT_ptr = ptr;
         break;
         }

      //
      // Process interval byte
      //

      if (status < 0x80)
         {
         //
         // Accumulate interval count
         //

         int_cnt += status;

         //
         // Skip delta time interval byte
         //

         ptr=AIL_ptr_add(ptr,1);
         continue;
         }

      switch (status)
         {
         //
         // Process MIDI meta-event, checking for end of sequence
         //

         case EV_META:

            ptr=AIL_ptr_add(ptr,1);

            type = *ptr;

            ptr=AIL_ptr_add(ptr,1);
            len = XMI_read_VLN(&ptr);

            if (type == META_EOT)
               {
               done = 1;
               }
            else
               {
               ptr=AIL_ptr_add(ptr,len);
               }
            break;

         //
         // Skip MIDI System Exclusive message
         //

         case EV_SYSEX:
         case EV_ESC:

            ptr=AIL_ptr_add(ptr,1);

            len = XMI_read_VLN(&ptr);

            ptr=AIL_ptr_add(ptr,len);
            break;

         //
         // Process MIDI channel voice message
         //

         default:

            //
            // Update sequence state table
            //

            XMI_write_log(&S->shadow, status, *((U8 FAR*)AIL_ptr_add(ptr,1)), *((U8 FAR*)AIL_ptr_add(ptr,2)));

            //
            // Advance past channel-voice message
            //

            ptr=AIL_ptr_add(ptr,XMI_message_size(status));

            //
            // If this was EV_NOTE_ON, advance past duration word
            //

            if ((status&0xf0) == EV_NOTE_ON)
               {
               XMI_read_VLN(&ptr);
               }

            break;
         }
      }

   //
   // Send MIDI traffic as necessary to bring synthesizer up to date with
   // changes made to sequence state table
   //

   XMI_update_sequence(S, &original, &S->shadow);
}

