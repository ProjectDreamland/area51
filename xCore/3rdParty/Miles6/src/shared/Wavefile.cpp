//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  WVFILE.C: Digital sound API module for digital sound file access      ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 9.0                  ##
//##                                                                        ##
//##  Version 1.00 of 15-Feb-95: Derived from WAILSFIL V1.00                ##
//##          1.10 of 11-May-97: Added IMA ADPCM support (Serge Plagnol)    ##
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
#include <string.h>

#include "mss.h"
#include "imssapi.h"

#ifndef IS_WIN32
#include <stdarg.h>
#endif

//
// Recognized file types
//

#define FTYP_VOC  0
#define FTYP_WAV  1
#define FTYP_ASI  2

//
// .WAV FACT chunk
//

typedef struct
{
   S8  FACT_string[4];
   U32 chunk_size;
   U32 samples;
}
FACT;

//
//
// .VOC terminator block
//

typedef struct
{
   U8 block_ID;
}
BLK_0;

//
// .VOC voice block
//

typedef struct
{
   U8 block_ID;
   U8 block_len[3];
   U8 time_constant;
   U8 pack_method;
}
BLK_1;

//
// .VOC continued voice block
//

typedef struct
{
   U8 block_ID;
   U8 block_len[3];
}
BLK_2;

//
// .VOC silence block
//

typedef struct
{
   U8  block_ID;
   U8  block_len[3];
   U16 pause_period;
   U8  time_constant;
}
BLK_3;

//
// .VOC marker block
//

typedef struct
{
   U8  block_ID;
   U8  block_len[3];
   S16 marker;
}
BLK_4;

//
// .VOC ASCIIZ comment block
//

typedef struct
{
   U8 block_ID;
   U8 block_len[3];
   S8 string;
}
BLK_5;

//
// .VOC repeat loop block
//

typedef struct
{
   U8  block_ID;
   U8  block_len[3];
   U16 repeat_count;
}
BLK_6;

//
// .VOC end-of-loop block
//

typedef struct
{
   U8 block_ID;
   U8 block_len[3];
}
BLK_7;

//
// .VOC extended attribute block
//
// (always followed by block 1)
//

typedef struct
{
   U8  block_ID;
   U8  block_len[3];
   U16 time_constant;
   U8  pack_method;
   U8  voice_mode;
}
BLK_8;

//
// .VOC extended voice block
//
// (replaces blocks 1 and 8)
//

typedef struct
{
   U8  block_ID;
   U8  block_len[3];
   U32 sample_rate;
   U8  bits_per_sample;
   U8  channels;
   U16 format;
   U8  reserved[4];
}
BLK_9;

//
// .WAV file headers
//

typedef struct
{
   S8  RIFF_string[4];
   U32 chunk_size;
   S8  ID_string[4];
   U8  data[1];
}
RIFF;

//
// .WAV PCM file format chunk
//

typedef struct
{
   S8   FMT_string[4];
   U32  chunk_size;

   S16  format_tag;
   S16  channels;
   S32  sample_rate;
   S32  average_data_rate;
   S16  alignment;
   S16  bits_per_sample;
   S16 extra;
   S16 samples_per_block;
}
FMT;

//
// .WAV file data chunk
//

typedef struct
{
   S8  DATA_string[4];
   U32 chunk_size;
   U8  data[1];
}
DATA;

extern C8 AIL_error[256];

#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## Locked code                                                            ##
//##                                                                        ##
//############################################################################

#define LOCK(x)   AIL_vmm_lock  (&(x),sizeof(x))
#define UNLOCK(x) AIL_vmm_unlock(&(x),sizeof(x))

static S32 locked = 0;

void AILSFILE_end(void);

void AILSFILE_start(void)
{
   if (!locked)
      {
      AIL_vmm_lock_range(AILSFILE_start, AILSFILE_end);

      locked = 1;
      }
}

#define DOLOCK() AILSFILE_start()

#else

#define DOLOCK()

#endif


//############################################################################
//##                                                                        ##
//## Get length of .VOC block                                               ##
//##                                                                        ##
//############################################################################

static U32 AIL_VOC_block_len(void FAR *block)
{
   return (*(U32 FAR *) block) >> 8;
}

//############################################################################
//##                                                                        ##
//## Terminate playback of .VOC file                                        ##
//##                                                                        ##
//## Invoke application callback function, if any, and release the sample   ##
//## allocated to play this file                                            ##
//##                                                                        ##
//############################################################################

static void AIL_VOC_terminate(HSAMPLE sample)
{
   if (sample->system_data[SSD_EOD_CALLBACK] != (U32) NULL)
      {
      MSS_do_cb1( (AILSAMPLECB),
        (AILSAMPLECB)sample->system_data[SSD_EOD_CALLBACK], sample->driver->callingDS,sample->system_data[SSD_EOD_CB_WIN32S],
          sample);
      }

   if (sample->system_data[SSD_RELEASE] > 0)
      {
      AIL_release_sample_handle(sample);
      }

   sample->system_data[SSD_RELEASE] = -1;
}

//############################################################################
//##                                                                        ##
//## Process .VOC file block                                                ##
//##                                                                        ##
//## Called by .VOC initialization code and as end-of-sample callback       ##
//## function (interrupt-based)                                             ##
//##                                                                        ##
//## If play_flag clear, search for first block after desired marker (if    ##
//## any) and return without playing it                                     ##
//##                                                                        ##
//############################################################################

static void AIL_process_VOC_block(HSAMPLE sample, S32 play_flag)
{
   S32  voice_block;
   void FAR *b;

   voice_block = 0;

   //
   // Loop until voice block is found
   //

   while (!voice_block)
      {
      b = (void FAR *) sample->system_data[VOC_BLK_PTR];

      switch (*(U8 FAR *) b)
         {
         //
         // Terminator block
         //

         case 0:

            //
            // Terminate playback, then return without trying to advance
            // to next block
            //

            AIL_VOC_terminate(sample);

            return;

         //
         // Voice block
         //

         case 1:

            //
            // Skip block if desired marker has not been found
            //

            if (!sample->system_data[VOC_MARKER_FOUND])
               {
               break;
               }

            //
            // Set up sample data and start playback
            //

            AIL_set_sample_address(sample,
                                   AIL_ptr_add(b, sizeof(BLK_1)),
                                   AIL_VOC_block_len(b) - 2);

            AIL_set_sample_playback_rate(sample,(U32)
                             1000000L / (256 - ((BLK_1 FAR *) b)->time_constant));

            AIL_set_sample_type(sample,DIG_F_MONO_8,0);

            if (play_flag)
               AIL_start_sample(sample);

            voice_block = 1;
            break;

         //
         // Marker block
         //

         case 4:

            //
            // Ignore if entire file to be played
            //

            if (sample->system_data[VOC_MARKER] == -1)
               {
               break;
               }

            //
            // If this is the desired marker, set MARKER_FOUND flag --
            // otherwise, clear MARKER_FOUND flag to prevent playback
            // of future voice blocks
            //

            if (sample->system_data[VOC_MARKER] == (S32)
                                                   ((BLK_4 FAR *) b)->marker)
               {
               sample->system_data[VOC_MARKER_FOUND] = 1;
               }
            else
               {
               sample->system_data[VOC_MARKER_FOUND] = 0;
               }

            break;

         //
         // Repeat block
         //

         case 6:

            //
            // Log repeat count and starting address of repeat block
            //

            sample->system_data[VOC_REP_BLK] = (U32) b;

            sample->system_data[VOC_N_REPS]  = (U32)
                                             ((BLK_6 FAR *) b)->repeat_count;
            break;

         //
         // End repeat block
         //

         case 7:

            //
            // If finite repeat block active, check and decrement repeat
            // count
            //

            if (sample->system_data[VOC_N_REPS] != 0xffff)
               {
               if (sample->system_data[VOC_N_REPS]-- == 0)
                  {
                  break;
                  }
               }

            b = (void FAR *) sample->system_data[VOC_REP_BLK];
            break;

         //
         // Extended attribute block
         // (followed by block 1)
         //

         case 8:

            //
            // Skip block if desired marker has not been found
            //

            if (!sample->system_data[VOC_MARKER_FOUND])
               {
               break;
               }

            //
            // Set up sample data and start playback
            //

            if (((BLK_8 FAR *) b)->voice_mode)
               {
               AIL_set_sample_type(sample,DIG_F_STEREO_8,0);

               AIL_set_sample_playback_rate(sample,(U32)
                  128000000L / (65536L - ((BLK_8 FAR *) b)->time_constant));
               }
            else
               {
               AIL_set_sample_type(sample,DIG_F_MONO_8,0);

               AIL_set_sample_playback_rate(sample,(U32)
                  256000000L / (65536L - ((BLK_8 FAR *) b)->time_constant));
               }

            //
            // Advance to paired voice block (type 1) in .VOC image
            //

            b = (C8 FAR *)AIL_ptr_add(b, AIL_VOC_block_len(b) + 4);

            //
            // Set sample address and size, and start playback
            //

            AIL_set_sample_address(sample,
                                   AIL_ptr_add(b, sizeof(BLK_1)),
                                   AIL_VOC_block_len(b) - 2);

            if (play_flag)
               AIL_start_sample(sample);

            voice_block = 1;
            break;

         //
         // Extended voice block
         //

         case 9:

            //
            // Skip block if desired marker has not been found
            //

            if (!sample->system_data[VOC_MARKER_FOUND])
               {
               break;
               }

            //
            // Set up sample data and start playback
            //

            AIL_set_sample_address(sample,
                                   AIL_ptr_add(b, sizeof(BLK_9)),
                                   AIL_VOC_block_len(b) - 12);

            AIL_set_sample_playback_rate(sample, ((BLK_9 FAR *) b)->sample_rate);

            if ((((BLK_9 FAR *) b)->channels == 1) &&
                (((BLK_9 FAR *) b)->format   == 0))
               {
               AIL_set_sample_type(sample,DIG_F_MONO_8,0);
               }
            else if ((((BLK_9 FAR *) b)->channels == 2) &&
                     (((BLK_9 FAR *) b)->format   == 0))
               {
               AIL_set_sample_type(sample,DIG_F_STEREO_8,0);
               }
            else if ((((BLK_9 FAR *) b)->channels == 1) &&
                     (((BLK_9 FAR *) b)->format   == 4))
               {
               AIL_set_sample_type(sample,DIG_F_MONO_16,DIG_PCM_SIGN);
               }
            else if ((((BLK_9 FAR *) b)->channels == 2) &&
                     (((BLK_9 FAR *) b)->format   == 4))
               {
               AIL_set_sample_type(sample,DIG_F_STEREO_16,DIG_PCM_SIGN);
               }

            if (play_flag)
               AIL_start_sample(sample);

            voice_block = 1;
            break;
         }

      //
      // Advance pointer to next block in .VOC image
      //

      sample->system_data[VOC_BLK_PTR] = (U32)
                                         AIL_ptr_add(b, AIL_VOC_block_len(b) + 4);
      }
}

//############################################################################
//##                                                                        ##
//## End-of-sample callback handler for .VOC file playback                  ##
//##                                                                        ##
//############################################################################

static void AILLIBCALLBACK AIL_VOC_EOS(HSAMPLE sample)
{
   AIL_process_VOC_block(sample,1);
}


//############################################################################
//##                                                                        ##
//## Create sample instance by parsing .WAV file                            ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_WAV_info(void const FAR* data, AILSOUNDINFO FAR* info)
{
   void  FAR *end;
   FMT   FAR *f;
   DATA  FAR *d;
   FACT  FAR *fa;

   DOLOCK();

   if (data==0)
     goto error;

   end=AIL_ptr_add(data,LE_SWAP32(&((RIFF FAR *) data)->chunk_size));

   if (AIL_strnicmp((char FAR*)(((RIFF FAR *) data)->ID_string),"WAVE",4))
      {
     error:
      AIL_strcpy(AIL_error,"Not a WAV file.");
      return(0);
      }

   //
   // Find mandatory <fmt-ck>
   //

   f = (FMT FAR *) (((RIFF *) data)->data);

   while (AIL_strnicmp((char FAR*)f->FMT_string,"fmt ",4))
      {
      U32 chunk_size = LE_SWAP32( &f->chunk_size );
      f = (FMT FAR *) AIL_ptr_add(f, chunk_size + 8 + (chunk_size & 1));
      if (AIL_ptr_dif(f,end)>=0)
        goto error;
      }

   info->format= LE_SWAP16( &f->format_tag );
   info->rate= LE_SWAP32( &f->sample_rate );
   info->bits= LE_SWAP16( &f->bits_per_sample );
   info->channels= LE_SWAP16( &f->channels );
   info->block_size= LE_SWAP16( &f->alignment );

   //
   // Find mandatory <data-ck>
   //

   d = (DATA FAR *) (((RIFF *) data)->data);

   while (AIL_strnicmp((char FAR*)d->DATA_string,"data",4))
      {
      U32 chunk_size = LE_SWAP32( &d->chunk_size );
      d = (DATA FAR *) AIL_ptr_add( d, chunk_size + 8 + (chunk_size & 1));
      if (AIL_ptr_dif(d,end)>=0)
        goto error;
      }

   info->data_ptr=d->data;
   info->data_len= LE_SWAP32( &d->chunk_size );

   if ((info->format==WAVE_FORMAT_IMA_ADPCM) && (info->bits==4))
   {

     fa = (FACT FAR *) (((RIFF *) data)->data);

     while (AIL_strnicmp((char FAR*)fa->FACT_string,"fact",4))
        {
        U32 chunk_size = LE_SWAP32( &fa->chunk_size );
        fa = (FACT FAR *) AIL_ptr_add( fa, chunk_size + 8 + (chunk_size & 1));
        if (AIL_ptr_dif(fa,end)>=0)
          {

          U32 samples_per_block = 4 << (info->channels/2);
          samples_per_block = 1 + (info->block_size-samples_per_block)*8 / samples_per_block;
          info->samples=((info->data_len+info->block_size-1)/info->block_size)*samples_per_block;

          goto nofact;
          }
       }
       info->samples= LE_SWAP32( &fa->samples );
     nofact:;
   } else
     info->samples=(info->bits==0)?0:((info->data_len*8)/info->bits);

   info->initial_ptr=info->data_ptr;

   return(1);
}


//############################################################################
//##                                                                        ##
//## Create sample instance by parsing .WAV file                            ##
//##                                                                        ##
//############################################################################

static void AIL_process_WAV_image(AILSOUNDINFO FAR * info, HSAMPLE sample)
{
  S32 format=0;
  S32 sign=0;

  if (info->channels == 2)
    {
   	format = DIG_F_STEREO_MASK ;
    }

  switch(info->bits)
    {
    case 4 :
    	format |= DIG_F_ADPCM_MASK ;
    case 16:
    	format |= DIG_F_16BITS_MASK ;
    	sign   = DIG_PCM_SIGN ;
    }

   AIL_set_sample_type(sample, format, sign);

   AIL_set_sample_playback_rate(sample,info->rate);

   //
   // Configure adpcm if required
   //

   if (format&DIG_F_ADPCM_MASK)
      {
      AIL_set_sample_adpcm_block_size(sample, info->block_size);
      }

   //
   // Configure sample address and length based on DATA chunk
   //

   AIL_set_sample_address(sample,info->data_ptr,info->data_len);
}

//############################################################################
//##                                                                        ##
//## End-of-sample callback handler for .WAV file playback                  ##
//##                                                                        ##
//############################################################################

static void AILLIBCALLBACK AIL_WAV_EOS(HSAMPLE sample)
{
   if (sample->system_data[SSD_EOD_CALLBACK] != (U32) NULL)
      {
      MSS_do_cb1( (AILSAMPLECB),
        (AILSAMPLECB)sample->system_data[SSD_EOD_CALLBACK], sample->driver->callingDS,sample->system_data[SSD_EOD_CB_WIN32S],
          sample);
      }

   if (sample->system_data[SSD_RELEASE] > 0)
      {
      AIL_release_sample_handle(sample);
      }

   sample->system_data[SSD_RELEASE] = -1;
}

//############################################################################
//##                                                                        ##
//## Play memory-resident file image                                        ##
//##                                                                        ##
//## Returns NULL on error, else handle to sample assigned to file          ##
//##                                                                        ##
//############################################################################

HSAMPLE AILCALL AIL_API_allocate_file_sample(HDIGDRIVER dig, void const FAR *file_image, S32 block)
{
   HSAMPLE sample;
   AILSOUNDINFO info;
   S32    type;
   char FAR * asifileext;

   DOLOCK();

   if (file_image==NULL)
     return(0);

   //
   // Identify file type
   //
   // Note: Currently only single-sample PCM .WAV files are supported, since
   // no known applications generate other formats for testing
   //

   if (!AIL_strnicmp((char FAR*)(((VOC FAR *) file_image)->ID_string),"Creative",8))
      {
      type = FTYP_VOC;
      }
   else if (AIL_WAV_info(file_image,&info))
      {
      if ((info.format==WAVE_FORMAT_PCM) || ((info.format==WAVE_FORMAT_IMA_ADPCM) && (info.bits==4)))
        type = FTYP_WAV;
      else
      {

        // handle asi wave files

        HPROVIDER ASI = RIB_find_file_dec_provider( "ASI codec",
                                                    "Input wave tag",
                                                    info.format,
                                                    "Output file types",
                                                    ".raw" );

        if (ASI==0)
        {
           asierr:
            AIL_set_error("Unsupported wave file format.");
            return NULL;
        }

        type = FTYP_ASI;

        PROVIDER_QUERY_ATTRIBUTE query_attribute = NULL;
        U32 token=0;

        RIB_request_interface_entry(ASI,
                                    "ASI codec",
                                    RIB_FUNCTION,
                                   "PROVIDER_query_attribute",
                        (U32 FAR *) &query_attribute);

        if (RIB_request_interface_entry(ASI,
                                        "ASI codec",
                                        RIB_ATTRIBUTE,
                                        "Input file types",
                                         &token) == RIB_NOERR)
        {
          asifileext = (char FAR*)query_attribute(token);

          //
          // try to find the extension
          //

          if (asifileext)
          {
            while ((asifileext[0]!=0) || (asifileext[1]!=0))
            {
              if ((asifileext[0]=='*') && (asifileext[1]=='.'))
                break;
              ++asifileext;
            }
          }

        }
        else
        {
          goto asierr;
        }

     }
   }
   else
      {
      AIL_set_error("Unknown file type.");
      return NULL;
      }

   //
   // Allocate and initialize sample for file
   //

   sample = AIL_allocate_sample_handle(dig);

   if (sample == NULL)
      {
      return NULL;
      }

   AIL_init_sample(sample);

   sample->system_data[SSD_EOD_CALLBACK] = (U32) NULL;

   //
   // Copy file attributes to sample
   //

   switch (type)
      {
      case FTYP_VOC:

         sample->system_data[VOC_BLK_PTR]      = (U32) AIL_ptr_add(file_image,
                                                 ((VOC FAR *) file_image)->
                                                 data_offset);

         sample->system_data[VOC_MARKER]       = block;
         sample->system_data[VOC_MARKER_FOUND] = (block == -1);
         sample->system_data[SSD_RELEASE]      = 1;

         AIL_register_EOS_callback(sample, AIL_VOC_EOS);

         AIL_process_VOC_block(sample,0);
         break;

      case FTYP_WAV:

         sample->system_data[SSD_RELEASE] = 1;

         AIL_register_EOS_callback(sample, AIL_WAV_EOS);

         AIL_process_WAV_image(&info,sample);
         break;

      case FTYP_ASI:
         sample->system_data[SSD_RELEASE] = 1;

         AIL_register_EOS_callback(sample, AIL_WAV_EOS);

         AIL_set_named_sample_file(sample,asifileext,info.data_ptr,info.data_len,block);
         break;

      }

   //
   // Return NULL if parser rejected sample file image, or handle if OK
   //

   if (sample->system_data[SSD_RELEASE] == -1)
      {
      AIL_set_error("Invalid or missing data block.");
      return NULL;
      }

   return sample;
}

//############################################################################
//##                                                                        ##
//## Set parameters of existing HSAMPLE according to file data              ##
//##                                                                        ##
//## Returns 0 on error, else 1                                             ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_set_sample_file(HSAMPLE S, void const FAR *file_image, S32 block)
{
   S32 type;
   AILSOUNDINFO info;

   DOLOCK();

   if ((S==NULL) || (file_image==NULL))
     return(0);

   //
   // Identify file type
   //
   //

   if (!AIL_strnicmp((char FAR*)(((VOC FAR *) file_image)->ID_string),"Creative",8))
      {
      type = FTYP_VOC;
      }
   else if (AIL_WAV_info(file_image,&info))
      {
      if ((info.format==WAVE_FORMAT_PCM) || (info.format==WAVE_FORMAT_IMA_ADPCM))
        type = FTYP_WAV;
      else
        {

        // handle asi wave files

        HPROVIDER ASI = RIB_find_file_dec_provider( "ASI codec",
                                                    "Input wave tag",
                                                    info.format,
                                                    "Output file types",
                                                    ".raw" );

        if (ASI)
        {
          PROVIDER_QUERY_ATTRIBUTE query_attribute = NULL;
          U32 token=0;

          RIB_request_interface_entry(ASI,
                                      "ASI codec",
                                      RIB_FUNCTION,
                                     "PROVIDER_query_attribute",
                          (U32 FAR *) &query_attribute);

          if (RIB_request_interface_entry(ASI,
                                          "ASI codec",
                                          RIB_ATTRIBUTE,
                                          "Input file types",
                                           &token) == RIB_NOERR)
          {
            char FAR* asifileext=0;
            asifileext = (char FAR*)query_attribute(token);

            //
            // try to find the extension
            //

            if (asifileext)
            {
              while ((asifileext[0]!=0) || (asifileext[1]!=0))
              {
                if ((asifileext[0]=='*') && (asifileext[1]=='.'))
                  break;
                ++asifileext;
              }
            }

            return( AIL_set_named_sample_file(S,asifileext,info.data_ptr,info.data_len,block));
          }
        }

        AIL_set_error("Unsupported wave file format.");
        return 0;
        }
      }
   else
      {
      AIL_set_error("Unknown file type.");
      return 0;
      }


   // reset default filter and ADPCM values
   S->left_val       =  0;
   S->right_val      =  0;
   S->src_fract      =  0;
   S->adpcm.blocksize=  256;
   S->adpcm.blockleft = 0;
   S->adpcm.extrasamples = 0;


   //
   // Copy file attributes to sample
   //

   switch (type)
      {
      case FTYP_VOC:

         S->system_data[VOC_BLK_PTR]      = (U32) AIL_ptr_add(file_image,
                                                 ((VOC FAR *) file_image)->
                                                 data_offset);

         S->system_data[VOC_MARKER]       = block;
         S->system_data[VOC_MARKER_FOUND] = (block == -1);

         S->system_data[SSD_RELEASE]      = 0;

         AIL_process_VOC_block(S,0);
         break;

      case FTYP_WAV:

         S->system_data[SSD_RELEASE] = 0;

         AIL_process_WAV_image(&info,S);
         break;
      }

   //
   // Return NULL if parser rejected sample file image, or 1 if OK
   //

   if (S->system_data[SSD_RELEASE] == -1)
      {
      AIL_set_error("Invalid or missing data block.");
      return 0;
      }

   return 1;
}

//############################################################################
//##                                                                        ##
//## Set parameters of existing HSAMPLE according to file data, using       ##
//## the file suffix to specify the file type                               ##
//##                                                                        ##
//## Returns 0 on error, else 1                                             ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_set_named_sample_file (HSAMPLE   S, //)
                                           C8   const FAR *file_type_suffix,
                                           void const FAR *file_image,
                                           S32       file_size,
                                           S32       block)
{
   DOLOCK();

   //
   // is this a wacky id3?
   //

   if ( ( ((U8 const FAR* )file_image)[ 0 ] == 0x49 ) && ( ((U8 const FAR* )file_image)[ 1 ] == 0x44 ) && ( ((U8 const FAR* )file_image)[ 2 ] == 0x33 ) &&
        ( ((U8 const FAR* )file_image)[ 3 ] < 0xff ) && ( ((U8 const FAR* )file_image)[ 4 ] < 0xff ) &&
        ( ((U8 const FAR* )file_image)[ 6 ] < 0x80 ) && ( ((U8 const FAR* )file_image)[ 7 ] < 0x80 ) && ( ((U8 const FAR* )file_image)[ 8 ] < 0x80 ) && ( ((U8 const FAR* )file_image)[ 9 ] < 0x80 ) )
   {
     U32 skip = 10 + ( (U32) ((U8 const FAR* )file_image)[ 9 ] ) | ( ( (U32) ((U8 const FAR* )file_image)[ 8 ] ) << 7 ) |
                   ( ( (U32) ((U8 const FAR* )file_image)[ 7 ] ) << 14 ) | ( ( (U32) ((U8 const FAR* )file_image)[ 6 ] ) << 21 );

     file_image = ( (U8 FAR*) file_image ) + skip;
     file_size -= skip;
     file_type_suffix = ".mp3";
   }

   //
   // If file type not specified, pass to .WAV / VOC handler
   //

   if ((file_type_suffix == NULL) ||
       (!AIL_strlen(file_type_suffix)))
      {
      return AIL_set_sample_file(S,
                                 file_image,
                                 block);
      }

   //
   // Search for ASI codec capable of processing this input file type
   //

   HPROVIDER HP = RIB_find_file_provider("ASI codec",
                                         "Input file types",
                                         file_type_suffix);

   //
   // If no specific provider available for this file type, try the default
   // .WAV/.VOC file handler
   // 

   if (!HP)
      {
      if (AIL_strlen(file_type_suffix) >= 4)
         {
         if ((!AIL_stricmp(&file_type_suffix[AIL_strlen(file_type_suffix)-4],
                        ".WAV"))
               ||
            (!AIL_stricmp(&file_type_suffix[AIL_strlen(file_type_suffix)-4],
                        ".VOC")))
            {
            return AIL_set_sample_file(S,
                                       file_image,
                                       block);
            }
         }

      //
      // Not a .WAV or .VOC file, fail call
      //

      return 0;
      }

   // reset default filter values
   S->left_val       =  0;
   S->right_val      =  0;
   S->src_fract      =  0;


   //
   // Otherwise, set sample address and size for encoded data image
   //

   AIL_set_sample_address(S,
                          file_image,
                          file_size);

   //
   // Set up to use specified ASI codec to decode data for mixer
   //

   AIL_set_sample_processor(S,
                            DP_ASI_DECODER,
                            HP);

   //
   // Return success
   //

   return 1;
}

//############################################################################
//##                                                                        ##
//## Set a RIB provider for a specified sample pipeline phase               ##
//##                                                                        ##
//############################################################################

HPROVIDER    AILCALL AIL_API_set_sample_processor  (HSAMPLE     S, //)
                                                    SAMPLESTAGE pipeline_stage,
                                                    HPROVIDER   provider)
{
   DOLOCK();

   S32 stage = pipeline_stage;

   if (stage == SAMPLE_ALL_STAGES)
      {
      stage = 0;
      }

   DPINFO FAR *DP;

   do
      {
      DP = &S->pipeline[stage];

      //
      // If existing stage is being replaced, shut it down first
      //

      if (DP->active)
         {
         switch (stage)
            {
            //
            // "MSS mixer services"
            //

            case DP_MERGE:
               break;

            //
            // "ASI codec stream"
            //

            case DP_ASI_DECODER:

               OutMilesMutex();
               DP->TYPE.ASI.ASI_stream_close(DP->TYPE.ASI.stream);
               InMilesMutex();
               break;

            //
            // "MSS pipeline filter"
            //

            case DP_FILTER:

               #if defined(IS_WIN32) || defined(IS_MAC)
               OutMilesMutex();
               DP->TYPE.FLT.provider->close_sample(DP->TYPE.FLT.sample_state);
               InMilesMutex();
               DP->TYPE.FLT.sample_state=0;
               #endif
               break;
            }

         DP->active = 0;
         }
      }
   while ((pipeline_stage == SAMPLE_ALL_STAGES) && (++stage < N_SAMPLE_STAGES));

   //
   // Set up to return previous provider
   //

   HPROVIDER previous = DP->provider;

   DP->provider = provider;

   //
   // If no new provider specified, exit normally after shutting down
   // existing provider
   //

   if (provider == NULL)
      {
      return previous;
      }

   //
   // Query required attributes from provider interface
   //

   switch (stage)
      {
      //
      // "MSS mixer services"
      //

      case DP_MERGE:
         {
         //
         // Get codec properties and store in pipeline-stage data structure
         //

         MIXSTAGE FAR *MIX = &DP->TYPE.MIX;

         RIB_INTERFACE_ENTRY MIXER[] =
            {
            { RIB_FUNCTION, "MIXER_merge", (U32) &MIX->MSS_mixer_merge, RIB_NONE },
            };

         RIB_request(DP->provider,"MSS mixer services",MIXER);

         //
         // Indicate installation of mixer module
         //

         DP->active = 1;
         break;
         }

      //
      // "MSS pipeline filter"
      //

      case DP_FILTER:
         {

         #if defined(IS_WIN32) || defined(IS_MAC)

         //
         // open the filter if it ain't already opened
         //

         AIL_open_filter( provider, S->driver );

         //
         // Find DRIVERSTATE wrapper for this provider and driver
         //

         FLTPROVIDER FAR *F = FLT_find_provider_instance(provider, S->driver);

         DP->TYPE.FLT.provider = F;

         DP->TYPE.FLT.sample_state = 0;

         if ((F==NULL) || (F->open_sample==NULL))
           {
           return NULL;
           }

         //
         // Create SAMPLESTATE structure for this HSAMPLE and HDRIVERSTATE
         //

         DP->TYPE.FLT.sample_state = F->open_sample(F->driver_state,
                                                    S);

         if (!DP->TYPE.FLT.sample_state)
           {
           return NULL;
           }

         //
         // Indicate installation of filter module
         //

         DP->active = 1;

         #endif

         break;
         }

      //
      // "ASI codec stream"
      //

      case DP_ASI_DECODER:
         {
         //
         // Get codec properties and store in pipeline-stage data structure
         //

         ASISTAGE FAR *ASI = &DP->TYPE.ASI;

         RIB_INTERFACE_ENTRY ASISTR[] =
            {
            { RIB_FUNCTION,   "ASI_stream_attribute",      (U32) &ASI->ASI_stream_attribute,      RIB_NONE },
            { RIB_FUNCTION,   "ASI_stream_open",           (U32) &ASI->ASI_stream_open,           RIB_NONE },
            { RIB_FUNCTION,   "ASI_stream_seek",           (U32) &ASI->ASI_stream_seek,           RIB_NONE },
            { RIB_FUNCTION,   "ASI_stream_close",          (U32) &ASI->ASI_stream_close,          RIB_NONE },
            { RIB_FUNCTION,   "ASI_stream_process",        (U32) &ASI->ASI_stream_process,        RIB_NONE },
            { RIB_FUNCTION,   "ASI_stream_set_preference", (U32) &ASI->ASI_stream_set_preference, RIB_NONE },
            { RIB_ATTRIBUTE,  "Input bit rate",            (U32) &ASI->INPUT_BIT_RATE,            RIB_NONE },
            { RIB_ATTRIBUTE,  "Input sample rate",         (U32) &ASI->INPUT_SAMPLE_RATE,         RIB_NONE },
            { RIB_ATTRIBUTE,  "Input sample width",        (U32) &ASI->INPUT_BITS,                RIB_NONE },
            { RIB_ATTRIBUTE,  "Input channels",            (U32) &ASI->INPUT_CHANNELS,            RIB_NONE },
            { RIB_ATTRIBUTE,  "Output bit rate",           (U32) &ASI->OUTPUT_BIT_RATE,           RIB_NONE },
            { RIB_ATTRIBUTE,  "Output sample rate",        (U32) &ASI->OUTPUT_SAMPLE_RATE,        RIB_NONE },
            { RIB_ATTRIBUTE,  "Output sample width",       (U32) &ASI->OUTPUT_BITS,               RIB_NONE },
            { RIB_ATTRIBUTE,  "Output channels",           (U32) &ASI->OUTPUT_CHANNELS,           RIB_NONE },
            { RIB_ATTRIBUTE,  "Position",                  (U32) &ASI->POSITION,                  RIB_NONE },
            { RIB_ATTRIBUTE,  "Percent done",              (U32) &ASI->PERCENT_DONE,              RIB_NONE },
            { RIB_ATTRIBUTE,  "Minimum input block size",  (U32) &ASI->MIN_INPUT_BLOCK_SIZE,      RIB_NONE },
            { RIB_PREFERENCE, "Raw source sample rate",    (U32) &ASI->RAW_RATE,                  RIB_NONE },
            { RIB_PREFERENCE, "Raw source sample width",   (U32) &ASI->RAW_BITS,                  RIB_NONE },
            { RIB_PREFERENCE, "Raw source channels",       (U32) &ASI->RAW_CHANNELS,              RIB_NONE },
            { RIB_PREFERENCE, "Requested sample rate",     (U32) &ASI->REQUESTED_RATE,            RIB_NONE },
            { RIB_PREFERENCE, "Requested sample width",    (U32) &ASI->REQUESTED_BITS,            RIB_NONE },
            { RIB_PREFERENCE, "Requested channels",        (U32) &ASI->REQUESTED_CHANS,           RIB_NONE }
            };

         RIB_request(DP->provider,"ASI stream",ASISTR);

         //
         // Open stream with codec, registering callback function
         //

         OutMilesMutex();
         ASI->stream = ASI->ASI_stream_open((U32) S,
                                                  DP_ASI_DECODER_callback,
                                                  S->len[0]);
         InMilesMutex();

         // jkr - exit on error
         if (ASI->stream==0)
         {
           DP->provider=0;
           return(previous);
         }

         //
         // Request codec output format which matches hardware format
         //

         ASI->ASI_stream_set_preference(ASI->stream,
                                        ASI->REQUESTED_RATE,
                                        &S->driver->DMA_rate);

         U32 achans=1 + ((S->driver->hw_format & DIG_F_STEREO_MASK) != 0);

         ASI->ASI_stream_set_preference(ASI->stream,
                                        ASI->REQUESTED_CHANS,
                                        &achans);

         U32 abits=8 + (8 * ((S->driver->hw_format & DIG_F_16BITS_MASK) != 0));

         ASI->ASI_stream_set_preference(ASI->stream,
                                        ASI->REQUESTED_BITS,
                                        &abits);

         //
         // Configure sample type and rate according to codec's actual output
         // format
         //

         U32 nch  = ASI->ASI_stream_attribute(ASI->stream, ASI->OUTPUT_CHANNELS);
         U32 rate = ASI->ASI_stream_attribute(ASI->stream, ASI->OUTPUT_SAMPLE_RATE);
         U32 bits = ASI->ASI_stream_attribute(ASI->stream, ASI->OUTPUT_BITS);

         AIL_set_sample_playback_rate(S, rate);

         if (nch == 2)
            {
            AIL_set_sample_type(S, 
                               (bits == 16) ? DIG_F_STEREO_16 : DIG_F_STEREO_8,
                               (bits == 16) ? DIG_PCM_SIGN    : 0);
            }
         else
            {
            AIL_set_sample_type(S, 
                               (bits == 16) ? DIG_F_MONO_16   : DIG_F_MONO_8, 
                               (bits == 16) ? DIG_PCM_SIGN    : 0);
            }

#if 0
         AIL_debug_printf("SSP: %d channels, %d Hz, %d bits\n",nch,rate,bits);
#endif

         //
         // Indicate installation of decoder module
         //

         DP->active = 1;
         break;
         }
      }

   //
   // Return success
   //

   return previous;
}


//############################################################################
//##                                                                        ##
//## Set a RIB provider for a specified driver pipeline phase               ##
//##                                                                        ##
//############################################################################

HPROVIDER    AILCALL AIL_API_set_digital_driver_processor(HDIGDRIVER  dig, //)
                                                          DIGDRVSTAGE pipeline_stage,
                                                          HPROVIDER   provider)
{
   DOLOCK();

   S32 stage = pipeline_stage;

   if (stage == DIGDRV_ALL_STAGES)
      {
      stage = 0;
      }

   DPINFO FAR *DP;

   //
   // handle the "default" pipeline stages specially
   //

   if ((pipeline_stage==DP_DEFAULT_FILTER) || (pipeline_stage==DP_DEFAULT_FILTER))
      {
      HPROVIDER previous=dig->pipeline[pipeline_stage].provider;
      dig->pipeline[pipeline_stage].provider=provider;
      return(previous);
      }

   do
      {
      DP = &dig->pipeline[stage];

      //
      // If existing stage is being replaced, shut it down first
      //

      if (DP->active)
         {
         switch (stage)
            {
            //
            // "MSS mixer services"
            //

            case DP_FLUSH:
            case DP_COPY:
               break;
            }

         DP->active = 0;
         }
      }
   while ((pipeline_stage == DIGDRV_ALL_STAGES) && (++stage < N_DIGDRV_STAGES));

   //
   // If no new provider specified, exit normally after shutting down
   // existing provider
   //

   if (provider == NULL)
      {
      return NULL;
      }

   //
   // Set up to return previous provider
   //

   HPROVIDER previous = DP->provider;

   DP->provider = provider;

   //
   // Query required attributes from provider interface
   //

   switch (stage)
      {
      //
      // "MSS mixer services"
      //

      case DP_FLUSH:
         {
         //
         // Get codec properties and store in pipeline-stage data structure
         //

         MIXSTAGE FAR *MIX = &DP->TYPE.MIX;

         RIB_INTERFACE_ENTRY MIXER[] =
            {
            { RIB_FUNCTION, "MIXER_flush", (U32) &MIX->MSS_mixer_flush, RIB_NONE },
            };

         RIB_request(DP->provider,"MSS mixer services",MIXER);

         //
         // Indicate installation of mixer module
         //

         DP->active = 1;
         break;
         }

      case DP_COPY:
         {
         //
         // Get codec properties and store in pipeline-stage data structure
         //

         MIXSTAGE FAR *MIX = &DP->TYPE.MIX;

         RIB_INTERFACE_ENTRY MIXER[] =
            {
            { RIB_FUNCTION, "MIXER_copy",  (U32) &MIX->MSS_mixer_copy,  RIB_NONE }
            };

         RIB_request(DP->provider,"MSS mixer services",MIXER);

         //
         // Indicate installation of mixer module
         //

         DP->active = 1;
         break;
         }
      }

   //
   // Return success
   //

   return previous;
}

//############################################################################
//##                                                                        ##
//## Install end-of-file callback handler                                   ##
//##                                                                        ##
//############################################################################

AILSAMPLECB AILCALL AIL_API_register_EOF_callback(HSAMPLE S, AILSAMPLECB EOFILE)
{
   AILSAMPLECB old;

   DOLOCK();

   if (S == NULL)
      {
      return NULL;
      }

   old = (AILSAMPLECB) S->system_data[SSD_EOD_CALLBACK];

   #ifdef IS_WIN16
     CheckWin32sCB(S->system_data[SSD_EOD_CB_WIN32S]);
   #endif

   S->system_data[SSD_EOD_CALLBACK] = (U32) EOFILE;

   return old;
}

S32 AILCALL AIL_API_digital_CPU_percent(HDIGDRIVER dig)
{
  S32 time;
  U32 diff;

  if (dig==0)
    return(0);

  time=AIL_ms_count();

  diff=time-dig->last_ms_polled;
  if (diff<150)
    return(dig->last_percent);

  dig->last_ms_polled=time;

  dig->ms_count+=(dig->us_count/1000);
  dig->us_count=dig->us_count%1000;

  time=(diff)?((dig->ms_count*100)/diff):0;
  dig->ms_count=0;

  dig->last_percent=time;

  if (time>100)
    time=100;

  return(time);
}

#define min(a,b)  (((a) < (b)) ? (a) : (b))

static S32 get_operation(U32 dest_rate,U32 dest_format,U32 src_rate,U32 src_bits, U32 src_chans)
{
  //
  // Set mixer operation code
  //

  S32 op = 0;

  if (dest_format & DIG_F_STEREO_MASK)
     {
     op |= M_DEST_STEREO;
     }

  if (src_chans==2)
     {
     op |= M_SRC_STEREO;
     }

  if (src_bits!=8)
     {
     op |= M_SRC_16;
     }

  //
  // Set M_SRC_SCALE
  //
  // Force M_SRC_SCALE if stereo source mixed into mono output
  //
  // (This routine assumes no volume scaling in use)
  //

  if ((op & M_SRC_STEREO) && (!(op & M_DEST_STEREO)))
     {
     op |= M_VOL_SCALING;
     }

  if (src_rate != dest_rate)
     {
     op |= M_RESAMPLE;
     }

  //
  // Enable filtering if preference set
  //

  if ((AIL_get_preference(DIG_ENABLE_RESAMPLE_FILTER)) && ((src_rate) != dest_rate))
     {
     op |= M_FILTER;
     }

  return(op);
}

//############################################################################
//##                                                                        ##
//## Return how much memory a conversion will require                       ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_size_processed_digital_audio(
                                 U32             dest_rate,
                                 U32             dest_format,
                                 S32             num_srcs,
                                 AILMIXINFO const FAR* src)
{
   if ((src==0) || (num_srcs==0))
     return(0);

   // a point is one sample in mono, or two samples in stereo -
   // it is one sample point in time

   S32 i;
   S32 max_src_points=0;

   //calculate the sample points for each input
   for(i=0;i<num_srcs;i++) {
     S32 points=src[i].Info.data_len;
     if (src[i].Info.format==WAVE_FORMAT_IMA_ADPCM)
       points<<=1;    //2 adpcm samples in a byte
     else
       if (src[i].Info.bits!=8)
         points>>=1;  //2 bytes in a 16-bit sample
     if (src[i].Info.channels==2)
       points>>=1;    //2 samples in a stereo point

     // adjust number of points for sample rate differences
     points=(S32)(((F64)points*(F64)dest_rate)/((F64)src[i].Info.rate));

     // keep track of the greatest number of points
     if (points>max_src_points)
       max_src_points=points;
   }

   // figure number of dest points
   S32 dest_point_size=((dest_format&DIG_F_STEREO_MASK)?2:1)*((dest_format&DIG_F_16BITS_MASK)?2:1);

   return( (dest_point_size*max_src_points)+256 );  // 256 for slop

}

//############################################################################
//##                                                                        ##
//## Ask MSS to reset the ASI at the end of the specified buffer            ##
//##                                                                        ##
//############################################################################

DXDEC  void     AILCALL AIL_request_EOB_ASI_reset   (HSAMPLE S,
                                                     U32     buff_num)
{
  if (S)
  {
    S->reset_ASI[buff_num]=1;
  }
}


//############################################################################
//##                                                                        ##
//## Convert data from one sample rate and format to another                ##
//##                                                                        ##
//############################################################################

#define NUMBUILDSAMPLES 2048

S32 AILCALL AIL_API_process_digital_audio(
                                 void FAR       *dest_buffer,
                                 S32             dest_buffer_size,
                                 U32             dest_rate,
                                 U32             dest_format,
                                 S32             num_srcs,
                                 AILMIXINFO FAR* src)
{

   DOLOCK();
   U32 nbytes_written=0;

   if ((src==0) || (dest_buffer==0) || (num_srcs==0))
     return(0);

   S32 adpcm_size=0;
   S16 FAR*adpcm_temp_buffer=0;

   S32 dest_chan=((dest_format&DIG_F_STEREO_MASK)?2:1);

   //
   // Get driver mixer providers
   //

   MIXER_FLUSH MIXER_flush;
   MIXER_MERGE MIXER_merge;
   MIXER_COPY  MIXER_copy;

   HPROVIDER HP;


   RIB_enumerate_providers("MSS mixer services",
                            NULL,
                           &HP);

   RIB_INTERFACE_ENTRY MIXER[] =
      {
      FN(MIXER_merge),
      FN(MIXER_flush),
      FN(MIXER_copy)
      };

   RIB_request(HP,"MSS mixer services",MIXER);


   S32 i;
   
   //calculate all of the operations
   S32 operations[256];
   for(i=0;i<num_srcs;i++)
     operations[i]=get_operation(dest_rate,dest_format,src[i].Info.rate,src[i].Info.bits,src[i].Info.channels);

  keepconverting:

   // a point is one sample in mono, or two samples in stereo -
   // it is one sample point in time

   S32 max_src_points=0;

   //calculate the sample points for each input
   for(i=0;i<num_srcs;i++) {
     S32 points=src[i].Info.data_len;
     if (src[i].Info.format==WAVE_FORMAT_IMA_ADPCM)
       points<<=1; //2 adpcm samples in a byte
     else
       if (src[i].Info.bits!=8)
         points>>=1;  //2 bytes in a 16-bit sample
     if (src[i].Info.channels==2)
       points>>=1;  //2 samples in a stereo point

     // adjust number of points for sample rate differences
     points=(S32)(((F64)points*(F64)dest_rate)/((F64)src[i].Info.rate));

     // keep track of the greatest number of points
     if (points>max_src_points)
       max_src_points=points;
   }

   // figure number of dest points
   S32 dest_point_size=dest_chan*((dest_format&DIG_F_16BITS_MASK)?2:1);
   S32 dest_points=dest_buffer_size/dest_point_size;

   //limit our outputs to the maximum number of inputs
   if (max_src_points<dest_points)
     dest_points=max_src_points;

   //just exit if we don't have any mixing to do
   if (dest_points==0)
     return(nbytes_written);


   // Init persistent variables for all passes
   for(i=0;i<num_srcs;i++) {
     if ((src[i].mss_adpcm.blocksize!=src[i].Info.block_size) || (src[i].Info.initial_ptr==src[i].Info.data_ptr)) {
       src[i].mss_adpcm.blocksize    = src[i].Info.block_size;
       src[i].mss_adpcm.blockleft    = 0;
       src[i].mss_adpcm.extrasamples = 0;
       src[i].src_fract=0;
       src[i].left_val=0;
       src[i].right_val=0;
     }
   }

   //ok, get down to the mixing
   U8 FAR* dest=(U8 FAR*)dest_buffer;

   S32 build_buffer[NUMBUILDSAMPLES];
   S32 build_points=NUMBUILDSAMPLES/dest_chan;

   while (dest_points) {

     //figure out how many points we're going to do this pass
     S32 points=build_points;
     if (points>dest_points)
       points=dest_points;

     S32 build_size=points*sizeof(S32)*dest_chan;

     //clear the output buffer
     MIXER_flush( build_buffer,
                  build_size,
                  NULL,
                  0
#ifdef IS_X86
                  ,AIL_get_preference(AIL_ENABLE_MMX_SUPPORT)
#endif                  
                  );

     S32 max_built_size=0;

     //now loop through the srcs mixing in a piece of each src's buffer
     for(i=0;i<num_srcs;i++) {

       //do we have anything to mix?
       if (src[i].Info.data_len==0)
         continue;

       void const FAR* mix_ptr;
       S32 mix_size;

       // if we're dealing with ADPCM, we have to decompress into a temp buffer first
       if (src[i].Info.format==WAVE_FORMAT_IMA_ADPCM) {

         void const FAR* src_orig=src[i].Info.data_ptr;

         mix_size=2*((S32)(((((F64)points)*((F64)src[i].Info.rate))/(F64)dest_rate)+0.5F))*src[i].Info.channels;

         //make sure the buffer is big enough
         if (mix_size>adpcm_size) {
           if (adpcm_temp_buffer)
             AIL_mem_free_lock(adpcm_temp_buffer);
           adpcm_size=mix_size;
           adpcm_temp_buffer=(S16 FAR*)AIL_mem_alloc_lock(adpcm_size);
           if (adpcm_temp_buffer==0)
             return(0);
         }

         S16 FAR* temp=adpcm_temp_buffer;

         if (src[i].Info.channels==2)
            {
            DecodeADPCM_STEREO(&temp, &src[i].Info.data_ptr, mix_size, src[i].Info.data_len, &src[i].mss_adpcm);
            }
         else
            {
            DecodeADPCM_MONO(&temp, &src[i].Info.data_ptr, mix_size, src[i].Info.data_len, &src[i].mss_adpcm);
            }

         U32 src_moved=AIL_ptr_dif(src[i].Info.data_ptr,src_orig);
         src[i].Info.data_len=(src[i].Info.data_len<=src_moved)?0:(src[i].Info.data_len-src_moved);

         mix_ptr=adpcm_temp_buffer;

         //don't worry about the fractional bit with adpcm
         src[i].src_fract=0;

       } else {

         mix_ptr=src[i].Info.data_ptr;
         mix_size=src[i].Info.data_len;

       }


       //
       // Call mixer provider
       //

       #ifdef IS_32

         void const FAR* mix_orig=mix_ptr;
         S32 FAR* build=build_buffer;

         MIXER_merge(&mix_ptr,
                     &src[i].src_fract,
                      AIL_ptr_add( mix_ptr, mix_size),
                     &build,
                      AIL_ptr_add( build, build_size),
                     &src[i].left_val,
                     &src[i].right_val,
                      (S32)((((F64) src[i].Info.rate) * 65536.0F) / (F64) dest_rate),
                      2047,
                      2047,
                      operations[i]
#ifdef IS_X86
                      ,AIL_get_preference(AIL_ENABLE_MMX_SUPPORT)
#endif
                      );

         S32 built_size=AIL_ptr_dif(build,build_buffer);
         U32 src_moved=AIL_ptr_dif(mix_ptr,mix_orig);

       #else

         U32 src_offset  = LOWORD(mix_ptr);
         U32 dest_offset = LOWORD(build_buffer);

         MIXER_merge(HIWORD(mix_ptr),
                     HIWORD(build_buffer),
                    &src[i].src_fract,
                    &src_offset,
                    &dest_offset,
                     src_offset+mix_size,
                     dest_offset+build_size,
                    &src[i].left_val,
                    &src[i].right_val,
                     (S32)((((F64) src[i].Info.rate) * 65536.0F) / (F64) dest_rate),
                     (2047 << 16) | 2047,
                     operations[i]);

         U32 src_moved=(src_offset - LOWORD(mix_ptr));
         S32 built_size=(dest_offset - LOWORD(build_buffer));

       #endif

       // don't adjust the pointers for adpcm (already done)
       if (!(src[i].Info.format==WAVE_FORMAT_IMA_ADPCM)) {
         src[i].Info.data_ptr = AIL_ptr_add(src[i].Info.data_ptr, src_moved);
         src[i].Info.data_len=(src[i].Info.data_len<=src_moved)?0:(src[i].Info.data_len-src_moved);
       }

       if (built_size>max_built_size)
         max_built_size=built_size;

     }

     if (max_built_size==0)
       break;

     //copy from the build buffer into dest
     MIXER_copy( build_buffer,
                 max_built_size,
                 dest,
                 dest_format,
#ifdef IS_X86
                 AIL_get_preference(AIL_ENABLE_MMX_SUPPORT)
#else
                 0
#endif
                 );

     dest=(U8 FAR*)AIL_ptr_add(dest,points*dest_point_size);
     dest_points-=points;
   }

   S32 written=AIL_ptr_dif(dest,dest_buffer);
   nbytes_written+=written;
   dest_buffer=dest;
   dest_buffer_size-=written;

   if (dest_buffer_size)
     goto keepconverting;

   if (adpcm_temp_buffer)
     AIL_mem_free_lock(adpcm_temp_buffer);

   return(nbytes_written);
}

//
// ASCII alternate redist directory
//

extern char AIL_redist_directory[260]="";

//############################################################################
//##                                                                        ##
//## Set the alternate MSS driver directory                                 ##
//##                                                                        ##
//############################################################################

extern "C"
char FAR*  AILEXPORT AIL_set_redist_directory(char const FAR* dir)
{
   S32 len;

   if (dir==NULL)
      *AIL_redist_directory=0;
   else
      {
#ifdef IS_MAC
      AIL_strcpy(AIL_redist_directory,dir);
        
      len=AIL_strlen(AIL_redist_directory);
      if (len)
      {
        if (AIL_redist_directory[len-1]!=':')
        {
          AIL_redist_directory[len]=':';
          AIL_redist_directory[len+1]=0;
        }
      }
#else
      AIL_strcpy(AIL_redist_directory,dir);
      len=AIL_strlen(AIL_redist_directory);
      if (len)
         {
         if ((AIL_redist_directory[len-1]!='\\') && (AIL_redist_directory[len-1]!=':'))
            {
            AIL_redist_directory[len]='\\';
            AIL_redist_directory[len+1]=0;
            }
         }
#endif
      }

   return (char FAR*) (AIL_redist_directory);
}




#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## End of locked code                                                     ##
//##                                                                        ##
//############################################################################

void AILSFILE_end(void)
{
   if (locked)
      {
      AIL_vmm_unlock_range(AILSFILE_start, AILSFILE_end);

      locked = 0;
      }
}

#endif

