//############################################################################
//##                                                                        ##
//##  DLSFILE.CPP                                                           ##
//##                                                                        ##
//##  DLS Level One utility routines                                        ##
//##                                                                        ##
//##  V1.00 of 14-Dec-97: Initial version                                   ##
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

#include "mss.h"
#include "imssapi.h"

#include "mssdls.h"
#include "dls1.h"
#include "synth.h"

#include <stdlib.h>

#ifndef WAVE_FORMAT_UNKNOWN
#define WAVE_FORMAT_UNKNOWN 0
#endif

#pragma pack(1)


#define min(a,b)  (((a) < (b)) ? (a) : (b))

#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## Locked code                                                            ##
//##                                                                        ##
//############################################################################

#define LOCK(x)   AIL_vmm_lock  (&(x),sizeof(x))
#define UNLOCK(x) AIL_vmm_unlock(&(x),sizeof(x))

static S32 locked = 0;

static void AIL_lock_end(void);

static void AIL_lock_start(void)
{
   if (!locked)
      {
      AIL_vmm_lock_range(AIL_lock_start, AIL_lock_end);

      LOCK(ASI_mem_src_ptr);
      LOCK(ASI_mem_src_len);
      LOCK(ASI_mem_src_pos);

      locked = 1;
      }
}

#define DOLOCK() AIL_lock_start()

#else

#define DOLOCK()

#endif

// ---------------------------------------------------------------------------
// ptr_freebase
//
// Adjusts huge pointer by difference between it and specified cloned pointer,
// then frees the cloned pointer before returning the modified huge pointer
// ---------------------------------------------------------------------------

static void FAR *ptr_freebase(void FAR *clone, void FAR *orig)
{
   void FAR* ptr=AIL_ptr_from_clone(clone,orig);

   AIL_ptr_free_clone(clone);

   return ptr;
}


// ---------------------------------------------------------------------------
// Find named FORM in IFF file
//
// Returns NULL if not found, else returns huge pointer to FORM
// ---------------------------------------------------------------------------

static U8 FAR * XMI_find_FORM(U8 FAR *image, S32 size, C8 FAR *name)
{
   U32 len = 0;

   U8 FAR *ptr = (U8 FAR *) AIL_ptr_alloc_clone(image);

   U8 FAR *end = (U8 FAR *) AIL_ptr_add(image, size);

   do
      {
      //
      // Skip previous block, if any
      //

      AIL_ptr_inc_clone(ptr, len);

      //
      // Exit if not FORM or CAT block
      //

      if (AIL_ptr_ge(ptr,end))
         {
         AIL_ptr_free_clone(ptr);
         return NULL;
         }

      if ((AIL_strnicmp((char*)ptr,"FORM",4)) &&
          (AIL_strnicmp((char*)ptr,"CAT ",4)))
         {
         AIL_ptr_free_clone(ptr);
         return NULL;
         }

      //
      // Continue searching if not named form
      //
      // XMIDI files always have even FORM lengths; therefore, no
      // odd-byte compensation is needed
      //

      len = 8 + BE_SWAP32( ((U8 FAR *) (ptr+4)));
      }
   while (AIL_strnicmp((char*)ptr+8,name,4));

   //
   // Return location relative to selector originally passed in
   //

   return (U8 FAR *) ptr_freebase(ptr, image);
}

// ---------------------------------------------------------------------------
// decode_ASI
//
// Decode specified number of bytes from ASI-encoded data source
//
// Backfill with silence if source stream ends prematurely
// ---------------------------------------------------------------------------

static S32 decode_ASI(C8   FAR *file_suffix, //)
                      void FAR *src,
                      S32       src_size,
                      void FAR *dest,
                      S32       dest_size,
                      S32       desired_bits_per_sample)
{
   //
   // Decode MPEG or other ASI-compressed data
   //

   HPROVIDER ASI = RIB_find_file_provider("ASI codec",
                                          "Input file types",
                                          file_suffix);

   if (!ASI)
      {
      AIL_debug_printf("DLSFILE: No ASI provider available for file type %s\n",
         file_suffix);
      return 0;
      }

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

   RIB_request(ASI,"ASI codec",ASICODEC);
   RIB_request(ASI,"ASI stream",ASISTR);

   //
   // Open stream with codec, registering callback function
   //

   ASI_mem_src_len = src_size;
   ASI_mem_src_ptr = (U8 FAR *) src;
   ASI_mem_src_pos = 0;

   HASISTREAM stream = ASI_stream_open(0,
                                       ASI_mem_stream_CB,
                                       ASI_mem_src_len);

   if (stream == NULL)
      {
      AIL_debug_printf("DLSFILE: Could not open stream (%s)\n",ASI_error());
      return 0;
      }

   S32 total_len = dest_size;

   //
   // Process input data
   //

   if (desired_bits_per_sample == 16)
      {
      S16 FAR *d = (S16 FAR *) dest;

      while (total_len)
         {
         S32 len = ASI_stream_process(stream,
                                      d,
                                      min(1024,total_len));

         if (!len)
            {
            if (total_len)
               {
               AIL_memset(d, 0, total_len);
               }

            break;
            }

         d = (S16 FAR *) AIL_ptr_add(d, len);

         total_len -= len;
         }
      }
   else
      {
      U8 FAR *d = (U8 FAR *) dest;

      while (total_len)
         {
         static S16 sign_buffer[1024];

         S32 bytes = ASI_stream_process(stream,
                                        sign_buffer,
                                        min(1024*sizeof(S16),total_len*2));

         S32 len = bytes >> 1;

         if (!len)
            {
            if (total_len)
               {
               AIL_memset(d, 0x80, total_len);
               }

            break;
            }
         else
            {
            for (S32 i=0; i < len; i++)
               {
               d[i] = (U8) (((sign_buffer[i] >> 8) & 0xff) ^ 0x80);
               }
            }

         d = (U8 FAR *) AIL_ptr_add(d, len);

         total_len -= len;
         }
      }

   //
   // Return success
   //

   ASI_stream_close(stream);

   return 1;
}

// ---------------------------------------------------------------------------
// get_expanded_DLS_size
// ---------------------------------------------------------------------------

static S32 get_expanded_DLS_size(void FAR *MLS, S32 MLS_size)
{
   //
   // Locate wave pool table in file image
   //

   CHUNK FAR *c1 = (CHUNK FAR *) AIL_ptr_alloc_clone(MLS);

   CHUNK FAR *n1 = (CHUNK FAR *) AIL_ptr_add(MLS, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize) & 1));
   CHUNK FAR *c2 = (CHUNK FAR *) AIL_ptr_alloc_clone(c1->data);

   while (AIL_ptr_lt(c2,n1))
      {
      AIL_ptr_fixup_clone(c2);

      if (c2->ckID == FOURCC_PTBL)
         {
         //
         // Look at cCues word in pool table structure -- if high byte zero,
         // assume this is an ADPCM-compressed file at 4:1
         //
         // Otherwise, the high byte is the approximate compression ratio
         // obtained from ASI encoding
         //

         POOLTABLE FAR *ptbl = (POOLTABLE FAR *)
            AIL_ptr_alloc_clone(((SUBCHUNK FAR *) c2)->data);

         U32 ratio = (LE_SWAP32( &ptbl->cCues) >> 24) & 0xff;

         //
         // Clear compression-ratio byte to restore DLS compatibility
         //

         STORE_LE_SWAP32( &ptbl->cCues,LE_SWAP32( &ptbl->cCues ) & 0x00ffffff );

         AIL_ptr_free_clone(ptbl);
         AIL_ptr_free_clone(c2);
         AIL_ptr_free_clone(c1);

         //
         // Return decompressed file size estimate (after conservative 
         // rounding at compression time)
         //

         if (ratio == 0)
            {
            return MLS_size * 4;
            }
         else
            {
            return MLS_size * ratio;
            }
         }

      AIL_ptr_inc_clone(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

// debug_printf("No PTBL chunk found\n");

   AIL_ptr_free_clone(c2);
   AIL_ptr_free_clone(c1);

   return 0;
}

// ---------------------------------------------------------------------------
// expand
// ---------------------------------------------------------------------------

static int expand(void FAR *out, S32 FAR *out_size,AILLENGTHYCB callback)
{
   //
   // Pass 1: Locate wave pool table in output file image
   //

   POOLTABLE FAR *ptbl = NULL;
   POOLCUE   FAR *cues = NULL;

   CHUNK FAR *c1 = (CHUNK FAR *) AIL_ptr_alloc_clone(out);

   CHUNK FAR *n1 = (CHUNK FAR *) AIL_ptr_add(out, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));
   CHUNK FAR *c2 = (CHUNK FAR *) AIL_ptr_alloc_clone(c1->data);

   while (AIL_ptr_lt(c2,n1))
      {
      AIL_ptr_fixup_clone(c2);

      if (c2->ckID == FOURCC_PTBL)
         {
         ptbl = (POOLTABLE FAR *) AIL_ptr_alloc_clone(((SUBCHUNK FAR *) c2)->data);
         cues = (POOLCUE   FAR *) AIL_ptr_alloc_clone(&((U8 FAR *) (ptbl))[LE_SWAP32( &ptbl->cbSize ) ]);
         break;
         }

      AIL_ptr_inc_clone(c2, LE_SWAP32( &c2->ckSize ) + 8 + ( LE_SWAP32( &c2->ckSize ) & 1));
      }

   if (ptbl == NULL)
      {
//      debug_printf("No PTBL chunk found\n");
      return 0;
      }

   AIL_ptr_free_clone(c2);
   AIL_ptr_free_clone(c1);

   //
   // Pass 2: Temporarily replace PTBL offsets with WAVE ordinal indexes
   //

   c1 = (CHUNK FAR *) AIL_ptr_alloc_clone(out);

   n1 = (CHUNK FAR *) AIL_ptr_add(out, LE_SWAP32( &c1->ckSize ) + 8 + ( LE_SWAP32( &c1->ckSize ) & 1));
   c2 = (CHUNK FAR *) AIL_ptr_alloc_clone(c1->data);

   U32 nth_wave = 0;

   U32 n_wave_cues = LE_SWAP32( &ptbl->cCues ) & 0xffffff;    // Low 3 bytes store # of cues

   U32 cues_so_far=0;

   while (AIL_ptr_lt(c2,n1))
      {
      AIL_ptr_fixup_clone(c2);

      if ((c2->ckID == FOURCC_LIST) && (c2->subID == FOURCC_WVPL))
         {
         CHUNK FAR *n2 = (CHUNK FAR *) AIL_ptr_alloc_clone(c2);
         AIL_ptr_inc_clone(n2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));

         CHUNK FAR *c3 = (CHUNK FAR *) AIL_ptr_alloc_clone(c2->data);

         while (AIL_ptr_lt(c3,n2))
            {
            AIL_ptr_fixup_clone(c3);

            if ((c3->ckID == FOURCC_LIST) && (c3->subID == FOURCC_wave))
               {
               U32 offset = AIL_ptr_dif(c3, c2->data);

               for (U32 index=0; index < n_wave_cues; index++)
                  {
                  if (LE_SWAP32( &cues[index].ulOffset ) == offset)
                     {
                     cues[index].ulOffset = nth_wave;
                     }
                  }

               ++nth_wave;
               }

            AIL_ptr_inc_clone(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
            }

         AIL_ptr_free_clone(c3);
         AIL_ptr_free_clone(n2);
         }

      AIL_ptr_inc_clone(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

   AIL_ptr_free_clone(c2);
   AIL_ptr_free_clone(c1);

   //
   // Pass 3: Generate PCM data to replace compressed chunks
   //

   c1 = (CHUNK FAR *) AIL_ptr_alloc_clone(out);
   c2 = (CHUNK FAR *) AIL_ptr_alloc_clone(c1->data);

   while (AIL_ptr_lt(c2, AIL_ptr_add(out, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1))))
      {
      AIL_ptr_fixup_clone(c2);

      if ((c2->ckID == FOURCC_LIST) && (c2->subID == FOURCC_WVPL))
         {
         CHUNK FAR *n2 = (CHUNK FAR *) AIL_ptr_alloc_clone(c2);
         AIL_ptr_inc_clone(n2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));

         CHUNK FAR *c3 = (CHUNK FAR *) AIL_ptr_alloc_clone(c2->data);

         while (AIL_ptr_lt(c3,n2))
            {
            AIL_ptr_fixup_clone(c3);

            if ((c3->ckID == FOURCC_LIST) && (c3->subID == FOURCC_wave))
               {
               CHUNK FAR *n3 = (CHUNK FAR *) AIL_ptr_alloc_clone(c3);
               AIL_ptr_inc_clone(n3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));

               CHUNK FAR *c4 = (CHUNK FAR *) AIL_ptr_alloc_clone(c3->data);

               ++cues_so_far;

               if (callback) {
                 F32 per=(F32)(cues_so_far*100)/(F32)n_wave_cues;
                 if (per>100.0F)
                   per=100.0F;

                 if (callback(AIL_LENGTHY_UPDATE,*(U32*)&per)==0) {
                   AIL_set_error("Decompression cancelled.");
                   return(0);
                 }
               }

               S32 total_samples     = 0;
               S32 bits_per_sample   = 0;
               S32 samples_per_block = 0;
               S32 alignment         = 0;
               S32 need_ASI          = 0;
               ASI_FMT ASI_chunk;

               while (AIL_ptr_lt(c4,n3))
                  {
                  AIL_ptr_fixup_clone(c4);

                  if (c4->ckID == FOURCC_PCM)
                     {
                     //
                     // PCM = original PCM FMT chunk -- determine # of bits
                     // per sample to unpack, and rename it to 'fmt'
                     //

                     c4->ckID = FOURCC_FMT;

                     bits_per_sample = LE_SWAP16( &((WAV_FMT FAR *) c4)->bits_per_sample );
                     }
                  else if (c4->ckID == FOURCC_FMT)
                     {
                     //
                     // FMT = IMA ADPCM FMT chunk -- read required packing
                     // information from it, and rename it to 'junk'
                     //
                     // Determine compression type (WAVE_FORMAT_IMA_ADPCM
                     // or WAVE_FORMAT_UNKNOWN) -- in the latter case, we'll
                     // need to search for an ASI codec to read the specified
                     // file type
                     //

                     c4->ckID = FOURCC_JUNK;

                     if (LE_SWAP16( &((IMA_FMT FAR *) c4)->format_tag ) == WAVE_FORMAT_IMA_ADPCM)
                        {
                        need_ASI = 0;
                        samples_per_block = LE_SWAP16( &((IMA_FMT FAR *) c4)->samples_per_block );
                        alignment         = LE_SWAP16( &((IMA_FMT FAR *) c4)->alignment );
                        }
                     else if (LE_SWAP16( &((IMA_FMT FAR *) c4)->format_tag ) == WAVE_FORMAT_UNKNOWN)
                        {
                        need_ASI = 1;
                        ASI_chunk = *((ASI_FMT FAR *) c4);
                        MEM_LE_SWAP32( &ASI_chunk.chunk_size );
                        MEM_LE_SWAP16( &ASI_chunk.format_tag );               // WAVE_FORMAT_UNKNOWN
                        MEM_LE_SWAP16( &ASI_chunk.channels );                 // # of channels
                        MEM_LE_SWAP32( &ASI_chunk.sample_rate );              // Sample rate in samples/second
                        MEM_LE_SWAP32( &ASI_chunk.average_data_rate );        // Stream rate in bytes per second
                        MEM_LE_SWAP16( &ASI_chunk.alignment );                // Always 1 (actual alignment constraints are determined by ASI decoder)
                        MEM_LE_SWAP16( &ASI_chunk.bits_per_sample );          // Bits/sample value from encoder output
                        MEM_LE_SWAP16( &ASI_chunk.extra );                    // Always 4
                        }
                     }
                  else if (c4->ckID == FOURCC_FACT)
                     {
                     //
                     // FACT contains information about the # of samples
                     // to be unpacked -- it's also renamed to 'junk' to
                     // ensure .WAV compatibility
                     //

                     c4->ckID = FOURCC_JUNK;

                     total_samples = LE_SWAP32( &((IMA_FACT FAR *) c4)->samples );
                     }
                  else if (c4->ckID == FOURCC_DATA)
                     {
                     //
                     // DATA will be encountered after the other chunk types,
                     // and can be expanded based on information already
                     // encountered
                     //

                     S32 size = total_samples * (bits_per_sample / 8);

                     WAV_DATA FAR *data = (WAV_DATA FAR *) AIL_mem_alloc_lock(size + 8 + 1);

                     if (data == NULL)
                        {
//                        debug_printf("Insufficient free memory to expand file\n");
                        return 0;
                        }

                     AIL_memcpy(data->DATA_string,"data",4);
                     data->chunk_size = LE_SWAP32( &size );

                     //
                     // Decode data stream
                     //

                     U8 FAR *s = (U8 FAR *) (((SUBCHUNK FAR *) c4)->data);

                     if (need_ASI == 1)
                        {
                        if (!decode_ASI(ASI_chunk.original_file_suffix,
                                        s,
                                        LE_SWAP32( &((IMA_DATA FAR*) c4)->chunk_size ),
                                        data->data,
                                        LE_SWAP32( &data->chunk_size ),
                                        bits_per_sample))
                           {
                           return 0;
                           }
                        }
                     else
                        {
                        //
                        // Decode ADPCM data
                        //

                        if (bits_per_sample == 16)
                           {
                           //
                           // Expand 16-bit WAVE data
                           //

                           S16 FAR *d = (S16 FAR*)data->data;

                           ADPCMDATA ad;

                           ad.blocksize = alignment;
                           ad.extrasamples=0;
                           ad.blockleft=0;

                           DecodeADPCM_MONO(&d,&s, size, LE_SWAP32( &((IMA_DATA FAR*)c4)->chunk_size ), &ad) ;

                           }
                        else
                           {
                           //
                           // Expand 8-bit WAVE data
                           //

                           U8 FAR *d = (U8 FAR*)data->data;

                           ADPCMDATA ad;

                           ad.blocksize = alignment;
                           ad.extrasamples=0;
                           ad.blockleft=0;

                           DecodeADPCM_MONO_8(&d,&s, size, LE_SWAP32( &((IMA_DATA FAR*)c4)->chunk_size ), &ad) ;
                           }
                        }

                     //
                     // Get change in size of expanded chunk
                     //

                     S32 old_size = LE_SWAP32( &c4->ckSize ) + 8 + (LE_SWAP32( &c4->ckSize ) & 1);
                     S32 new_size = LE_SWAP32( &data->chunk_size ) + 8 + (LE_SWAP32( &data->chunk_size ) & 1);
                     S32 delta_size = new_size - old_size;

                     //
                     // Resize memory image
                     //

                     void FAR *end = AIL_ptr_add(out, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));

                     CHUNK FAR *nxt = (CHUNK FAR *) AIL_ptr_alloc_clone(c4);
                     AIL_ptr_inc_clone(nxt, old_size);

                     void FAR *dest = AIL_ptr_alloc_clone(c4);
                     AIL_ptr_inc_clone(dest, new_size);

                     AIL_memmove(dest, nxt, AIL_ptr_dif(end,nxt));

                     AIL_ptr_free_clone(dest);
                     AIL_ptr_free_clone(nxt);

                     //
                     // Copy new chunk data
                     //

                     AIL_memcpy(c4, data, LE_SWAP32( &data->chunk_size ) + 8);

                     //
                     // Adjust file and chunk sizes
                     //

                     *out_size += delta_size;

                     AIL_ptr_inc_clone(n3, delta_size);
                     AIL_ptr_inc_clone(n2, delta_size);

                     STORE_LE_SWAP32( &c1->ckSize, LE_SWAP32( &c1->ckSize) + delta_size );
                     STORE_LE_SWAP32( &c2->ckSize, LE_SWAP32( &c2->ckSize) + delta_size );
                     STORE_LE_SWAP32( &c3->ckSize, LE_SWAP32( &c3->ckSize) + delta_size );

                     //
                     // Free working copy of DATA chunk
                     //

                     AIL_mem_free_lock(data);
                     }

                  AIL_ptr_inc_clone(c4, LE_SWAP32( &c4->ckSize ) + 8 + (LE_SWAP32( &c4->ckSize ) & 1));
                  }

               AIL_ptr_free_clone(c4);
               AIL_ptr_free_clone(n3);
               }

            AIL_ptr_inc_clone(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
            }

         AIL_ptr_free_clone(c3);
         AIL_ptr_free_clone(n2);
         }

      AIL_ptr_inc_clone(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

   AIL_ptr_free_clone(c2);
   AIL_ptr_free_clone(c1);

   //
   // Pass 4: Replace PTBL ordinals with offsets to new WAVEs
   //

   c1 = (CHUNK FAR *) AIL_ptr_alloc_clone(out);

   n1 = (CHUNK FAR *) AIL_ptr_add(out, LE_SWAP32( &c1->ckSize ) + 8 + (LE_SWAP32( &c1->ckSize ) & 1));
   c2 = (CHUNK FAR *) AIL_ptr_alloc_clone(c1->data);

   nth_wave = 0;

   while (AIL_ptr_lt(c2,n1))
      {
      AIL_ptr_fixup_clone(c2);

      if ((c2->ckID == FOURCC_LIST) && (c2->subID == FOURCC_WVPL))
         {
         CHUNK FAR *n2 = (CHUNK FAR *) AIL_ptr_alloc_clone(c2);
         AIL_ptr_inc_clone(n2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));

         CHUNK FAR *c3 = (CHUNK FAR *) AIL_ptr_alloc_clone(c2->data);

         while (AIL_ptr_lt(c3,n2))
            {
            AIL_ptr_fixup_clone(c3);

            if ((c3->ckID == FOURCC_LIST) && (c3->subID == FOURCC_wave))
               {
               U32 offset = AIL_ptr_dif(c3, c2->data);

               for (U32 index=0; index < n_wave_cues; index++)
                  {
                  if (cues[index].ulOffset == nth_wave)
                     {
                     cues[index].ulOffset = LE_SWAP32( &offset );
                     }
                  }

               ++nth_wave;
               }

            AIL_ptr_inc_clone(c3, LE_SWAP32( &c3->ckSize ) + 8 + (LE_SWAP32( &c3->ckSize ) & 1));
            }

         AIL_ptr_free_clone(c3);
         AIL_ptr_free_clone(n2);
         }

      AIL_ptr_inc_clone(c2, LE_SWAP32( &c2->ckSize ) + 8 + (LE_SWAP32( &c2->ckSize ) & 1));
      }

   AIL_ptr_free_clone(c2);
   AIL_ptr_free_clone(c1);

   //
   // Free pool table and cue list selectors and return success
   //

   AIL_ptr_free_clone(ptbl);
   AIL_ptr_free_clone(cues);

   return 1;
}

// ---------------------------------------------------------------------------
// AIL_extract_DLS
// ---------------------------------------------------------------------------

extern "C" S32 AILCALL AIL_API_extract_DLS(void const FAR       *source_image, //)
                                  U32             source_size,
                                  void FAR * FAR *XMI_output_data,
                                  U32  FAR       *XMI_output_size,
                                  void FAR * FAR *DLS_output_data,
                                  U32  FAR       *DLS_output_size,
                                  AILLENGTHYCB      callback)
{
   DOLOCK();

   //
   // Invalidate both output fields
   //

   if (XMI_output_data) *XMI_output_data = NULL;
   if (DLS_output_data) *DLS_output_data = NULL;
   if (XMI_output_size) *XMI_output_size = NULL;
   if (DLS_output_size) *DLS_output_size = NULL;

   U8 FAR *XMI_start = NULL;

   U8 FAR *XLS_start = NULL;
   S32     XLS_size  = 0;

   U8 FAR *XDLS_start = NULL;

   //
   // If this file's first four characters are "FORM" or "CAT " (i.e., it's
   // an EA IFF file), search for FORMs in incoming file
   //

   S32 is_IFF = (!AIL_strnicmp((char FAR*)source_image,"FORM",4)) ||
                (!AIL_strnicmp((char FAR*)source_image,"CAT ",4));

   if (is_IFF)
      {
      //
      // Locate optional FORM/CAT XMID/XDIR in incoming IFF file
      //

      XMI_start = XMI_find_FORM((U8 FAR *) source_image,
                                           source_size,
                                          "XDIR");

      if (XMI_start == NULL)
         {
         XMI_start = XMI_find_FORM((U8 FAR *) source_image,
                                              source_size,
                                             "XMID");
         }

      //
      // Locate optional FORM XDLS block in incoming IFF file
      //

      XDLS_start = XMI_find_FORM((U8 FAR *) source_image,
                                            source_size,
                                           "XDLS");
      if (XDLS_start != NULL)
         {
         XLS_start = (U8 FAR *) AIL_ptr_add(XDLS_start, 12);
         XLS_size  = BE_SWAP32( ((U8 FAR *) (XDLS_start+4)));
         }
      }
   else
      {
      //
      // This is not an IFF file -- set up to see if it's a RIFF MLS or
      // RIFF DLS file
      //

      XLS_start = (U8 FAR *) source_image;
      XLS_size  = source_size;
      }

   if (callback)
     if (callback(AIL_LENGTHY_INIT,0)==0) {
       AIL_set_error("Decompression cancelled.");
       return(0);
     }

   //
   // Return unpacked DLS data to application
   //
   // If this DLS file is compressed (type MLS with 4:1 ADPCM), reallocate
   // its memory block conservatively and decompress it
   //

   U8 FAR *DLS_start = NULL;
   S32     DLS_size  = 0;

   if (XLS_start != NULL)
      {
      CHUNK FAR *c1 = (CHUNK FAR *) XLS_start;

      if (c1->subID == FOURCC_MLS)
      {
         S32 new_size = get_expanded_DLS_size(XLS_start, XLS_size);

         c1->subID = FOURCC_DLS;

         if (DLS_output_data)
         {
           DLS_start = (U8 FAR *) AIL_mem_alloc_lock(new_size);
           DLS_size  = XLS_size;

           if (DLS_start == NULL)
           {
             return(0);
           }

           AIL_memcpy(DLS_start, XLS_start, DLS_size);

           expand(DLS_start, &DLS_size,callback);

           *DLS_output_data = DLS_start;
         }

         if (DLS_output_size) *DLS_output_size = DLS_size;
      }
      else if (c1->subID == FOURCC_DLS)
      {
         if (DLS_output_data)
         {
           void FAR *DLS = AIL_mem_alloc_lock(XLS_size);

           if (DLS == NULL)
           {
             return(0);
           }

           AIL_memcpy(DLS, XLS_start, XLS_size);

           *DLS_output_data = DLS;
         }

         if (DLS_output_size) *DLS_output_size = XLS_size;
      }
    } else {
      AIL_set_error("Unknown file type.");
      return(0);
    }

   //
   // Return XMIDI data to application
   //

   if (XMI_start != NULL)
   {
      //
      // Return XMIDI data to application
      //

      S32 XMI_size;

      if (XDLS_start != NULL)
         {
         XMI_size = AIL_ptr_dif(XDLS_start, XMI_start);
         }
      else
         {
         XMI_size = source_size;
         }

      if (XMI_output_data)
      {
        void FAR *XMI = AIL_mem_alloc_lock(XMI_size);

        if (XMI == NULL)
        {
          return(0);
        }

        AIL_memcpy(XMI, XMI_start, XMI_size);

        *XMI_output_data = XMI;
      }

      if (XMI_output_size) *XMI_output_size = XMI_size;
   }

   if (callback)
     callback(AIL_LENGTHY_DONE,0);

   return(1);
}


extern "C" S32 AILCALL AIL_API_file_type(void const FAR* data, U32 size)
{
  DOLOCK();

  AILSOUNDINFO si;

  if ((data==0) || (size<8))
    return(AILFILETYPE_UNKNOWN);

  if (AIL_WAV_info(data,&si))
  {
    switch (si.format)
    {
      case 1:
        return(AILFILETYPE_PCM_WAV);
      case WAVE_FORMAT_IMA_ADPCM:
        if (si.bits==4)
          return(AILFILETYPE_ADPCM_WAV);
        else
          return(AILFILETYPE_OTHER_WAV);
      case 85:  //mpeg wrapped in a WAV
        data=si.data_ptr;
        size=si.data_len;
        break;
      default:
        //
        // see if we have an ASI that can read it...
        //
        return(
                ( RIB_find_file_dec_provider( "ASI codec",
                                              "Input wave tag",
                                              si.format,
                                              "Output file types",
                                              ".raw" )) ?
                AILFILETYPE_OTHER_ASI_WAV:
                AILFILETYPE_OTHER_WAV
              );
    }
  }

  if (!AIL_strnicmp((char*)(((VOC FAR *) data)->ID_string),"Creative",8))
    return( AILFILETYPE_VOC );

  if ((!AIL_strnicmp((char FAR*) (data)  , "FORM", 4)) &&
      (!AIL_strnicmp((char FAR*) (data)+8, "XDIR", 4))) {

    //figure out type of XMIDI

    U8 FAR* XDLS_start = XMI_find_FORM((U8 FAR *) data,
                                                  size,
                                                  "XDLS");
    if (XDLS_start != NULL)
       {
       XDLS_start = (U8 FAR *) AIL_ptr_add(XDLS_start, 12);

       if (((CHUNK FAR*)XDLS_start)->subID == FOURCC_DLS)
         return(AILFILETYPE_XMIDI_DLS);

       if (((CHUNK FAR*)XDLS_start)->subID == FOURCC_MLS)
         return(AILFILETYPE_XMIDI_MLS);

       }

    return(AILFILETYPE_XMIDI);
  }

  if (((CHUNK FAR*)data)->ckID == FOURCC_RIFF) {

    if (((CHUNK FAR*)data)->subID == FOURCC_DLS)
      return(AILFILETYPE_DLS);

    if (((CHUNK FAR*)data)->subID == FOURCC_MLS)
      return(AILFILETYPE_MLS);

  }

  //
  // We'll assume this is an MPEG file if the first word
  // contains an MPEG header (12 successive '1' bits) and the first DWORD is
  // not 0xffffffff
  //

  {
    U8 FAR* src=(U8 FAR*)data;
    U32 len;

    len = size;

    if ( ( src[ 0 ] == 0x49 ) && ( src[ 1 ] == 0x44 ) && ( src[ 2 ] == 0x33 ) &&
         ( src[ 3 ] < 0xff ) && ( src[ 4 ] < 0xff ) &&
         ( src[ 6 ] < 0x80 ) && ( src[ 7 ] < 0x80 ) && ( src[ 8 ] < 0x80 ) && ( src[ 9 ] < 0x80 ) )
    {
      U32 skip = 10 + ( (U32) src[ 9 ] ) | ( ( (U32) src[ 8 ] ) << 7 ) |
                    ( ( (U32) src[ 7 ] ) << 14 ) | ( ( (U32) src[ 6 ] ) << 21 );
      src+=skip;
      len-=skip;
    }

    S32 type=0;

    if (len>AIL_MAX_FILE_HEADER_SIZE)
      len=AIL_MAX_FILE_HEADER_SIZE;

    while (len-- >= 4L)
    {

      if ( ( LE_SWAP16( src) & 0xF0FF) == 0xF0FF)
      {
        if  ((LE_SWAP32( src ) != 0xFFFFFFFF) &&
           (((LE_SWAP32( src )>>18)&0x3f) != 0x3F))
        {
          switch ((LE_SWAP16( src)>>9)&3)
          {
            case 1:  //yes, 1 for layer 3
              return AILFILETYPE_MPEG_L3_AUDIO;
            case 2:
              type=AILFILETYPE_MPEG_L2_AUDIO;
              break;
            case 3:  //yes, 3 for layer 1
              type=AILFILETYPE_MPEG_L1_AUDIO;
              break;
          }
          src+=3; // skip this DWORD
        }
        else
        {
          src+=3; // skip this DWORD
        }
      }
      ++src;
    }

    if (type != 0)
      return( type );
  }

  //
  // Check for MIDI files by scanning entire file for 'MThd' header
  //
  // Note that this is VERY likely to falsely identify any long data
  // file (e.g., mummer.mp3) as a MIDI file, so it should be the very
  // last test performed before giving up
  //

  {
   U8 FAR*src;
   U32 len;

   src = (U8 FAR*)data;
   len = size;
   while (len-- >= 4L)
      {
      if (!AIL_strnicmp((char FAR*) src,"MThd",4))
         {
         return(AILFILETYPE_MIDI);
         }
      src=(U8 FAR*)AIL_ptr_add(src,1);
      }

   }

  return(AILFILETYPE_UNKNOWN);
}


extern "C" S32 AILCALL AIL_API_find_DLS       (void const FAR* data, U32 size,
                                       void FAR* FAR*xmi, U32 FAR* xmisize,
                                       void FAR* FAR*dls, U32 FAR* dlssize)
{
  DOLOCK();

   if (((CHUNK FAR*)data)->ckID == FOURCC_RIFF)
      {
      if ((((CHUNK FAR*)data)->subID == FOURCC_DLS) ||
          (((CHUNK FAR*)data)->subID == FOURCC_MLS))
         {
         if (dls)     *dls = (void FAR*)data;
         if (dlssize) *dlssize = LE_SWAP32( &((CHUNK FAR *) data)->ckSize) + 8;
         return 1;
         }
      }

  if ((!AIL_strnicmp((char FAR*) (data)  , "FORM", 4)) &&
      (!AIL_strnicmp((char FAR*) (data)+8, "XDIR", 4))) {

    //figure out type of XMIDI

    U8 FAR* XDLS_start = XMI_find_FORM((U8 FAR *) data,
                                                  size,
                                                  "XDLS");
    if (XDLS_start != NULL)
       {
       XDLS_start = (U8 FAR *) AIL_ptr_add(XDLS_start, 12);

       if (dls)
         *dls=XDLS_start;

       if (dlssize)
         *dlssize=LE_SWAP32( &(((CHUNK FAR*)XDLS_start)->ckSize ) )+8;

       } else {

         if (dls)
           *dls=0;

         if (dlssize)
           *dlssize=0;

       }

    if (xmi)
      *xmi=(void FAR*)data;

    if (xmisize) {
      CHUNK FAR* ch=(CHUNK FAR*)data;

      U32 size=0;
      U32 chsize=0;
      do {
        ch=(CHUNK FAR*)AIL_ptr_add(ch,chsize);
        chsize=BE_SWAP32( &ch->ckSize )+8;
        size+=chsize;
      } while (AIL_strnicmp((char FAR*)ch,"CAT ",4));

      *xmisize=size;
    }

    return(1);
  } else {
    AIL_set_error("Not an XMI file.");
    return(0);
  }
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

