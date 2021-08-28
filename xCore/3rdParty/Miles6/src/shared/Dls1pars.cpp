//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  DLS1PARS.CPP: DLS1 file parser                                        ##
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

#include <stdio.h>

#include "mss.h"
#include "imssapi.h"

#include "mssdls.h"
#include "dls1.h"
#include "synth.h"

#ifdef IS_MAC
#include <TextUtils.h>
#endif

//
// File and instrument collections
//

const CONN src[12] =
{
{"CONN_SRC_INVALID      ", 0xffff},
{"CONN_SRC_NONE         ", 0x0000},
{"CONN_SRC_LFO          ", 0x0001},
{"CONN_SRC_KEYONVELOCITY", 0x0002},
{"CONN_SRC_KEYNUMBER    ", 0x0003},
{"CONN_SRC_EG1          ", 0x0004},
{"CONN_SRC_EG2          ", 0x0005},
{"CONN_SRC_PITCHWHEEL   ", 0x0006},
{"CONN_SRC_CC1          ", 0x0081},
{"CONN_SRC_CC7          ", 0x0087},
{"CONN_SRC_CC10         ", 0x008a},
{"CONN_SRC_CC11         ", 0x008b}
};

const CONN dest[18] =
{
{"CONN_DST_INVALID         ", 0xffff},
{"CONN_DST_NONE            ", 0x0000},
{"CONN_DST_ATTENUATION     ", 0x0001},
{"CONN_DST_RESERVED        ", 0x0002},
{"CONN_DST_PITCH           ", 0x0003},
{"CONN_DST_PAN             ", 0x0004},
{"CONN_DST_LFO_FREQUENCY   ", 0x0104},
{"CONN_DST_LFO_STARTDELAY  ", 0x0105},
{"CONN_DST_EG1_ATTACKTIME  ", 0x0206},
{"CONN_DST_EG1_DECAYTIME   ", 0x0207},
{"CONN_DST_EG1_RESERVED    ", 0x0208},
{"CONN_DST_EG1_RELEASETIME ", 0x0209},
{"CONN_DST_EG1_SUSTAINLEVEL", 0x020a},
{"CONN_DST_EG2_ATTACKTIME  ", 0x030a},
{"CONN_DST_EG2_DECAYTIME   ", 0x030b},
{"CONN_DST_EG2_RESERVED    ", 0x030c},
{"CONN_DST_EG2_RELEASETIME ", 0x030d},
{"CONN_DST_EG2_SUSTAINLEVEL", 0x030e}
};

const CID CID_list[29] =
{
{CONN_SRC_NONE,         CONN_SRC_NONE, CONN_DST_LFO_FREQUENCY,   CONN_TRN_NONE},
{CONN_SRC_NONE,         CONN_SRC_NONE, CONN_DST_LFO_STARTDELAY,  CONN_TRN_NONE},
{CONN_SRC_LFO ,         CONN_SRC_NONE, CONN_DST_ATTENUATION,     CONN_TRN_NONE},
{CONN_SRC_LFO ,         CONN_SRC_NONE, CONN_DST_PITCH,           CONN_TRN_NONE},
{CONN_SRC_LFO ,         CONN_SRC_CC1,  CONN_DST_ATTENUATION,     CONN_TRN_NONE},
{CONN_SRC_LFO ,         CONN_SRC_CC1 , CONN_DST_PITCH,           CONN_TRN_NONE},
{CONN_SRC_NONE,         CONN_SRC_NONE, CONN_DST_EG1_ATTACKTIME,  CONN_TRN_NONE},
{CONN_SRC_NONE,         CONN_SRC_NONE, CONN_DST_EG1_DECAYTIME,   CONN_TRN_NONE},
{CONN_SRC_NONE,         CONN_SRC_NONE, CONN_DST_EG1_SUSTAINLEVEL,CONN_TRN_NONE},
{CONN_SRC_NONE,         CONN_SRC_NONE, CONN_DST_EG1_RELEASETIME, CONN_TRN_NONE},
{CONN_SRC_KEYONVELOCITY,CONN_SRC_NONE, CONN_DST_EG1_ATTACKTIME,  CONN_TRN_NONE},
{CONN_SRC_KEYNUMBER,    CONN_SRC_NONE, CONN_DST_EG1_DECAYTIME,   CONN_TRN_NONE},
{CONN_SRC_NONE,         CONN_SRC_NONE, CONN_DST_EG2_ATTACKTIME,  CONN_TRN_NONE},
{CONN_SRC_NONE,         CONN_SRC_NONE, CONN_DST_EG2_DECAYTIME,   CONN_TRN_NONE},
{CONN_SRC_NONE,         CONN_SRC_NONE, CONN_DST_EG2_SUSTAINLEVEL,CONN_TRN_NONE},
{CONN_SRC_NONE,         CONN_SRC_NONE, CONN_DST_EG2_RELEASETIME, CONN_TRN_NONE},
{CONN_SRC_KEYONVELOCITY,CONN_SRC_NONE, CONN_DST_EG2_ATTACKTIME,  CONN_TRN_NONE},
{CONN_SRC_KEYNUMBER,    CONN_SRC_NONE, CONN_DST_EG2_DECAYTIME,   CONN_TRN_NONE},
{CONN_SRC_NONE,         CONN_SRC_NONE, CONN_DST_PAN,             CONN_TRN_NONE},
{CONN_SRC_EG1,          CONN_SRC_NONE, CONN_DST_ATTENUATION,     CONN_TRN_NONE},
{CONN_SRC_EG2,          CONN_SRC_NONE, CONN_DST_PITCH,           CONN_TRN_NONE},
{CONN_SRC_KEYONVELOCITY,CONN_SRC_NONE, CONN_DST_ATTENUATION,     CONN_TRN_CONCAVE},
{CONN_SRC_PITCHWHEEL,(U16)CONN_SRC_RPN0, CONN_DST_PITCH,         CONN_TRN_NONE},
{CONN_SRC_KEYNUMBER,    CONN_SRC_NONE, CONN_DST_PITCH,           CONN_TRN_NONE},
{CONN_SRC_CC7,          CONN_SRC_NONE, CONN_DST_ATTENUATION,     CONN_TRN_CONCAVE},
{CONN_SRC_CC10,         CONN_SRC_NONE, CONN_DST_PAN,             CONN_TRN_NONE},
{CONN_SRC_CC11,         CONN_SRC_NONE, CONN_DST_ATTENUATION,     CONN_TRN_CONCAVE},
{(U16)CONN_SRC_RPN1,    CONN_SRC_NONE, CONN_DST_PITCH,           CONN_TRN_NONE},
{(U16)CONN_SRC_RPN2,    CONN_SRC_NONE, CONN_DST_PITCH,           CONN_TRN_NONE}

};

const ARTICULATION ARTDEFAULT =
{
   {
   0xFCACAE9C,    // LFO
   ABSOLUTE_ZERO,
   ABSOLUTE_ZERO,
   ABSOLUTE_ZERO,
   ABSOLUTE_ZERO,
   ABSOLUTE_ZERO,

   ABSOLUTE_ZERO,
   ABSOLUTE_ZERO,
   0x3e80000,     // EG1
   ABSOLUTE_ZERO,
   ABSOLUTE_ZERO,
   ABSOLUTE_ZERO,

   ABSOLUTE_ZERO,
   ABSOLUTE_ZERO,
   0x3e80000,     // EG2
   ABSOLUTE_ZERO,
   ABSOLUTE_ZERO,
   ABSOLUTE_ZERO,

   0,             // DEFAULT_PAN

   -96 * 655360,
   ABSOLUTE_ZERO,
   -96 * 655360,
   IMPLICIT,      // RPN 0
   12800,
   -96 * 655360,
   -96 * 655360,
   -96 * 655360,
   IMPLICIT,      // RPN 1,2
   IMPLICIT
   }
};

//
// Default WSMPL -- used if neither WAVE nor RGN contains a valid WSMP chunk
//

const WSMPL WSMPLDEFAULT =
{
   20,   // cbSize
   60,   // usUnityNote
   0,    // sFineTune
   0,    // lAttenuation,
   0,    // fulOptions
   0     // cSampleLoops
};

FileListMgr FAR *file_list = NULL;
InsListMgr  FAR *instrument_list = NULL;

//
// Single global instance of synthesizer device
//

struct SYNTHDEVICE FAR *DLS;


#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## Locked code                                                            ##
//##                                                                        ##
//############################################################################

#define LOCK(x)   AIL_vmm_lock  ((void*)&(x),sizeof(x))
#define LOCKPTR(x) AIL_vmm_lock  ((void*)(x),sizeof(x))
#define UNLOCK(x) AIL_vmm_unlock(&(x),sizeof(x))

static S32 locked = 0;

static void AIL_lock_end(void);

static void AIL_lock_start(void)
{
   if (!locked)
      {
      AIL_vmm_lock_range(AIL_lock_start, AIL_lock_end);

      LOCK(file_list);
      LOCK(instrument_list);
      LOCKPTR(src);
      LOCKPTR(dest);
      LOCKPTR(CID_list);
      LOCK(WSMPLDEFAULT);
      LOCK(DLS);

      locked = 1;
      }
}

#define DOLOCK() AIL_lock_start()

#else

#define DOLOCK()

#endif

// ---------------------------------------------------------------------------
// in_chunk
// ---------------------------------------------------------------------------

static U32 in_chunk(CHUNK FAR *current, CHUNK FAR *outer)
{
#ifndef IS_32

   U32 xx = GetSelectorBase(HIWORD(outer))   + LOWORD(outer);
   U32 yy = GetSelectorBase(HIWORD(current)) + LOWORD(current);

   return (yy < (xx + outer->ckSize + 8 + (outer->ckSize & 1)));

#else

   CHUNK FAR *end = (CHUNK FAR *) AIL_ptr_add(outer, LE_SWAP32( &outer->ckSize ) + 8 + ( LE_SWAP32( &outer->ckSize ) & 1));

   return (current < end);

#endif
}

//############################################################################
//#                                                                          #
//#  ART1_parse(): Parse instrument articulator list entry                   #
//#                                                                          #
//#  May be called from either region (drum) or instrument (melodic) lexical #
//#  level                                                                   #
//#                                                                          #
//############################################################################

static void ART1_parse(INSTRUMENT *INS, U32 index, CHUNK FAR *c1)
{
   //
   // Initialize connection scalars to default values
   //

   INS->art_list[index] = ARTDEFAULT;

   //
   // Override default scale values for any explicitly-specified connections
   //

   CONNECTIONLIST FAR *cl = (CONNECTIONLIST FAR *) (((SUBCHUNK FAR *) c1)->data);

   CONNECTION FAR *c = (CONNECTION FAR *) (((U8 FAR *) cl) + LE_SWAP32( &cl->cbSize ) );

   INS->region_list[index].art1 = cl;

   for (U32 i=0; i < LE_SWAP32( &cl->cConnections ); i++,c++)
      {
      S32 S=0,C=0,D=0;

      S32 j;
      for (j=0; j < ARYSIZE(src); j++)
         {
         if (LE_SWAP16( &c->usSource )  == src[j].val) S = j;
         if (LE_SWAP16( &c->usControl ) == src[j].val) C = j;
         }

      for (j=0; j < ARYSIZE(dest); j++)
         {
         if (LE_SWAP16( &c->usDestination ) == dest[j].val) D = j;
         }

      S32 cid;
      for (cid=0; cid < ARYSIZE(CID_list); cid++)
         {
         if ((LE_SWAP16( &c->usSource )      == CID_list[cid].usSource) &&
             (LE_SWAP16( &c->usControl )     == CID_list[cid].usControl) &&
             (LE_SWAP16( &c->usDestination ) == CID_list[cid].usDestination) &&
             (LE_SWAP16( &c->usTransform )   == CID_list[cid].usTransform))
             {
             break;
             }
         }

      //
      // Assign c->usScale to connection cid, if cid is valid articulator
      // for DLS level 1
      //

      if (cid < ARYSIZE(CID_list))
         {
         INS->art_list[index].lScale[cid] = LE_SWAP32( &c->lScale );
         }
      else
         {
         if ((LE_SWAP16( &c->usDestination ) != CONN_DST_RESERVED)     &&
             (LE_SWAP16( &c->usDestination ) != CONN_DST_EG1_RESERVED) &&
             (LE_SWAP16( &c->usDestination ) != CONN_DST_EG2_RESERVED))
            {
//            debug_printf("WAILDLS1: Ignoring unsupported articulator: S=%Xh C=%Xh D=%Xh T=%Xh\n",
//               c->usSource, c->usControl, c->usDestination, c->usTransform);
            }
         }
#if 0
      debug_printf("  (%2ld): %s %s %s %X = %lX\n",cid,src[S].name,src[C].name,dest[D].name,c->usTransform,c->lScale);
#endif
      }
}

//############################################################################
//#                                                                          #
//#  RGN_parse(): Parse instrument region list entry                         #
//#                                                                          #
//############################################################################

static void RGN_parse(INSTRUMENT FAR*INS, U32 index, CHUNK FAR *c1)
{
   //
   // Assume no WSMP chunk present
   //

   INS->region_list[index].sample.cbSize = 0;

   //
   // Interpret subchunks until end of <rgn-list> reached
   //

   CHUNK FAR *c2 = (CHUNK FAR *) c1->data;

#ifndef IS_32
   U32 c1_end = AIL_ptr_lin_addr(c1) + (LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));

   U32 c1_base = GetSelectorBase(HIWORD(c1));
   U32 c1_limit = GetSelectorLimit(HIWORD(c1));

   while (AIL_ptr_lin_addr(c2) < c1_end)
#else
   while (in_chunk(c2,c1))
#endif
      {
      AIL_ptr_fixup_clone(c2);

      switch (c2->ckID)
         {
         case FOURCC_RGNH:

            INS->region_list[index].header = *(RGNHEADER FAR *) (((SUBCHUNK FAR *) c2)->data);
            MEM_LE_SWAP16(&INS->region_list[index].header.RangeKey.usLow);
            MEM_LE_SWAP16(&INS->region_list[index].header.RangeKey.usHigh);
            MEM_LE_SWAP16(&INS->region_list[index].header.RangeVelocity.usLow);
            MEM_LE_SWAP16(&INS->region_list[index].header.RangeVelocity.usHigh);
            MEM_LE_SWAP16(&INS->region_list[index].header.fusOptions);
            MEM_LE_SWAP16(&INS->region_list[index].header.usKeyGroup);
            break;

         case FOURCC_WSMP:
            {
            WSMPL FAR *ptr = (WSMPL FAR *) (((SUBCHUNK FAR *) c2)->data);

            INS->region_list[index].sample = *ptr;
            MEM_LE_SWAP32(&INS->region_list[index].sample.cbSize);
            MEM_LE_SWAP16(&INS->region_list[index].sample.usUnityNote);  // MIDI unity playback note
            MEM_LE_SWAP16(&INS->region_list[index].sample.sFineTune);    // Fine tune in log tuning
            MEM_LE_SWAP32(&INS->region_list[index].sample.lAttenuation); // Overall attenuation to be applied to data
            MEM_LE_SWAP32(&INS->region_list[index].sample.fulOptions);   // Flag options
            MEM_LE_SWAP32(&INS->region_list[index].sample.cSampleLoops);

            if ( ptr->cSampleLoops )
               {
               INS->region_list[index].loop =
                  *(WLOOP FAR *) (((U8 FAR *) ptr) + LE_SWAP32( &ptr->cbSize ) );
               MEM_LE_SWAP32(&INS->region_list[index].loop.cbSize);
               MEM_LE_SWAP32(&INS->region_list[index].loop.ulType);     // Loop type
               MEM_LE_SWAP32(&INS->region_list[index].loop.ulStart);    // Start of loop in samples
               MEM_LE_SWAP32(&INS->region_list[index].loop.ulLength);
               }

            break;
            }

         case FOURCC_WLNK:

            INS->region_list[index].wave = *(WAVELINK FAR *) (((SUBCHUNK FAR *) c2)->data);
            MEM_LE_SWAP16(&INS->region_list[index].wave.fusOptions);   // options flags for this wave
            MEM_LE_SWAP16(&INS->region_list[index].wave.usPhaseGroup); // phase grouping for locking channels
            MEM_LE_SWAP32(&INS->region_list[index].wave.ulChannel);    // channel placement
            MEM_LE_SWAP32(&INS->region_list[index].wave.ulTableIndex); //
            break;

         case FOURCC_LIST:

            switch (c2->subID)
               {
               //
               // Drum instruments have an articulation list at each region
               //

               case FOURCC_LART:
                  {
                  CHUNK FAR *c3 = (CHUNK FAR *) c2->data;

               #ifndef IS_32
                  U32 c2_end = AIL_ptr_lin_addr(c2) + (LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));

                  U32 c2_base = GetSelectorBase(HIWORD(c2));
                  U32 c2_limit = GetSelectorLimit(HIWORD(c2));

                  while (AIL_ptr_lin_addr(c3) < c2_end)
               #else
                  while (in_chunk(c3,c2))
               #endif
                     {
                     AIL_ptr_fixup_clone(c3);

                     if (c3->ckID == FOURCC_ART1)
                        {
                        ART1_parse(INS, index, c3);
                        break;
                        }

                     AIL_ptr_inc_clone(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
                     }

               #ifndef IS_32
                  SetSelectorBase(HIWORD(c2), c2_base);
                  DPMISetSelectorLimit(HIWORD(c2), c2_limit);
               #endif
                  break;
                  }
               }
            break;
         }

      AIL_ptr_inc_clone(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

#ifndef IS_32
   SetSelectorBase(HIWORD(c1), c1_base);
   DPMISetSelectorLimit(HIWORD(c1), c1_limit);
#endif

   //
   // Assign articulation to region
   //
   // If drum kit, each regions has its own connection list
   // If melodic instrument, all regions use connection list 0
   //

   if (INS->header.Locale.ulBank & F_INSTRUMENT_DRUMS)
      {
      INS->region_list[index].connection = &INS->art_list[index];
      }
   else
      {
      INS->region_list[index].connection = &INS->art_list[0];
      }
}

//############################################################################
//#                                                                          #
//#  INS_parse(): Parse instrument list entry                                #
//#                                                                          #
//############################################################################

static void INS_parse(DLS_FILE FAR*file, CHUNK FAR *c1)
{
   //
   // Allocate INSTRUMENT entry
   //

   INSTRUMENT FAR*INS = (INSTRUMENT FAR*) instrument_list->alloc();

   if (INS == NULL)
      {
//      debug_printf("WAILDLS1: No memory for instrument list entry\n");
      return;
      }

   INS->DLS_file = file->entry_num;

   //
   // First pass through INS list: Find INSH header
   //

   CHUNK FAR *c2 = (CHUNK FAR *) c1->data;

#ifndef IS_32
   U32 c1_end = AIL_ptr_lin_addr(c1) + (LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));

   U32 c1_base = GetSelectorBase(HIWORD(c1));
   U32 c1_limit = GetSelectorLimit(HIWORD(c1));

   while (AIL_ptr_lin_addr(c2) < c1_end)
#else
   while (in_chunk(c2,c1))
#endif
      {
      AIL_ptr_fixup_clone(c2);

      switch (c2->ckID)
         {
         case FOURCC_INSH:

            INS->header = *(INSTHEADER FAR *) (((SUBCHUNK FAR *) c2)->data);
            MEM_LE_SWAP32( &INS->header.cRegions );
            MEM_LE_SWAP32( &INS->header.Locale.ulBank );
            MEM_LE_SWAP32( &INS->header.Locale.ulInstrument );
            break;
         }

      AIL_ptr_inc_clone(c2,LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

#ifndef IS_32
   SetSelectorBase(HIWORD(c1), c1_base);
   DPMISetSelectorLimit(HIWORD(c1), c1_limit);
#endif

   //
   // Allocate region list and initialize all fields to 0
   //
   // sizeof(REGION) = 64 bytes, 128 max per instrument
   //

   INS->region_list = (REGION FAR *)
      AIL_mem_alloc_lock(INS->header.cRegions * sizeof(REGION));

   if (INS->region_list == NULL)
      {
//      debug_printf("WAILDLS1: No memory for region list entry\n");
      instrument_list->dealloc(INS);
      return;
      }

   AIL_memset(INS->region_list, 0, (size_t) (INS->header.cRegions * sizeof(REGION)));

   //
   // Allocate articulation list and initialize all fields to 0
   //
   // One global articulation list if this is a melodic instrument
   //
   // One articulation list per region if this is a drum kit
   //
   // sizeof(ARTICULATION) = 116 bytes, 128 max per instrument
   //

   if (INS->header.Locale.ulBank & F_INSTRUMENT_DRUMS)
      {
      INS->art_cnt = INS->header.cRegions;
      }
   else
      {
      INS->art_cnt = 1;
      }

   INS->art_list = (ARTICULATION FAR *)
      AIL_mem_alloc_lock(INS->art_cnt * sizeof(ARTICULATION));

   if (INS->art_list == NULL)
      {
//      debug_printf("WAILDLS1: No memory for articulation list\n");
      instrument_list->dealloc(INS);
      return;
      }

   AIL_memset(INS->art_list, 0, (size_t) (INS->art_cnt * sizeof(ARTICULATION)));

   //
   // Second pass through INS list: Interpret subchunks until end of
   // list chunk reached
   //

   U32 region_index = 0;

   c2 = (CHUNK FAR *) c1->data;

#ifndef IS_32
   c1_end = AIL_ptr_lin_addr(c1) + (LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));

   c1_base = GetSelectorBase(HIWORD(c1));
   c1_limit = GetSelectorLimit(HIWORD(c1));

   while (AIL_ptr_lin_addr(c2) < c1_end)
#else
   while (in_chunk(c2,c1))
#endif
      {
      AIL_ptr_fixup_clone(c2);

      switch (c2->ckID)
         {
         case FOURCC_LIST:

            switch (c2->subID)
               {
               case FOURCC_LRGN:
                  {
                  CHUNK FAR *c3 = (CHUNK FAR *) c2->data;

               #ifndef IS_32
                  U32 c2_end = AIL_ptr_lin_addr(c2) + (LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));

                  U32 c2_base = GetSelectorBase(HIWORD(c2));
                  U32 c2_limit = GetSelectorLimit(HIWORD(c2));

                  while ((AIL_ptr_lin_addr(c3) < c2_end) && (region_index < INS->header.cRegions))
               #else
                  while (in_chunk(c3,c2) && (region_index < INS->header.cRegions))
               #endif
                     {
                     AIL_ptr_fixup_clone(c3);

                     if ((c3->ckID  == FOURCC_LIST) &&
                         (c3->subID == FOURCC_RGN))
                        {
                        RGN_parse(INS, region_index++, c3);
                        }

                     AIL_ptr_inc_clone(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
                     }

               #ifndef IS_32
                  SetSelectorBase(HIWORD(c2), c2_base);
                  DPMISetSelectorLimit(HIWORD(c2), c2_limit);
               #endif
                  break;
                  }

               case FOURCC_LART:
                  {
                  CHUNK FAR *c3 = (CHUNK FAR *) c2->data;

               #ifndef IS_32
                  U32 c2_end = AIL_ptr_lin_addr(c2) + (LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));

                  U32 c2_base = GetSelectorBase(HIWORD(c2));
                  U32 c2_limit = GetSelectorLimit(HIWORD(c2));

                  while (AIL_ptr_lin_addr(c3) < c2_end)
               #else
                  while (in_chunk(c3,c2))
               #endif
                     {
                     AIL_ptr_fixup_clone(c3);

                     if (c3->ckID == FOURCC_ART1)
                        {
                        ART1_parse(INS, 0, c3);
                        break;
                        }

                     AIL_ptr_inc_clone(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
                     }

               #ifndef IS_32
                  SetSelectorBase(HIWORD(c2), c2_base);
                  DPMISetSelectorLimit(HIWORD(c2), c2_limit);
               #endif
                  break;
                  }
               }
            break;
         }

      AIL_ptr_inc_clone(c2,LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

#ifndef IS_32
   SetSelectorBase(HIWORD(c1), c1_base);
   DPMISetSelectorLimit(HIWORD(c1), c1_limit);
#endif
}

//############################################################################
//#                                                                          #
//#  WAVE_parse(): Parse wave entry                                          #
//#                                                                          #
//############################################################################

static void WAVE_parse(DLS_FILE FAR*file, CHUNK FAR *c1, U32 offset, U32 flags)
{
   flags = flags; // for watcom's benefit

#if defined( IS_WIN32 ) || defined( IS_MAC )
   if (flags & AILDLSLIST_DUMP_WAVS)
      {
      //
      // Derive directory name from DLS filename
      //

      C8 wav_dir[256];

      strcpy(wav_dir,file->filename);

      U32 i;

      for (i=0; i < strlen(wav_dir); i++)
         {
         if (wav_dir[i] == '.')
            {
            break;
            }
         }

      strcpy(&wav_dir[i]," Samples");

      //
      // Create directory, if it doesn't already exist
      //

      static S32 wav_num = 0;

#ifdef IS_MAC
      FSSpec			fsSpec;
      char              dir[256];
      FSMakeFSSpec(0,0,(U8 *)"\pa.tmp", &fsSpec);
      HParamBlockRec	HParam;
      HFileParam *	FParam = (HFileParam *)&HParam;
      FParam->ioVRefNum = fsSpec.vRefNum;
      FParam->ioDirID = fsSpec.parID;
      dir[0]=AIL_strlen(wav_dir);
      AIL_memcpy(dir+1,wav_dir,dir[0]);
      FParam->ioNamePtr = (U8 FAR*)dir;
      PBDirCreate(&HParam, FALSE);
#else
      if (CreateDirectory(wav_dir,NULL))
         {
         wav_num = 0;
         }
#endif

      //
      // Init filename with serialized value
      //
      // Override with actual instrument file name, if available
      //

      C8 wav_fn[128];

#ifdef IS_MAC
      sprintf(wav_fn,":%s:%d.wav",wav_dir,wav_num++);
#else
      sprintf(wav_fn,"%s\\%d.wav",wav_dir,wav_num++);
#endif

      //
      // Try to find INAM chunk inside LIST INFO, to determine actual
      // filename of instrument sample
      //

      CHUNK FAR *c1_end = (CHUNK FAR *) AIL_ptr_add(c1, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));
      CHUNK FAR *c2 = (CHUNK FAR *) c1->data;

      while (c2 < c1_end)
         {
         if ((c2->ckID == FOURCC_LIST) && (c2->subID == FOURCC_INFO))
            {
            SUBCHUNK FAR *c2_end = (SUBCHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
            SUBCHUNK FAR *c3 = (SUBCHUNK FAR *) c2->data;

            while (c3 < c2_end)
               {
               if (c3->ckID == FOURCC_INAM)
                  {
#ifdef IS_MAC
                  sprintf(wav_fn,":%s:%s.wav",wav_dir,c3->data);
#else
                  sprintf(wav_fn,"%s\\%s.wav",wav_dir,c3->data);
#endif
                  }

               c3 = (SUBCHUNK FAR *) AIL_ptr_add(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
               }
            }

         c2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
         }

      //
      // Save .WAV file
      //

      U32 ckID = c1->ckID;
      U32 subID = c1->subID;
      c1->ckID = FOURCC_RIFF;
      c1->subID = mmioFOURCC('W','A','V','E');

#ifdef IS_MAC
      FSSpec spec;
      if (AIL_create_and_get_FSSpec(wav_fn,&spec, 'WAVE','MMSS' ))
        AIL_file_fss_attrib_write( &spec, c1, LE_SWAP32( &c1->ckSize), 'WAVE', 'MMSS' );
#else
      AIL_file_write(wav_fn,c1,c1->ckSize + 8);
#endif
      c1->ckID = ckID;
      c1->subID = subID;
      }
#endif

   // remove the compression ratio if this is an ASI sample
   STORE_LE_SWAP32( &file->ptbl->cCues, LE_SWAP32( &file->ptbl->cCues) & 0xffffff );

   //
   // Find wave pool cue that matches this WAVE's offset
   // If this is an orphan (unreferenced) WAVE, return
   //

   U32 index;
   for (index=0; index < LE_SWAP32( &file->ptbl->cCues ); index++)
      {
      if (LE_SWAP32( &file->cues[index].ulOffset ) == offset)
         {
         break;
         }
      }

   if (index == LE_SWAP32( &file->ptbl->cCues ) )
      {
//      debug_printf("WAILDLS1: Ignoring orphaned WAVE at offset %lX\n",
//         offset);

      return;
      }

   //
   // Find mandatory <fmt-ck>
   //

   WAV_FMT FAR *f = (WAV_FMT FAR *) AIL_ptr_alloc_clone(c1->data);

   while (AIL_strnicmp((char*)f->FMT_string,"fmt ",4))
      {
      AIL_ptr_inc_clone(f, LE_SWAP32( &f->chunk_size ) + 8 + (LE_SWAP32( &f->chunk_size ) & 1));
      }

   //
   // If <fmt-ck> is not standard PCM, use <pcm-ck> instead
   //

   if (LE_SWAP16( &f->format_tag ) != WAVE_FORMAT_PCM)
      {
      AIL_ptr_free_clone(f);

      f = (WAV_FMT FAR *) AIL_ptr_alloc_clone(c1->data);

      while (AIL_strnicmp((char*)f->FMT_string,"pcm ",4))
         {
         AIL_ptr_inc_clone(f, LE_SWAP32( &f->chunk_size ) + 8 + (LE_SWAP32( &f->chunk_size ) & 1));
         }
      }

   //
   // Configure sample type and rate based on FMT chunk
   //

   S32 format = DIG_F_MONO_8;

   if ((LE_SWAP16( &f->channels )        == 1) &&
       (LE_SWAP16( &f->bits_per_sample ) == 8))
      {
      format = DIG_F_MONO_8;
      }
   else if ((LE_SWAP16( &f->channels )        == 2) &&
            (LE_SWAP16( &f->bits_per_sample ) == 8))
      {
      format = DIG_F_STEREO_8;
      }
   else if ((LE_SWAP16( &f->channels )        == 1) &&
            (LE_SWAP16( &f->bits_per_sample ) == 16))
      {
      format = DIG_F_MONO_16;
      }
   else if ((LE_SWAP16( &f->channels )        == 2) &&
            (LE_SWAP16( &f->bits_per_sample ) == 16))
      {
      format = DIG_F_STEREO_16;
      }

   //
   // Find mandatory <data-ck>
   //

   WAV_DATA FAR *d = (WAV_DATA FAR *) AIL_ptr_alloc_clone(c1->data);

   while (AIL_strnicmp((char*)d->DATA_string,"data",4))
      {
      AIL_ptr_inc_clone(d, LE_SWAP32( &d->chunk_size ) + 8 + (LE_SWAP32( &d->chunk_size ) & 1));
      }

   //
   // Record information about this WAVE
   //

   file->WAVE_list[index].format = format;
   file->WAVE_list[index].rate   = F32(LE_SWAP32( &f->sample_rate));
   file->WAVE_list[index].len    = LE_SWAP32( &d->chunk_size );
   file->WAVE_list[index].data   = AIL_ptr_from_clone(d->data, file->image);

   AIL_ptr_free_clone(d);
   AIL_ptr_free_clone(f);

   //
   // Search for optional WSMP chunk at WAVE level
   // (if present, used as default if no WSMP chunk present at
   // the RGN level)
   //

   file->WAVE_list[index].sample.cbSize = 0;

   CHUNK FAR *c2 = (CHUNK FAR *) c1->data;

#ifndef IS_32
   U32 c1_end = AIL_ptr_lin_addr(c1) + (LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));

   U32 c1_base = GetSelectorBase(HIWORD(c1));
   U32 c1_limit = GetSelectorLimit(HIWORD(c1));

   while (AIL_ptr_lin_addr(c2) < c1_end)
#else
   while (in_chunk(c2,c1))
#endif
      {
      AIL_ptr_fixup_clone(c2);

      switch (c2->ckID)
         {
         case FOURCC_WSMP:
            {
            WSMPL FAR *ptr = (WSMPL FAR *) (((SUBCHUNK FAR *) c2)->data);

            file->WAVE_list[index].sample = *ptr;
            MEM_LE_SWAP32( &file->WAVE_list[index].sample.cbSize );
            MEM_LE_SWAP16( &file->WAVE_list[index].sample.usUnityNote );  // MIDI unity playback note
            MEM_LE_SWAP16( &file->WAVE_list[index].sample.sFineTune );    // Fine tune in log tuning
            MEM_LE_SWAP32( &file->WAVE_list[index].sample.lAttenuation ); // Overall attenuation to be applied to data
            MEM_LE_SWAP32( &file->WAVE_list[index].sample.fulOptions );   // Flag options
            MEM_LE_SWAP32( &file->WAVE_list[index].sample.cSampleLoops ); //

            if (ptr->cSampleLoops)
               {
               file->WAVE_list[index].loop =
                  *(WLOOP FAR *) (((U8 FAR *) ptr) + LE_SWAP32( &ptr->cbSize ));
               MEM_LE_SWAP32( &file->WAVE_list[index].loop.cbSize );
               MEM_LE_SWAP32( &file->WAVE_list[index].loop.ulType );     // Loop type
               MEM_LE_SWAP32( &file->WAVE_list[index].loop.ulStart );    // Start of loop in samples
               MEM_LE_SWAP32( &file->WAVE_list[index].loop.ulLength );   // Length of loop in samples
               }

            file->WAVE_list[index].WSMP_valid = 1;
            break;
            }
         }

      AIL_ptr_inc_clone(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

#ifndef IS_32
   SetSelectorBase(HIWORD(c1), c1_base);
   DPMISetSelectorLimit(HIWORD(c1), c1_limit);
#endif

   //
   // Mark this WAVEDESC structure valid
   //

   file->WAVE_list[index].valid = 1;
}


static S32 Iinited=0;

S32 InstrumentsInit()
{
  MSSLockedIncrement(Iinited);

  if (Iinited==1) {

    //
    // Initialize file and instrument list maintenance objects, and call
    // their "constructors" manually (can't use C++ new operator, because it
    // doesn't page-lock the memory...)
    //

    file_list = (FileListMgr FAR *) AIL_mem_alloc_lock(sizeof(FileListMgr));

    if (file_list == NULL)
       {
       MSSLockedDecrement(Iinited);
       return 0;
       }

    instrument_list = (InsListMgr FAR *) AIL_mem_alloc_lock(sizeof(InsListMgr));

    if (instrument_list == NULL)
       {
       AIL_mem_free_lock(file_list);
       file_list=0;
       MSSLockedDecrement(Iinited);
       return 0;
       }

    file_list->FileListMgr_construct();
    instrument_list->InsListMgr_construct();

   }
   return (1);

}

void InstrumentsDeinit()
{
  MSSLockedDecrement(Iinited);

  if (Iinited==0) {

   //
   // Call explicit destructors on file and instrument list maintainers
   //

   file_list->cleanup();
   instrument_list->cleanup();

   AIL_mem_free_lock(file_list);
   AIL_mem_free_lock(instrument_list);
   file_list=0;
   instrument_list=0;

   }

}

//############################################################################
//#                                                                          #
//#  DLSFILE_parse(): Entry point to parse file and build data structures    #
//#                                                                          #
//############################################################################

S32 DLSFILE_parse(void const FAR *data, DLS_FILE FAR * FAR *file_var, C8 const FAR *lpFileName, U32 flags)
{
   //
   // Allocate entry for file
   //

   DOLOCK();

   DLS_FILE FAR*file = (DLS_FILE FAR*) file_list->alloc(lpFileName);

   if (file == NULL)
      {
      //AIL_mem_free_lock(data);
      return DLS_OUT_OF_MEM;
      }

   AIL_strcpy(file->filename, lpFileName);
   file->image = data;

   //
   // Make sure this is a DLS or MLS file
   //
   // (MLS files supported for DLSLIST compatibility only)
   //

   CHUNK FAR *outer = (CHUNK FAR *) AIL_ptr_alloc_clone(data);

   if ((outer->subID != FOURCC_DLS) && (outer->subID != FOURCC_MLS))
      {
      AIL_ptr_free_clone(outer);
      file_list->dealloc(file);
      return DLS_INVALID_FILE;
      }

   //
   // Pass 1: Find PTBL chunk (list of WAVE offsets)
   //

   CHUNK FAR *c2 = (CHUNK FAR *) outer->data;

#ifndef IS_32
   U32 outer_end = AIL_ptr_lin_addr(outer) + (LE_SWAP32( &outer->ckSize ) + 8 + (LE_SWAP32( &outer->ckSize ) & 1));

   U32 outer_base = GetSelectorBase(HIWORD(outer));
   U32 outer_limit = GetSelectorLimit(HIWORD(outer));

   while (AIL_ptr_lin_addr(c2) < outer_end)
#else
   while (in_chunk(c2,outer))
#endif
      {
      AIL_ptr_fixup_clone(c2);

      //
      // Interpret second-level chunks
      //

      switch (c2->ckID)
         {
         case FOURCC_PTBL:

            file->ptbl = (POOLTABLE FAR *) AIL_ptr_alloc_clone(((SUBCHUNK FAR *) c2)->data);
            file->cues = (POOLCUE   FAR *) AIL_ptr_alloc_clone(&((U8 FAR *) (file->ptbl))[LE_SWAP32( &file->ptbl->cbSize )]);
            break;
         }

      AIL_ptr_inc_clone(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

#ifndef IS_32
   SetSelectorBase(HIWORD(outer), outer_base);
   DPMISetSelectorLimit(HIWORD(outer), outer_limit);
#endif

   // remove the compression ratio if this is an ASI sample
   STORE_LE_SWAP32( &file->ptbl->cCues, LE_SWAP32( &file->ptbl->cCues) &0xffffff );

   //
   // Allocate our WAVEDESC structure list for fast reference (to avoid
   // need to parse WAVE LIST chunk on demand)
   //

   file->WAVE_list = (WAVEDESC FAR *)
      AIL_mem_alloc_lock(LE_SWAP32( &file->ptbl->cCues ) * sizeof(WAVEDESC));

   if (file->WAVE_list == NULL)
      {
      AIL_ptr_free_clone(outer);
      file_list->dealloc(file);
      return DLS_OUT_OF_MEM;
      }

   //
   // Set all WAVEDESC fields, including "valid" fields, to 0
   //

   AIL_memset(file->WAVE_list, 0, (size_t) (LE_SWAP32( &file->ptbl->cCues ) * sizeof(WAVEDESC)));

   //
   // Pass 2: Walk through file and interpret remaining second-level chunks
   // until end of first-level chunk reached
   //

   c2 = (CHUNK FAR *) outer->data;

#ifndef IS_32
   outer_end = AIL_ptr_lin_addr(outer) + (LE_SWAP32( &outer->ckSize ) + 8 + (LE_SWAP32( &outer->ckSize ) & 1));

   outer_base = GetSelectorBase(HIWORD(outer));
   outer_limit = GetSelectorLimit(HIWORD(outer));

   while (AIL_ptr_lin_addr(c2) < outer_end)
#else
   while (in_chunk(c2,outer))
#endif
      {
      AIL_ptr_fixup_clone(c2);

      switch (c2->ckID)
         {
         case FOURCC_COLH:

            file->cInstruments = LE_SWAP32( (((SUBCHUNK FAR *) c2)->data) );
            break;

         case FOURCC_LIST:

            switch (c2->subID)
               {
               case FOURCC_LINS:
                  {
                  CHUNK FAR *c3 = (CHUNK FAR *) c2->data;

               #ifndef IS_32
                  U32 c2_end = AIL_ptr_lin_addr(c2) + (LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));

                  U32 c2_base = GetSelectorBase(HIWORD(c2));
                  U32 c2_limit = GetSelectorLimit(HIWORD(c2));

                  while ((AIL_ptr_lin_addr(c3) < c2_end))
               #else
                  while (in_chunk(c3,c2))
               #endif
                     {
                     AIL_ptr_fixup_clone(c3);

                     if ((c3->ckID  == FOURCC_LIST) &&
                         (c3->subID == FOURCC_INS))
                        {
                        INS_parse(file, c3);
                        }

                     AIL_ptr_inc_clone(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
                     }

               #ifndef IS_32
                  SetSelectorBase(HIWORD(c2), c2_base);
                  DPMISetSelectorLimit(HIWORD(c2), c2_limit);
               #endif
                  break;
                  }

               case FOURCC_WVPL:
                  {
                  CHUNK FAR *c3 = (CHUNK FAR *) c2->data;

               #ifndef IS_32
                  U32 c2_data = AIL_ptr_lin_addr(c2->data);
                  U32 c2_end = AIL_ptr_lin_addr(c2) + (LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));

                  U32 c2_base = GetSelectorBase(HIWORD(c2));
                  U32 c2_limit = GetSelectorLimit(HIWORD(c2));

                  while ((AIL_ptr_lin_addr(c3) < c2_end))
               #else
                  while (in_chunk(c3,c2))
               #endif
                     {
                     AIL_ptr_fixup_clone(c3);

                     if ((c3->ckID  == FOURCC_LIST) &&
                         (c3->subID == FOURCC_wave))
                        {
                     #ifndef IS_32
                        WAVE_parse(file, c3, AIL_ptr_lin_addr(c3) - c2_data, flags);
                     #else
                        WAVE_parse(file, c3, AIL_ptr_dif(c3, (U8 FAR *) c2->data), flags);
                     #endif
                        }

                     AIL_ptr_inc_clone(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
                     }

               #ifndef IS_32
                  SetSelectorBase(HIWORD(c2), c2_base);
                  DPMISetSelectorLimit(HIWORD(c2), c2_limit);
               #endif
                  break;
                  }
               }

            break;
         }

      AIL_ptr_inc_clone(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

#ifndef IS_32
   SetSelectorBase(HIWORD(outer), outer_base);
   DPMISetSelectorLimit(HIWORD(outer), outer_limit);
#endif

   //
   // Walk all regions for all instruments, ensuring that valid WSMPL data
   // exists for each region
   //

   S32 ins_cnt=0;

   for (INSTRUMENT FAR*INS = instrument_list->used; INS; INS=INS->next)
      {
      ++ins_cnt;

      for (U32 r=0; r < INS->header.cRegions; r++)
         {
         REGION FAR *R = &INS->region_list[r];

         if (R->sample.cbSize)
            {
            continue;
            }

         //
         // No WSMPL descriptor provided for region -- try to fetch it
         // from the associated WAVE chunk
         //

         U32 index = R->wave.ulTableIndex;

         if (file->WAVE_list[index].sample.cbSize)
            {
            R->sample = file->WAVE_list[index].sample;
            R->loop   = file->WAVE_list[index].loop;
            continue;
            }

         //
         // No WSMPL descriptor provided for region or WAVE chunk -- use
         // default parameters
         //

         R->sample = WSMPLDEFAULT;
         }
      }

   //
   // Return success
   //

   AIL_ptr_free_clone(outer);

//   debug_printf("WAILDLS1: Loaded %ld instruments, %ld expected\n",
//      ins_cnt, file->cInstruments);

   *file_var = file;

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

