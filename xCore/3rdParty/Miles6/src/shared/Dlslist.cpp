//############################################################################
//##                                                                        ##
//##  DLSLIST.CPP                                                           ##
//##                                                                        ##
//##  DLS Level One dump utility                                            ##
//##                                                                        ##
//##  V1.00 of 22-Nov-97: Initial version                                   ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  For technical support, contact RAD Game Tools at 425-893-4300.        ##
//##                                                                        ##
//############################################################################

#include <math.h>

#include "mss.h"
#include "imssapi.h"

#include "mssdls.h"
#include "dls1.h"
#include "synth.h"

//
// Misc. equates / structures
//

//
// statics
//

static HMEMDUMP memout=0;

//
// Prototypes and structures
//

static void print_dB     (S32 value);
static void print_ms     (S32 value);
static void print_percent(S32 value);
static void print_Hz     (S32 value);
static void print_cents  (S32 value);
static void print_literal(S32 value);

struct ARTDIAG
{
   C8 FAR*name;
   void (*print_fn)(S32 value);
};

static ARTDIAG art_diag[] =
{
   "LFO Frequency                     ", print_Hz,
   "LFO Start Delay                   ", print_ms,
   "LFO Attenuation Scale             ", print_dB,
   "LFO Pitch Scale                   ", print_cents,
   "LFO Modw to Attenuation           ", print_dB,
   "LFO Modw to Pitch                 ", print_cents,
   "Vol EG Attack Time                ", print_ms,
   "Vol EG Decay Time                 ", print_ms,
   "Vol EG Sustain Level              ", print_percent,
   "Vol EG Release Time               ", print_ms,
   "Vol EG Velocity to Attack Time    ", print_ms,
   "Vol EG Key to Decay Time          ", print_ms,
   "Pitch EG Attack Time              ", print_ms,
   "Pitch EG Decay Time               ", print_ms,
   "Pitch EG Sustain Level            ", print_percent,
   "Pitch EG Release Time             ", print_ms,
   "Pitch EG Velocity to Attack Time  ", print_ms,
   "Pitch EG Key to Decay Time        ", print_ms,
   "Default Pan                       ", print_percent,
   "Vol EG to Attenuation             ", print_dB,
   "Pitch EG to Pitch                 ", print_cents,
   "Key On Velocity to Attenuation    ", print_dB,
   "Pitch Wheel to Pitch              ", print_literal,
   "Key Number to Pitch               ", print_cents,
   "MIDI Controller 7 to Attenuation  ", print_dB,
   "MIDI Controller 10 to Pan         ", print_dB,
   "MIDI Controller 11 to Attenuation ", print_dB,
   "RPN1 to Pitch                     ", print_literal,
   "RPN2 to Pitch                     ", print_literal
};


// ---------------------------------------------------------------------------
// print_string
// ---------------------------------------------------------------------------

static void print_string(C8 FAR*string, S32 value)
{
   AIL_mem_prints(memout,string);

   for (S32 i=0; i < 16-(S32)(AIL_strlen(string)); i++)
      {
      AIL_mem_prints(memout," ");
      }

   AIL_mem_printf(memout,"(%X)\r\n",value);
}

// ---------------------------------------------------------------------------
// print_dB
// ---------------------------------------------------------------------------

static void print_dB(S32 value)
{
   C8 string[256];

   if (value == ABSOLUTE_ZERO)
      {
      AIL_sprintf(string,"Absolute zero");
      }
   else
      {
      AIL_sprintf(string,"%.1f dB",F32(value) / 655360.0F);
      }

   print_string(string,value);
}

// ---------------------------------------------------------------------------
// print_ms
// ---------------------------------------------------------------------------

static void print_ms(S32 value)
{
   C8 string[256];

   if (value == ABSOLUTE_ZERO)
      {
      AIL_sprintf(string,"Absolute zero");
      }
   else
      {
      F32 ms = F32(1000.0F * pow(2.0F, F32(value) / F32(65536*1200)));

      AIL_sprintf(string,"%.1f ms",ms);
      }

   print_string(string,value);
}

// ---------------------------------------------------------------------------
// print_percent
// ---------------------------------------------------------------------------

static void print_percent(S32 value)
{
   C8 string[256];

   AIL_sprintf(string,"%.1f%%", F32(value) / 655360.0F);

   print_string(string,value);
}

// ---------------------------------------------------------------------------
// print_Hz
// ---------------------------------------------------------------------------

static void print_Hz(S32 value)
{
   C8 string[256];

   F32 frequency = (F32)(pow(2.0F,
      ((((F32)(value) / 65536.0F) - 6900.0F) / 1200.0F))) * 440.0F;

   AIL_sprintf(string,"%.1f Hz",frequency);
   print_string(string,value);
}

// ---------------------------------------------------------------------------
// print_cents
// ---------------------------------------------------------------------------

static void print_cents(S32 value)
{
   C8 string[256];

   AIL_sprintf(string,"%.1f cents", F32(value) / 65536.0F);
   print_string(string,value);
}

// ---------------------------------------------------------------------------
// print_literal
// ---------------------------------------------------------------------------

static void print_literal(S32 value)
{
   C8 string[256];

   AIL_sprintf(string,"Implicit");

   print_string(string,value);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

DXDEF
S32       AILEXPORT AIL_list_DLS          (void const FAR* DLS,
                                           char FAR* FAR* list,
                                           U32  FAR* list_size,
                                           S32       flags,
                                           C8   FAR* title)
{
   char err[128];

   if (InstrumentsInit()==0)
     return(0);

   memout=AIL_mem_create();
   if (memout==0) {
    error:
     InstrumentsDeinit();
     return(0);
   }

   DLS_FILE FAR*file;

   S32 arg_a  = (flags&AILDLSLIST_ARTICULATION)?1:0;
   S32 result = DLSFILE_parse(DLS, &file, "(Memory file)"       , flags);

   if (result != DLS_NOERR)
      {
      switch (result)
         {
         case DLS_INVALID_FILE:  AIL_set_error("Not a DLS file."); break;
         case DLS_OUT_OF_MEM:    AIL_set_error("Out of memory.");  break;
         default:                AIL_sprintf(err,"Error %d parsing file.",result); AIL_set_error(err); break;
         }

      goto error;
      }

   //
   // Dump all instruments ordered by bank #, and their regions...
   //

   AIL_mem_prints(memout,"DLS Listing - Version " MSS_VERSION "        " MSS_COPYRIGHT "\r\n");
   AIL_mem_prints(memout,"____________________________________________________________________________\r\n\r\n");

   AIL_mem_printf(memout,"Contents of DLS 1.0 file %s (%d instruments):\r\n",
      title,
      file->cInstruments);

   for (S32 bank = 0; bank < 16384; bank++)
      {
      for (INSTRUMENT FAR *INS = instrument_list->used; INS; INS=INS->next)
         {
         //
         // Dump instrument header
         //

         S32 b = INS->header.Locale.ulBank & ~F_INSTRUMENT_DRUMS;

         if (bank != b)
            {
            continue;
            }

         AIL_mem_prints(memout,"_______________________________________________________________________________\r\n");
         AIL_mem_printf(memout,"\r\nBank %d (%d:%d), patch %d: ",
            b,
            b / 128,
            b % 128,
            INS->header.Locale.ulInstrument);

         if (INS->header.Locale.ulBank & F_INSTRUMENT_DRUMS)
            {
            AIL_mem_prints(memout,"Drum kit");
            }
         else
            {
            AIL_mem_prints(memout,"Melodic instrument");
            }

         AIL_mem_printf(memout,", %d region(s)\r\n",INS->header.cRegions);
         AIL_mem_prints(memout,"_______________________________________________________________________________\r\n");

         //
         // Dump regions
         //

         REGION FAR*RGN = INS->region_list;

         for (U32 i=0; i < INS->header.cRegions; i++,RGN++)
            {
            AIL_mem_printf(memout,"\r\n  Key region %d - %d, key group %d, option(s) %X",
               RGN->header.RangeKey.usLow,
               RGN->header.RangeKey.usHigh,
               RGN->header.usKeyGroup,
               RGN->header.fusOptions);

            if (RGN->header.fusOptions & F_RGN_OPTION_SELFNONEXCLUSIVE)
               {
               AIL_mem_printf(memout," (Not self-exclusive)");
               }

            AIL_mem_prints(memout,"\r\n");

            //
            // Dump WSMPL chunk
            //

            AIL_mem_printf(memout,"\r\n       WSMPL: Unity note %d\r\n",RGN->sample.usUnityNote);
            AIL_mem_printf(memout,"              Fine tune %d cents\r\n",
               RGN->sample.sFineTune);

            AIL_mem_prints(memout,"              Attenuation ");
            print_dB(RGN->sample.lAttenuation);

            AIL_mem_printf(memout,"              Option(s): %X",
               RGN->sample.fulOptions);

            if (RGN->sample.fulOptions & 3)
               {
               if (RGN->sample.fulOptions & F_WSMP_NO_TRUNCATION)
                  {
                  AIL_mem_prints(memout," (No truncation)");
                  }

               if (RGN->sample.fulOptions & F_WSMP_NO_COMPRESSION)
                  {
                  AIL_mem_prints(memout," (No compression)");
                  }
               }

            AIL_mem_prints(memout,"\r\n");

            //
            // Dump WAVEDESC structure
            //

            WAVEDESC FAR*wave = &file->WAVE_list[RGN->wave.ulTableIndex];

            AIL_mem_printf(memout,"\r\n        WAVE: %d bytes, %d Hz %d-bit %s\r\n",
               wave->len,
               (S32)(wave->rate),
               (wave->format & DIG_F_16BITS_MASK) ? 16 : 8,
               (wave->format & DIG_F_STEREO_MASK) ? "stereo (ILLEGAL)" : "mono");

            //
            // Dump WLOOP chunk, if any
            //

            if (RGN->sample.cSampleLoops > 1)
               {
               AIL_mem_printf(memout,"              *** ILLEGAL LOOP COUNT: %d ***\r\n",
                  RGN->sample.cSampleLoops);
               }
            else if (RGN->sample.cSampleLoops == 1)
               {
               AIL_mem_printf(memout,"\r\n       WLOOP: Type %d (%s)\r\n",
                  RGN->loop.ulType,
                  (RGN->loop.ulType == 0) ? "Forward" : "*** ILLEGAL ***");

               S32 BPS = (wave->format & DIG_F_16BITS_MASK) ? 2 : 1;

               AIL_mem_printf(memout,"              Start %d (Offset %d)\r\n",
                  RGN->loop.ulStart, RGN->loop.ulStart * BPS);

               AIL_mem_printf(memout,"              Length %d (%d bytes)\r\n",
                  RGN->loop.ulLength, RGN->loop.ulLength * BPS);
               }

            //
            // Dump WAVELINK chunk
            //

            AIL_mem_printf(memout,"\r\n    WAVELINK: Option(s): %X ",
               RGN->wave.fusOptions);

            if (RGN->wave.fusOptions & F_WAVELINK_PHASE_MASTER)
               {
               AIL_mem_prints(memout,"(Phase master)");
               }

            AIL_mem_prints(memout,"\r\n");

            AIL_mem_printf(memout,"              Phase group %d\r\n",RGN->wave.usPhaseGroup);
            AIL_mem_printf(memout,"              Channel placement %d\r\n",RGN->wave.ulChannel);
            AIL_mem_printf(memout,"              Pool table index %d\r\n",RGN->wave.ulTableIndex);

            //
            // Dump articulation connections (optional)
            //

            if (arg_a)
               {
               AIL_mem_prints(memout,"\r\n");

               CONNECTIONLIST FAR*cl;
               
               if (INS->header.Locale.ulBank & F_INSTRUMENT_DRUMS)
                  {
                  cl = RGN->art1;
                  }
               else
                  {
                  cl = INS->region_list[0].art1;
                  }

               CONNECTION FAR*c = (CONNECTION FAR*) (((U8 FAR*) cl) + LE_SWAP32( &cl->cbSize ) );

               for (U32 i=0; i < LE_SWAP32( &cl->cConnections ); i++,c++)
                  {
                  S32 S=-1,C=-1,D=-1;

                  S32 j;
                  for (j=0; j < ARYSIZE(src); j++)
                     {
                     if ( LE_SWAP16( &c->usSource)  == src[j].val) S = j;
                     if ( LE_SWAP16( &c->usControl) == src[j].val) C = j;
                     }

                  for (j=0; j < ARYSIZE(dest); j++)
                     {
                     if ( LE_SWAP16( &c->usDestination ) == dest[j].val) D = j;
                     }

                  S32 cid;
                  for (cid=0; cid < ARYSIZE(CID_list); cid++)
                     {
                     if (( LE_SWAP16( &c->usSource )      == CID_list[cid].usSource) &&
                         ( LE_SWAP16( &c->usControl )     == CID_list[cid].usControl) &&
                         ( LE_SWAP16( &c->usDestination ) == CID_list[cid].usDestination) &&
                         ( LE_SWAP16( &c->usTransform )   == CID_list[cid].usTransform))
                        {
                        break;
                        }
                     }

                  //
                  // Assign c->usScale to connection cid, if cid is valid articulator
                  // for DLS level 1
                  //

                  if ((S != -1) && (C != -1) && (D != -1))
                     {
                     //
                     // Connection source/control/destination components are
                     // all valid, print them
                     //

                     C8 FAR*xf[2] = {"TRN_NONE   ", "TRN_CONCAVE"};

                     if (cid < ARYSIZE(CID_list))
                        {
                        AIL_mem_printf(memout,"    %s",art_diag[cid].name);
                        art_diag[cid].print_fn( LE_SWAP32( &c->lScale ) );
                        }
                     else
                        {
                        C8 xform_string[256];

                        if ( LE_SWAP16( &c->usTransform ) >= ARYSIZE(xf))
                           {
                           AIL_sprintf(xform_string,"TRN_UNKNOWN (%Xh)",c->usTransform);
                           }
                        else
                           {
                           AIL_sprintf(xform_string,xf[ LE_SWAP16( &c->usTransform ) ]);
                           }

                        AIL_mem_printf(memout,"    CID ??: %s %s %s %s = %X\r\n",
                           src[S].name,
                           src[C].name,
                           dest[D].name,
                           xform_string,
                           LE_SWAP32( &c->lScale ) );
                        }
                     }
                  else
                     {
                     AIL_mem_printf(memout,"    *** ILLEGAL SCALAR *** S=%Xh C=%Xh D=%Xh T=%Xh\r\n",
                     LE_SWAP16( &c->usSource ), LE_SWAP16( &c->usControl ), LE_SWAP16( &c->usDestination ), LE_SWAP16( &c->usTransform ) );
                     }
                  }
               }
            }
         }
      }

   //
   // Return success
   //

   AIL_mem_prints(memout,"\r\n(End of list)\r\n");

   AIL_mem_printc(memout,0);

   InstrumentsDeinit();

   if ( !AIL_mem_close(memout,(void FAR* FAR*)list,list_size) )
     AIL_set_error("Out of memory.");

   return(1);
}

