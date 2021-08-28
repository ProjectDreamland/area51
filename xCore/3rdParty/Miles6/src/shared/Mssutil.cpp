//############################################################################
//##                                                                        ##
//##  MSSUTIL.CPP                                                           ##
//##                                                                        ##
//##  DLS Level One utility routines                                        ##
//##                                                                        ##
//##  V1.00 of 14-Sep-98: Initial version                                   ##
//##                                                                        ##
//##  Author: John Miles and Jeff Roberts                                   ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  For technical support, contact RAD Game Tools at 425-893-4300.        ##
//##                                                                        ##
//############################################################################

#include "mss.h"
#include "imssapi.h"

#include "mssdls.h"
#include "dls1.h"
#include "synth.h"

#include <stdlib.h>

#ifndef WAVE_FORMAT_UNKNOWN
#define WAVE_FORMAT_UNKNOWN 0
#endif


//############################################################################
//##                                                                        ##
//## Set the compression preferences                                        ##
//##                                                                        ##
//############################################################################

HPROVIDER comp_ASI;
HASISTREAM comp_stream=0;
ASI_STREAM_SET_PREFERENCE ASI_stream_set_preference;

static S32 AILCALLBACK compress_setpref(char const FAR* preference,U32 value)
{
   U32 token;

   if (RIB_request_interface_entry(comp_ASI,"ASI stream",RIB_PREFERENCE,preference,&token))
     return(0);

   ASI_stream_set_preference(comp_stream,
                             token,
                             &value);
   return(1);
}


//############################################################################
//##                                                                        ##
//## Compress with ASI codec                                                ##
//##                                                                        ##
//############################################################################

static S32 AILCALL compress_ASI(AILSOUNDINFO const FAR* info, //)
                                void FAR* FAR* outdata,
                                U32 FAR* outsize,
                                AILLENGTHYCB callback)
{
   //
   // ASI functions
   //

   ASI_ERROR                 ASI_error;

   ASI_STREAM_OPEN           ASI_stream_open;
   ASI_STREAM_PROCESS        ASI_stream_process;
   ASI_STREAM_SEEK           ASI_stream_seek;
   ASI_STREAM_CLOSE          ASI_stream_close;
   ASI_STREAM_ATTRIBUTE      ASI_stream_attribute;
   ASI_STREAM_SET_PREFERENCE ASI_stream_set_preference;

   HATTRIB INPUT_BIT_RATE;
   HATTRIB INPUT_SAMPLE_RATE;
   HATTRIB INPUT_BITS;
   HATTRIB INPUT_CHANNELS;
   HATTRIB OUTPUT_BIT_RATE;
   HATTRIB OUTPUT_SAMPLE_RATE;
   HATTRIB OUTPUT_BITS;
   HATTRIB OUTPUT_CHANNELS;
   HATTRIB POSITION;
   HATTRIB PERCENT_DONE;
   HATTRIB MIN_INPUT_BLOCK_SIZE;
   HATTRIB RAW_RATE;
   HATTRIB RAW_BITS;
   HATTRIB RAW_CHANNELS;
   HATTRIB REQUESTED_BIT_RATE;
   HATTRIB REQUESTED_RATE;
   HATTRIB REQUESTED_BITS;
   HATTRIB REQUESTED_CHANS;

   RIB_INTERFACE_ENTRY ASICODEC[] =
      {
      FN(ASI_error)
      };

   RIB_INTERFACE_ENTRY ASISTR[] =
      {
      FN(ASI_stream_attribute),
      FN(ASI_stream_open),
      FN(ASI_stream_seek),
      FN(ASI_stream_close),
      FN(ASI_stream_process),
      FN(ASI_stream_set_preference),

      AT("Input bit rate",           INPUT_BIT_RATE),
      AT("Input sample rate",        INPUT_SAMPLE_RATE),
      AT("Input sample width",       INPUT_BITS),
      AT("Input channels",           INPUT_CHANNELS),
      AT("Output bit rate",          OUTPUT_BIT_RATE),
      AT("Output sample rate",       OUTPUT_SAMPLE_RATE),
      AT("Output sample width",      OUTPUT_BITS),
      AT("Output channels",          OUTPUT_CHANNELS),
      AT("Position",                 POSITION),
      AT("Percent done",             PERCENT_DONE),
      AT("Minimum input block size", MIN_INPUT_BLOCK_SIZE),
      PR("Raw source sample rate",   RAW_RATE),
      PR("Raw source sample width",  RAW_BITS),
      PR("Raw source channels",      RAW_CHANNELS),
      PR("Requested bit rate",       REQUESTED_BIT_RATE),
      PR("Requested sample rate",    REQUESTED_RATE),
      PR("Requested sample width",   REQUESTED_BITS),
      PR("Requested channels",       REQUESTED_CHANS)
      };

   RIB_request(comp_ASI,"ASI codec",ASICODEC);
   RIB_request(comp_ASI,"ASI stream",ASISTR);

   //
   // Open stream with codec, registering callback function
   //

   ASI_mem_src_ptr = (U8 FAR *) info->data_ptr;
   ASI_mem_src_len = info->data_len;
   ASI_mem_src_pos = 0;

   comp_stream = ASI_stream_open(0,
                                 ASI_mem_stream_CB,
                                 ASI_mem_src_len);

   if (comp_stream == NULL)
      {
      AIL_set_error(ASI_error());
      return 0;
      }

   //
   // Set source stream parameters
   //

   ASI_stream_set_preference(comp_stream,
                             RAW_RATE,
                             &info->rate);

   ASI_stream_set_preference(comp_stream,
                             RAW_BITS,
                             &info->bits);

   ASI_stream_set_preference(comp_stream,
                             RAW_CHANNELS,
                             &info->channels);

   if (callback)
     if (callback(AIL_LENGTHY_SET_PREFERENCE,(U32)compress_setpref)==0) {
       ASI_stream_close(comp_stream);
       comp_stream=0;
       AIL_set_error("Compression cancelled.");
       return(0);
     }

   //
   // Allocate output block equal in size to input data + 64K for overhead
   //

   void FAR *out = AIL_mem_alloc_lock(ASI_mem_src_len + 65536L);

   if (out == NULL)
      {
      ASI_stream_close(comp_stream);
      return 0;
      }

   //
   // Process input data
   //

   S32 len;
   S32 total_len = 0;
   void FAR *dest = out;

   while ((len = ASI_stream_process(comp_stream,
                                    dest,
                                    1024)) != 0)
      {
      dest = AIL_ptr_add(dest, len);
      total_len += len;
      }

   if (outsize != NULL)
      {
      *outsize = total_len;
      }

   if (outdata != NULL)
      {
      *outdata = out;
      }

#if 0
   AIL_debug_printf("%d bytes generated by encoder\n",total_len);
#endif

   ASI_stream_close(comp_stream);

   return 1;
}

// ---------------------------------------------------------------------------
// merge
// ---------------------------------------------------------------------------

DXDEF
S32 AILEXPORT AIL_merge_DLS_with_XMI(void const FAR* xmi, void const FAR* dls,
                                  void FAR* FAR* mss, U32 FAR* msssize)
{
  if ((xmi==0) || (dls==0))
    return(0);

   if ((AIL_strnicmp((char FAR*) xmi, "FORM", 4)) ||
       (AIL_strnicmp(((char FAR*) xmi)+8, "XDIR", 4)))
     {
     AIL_set_error("Not an XMI file.");
     return(0);
     }

  if ((((CHUNK FAR*)dls)->subID != FOURCC_DLS) && (((CHUNK FAR*)dls)->subID != FOURCC_MLS))
     {
     AIL_set_error("Not a DLS file.");
     return(0);
     }

  CHUNK FAR* ch=(CHUNK FAR*)xmi;

  U32 xmi_size=0;
  U32 chsize=0;
  do {
    ch=(CHUNK FAR*)AIL_ptr_add(ch,chsize);
    chsize=BE_SWAP32( &ch->ckSize)+8;
    xmi_size+=chsize;
  } while (AIL_strnicmp((char FAR*)ch,"CAT ",4));

  HMEMDUMP m=AIL_mem_create();

  AIL_mem_write(m,xmi,xmi_size);

  AIL_mem_prints(m,"FORM");

  U32 dls_size=LE_SWAP32( &((CHUNK FAR*)dls)->ckSize) + 8;

  AIL_mem_write(m,&dls_size,4);

  AIL_mem_prints(m,"XDLS");

  AIL_mem_write(m,dls,LE_SWAP32( &((CHUNK FAR*)dls)->ckSize ) +8);

  if ( !AIL_mem_close(m,mss,msssize) )
    AIL_set_error("Out of memory.");

  return(1);
}

// ---------------------------------------------------------------------------
// remove_chunk
//
// Removes chunk at given address from RIFF file at given address,
// returning # of bytes removed from file
// ---------------------------------------------------------------------------

static S32 remove_chunk(CHUNK FAR *chunk, void FAR*form)
{
   struct CDESC
      {
      CHUNK FAR *c;  // current LIST or RIFF chunk header
      CHUNK FAR *n;  // end of LIST or RIFF chunk
      };

   CDESC chunk_stack[256];

   //
   // Traverse file until specified chunk found
   //

   CDESC FAR *c = chunk_stack;

   c[0].n = c[0].c = NULL;

   CHUNK FAR *cur = (CHUNK FAR *) form;

   S32 end = (S32)(form) + (LE_SWAP32( &cur->ckSize ) + 8 + (LE_SWAP32( &cur->ckSize ) & 1));

   S32 done = 0;

   while (!done)
      {
      //
      // Get size and address of next chunk
      //

      S32 size = LE_SWAP32( &cur->ckSize ) + 8 + (LE_SWAP32( &cur->ckSize ) & 1);

      CHUNK FAR *nxt = (CHUNK FAR *) AIL_ptr_add(cur, size);

      //
      // Process chunk
      //
      // If this chunk matches the address of the one to be removed, kill it
      //

      if (cur == chunk)
         {
         //
         // Remove chunk from memory image
         //

         AIL_memcpy(cur, nxt, end - (S32)(nxt));

         //
         // Adjust size of all higher-level directory chunks to compensate for
         // removed subchunk
         //

         while (c != chunk_stack)
            {
            STORE_LE_SWAP32( &c->c->ckSize, LE_SWAP32( &c->c->ckSize ) - size );
            --c;
            }

         //
         // Return # of bytes removed from file
         //

         return size;
         }

      //
      // If current chunk is a LIST or RIFF, save address of next chunk at
      // outer level, and descend into inner chunk
      //

      if ((cur->ckID == FOURCC_LIST) || (cur->ckID == FOURCC_RIFF))
         {
         c++;
         c->c = cur;
         c->n = nxt;

         cur = (CHUNK FAR *) cur->data;
         continue;
         }

      //
      // Walk to next chunk at current level
      //

      cur = nxt;

      //
      // Next chunk = end of current chunk?  If so, pop back to outer
      // subchunk and continue traversal
      //

      while ((cur >= c->n) && (c != chunk_stack))
         {
         cur = c->n;
         c--;

         if (c == chunk_stack)
            {
            //
            // End of outermost chunk reached, exit
            //

            done = 1;
            break;
            }
         }
      }

   return 0;
}

// ---------------------------------------------------------------------------
// insert_chunk
//
// Inserts chunk at given address into RIFF file before specified chunk,
// returning # of bytes added to file
// ---------------------------------------------------------------------------

static S32 insert_chunk(CHUNK FAR *insertion_point, void FAR*form, CHUNK FAR *incoming)
{
   struct CDESC
      {
      CHUNK FAR *c;  // current LIST or RIFF chunk header
      CHUNK FAR *n;  // end of LIST or RIFF chunk
      };

   CDESC chunk_stack[256];

   //
   // Traverse file until specified chunk found
   //

   CDESC FAR *c = chunk_stack;

   c[0].n = c[0].c = NULL;

   CHUNK FAR *cur = (CHUNK FAR *) form;

   S32 end = (S32)(form) + (LE_SWAP32( &cur->ckSize ) + 8 + (LE_SWAP32( &cur->ckSize ) & 1));

   S32 done = 0;

   while (!done)
      {
      //
      // Get size and address of next chunk
      //

      S32 size = LE_SWAP32( &cur->ckSize ) + 8 + (LE_SWAP32( &cur->ckSize ) & 1);

      CHUNK FAR *nxt = (CHUNK FAR *) AIL_ptr_add(cur, size);

      //
      // Process chunk
      //

      if (cur == insertion_point)
         {
         //
         // Make room for incoming chunk in memory image
         //

         S32 new_size = LE_SWAP32( &incoming->ckSize ) + 8 + (LE_SWAP32( &incoming->ckSize ) & 1);

         AIL_memmove(AIL_ptr_add(cur, new_size), cur, end - (S32)(cur));

         //
         // Copy chunk into memory image
         //

         AIL_memcpy(cur, incoming, LE_SWAP32( &incoming->ckSize ) + 8);

         //
         // Adjust size of all higher-level directory chunks to compensate for
         // added subchunk
         //

         while (c != chunk_stack)
            {
            STORE_LE_SWAP32( &c->c->ckSize, LE_SWAP32( &c->c->ckSize ) + new_size );
            --c;
            }

         //
         // Return # of bytes added to file
         //

         return new_size;
         }

      //
      // If current chunk is a LIST or RIFF, save address of next chunk at
      // outer level, and descend into inner chunk
      //

      if ((cur->ckID == FOURCC_LIST) || (cur->ckID == FOURCC_RIFF))
         {
         c++;
         c->c = cur;
         c->n = nxt;

         cur = (CHUNK FAR *) cur->data;
         continue;
         }

      //
      // Walk to next chunk at current level
      //

      cur = nxt;

      //
      // Next chunk = end of current chunk?  If so, pop back to outer
      // subchunk and continue traversal
      //

      while ((cur >= c->n) && (c != chunk_stack))
         {
         cur = c->n;
         c--;

         if (c == chunk_stack)
            {
            //
            // End of outermost chunk reached, exit
            //

            done = 1;
            break;
            }
         }
      }

   return 0;
}

// ---------------------------------------------------------------------------
// replace_chunk
//
// Replace existing chunk, returning change in file image size
// ---------------------------------------------------------------------------

static S32 replace_chunk(CHUNK FAR *target, void FAR *form, CHUNK FAR *incoming)
{
   struct CDESC
      {
      CHUNK FAR *c;  // current LIST or RIFF chunk header
      CHUNK FAR *n;  // end of LIST or RIFF chunk
      };

   CDESC chunk_stack[256];

   //
   // Traverse file until specified chunk found
   //

   CDESC FAR *c = chunk_stack;

   c[0].n = c[0].c = NULL;

   CHUNK FAR *cur = (CHUNK FAR *) form;

   S32 end = (S32)(form) + (LE_SWAP32( &cur->ckSize ) + 8 + ( LE_SWAP32( &cur->ckSize ) & 1));

   S32 done = 0;

   while (!done)
      {
      //
      // Get size and address of next chunk
      //

      S32 size = LE_SWAP32( &cur->ckSize ) + 8 + (LE_SWAP32( &cur->ckSize ) & 1);

      CHUNK FAR *nxt = (CHUNK FAR *) AIL_ptr_add(cur, size);

      //
      // Process chunk
      //

      if (cur == target)
         {
         //
         // Get # of bytes to add/subtract from ckSize parameter
         //

         S32 new_size = LE_SWAP32( &incoming->ckSize ) + 8 + (LE_SWAP32( &incoming->ckSize ) & 1);

         S32 delta = new_size - size;

         //
         // Resize memory image
         //

         AIL_memmove(AIL_ptr_add(cur, new_size), nxt, end - (S32)(nxt));

         //
         // Copy new chunk data
         //

         AIL_memcpy(cur, incoming, LE_SWAP32( &incoming->ckSize ) + 8);

         //
         // Adjust size of all higher-level directory chunks to compensate for
         // added subchunk
         //

         while (c != chunk_stack)
            {
            STORE_LE_SWAP32( &c->c->ckSize, LE_SWAP32( &c->c->ckSize ) + delta );
            --c;
            }

         //
         // Return # of bytes added to file
         //

         return delta;
         }

      //
      // If current chunk is a LIST or RIFF, save address of next chunk at
      // outer level, and descend into inner chunk
      //

      if ((cur->ckID == FOURCC_LIST) || (cur->ckID == FOURCC_RIFF))
         {
         c++;
         c->c = cur;
         c->n = nxt;

         cur = (CHUNK FAR *) cur->data;
         continue;
         }

      //
      // Walk to next chunk at current level
      //

      cur = nxt;

      //
      // Next chunk = end of current chunk?  If so, pop back to outer
      // subchunk and continue traversal
      //

      while ((cur >= c->n) && (c != chunk_stack))
         {
         cur = c->n;
         c--;

         if (c == chunk_stack)
            {
            //
            // End of outermost chunk reached, exit
            //

            done = 1;
            break;
            }
         }
      }

   return 0;
}

// ---------------------------------------------------------------------------
// compress
//
// If encoder == NULL, use standard IMA ADPCM encoding
// Otherwise, use specified ASI encoder to compress data
//
// ---------------------------------------------------------------------------

DXDEF
S32 AILEXPORT AIL_compress_DLS(void const FAR* dls, //)
                               char const FAR* compress_extension,
                               void FAR* FAR* mls, U32 FAR* mlssize,
                               AILLENGTHYCB callback)
{

   //
   // find an encoder
   //

   comp_ASI = (compress_extension==0)?0:RIB_find_file_provider("ASI codec",
                                                               "Output file types",
                                                               compress_extension);
   //
   // Pass 1: Locate wave pool table in output file image
   //

   void FAR* out;
   U32 orig_size,osize;

   POOLTABLE FAR *ptbl = NULL;
   POOLCUE   FAR *cues = NULL;

   CHUNK FAR *c1 = (CHUNK FAR *) dls;

   if (c1->subID != FOURCC_DLS)
      {
      AIL_set_error("Not a DLS file.");
      return(0);
      }

   orig_size = osize = LE_SWAP32( &c1->ckSize ) + 8;

   out = AIL_mem_alloc_lock(osize);

   if (out==0) {
     return(0);
   }

   if (callback)
     if (callback(AIL_LENGTHY_INIT,0)==0) {
      cancelled:
       AIL_mem_free_lock(out);
       AIL_set_error("Compression cancelled.");
       return(0);
     }

   AIL_memcpy(out,c1,osize);

   c1 = (CHUNK FAR *) out;
   c1->subID = FOURCC_MLS;

   CHUNK FAR *n1 = (CHUNK FAR *) AIL_ptr_add(c1, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));
   CHUNK FAR *c2 = (CHUNK FAR *) c1->data;

   while (c2 < n1)
      {
      if (c2->ckID == FOURCC_PTBL)
         {
         ptbl = (POOLTABLE FAR *) /*&*/((SUBCHUNK FAR *) c2)->data;
         cues = (POOLCUE   FAR *) &((U8 FAR *) (ptbl))[LE_SWAP32( &ptbl->cbSize )];
         break;
         }

      c2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

   if (ptbl == NULL)
      {
      AIL_set_error("No PTBL chunk found.");
      return(0);
      }

   //
   // Pass 2: Temporarily replace PTBL offsets with WAVE ordinal indexes
   //

   n1 = (CHUNK FAR *) AIL_ptr_add(c1, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));
   c2 = (CHUNK FAR *) c1->data;

   U32 nth_wave = 0;

   U32 n_wave_cues = LE_SWAP32( &ptbl->cCues ) & 0xffffff;
   U32 cues_so_far = 0;

   while (c2 < n1)
      {
      if ((c2->ckID == FOURCC_LIST) && (c2->subID == FOURCC_WVPL))
         {
         CHUNK FAR *n2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
         CHUNK FAR *c3 = (CHUNK FAR *) c2->data;

         while (c3 < n2)
            {
            if ((c3->ckID == FOURCC_LIST) && (c3->subID == FOURCC_wave))
               {
               U32 offset = ((U32) c3) - ((U32) c2->data);

               for (U32 index=0; index < n_wave_cues; index++)
                  {
                  if (LE_SWAP32( &cues[index].ulOffset ) == offset)
                     {
                     cues[index].ulOffset = nth_wave;
                     }
                  }

               ++nth_wave;
               }

            c3 = (CHUNK FAR *)AIL_ptr_add(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
            }
         }

      c2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

   //
   // Pass 3: Replace PCM chunks with compressed chunks
   //
   // Perform operation from end of file towards beginning to minimize time
   // spent moving memory blocks
   //

   while (1)
      {
      CHUNK FAR *old_fmt = NULL;
      CHUNK FAR *old_data = NULL;

      CHUNK FAR *c1 = (CHUNK FAR *) out;

      CHUNK FAR *n1 = (CHUNK FAR *) AIL_ptr_add(c1, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));
      CHUNK FAR *c2 = (CHUNK FAR *) c1->data;

      CHUNK FAR *chunk_list = NULL;

      while (c2 < n1)
         {
         if ((c2->ckID == FOURCC_LIST) && (c2->subID == FOURCC_WVPL))
            {
            CHUNK FAR *n2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
            CHUNK FAR *c3 = (CHUNK FAR *) c2->data;

            while (c3 < n2)
               {
               if ((c3->ckID == FOURCC_LIST) && (c3->subID == FOURCC_wave))
                  {
                  CHUNK FAR *pcm        = NULL;
                  CHUNK FAR *found_data = NULL;
                  CHUNK FAR *found_fmt  = NULL;
                  CHUNK FAR *found_list = NULL;

                  CHUNK FAR *n3 = (CHUNK FAR *) AIL_ptr_add(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
                  CHUNK FAR *c4 = (CHUNK FAR *) c3->data;

                  found_list = c4;

                  while (c4 < n3)
                     {
                     if (c4->ckID == FOURCC_DATA)
                        {
                        found_data = c4;
                        }
                     else if (c4->ckID == FOURCC_FMT)
                        {
                        found_fmt = c4;
                        }
                     else if (c4->ckID == FOURCC_PCM)
                        {
                        pcm = c4;
                        }

                     c4 = (CHUNK FAR *) AIL_ptr_add(c4, LE_SWAP32( &c4->ckSize ) + 8 + (LE_SWAP32( &c4->ckSize ) & 1));
                     }

                  //
                  // If PCM chunk found, we've already processed this WAVE,
                  // so skip it
                  //

                  if (pcm == NULL)
                     {
                     old_data   = found_data;
                     old_fmt    = found_fmt;
                     chunk_list = found_list;
                     }
                  }

               c3 = (CHUNK FAR *) AIL_ptr_add(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
               }
            }

         c2 = (CHUNK FAR *)AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
         }

      //
      // Exit loop if no more LISTs to modify
      //

      if ((old_fmt == NULL) || (old_data == NULL) || (chunk_list == NULL))
         {
         break;
         }

      //
      // Rename old FMT chunk to PCM to preserve original format information
      //

      WAV_FMT PCM[2];
      AIL_memcpy(PCM, old_fmt,LE_SWAP32( &((WAV_FMT FAR*)old_fmt)->chunk_size ) + 8);  // use two for extra words
      AIL_memcpy(PCM[0].FMT_string,"pcm ",4);

      //
      // Generate new data, format, and fact chunks for compressed data
      //

      void FAR* new_ptr;

      AILSOUNDINFO info;
      info.format=WAVE_FORMAT_PCM;
      info.data_ptr=((SUBCHUNK FAR*)old_data)->data;
      info.data_len=LE_SWAP32( &((SUBCHUNK FAR*)old_data)->ckSize );
      info.rate=LE_SWAP32( &PCM->sample_rate );
      info.bits=LE_SWAP16( &PCM->bits_per_sample );
      info.channels=LE_SWAP16( &PCM->channels );
      info.block_size=LE_SWAP16( &PCM->alignment );

      ++cues_so_far;

      if (callback) {
        F32 per=(F32)(cues_so_far*100)/(F32)n_wave_cues;
        if (per>100.0F)
          per=100.0F;

        if (callback(AIL_LENGTHY_UPDATE,*(U32*)&per)==0)
          goto cancelled;
      }

      if ((comp_ASI == NULL) || ((info.data_len/LE_SWAP16( &PCM->alignment ) ) < (U32) AIL_get_preference(DLS_ADPCM_TO_ASI_THRESHOLD)))
         {
         if (AIL_compress_ADPCM(&info,&new_ptr,0)==0)
            {
            AIL_mem_free_lock(out);
            return(0);
            }
         }
      else
         {

         //
         // Compress data with specified ASI provider
         //

         U32 out_len;

         if (compress_ASI(&info,&new_ptr,&out_len,callback)==0)
            {
            AIL_mem_free_lock((void FAR*)info.data_ptr);
            AIL_mem_free_lock(out);
            return(0);
            }

         //
         // Prepend ASIOUT file header structure to data
         //

         struct ASIOUT
            {
            U32 riffmark;
            U32 rifflen;
            U32 wavemark;
            U32 fmtmark;
            U32 fmtlen;
            U16 fmttag;
            U16 channels;
            U32 sampersec;
            U32 avepersec;
            U16 blockalign;
            U16 bitspersam;
            S16 extra;
            C8  compressed_suffix[4];  // was S16 samples_per_block
            U32 factmark;
            U32 factlen;
            U32 samples;
            U32 datamark;
            U32 datalen;
            };

         ASIOUT FAR *adp = (ASIOUT FAR *)
            AIL_mem_alloc_lock(sizeof(ASIOUT) + out_len + 4);

         if (adp == NULL)
            {
            AIL_mem_free_lock(out);
            return(0);
            }

         AIL_memcpy(AIL_ptr_add(adp, sizeof(ASIOUT)),
                    new_ptr,
                    out_len);

         AIL_mem_free_lock(new_ptr);

         AIL_memcpy(&adp->riffmark,"RIFF",4);
         adp->rifflen=sizeof(ASIOUT)-8;
         AIL_memcpy(&adp->wavemark,"WAVE",4);
         AIL_memcpy(&adp->fmtmark,"fmt ",4);
         adp->fmtlen=22;   // was 20
         adp->fmttag=WAVE_FORMAT_UNKNOWN;
         adp->channels=(U16)info.channels;
         adp->sampersec=info.rate;
         adp->avepersec=info.rate * (info.bits/8) * info.channels;   // orig, not output
         adp->blockalign=1;
         adp->bitspersam=0;
         adp->extra=4;     // was 2
         AIL_memcpy(&adp->factmark,"fact",4);
         adp->factlen=4;
         adp->samples = (info.data_len / (info.bits/8)) / info.channels ;
         AIL_memcpy(&adp->datamark,"data",4);
         adp->datalen = out_len;
         adp->rifflen += adp->datalen;

         AIL_strcpy(adp->compressed_suffix,"MP3");

         new_ptr = adp;
         }

      IMA_FMT FAR* IMA_fmt=(IMA_FMT FAR*)((CHUNK FAR*)new_ptr)->data;
      IMA_FACT FAR* IMA_fact=(IMA_FACT FAR *) AIL_ptr_add( IMA_fmt, LE_SWAP32( &IMA_fmt->chunk_size ) + 8 );
      CHUNK FAR* new_data=(CHUNK FAR *) AIL_ptr_add( IMA_fact, LE_SWAP32( &IMA_fact->chunk_size ) + 8 );

      //
      // Replace DATA chunk with new ADCPM copy
      //

      osize += replace_chunk(old_data, out, (CHUNK FAR *) new_data);

      //
      // Remove old FMT chunk from list
      //

      osize -= remove_chunk(old_fmt, out);

      //
      // Insert new FACT and FMT chunks at beginning of chunk list
      //

      osize += insert_chunk(chunk_list, out, (CHUNK FAR *) IMA_fact);
      osize += insert_chunk(chunk_list, out, (CHUNK FAR *) IMA_fmt);

      AIL_mem_free_lock(new_ptr);

      //
      // Re-insert old FMT chunk as PCM chunk at beginning of list
      //

      osize += insert_chunk(chunk_list, out, (CHUNK FAR *) PCM);

      }

   //
   // Pass 4: Replace PTBL ordinals with offsets to new WAVEs
   //

   c1 = (CHUNK FAR *) out;

   n1 = (CHUNK FAR *) AIL_ptr_add(c1, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));
   c2 = (CHUNK FAR *) c1->data;

   nth_wave = 0;

   while (c2 < n1)
      {
      if ((c2->ckID == FOURCC_LIST) && (c2->subID == FOURCC_WVPL))
         {
         CHUNK FAR *n2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
         CHUNK FAR *c3 = (CHUNK FAR *) c2->data;

         while (c3 < n2)
            {
            if ((c3->ckID == FOURCC_LIST) && (c3->subID == FOURCC_wave))
               {
               U32 offset = ((U32) c3) - ((U32) c2->data);

               for (U32 index=0; index < n_wave_cues; index++)
                  {
                  if (cues[index].ulOffset == nth_wave)
                     {
                     cues[index].ulOffset = LE_SWAP32( &offset );
                     }
                  }

               ++nth_wave;
               }

            c3 = (CHUNK FAR *) AIL_ptr_add(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
            }
         }

      c2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

  if (mlssize)
    *mlssize=osize;

  //
  // If ASI encoder used, store overall file compression ratio, rounded up,
  // in high byte of ptbl->cCues
  //
  // Otherwise, we leave the high byte clear, for a default (ADPCM) ratio of
  // 4:1.  This ensures that ADPCM-compressed MLS files remain compatible
  // with third-party ADPCM-compatible DLS tools.
  //

  if (comp_ASI != NULL)
   {
   U32 ratio = ((orig_size + (osize / 2)) / osize) + 1;

   STORE_LE_SWAP32( &ptbl->cCues, (LE_SWAP32( &ptbl->cCues ) & 0x00ffffff) | (ratio << 24) );
   }

  if (mls)
    *mls=out;
  else
    AIL_mem_free_lock(out);

   if (callback)
     callback(AIL_LENGTHY_DONE,0);

  return(1);
}


DXDEF
S32 AILEXPORT AIL_filter_DLS_with_XMI(void const FAR* xmiptr, void const FAR* dls,
                                      void FAR* FAR* dlsout, U32 FAR* dlssize,
                                      S32 flags, AILLENGTHYCB callback)
{
  //
  // Compile list of patch numbers used (independent of bank #s) by any/all
  // sequences in file
  //

  if ((xmiptr==0) || (dls==0))
    return(0);

  if ((((CHUNK FAR*)dls)->subID != FOURCC_DLS) && (((CHUNK FAR*)dls)->subID != FOURCC_MLS))
     {
     AIL_set_error("Not a DLS file.");
     return(0);
     }


  void FAR* dlsdata=AIL_mem_alloc_lock( LE_SWAP32( &((CHUNK FAR*)dls)->ckSize ) );
  if (dlsdata==0)
     {
    err:
     return(0);
     }

  S32 patch[16];
  AIL_memset(patch, 0, 16*sizeof(S32));

  U8 patch_scoreboard[128];
  AIL_memset(patch_scoreboard, 0, 128);

  U8 FAR* region_scoreboard=(U8 FAR*)AIL_mem_alloc_lock((128L*128L*1)+(128L*128L*1));

  if (region_scoreboard==0) {
   err1:
    AIL_mem_free_lock(dlsdata);
    goto err;
  }

  AIL_memset(region_scoreboard, 0, 128*128);

  U8 drum_scoreboard[128];
  AIL_memset(drum_scoreboard, 0, 128);

  U8 FAR* bank_scoreboard=((U8 FAR*)region_scoreboard)+(128*128*1);
  AIL_memset(bank_scoreboard, 0, 128*128);

  S32 bank_l[128];
  AIL_memset(bank_l, 0, 128*sizeof(S32));

  S32 bank_h[128];
  AIL_memset(bank_h, 0, 128*sizeof(S32));

  S32 rpn_l[128];
  AIL_memset(rpn_l, 0, 128*sizeof(S32));

  S32 rpn_h[128];
  AIL_memset(rpn_h, 0, 128*sizeof(S32));

  S32 coarse_tune[128];
  AIL_memset(coarse_tune, 0, 128*sizeof(S32));

  U32 numxmis;
  U32 localxmis[1];

  void FAR* FAR* xmis;

  if (flags&AILFILTERDLS_USINGLIST) {
    numxmis=*((U32 FAR*)xmiptr);
    xmis=(void FAR* FAR*)AIL_ptr_add(xmiptr,4);
  } else {
    numxmis=1;
    xmis=(void FAR* FAR*)localxmis;
    localxmis[0]=(U32)xmiptr;
  }


  if (callback)
    if (callback(AIL_LENGTHY_INIT,0)==0) {
     cancelled:
      AIL_mem_free_lock(dlsdata);
      AIL_set_error("Filtering cancelled.");
      return(0);
    }


  while (numxmis--) {
    void FAR* curxmi=xmis[numxmis];

    if ((AIL_strnicmp((char FAR*) curxmi, "FORM", 4)) ||
        (AIL_strnicmp(((char FAR*) curxmi)+8, "XDIR", 4)))
      {
      AIL_set_error("Not an XMI file.");
      return(0);
      }

    S32 XMI_cnt = 0;

    U8 const FAR *image;

    AIL_memcpy(dlsdata,dls,LE_SWAP32( &((CHUNK FAR*)dls)->ckSize ) );

    while ((image = (U8 FAR*)XMI_find_sequence((U8 FAR*)curxmi,XMI_cnt)) != NULL)
       {
       //
       // Locate EVNT chunk within FORM XMID
       //

       U32     len = 8 + BE_SWAP32( ((U8 FAR *) (image+4)));
       U8 const FAR *end = image + len;

       image += 12;

       while (image < end)
          {
          if (!AIL_strnicmp((char*)image,"EVNT",4))
             {
             //
             // Walk through all events in EVNT chunk, keeping track of
             // instrument regions used
             //

             U8 const FAR *ptr = (U8 FAR *) image + 8;

             S32 done = 0;

             while (!done)
                {
                S32 status = *ptr;
                S32 channel,type,len;

                if (status < 0x80)
                   {
                   //
                   // Skip delta time interval byte
                   //

                   ++ptr;
                   continue;
                   }

                switch (status)
                   {
                   //
                   // Process MIDI meta-event, checking for end of sequence
                   //

                   case EV_META:

                      type = ptr[1];

                      ptr += 2;
                      len = XMI_read_VLN(&ptr);

                      if (type == META_EOT)
                         {
                         done = 1;
                         }
                      else
                         {
                         ptr += len;
                         }
                      break;

                   //
                   // Skip MIDI System Exclusive message
                   //

                   case EV_SYSEX:
                   case EV_ESC:

                      ptr += 1;

                      len = XMI_read_VLN(&ptr);

                      ptr += len;
                      break;

                   //
                   // Process MIDI channel voice message
                   //

                   default:

                      channel = status & 0x0f;
                      status  = status & 0xf0;

                      switch (status)
                         {
                         case EV_PROGRAM:

                            //
                            // Log patch # for this channel
                            //

                            patch[channel] = ptr[1];
                            break;

                         case EV_CONTROL:

                            switch (ptr[1])
                               {
                               case GM_BANK_MSB:
                                  bank_h[channel] = ptr[2];
                                  break;

                               case GM_BANK_LSB:
                                  bank_l[channel] = ptr[2];
                                  break;

                               case PATCH_BANK_SEL:
                                  bank_l[channel] = ptr[2];
                                  bank_h[channel] = 0;
                                  break;

                               case RPN_LSB:
                                  rpn_l[channel] = ptr[2];
                                  break;

                               case RPN_MSB:
                                  rpn_h[channel] = ptr[2];
                                  break;

                               case DATA_MSB:
                                  if ((rpn_l[channel] == 2) && (rpn_h[channel] == 0))
                                     {
                                     coarse_tune[channel] = (S32)(ptr[2]) - 64;
                                     }
                                  break;
                               }
                            break;

                         case EV_NOTE_ON:
                            {
                            //
                            // Log drum or melodic key event
                            //

                            S32 key = ptr[1];

                            if (channel == PERCUSS_CHAN)
                               {
                               drum_scoreboard[key] = 1;
                               }
                            else
                               {
                               //
                               // Modify key # by coarse tuning value for
                               // this channel
                               //

                               key += coarse_tune[channel];

                               if (key < 0)   key = 0;
                               if (key > 127) key = 127;

                               region_scoreboard[patch[channel]*128+key] = 1;
                               }

                            patch_scoreboard[patch[channel]] = 1;
                            bank_scoreboard[(bank_h[channel] << 7) + bank_l[channel]] = 1;
                            break;
                            }
                         }

                      //
                      // Advance past channel-voice message
                      //

                      ptr += XMI_message_size(status);

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
             }

          image += 8 + BE_SWAP32(((U8 FAR *) (image+4)));
          }

       ++XMI_cnt;
       }
    }

  //
  // Find COLH collection header in DLS file
  //

  S32 FAR *ins_cnt = NULL;

  CHUNK FAR* c1 = (CHUNK FAR *) dlsdata;

  CHUNK FAR *c2 = (CHUNK FAR *) c1->data;

  while (c2 < (CHUNK FAR *) AIL_ptr_add(c1, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1)))
     {
     if (c2->ckID == FOURCC_COLH)
        {
        ins_cnt = (S32 FAR *) /*&*/((SUBCHUNK FAR *) c2)->data;
        }

     c2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
     }

  //
  // Find PTBL chunk (list of WAVE offsets) and allocate a matching array
  // to keep track of which WAVE entries are used by the set of
  // instruments needed to play the XMIDI file
  //

  POOLTABLE FAR *ptbl;
  POOLCUE   FAR *cues;

  c1 = (CHUNK FAR *) dlsdata;
  c2 = (CHUNK FAR *) c1->data;

  while (c2 < (CHUNK FAR *) AIL_ptr_add(c1, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1)))
     {
     if (c2->ckID == FOURCC_PTBL)
        {
        ptbl = (POOLTABLE FAR *) /*&*/((SUBCHUNK FAR *) c2)->data;
        cues = (POOLCUE   FAR *) &((U8 FAR *) (ptbl))[LE_SWAP32( &ptbl->cbSize ) ];
        break;
        }

     c2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
     }

  U32 n_wave_cues = LE_SWAP32( &ptbl->cCues ) & 0xffffff;
  U32 cues_so_far=0;

  U8 FAR *wave_scoreboard = (U8 FAR *) AIL_mem_alloc_lock(n_wave_cues);

  if (wave_scoreboard == NULL)
     {
     AIL_mem_free_lock(region_scoreboard);
     goto err1;
     }

  AIL_memset(wave_scoreboard, 0, n_wave_cues);

  //
  // Find LINS instrument list chunk in DLS file
  //

  c1 = (CHUNK FAR *) dlsdata;
  c2 = (CHUNK FAR *) c1->data;

  while (c2 < (CHUNK FAR *) AIL_ptr_add(c1, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1)))
     {
     //
     // Look for outer LINS (instrument list) chunk
     //

     if ((c2->ckID == FOURCC_LIST) && (c2->subID == FOURCC_LINS))
        {
        CHUNK FAR *c3 = (CHUNK FAR *) c2->data;

        //
        // For each INS (instrument) chunk in instrument list...
        //

        while (c3 < (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1)))
           {
           if ((c3->ckID == FOURCC_LIST) && (c3->subID == FOURCC_INS))
              {
              S32        remove = 0;
              S32        drum_kit = 0;
              INSTHEADER header;

              ++cues_so_far;

              if (callback) {
                F32 per=(F32)(cues_so_far*50)/(F32)n_wave_cues;
                if (per>50.0F)
                  per=50.0F;

                if (callback(AIL_LENGTHY_UPDATE,*(U32*)&per)==0)
                  goto cancelled;
              }

              //
              // Look for INSH instrument header to determine patch
              // assignment
              //

              CHUNK FAR *c4 = (CHUNK FAR *) c3->data;

              while (c4 < (CHUNK FAR *) AIL_ptr_add(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1)))
                 {
                 if (c4->ckID == FOURCC_INSH)
                    {
                    header = *(INSTHEADER FAR *) (((SUBCHUNK FAR *) c4)->data);
                    MEM_LE_SWAP32( &header.cRegions );
                    MEM_LE_SWAP32( &header.Locale.ulBank );
                    MEM_LE_SWAP32( &header.Locale.ulInstrument );

                    //
                    // Flag this instrument for removal if either its
                    // bank or its patch is not referenced by the XMIDI
                    // file
                    //
                    // Instruments are also removed if their
                    // bank #s are never referenced in the XMIDI file
                    //

                    drum_kit = 0;

                    if (header.Locale.ulBank & F_INSTRUMENT_DRUMS)
                       {
                       drum_kit = 1;
                       }

                    if (!patch_scoreboard[header.Locale.ulInstrument])
                       {
                       remove = 1;
                       }

                    if (!bank_scoreboard[header.Locale.ulBank & 0x3fff])
                       {
                       remove = 1;
                       }
                    }

                 c4 = (CHUNK FAR *) AIL_ptr_add(c4, LE_SWAP32( &c4->ckSize ) + 8 + (LE_SWAP32( &c4->ckSize ) & 1));
                 }

              //
              // If this instrument is flagged for removal, excise it from
              // the DLS file memory image and continue
              //

              if (remove)
                 {
                 remove_chunk(c3, dlsdata);

                 if (ins_cnt != NULL)
                    {
                    STORE_LE_SWAP32( ins_cnt, LE_SWAP32( ins_cnt ) - 1 );
                    }

                 continue;
                 }

              //
              // Otherwise, if the instrument was NOT flagged for removal,
              // flag wave pool table entries referenced by all of its
              // regions as "in use"
              //

              c4 = (CHUNK FAR *) c3->data;

              while (c4 < (CHUNK FAR *) AIL_ptr_add(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1)))
                 {
                 if ((c4->ckID == FOURCC_LIST) && (c4->subID == FOURCC_LRGN))
                    {
                    CHUNK FAR *c5 = (CHUNK FAR *) c4->data;

                    //
                    // For each region in instrument...
                    //

                    while (c5 < (CHUNK FAR *) AIL_ptr_add(c4, LE_SWAP32( &c4->ckSize ) + 8 + (LE_SWAP32( &c4->ckSize ) & 1)))
                       {
                       if ((c5->ckID == FOURCC_LIST) && (c5->subID == FOURCC_RGN))
                          {
                          S32 keep  = 0;
                          S32 index = -1;

                          //
                          // Keep only the wave pool entries from regions
                          // which are needed to play the XMIDI file
                          //

                          CHUNK FAR *c6 = (CHUNK FAR *) c5->data;

                          while (c6 < (CHUNK FAR *) AIL_ptr_add(c5, LE_SWAP32( &c5->ckSize ) + 8 + (LE_SWAP32( &c5->ckSize ) & 1)))
                             {
                             if (c6->ckID == FOURCC_RGNH)
                                {
                                RGNHEADER FAR *region = (RGNHEADER FAR *) (((SUBCHUNK FAR *) c6)->data);

                                for (S32 i=(S32)LE_SWAP16( &region->RangeKey.usLow ); i <= (S32)LE_SWAP16( &region->RangeKey.usHigh ); i++)
                                   {
                                   if (drum_kit)
                                      {
                                      if (drum_scoreboard[i])
                                         {
                                         keep = 1;
                                         break;
                                         }
                                      }
                                   else
                                      {
                                      if (region_scoreboard[header.Locale.ulInstrument*128+i])
                                         {
                                         keep = 1;
                                         break;
                                         }
                                      }
                                   }
                                }

                             c6 = (CHUNK FAR *) AIL_ptr_add(c6, LE_SWAP32( &c6->ckSize ) + 8 + (LE_SWAP32( &c6->ckSize ) & 1));
                             }

                          if (keep)
                             {
                             //
                             // If we are keeping this region's WAVE data,
                             // look for the region's WLNK chunk and mark
                             // its wave link index for preservation
                             //

                             CHUNK FAR *c6 = (CHUNK FAR *) c5->data;

                             while (c6 < (CHUNK FAR *) AIL_ptr_add(c5, LE_SWAP32( &c5->ckSize ) + 8 + (LE_SWAP32( &c5->ckSize ) & 1)))
                                {
                                if (c6->ckID == FOURCC_WLNK)
                                   {
                                   WAVELINK FAR *link = (WAVELINK FAR *) (((SUBCHUNK FAR *) c6)->data);

                                   wave_scoreboard[LE_SWAP32( &link->ulTableIndex )] = 1;
                                   }

                                c6 = (CHUNK FAR *) AIL_ptr_add(c6, LE_SWAP32( &c6->ckSize ) + 8 + (LE_SWAP32( &c6->ckSize ) & 1));
                                }
                             }

                          }

                       c5 = (CHUNK FAR *) AIL_ptr_add(c5, LE_SWAP32( &c5->ckSize ) + 8 + (LE_SWAP32( &c5->ckSize ) & 1));
                       }
                    }

                 c4 = (CHUNK FAR *) AIL_ptr_add(c4, LE_SWAP32( &c4->ckSize ) + 8 + (LE_SWAP32( &c4->ckSize ) & 1));
                 }
              }

           c3 = (CHUNK FAR *) AIL_ptr_add(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
           }
        }

     c2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
     }

  //
  // PTBL chunk has probably been moved -- find it again
  //

  c1 = (CHUNK FAR *) dlsdata;
  c2 = (CHUNK FAR *) c1->data;

  while (c2 < (CHUNK FAR *) AIL_ptr_add(c1, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1)))
     {
     if (c2->ckID == FOURCC_PTBL)
        {
        ptbl = (POOLTABLE FAR *) /*&*/((SUBCHUNK FAR *) c2)->data;
        cues = (POOLCUE   FAR *) &((U8 FAR *) (ptbl))[ LE_SWAP32( &ptbl->cbSize ) ];
        break;
        }

     c2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
     }

  n_wave_cues = LE_SWAP32( &ptbl->cCues ) & 0xffffff;

  //
  // Set pool table offsets for all unused WAVEs to 0
  //

  for (U32 index=0; index < n_wave_cues; index++)
     {
     if (!wave_scoreboard[index])
        {
        cues[index].ulOffset = 0;
        }
     }

  cues_so_far=0;

  //
  // Remove DATA chunks from all LIST WAVE entries in the WVPL list
  // which are not used by any referenced instrument regions
  //

  c1 = (CHUNK FAR *) dlsdata;
  c2 = (CHUNK FAR *) c1->data;

  while (c2 < (CHUNK FAR *) AIL_ptr_add(c1, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1)))
     {
     if ((c2->ckID == FOURCC_LIST) && (c2->subID == FOURCC_WVPL))
        {
        CHUNK FAR *c3 = (CHUNK FAR *) c2->data;

        while (c3 < (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1)))
           {
           if ((c3->ckID == FOURCC_LIST) && (c3->subID == FOURCC_wave))
              {
              U32 offset = ((U32) c3) - ((U32) c2->data);

              U32 index;
              for (index=0; index < n_wave_cues; index++)
                 {
                 if (LE_SWAP32( &cues[index].ulOffset ) == offset)
                    {
                    break;
                    }
                 }

              ++cues_so_far;

              if (callback) {
                F32 per=((F32)(cues_so_far*50)/(F32)n_wave_cues)+50.0F;
                if (per>100.0F)
                  per=100.0F;

                if (callback(AIL_LENGTHY_UPDATE,*(U32*)&per)==0)
                  goto cancelled;
              }

              //
              // If this WAVE is not referenced by any surviving
              // instruments, remove it, and adjust all subsequent pool
              // table offsets accordingly
              //

              if (index == n_wave_cues)
                 {
                 S32 delta = remove_chunk(c3, dlsdata);

                 for (index=0; index < n_wave_cues; index++)
                    {
                    if (wave_scoreboard[index] &&
                       (LE_SWAP32( &cues[index].ulOffset ) > offset))
                       {
                       STORE_LE_SWAP32( &cues[index].ulOffset, LE_SWAP32( &cues[index].ulOffset ) - delta );
                       }
                    }

                 continue;
                 }

              //
              // Show progress indicator
              //

              }

           c3 = (CHUNK FAR *) AIL_ptr_add(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
           }
        }

     c2 = (CHUNK FAR *) AIL_ptr_add(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
     }

  AIL_mem_free_lock(region_scoreboard);
  AIL_mem_free_lock(wave_scoreboard);

  if (dlssize)
    *dlssize=LE_SWAP32( &((CHUNK FAR*)dlsdata)->ckSize );

  if (dlsout)
    *dlsout=dlsdata;
  else
    AIL_mem_free_lock(dlsdata);

  if (callback)
    callback(AIL_LENGTHY_DONE,0);

  return(1);
}


//############################################################################
//##                                                                        ##
//## Compress with ASI codec                                                ##
//##                                                                        ##
//############################################################################

DXDEF S32 AILEXPORT AIL_compress_ASI(AILSOUNDINFO const FAR* info, //)
                                     char const FAR* filename_ext,
                                     void FAR* FAR* outdata,
                                     U32 FAR* outsize,
                                     AILLENGTHYCB callback)
{
   //
   // ASI functions
   //

   ASI_ERROR                 ASI_error;

   ASI_STREAM_OPEN           ASI_stream_open;
   ASI_STREAM_PROCESS        ASI_stream_process;
   ASI_STREAM_SEEK           ASI_stream_seek;
   ASI_STREAM_CLOSE          ASI_stream_close;
   ASI_STREAM_ATTRIBUTE      ASI_stream_attribute;

   HATTRIB INPUT_BIT_RATE;
   HATTRIB INPUT_SAMPLE_RATE;
   HATTRIB INPUT_BITS;
   HATTRIB INPUT_CHANNELS;
   HATTRIB OUTPUT_BIT_RATE;
   HATTRIB OUTPUT_SAMPLE_RATE;
   HATTRIB OUTPUT_BITS;
   HATTRIB OUTPUT_CHANNELS;
   HATTRIB POSITION;
   HATTRIB PERCENT_DONE;
   HATTRIB MIN_INPUT_BLOCK_SIZE;
   HATTRIB RAW_RATE;
   HATTRIB RAW_BITS;
   HATTRIB RAW_CHANNELS;
   HATTRIB REQUESTED_BIT_RATE;
   HATTRIB REQUESTED_RATE;
   HATTRIB REQUESTED_BITS;
   HATTRIB REQUESTED_CHANS;

   RIB_INTERFACE_ENTRY ASICODEC[] =
      {
      FN(ASI_error)
      };

   RIB_INTERFACE_ENTRY ASISTR[] =
      {
      FN(ASI_stream_attribute),
      FN(ASI_stream_open),
      FN(ASI_stream_seek),
      FN(ASI_stream_close),
      FN(ASI_stream_process),
      FN(ASI_stream_set_preference),

      AT("Input bit rate",           INPUT_BIT_RATE),
      AT("Input sample rate",        INPUT_SAMPLE_RATE),
      AT("Input sample width",       INPUT_BITS),
      AT("Input channels",           INPUT_CHANNELS),
      AT("Output bit rate",          OUTPUT_BIT_RATE),
      AT("Output sample rate",       OUTPUT_SAMPLE_RATE),
      AT("Output sample width",      OUTPUT_BITS),
      AT("Output channels",          OUTPUT_CHANNELS),
      AT("Position",                 POSITION),
      AT("Percent done",             PERCENT_DONE),
      AT("Minimum input block size", MIN_INPUT_BLOCK_SIZE),
      PR("Raw source sample rate",   RAW_RATE),
      PR("Raw source sample width",  RAW_BITS),
      PR("Raw source channels",      RAW_CHANNELS),
      PR("Requested bit rate",       REQUESTED_BIT_RATE),
      PR("Requested sample rate",    REQUESTED_RATE),
      PR("Requested sample width",   REQUESTED_BITS),
      PR("Requested channels",       REQUESTED_CHANS)
      };

   comp_ASI = (filename_ext==0)?0:RIB_find_file_provider("ASI codec",
                                                         "Output file types",
                                                         filename_ext);

   if (comp_ASI==0) {
     AIL_set_error("No codec found for requested output type.");
     return(0);
   }

   if (callback)
     if (callback(AIL_LENGTHY_INIT,0)==0) {
      cancelled:
       AIL_set_error("Compression cancelled.");
       return(0);
     }

   RIB_request(comp_ASI,"ASI codec",ASICODEC);
   RIB_request(comp_ASI,"ASI stream",ASISTR);

   ASI_mem_src_ptr = (U8 FAR *) info->data_ptr;
   ASI_mem_src_len = info->data_len;

   S32 use_rate=info->rate;
   U8 FAR* new_ptr=0;

   if (info->format==WAVE_FORMAT_IMA_ADPCM) {
     AILMIXINFO mix;

    convert_or_resample:
     AIL_memcpy(&mix.Info,info,sizeof(mix.Info));

     S32 mssf=((info->channels==2)?DIG_F_STEREO_MASK:0)|((info->bits==16)?DIG_F_16BITS_MASK:0);
     U32 len=AIL_size_processed_digital_audio(use_rate,mssf,1,&mix);

     new_ptr=(U8 FAR*)AIL_mem_alloc_lock(len);
     if (new_ptr==0)
       return(0);

     len=AIL_process_digital_audio(new_ptr,len,use_rate,mssf,1,&mix);

     if (len==0) {
       AIL_mem_free_lock(new_ptr);
       return(0);
     }

     ASI_mem_src_ptr=(U8 FAR*)new_ptr;
     ASI_mem_src_len=len;
   }

   //
   // Open stream with codec, registering callback function
   //

   ASI_mem_src_pos = 0;

   comp_stream = ASI_stream_open(0,
                                       ASI_mem_stream_CB,
                                       ASI_mem_src_len);

   if (comp_stream == NULL)
      {
      AIL_set_error(ASI_error());
     err:
      if (new_ptr)
        AIL_mem_free_lock(new_ptr);
      return 0;
      }

   //
   // Set source stream parameters
   //

   ASI_stream_set_preference(comp_stream,
                             RAW_RATE,
                             &use_rate);

   ASI_stream_set_preference(comp_stream,
                             RAW_BITS,
                             &info->bits);

   ASI_stream_set_preference(comp_stream,
                             RAW_CHANNELS,
                             &info->channels);

   if (callback)
     if (callback(AIL_LENGTHY_SET_PREFERENCE,(U32)compress_setpref)==0) {
       ASI_stream_close(comp_stream);
       comp_stream=0;
       if (new_ptr)
         AIL_mem_free_lock(new_ptr);
       goto cancelled;
     }


   //
   // Allocate an output memory dump
   //

   HMEMDUMP mem=AIL_mem_create();

   if (mem == NULL)
      {
      ASI_stream_close(comp_stream);
      comp_stream=0;
      goto err;
      }

   //
   // Process input data
   //

   U8 temp_buf[1024];
   S32 len;

   len=ASI_stream_process(comp_stream,temp_buf,1024);

   if (ASI_stream_attribute(comp_stream,OUTPUT_SAMPLE_RATE)!=use_rate) {
     use_rate=ASI_stream_attribute(comp_stream,OUTPUT_SAMPLE_RATE);
     ASI_stream_close(comp_stream);
     comp_stream=0;
     AIL_mem_close(mem,0,0);
     if (new_ptr)
       AIL_mem_free_lock(new_ptr);
     goto convert_or_resample;
   }

   S32 cancel=0;

   while ((len!=0) && (cancel==0))
      {
      if (callback)
        if (callback(AIL_LENGTHY_UPDATE,ASI_stream_attribute(comp_stream,PERCENT_DONE))==0)
          cancel=1;
      AIL_mem_write(mem,temp_buf,len);
      len=ASI_stream_process(comp_stream, temp_buf,1024);
      }

   if ( !AIL_mem_close(mem,outdata,outsize) )
     AIL_set_error( "Out of memory." );

   ASI_stream_close(comp_stream);
   comp_stream=0;

   if (new_ptr)
     AIL_mem_free_lock(new_ptr);

   if (callback)
     if (callback(AIL_LENGTHY_DONE,0)==0)
       cancel=1;

   if (cancel)
     goto cancelled;

   return( 1 );
}


//############################################################################
//##                                                                        ##
//## Compress with ASI codec                                                ##
//##                                                                        ##
//############################################################################

DXDEF S32 AILEXPORT AIL_decompress_ASI(void const FAR* indata, //)
                                       U32 insize,
                                       char const FAR* filename_ext,
                                       void FAR* FAR* outdata,
                                       U32 FAR* outsize,
                                       AILLENGTHYCB callback)
{

   if ((indata==0) || (insize==0))
     return(0);

   //
   // ASI functions
   //

   ASI_ERROR                 ASI_error;

   ASI_STREAM_OPEN           ASI_stream_open;
   ASI_STREAM_PROCESS        ASI_stream_process;
   ASI_STREAM_SEEK           ASI_stream_seek;
   ASI_STREAM_CLOSE          ASI_stream_close;
   ASI_STREAM_ATTRIBUTE      ASI_stream_attribute;

   HATTRIB INPUT_BIT_RATE;
   HATTRIB INPUT_SAMPLE_RATE;
   HATTRIB INPUT_BITS;
   HATTRIB INPUT_CHANNELS;
   HATTRIB OUTPUT_BIT_RATE;
   HATTRIB OUTPUT_SAMPLE_RATE;
   HATTRIB OUTPUT_BITS;
   HATTRIB OUTPUT_CHANNELS;
   HATTRIB POSITION;
   HATTRIB PERCENT_DONE;
   HATTRIB MIN_INPUT_BLOCK_SIZE;
   HATTRIB RAW_RATE;
   HATTRIB RAW_BITS;
   HATTRIB RAW_CHANNELS;
   HATTRIB REQUESTED_BIT_RATE;
   HATTRIB REQUESTED_RATE;
   HATTRIB REQUESTED_BITS;
   HATTRIB REQUESTED_CHANS;

   RIB_INTERFACE_ENTRY ASICODEC[] =
      {
      FN(ASI_error)
      };

   RIB_INTERFACE_ENTRY ASISTR[] =
      {
      FN(ASI_stream_attribute),
      FN(ASI_stream_open),
      FN(ASI_stream_seek),
      FN(ASI_stream_close),
      FN(ASI_stream_process),
      FN(ASI_stream_set_preference),

      AT("Input bit rate",           INPUT_BIT_RATE),
      AT("Input sample rate",        INPUT_SAMPLE_RATE),
      AT("Input sample width",       INPUT_BITS),
      AT("Input channels",           INPUT_CHANNELS),
      AT("Output bit rate",          OUTPUT_BIT_RATE),
      AT("Output sample rate",       OUTPUT_SAMPLE_RATE),
      AT("Output sample width",      OUTPUT_BITS),
      AT("Output channels",          OUTPUT_CHANNELS),
      AT("Position",                 POSITION),
      AT("Percent done",             PERCENT_DONE),
      AT("Minimum input block size", MIN_INPUT_BLOCK_SIZE),
      PR("Raw source sample rate",   RAW_RATE),
      PR("Raw source sample width",  RAW_BITS),
      PR("Raw source channels",      RAW_CHANNELS),
      PR("Requested bit rate",       REQUESTED_BIT_RATE),
      PR("Requested sample rate",    REQUESTED_RATE),
      PR("Requested sample width",   REQUESTED_BITS),
      PR("Requested channels",       REQUESTED_CHANS)
      };

   comp_ASI = (filename_ext==0)?0:RIB_find_file_provider("ASI codec",
                                                         "Input file types",
                                                         filename_ext);

   if (comp_ASI==0) {
     AIL_set_error("No codec found for requested input type.");
     return(0);
   }

   if (callback)
     if (callback(AIL_LENGTHY_INIT,0)==0) {
      cancelled:
       AIL_set_error("Decompression cancelled.");
       return(0);
     }

   RIB_request(comp_ASI,"ASI codec",ASICODEC);
   RIB_request(comp_ASI,"ASI stream",ASISTR);

   ASI_mem_src_ptr = (U8 FAR*)indata;
   ASI_mem_src_len = insize;

   //
   // Open stream with codec, registering callback function
   //

   ASI_mem_src_pos = 0;

   comp_stream = ASI_stream_open(0,
                                 ASI_mem_stream_CB,
                                 ASI_mem_src_len);

   if (comp_stream == NULL)
      {
      AIL_set_error(ASI_error());
      return 0;
      }

   if (callback)
     if (callback(AIL_LENGTHY_SET_PREFERENCE,(U32)compress_setpref)==0) {
       ASI_stream_close(comp_stream);
       comp_stream=0;
       goto cancelled;
     }


   //
   // Allocate an output memory dump
   //

   HMEMDUMP mem=AIL_mem_create();

   if (mem == NULL)
      {
      ASI_stream_close(comp_stream);
      comp_stream=0;
      return(0);
      }

   WAVEOUT wav;

   AIL_mem_write(mem,&wav,sizeof(wav));

   //
   // Process input data
   //

   U8 temp_buf[1024];
   S32 len;
   S32 cancel=0;

   while (((len=ASI_stream_process(comp_stream, temp_buf,1024))!=0) && (cancel==0))
      {
      if (callback)
        if (callback(AIL_LENGTHY_UPDATE,ASI_stream_attribute(comp_stream,PERCENT_DONE))==0)
          cancel=1;
      AIL_mem_write(mem,temp_buf,len);
      }

   //create the wave header
   AIL_memcpy(&wav.riffmark,"RIFF",4);
   STORE_LE_SWAP32( &wav.rifflen, AIL_mem_size(mem)-8 );
   AIL_memcpy(&wav.wavemark,"WAVE",4);
   AIL_memcpy(&wav.fmtmark,"fmt ",4);
   STORE_LE_SWAP32( &wav.fmtlen, 16 );
   STORE_LE_SWAP16( &wav.fmttag, WAVE_FORMAT_PCM );
   STORE_LE_SWAP16( &wav.channels,(S16)ASI_stream_attribute(comp_stream,OUTPUT_CHANNELS) );
   STORE_LE_SWAP32( &wav.sampersec,ASI_stream_attribute(comp_stream,OUTPUT_SAMPLE_RATE) );
   STORE_LE_SWAP16( &wav.blockalign,(S16)((16*(U32)wav.channels) / 8) );
   STORE_LE_SWAP32( &wav.avepersec, (wav.sampersec*16*(U32)wav.channels) / 8 );
   STORE_LE_SWAP16( &wav.bitspersam, 16 );
   AIL_memcpy(&wav.datamark,"data",4);
   STORE_LE_SWAP32( &wav.datalen,AIL_mem_size(mem)-sizeof(wav) );

   //write out the wave header
   AIL_mem_seek(mem,0);
   AIL_mem_write(mem,&wav,sizeof(wav));
   if ( !AIL_mem_close(mem,outdata,outsize) )
     AIL_set_error( "Out of memory." );

   ASI_stream_close(comp_stream);
   comp_stream=0;

   if (callback)
     callback(AIL_LENGTHY_DONE,0);

   if (cancel)
     goto cancelled;

   return( 1 );
}


