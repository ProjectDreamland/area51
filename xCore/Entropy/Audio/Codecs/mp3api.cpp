//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: ASI decoder module for MPEG audio                            ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 8-Apr-98: Initial                                     ##
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
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "mss.h"
#include "mp3dec.h"

#include "x_files.hpp"

S32 ASI_started = 0;

C8 ASI_error_text[256];

//
// Attribute tokens
//

enum ATTRIB
{
   //
   // Additional decoder attribs (beyond standard RIB PROVIDER_ attributes)
   //

   IN_FTYPES,               // STRING supported input file types
   IN_WTAG,                 // Tag used for this data in a wave file
   OUT_FTYPES,              // STRING supported output file types
   FRAME_SIZE,              // S32 worst-case frame buffer size

   //
   // Generic ASI stream attribs
   //

   INPUT_BIT_RATE,          // S32 input bit rate
   INPUT_SAMPLE_RATE,       // S32 sample rate of input data
   INPUT_BITS,              // S32 bit width per sample of input data
   INPUT_CHANNELS,          // S32 # of channels in input data

   OUTPUT_BIT_RATE,         // S32 output bit rate
   OUTPUT_SAMPLE_RATE,      // S32 output sample rate
   OUTPUT_BITS,             // S32 bit width per sample of output data
   OUTPUT_CHANNELS,         // S32 # of channels in output data

   POSITION,                // S32 bytes processed so far
   PERCENT_DONE,            // % percent done
   MIN_INPUT_BLOCK_SIZE,    // S32 minimum block size for input

   //
   // Codec-specific stream attribs
   //

   MPEG_VERSION,            // S32 MPEG version
   MPEG_LAYER,              // S32 MPEG layer

   //
   // Stream preferences
   //

   REQUESTED_RATE,          // S32 requested rate for output data
   REQUESTED_BITS,          // S32 requested bit width for output data
   REQUESTED_CHANS,         // S32 requested # of channels for output data
};

//############################################################################
//#                                                                          #
//# Read one frame's worth of audio data into stream buffer                  #
//#                                                                          #
//# Return byte offset of beginning of new data in buffer, or -1 on error    #
//#                                                                          #
//############################################################################

S32 fetch_audio_data(ASISTREAM* STR)
{
   //
   // See if incoming data block will fit in buffer at current write position
   // If not, discard data in lower half of buffer and move newer data down to
   // make room
   //
   // Ensure that 4 bytes of valid overrun data always exists at end of 
   // buffer, to avoid page faults in read_bits()
   //
   // Incoming data block is assumed to be smaller than (STREAM_BUFSIZE / 2)!
   // Furthermore, we assume that read and write cursors are in the second
   // half of the buffer by the time an overflow condition occurs.  Two 4K 
   // buffer halves will be more than sufficient to guarantee these criteria
   // for all valid MPEG frame sizes.
   //

   S32 half = STREAM_BUFSIZE / 2;

   if (STR->write_cursor + STR->data_size >= (STREAM_BUFSIZE-4))
      {
      x_memcpy(STR->audio_buffer, 
                &STR->audio_buffer[half],
                 half);

      STR->write_cursor -= half;

      STR->apos -= (half * 8);
      }

   S32 result = STR->fetch_CB(STR->user,
                             &STR->audio_buffer[STR->write_cursor],
                              STR->data_size,
                             -1);

   if (result < STR->data_size)
      {
#if 0
      sprintf(ASI_error_text,"Truncated MPEG audio stream, %ld bytes read, %ld expected",
         result, STR->data_size);
      return -1;
#else
   //
   // Reference encoder seems to truncate final frame, so we'll do the padding
   // here...
   //

   x_memset(&STR->audio_buffer[result],
               0,
               STR->data_size - result);
#endif
      }

   STR->current_offset += STR->data_size;

   //
   // Advance write cursor and return its original value
   //

   STR->write_cursor += STR->data_size;

   return STR->write_cursor - STR->data_size;
}

//############################################################################
//#                                                                          #
//# Seek next frame header in stream, and write current frame information    #
//# to ASISTREAM structure                                                   #
//#                                                                          #
//############################################################################

S32 seek_frame(ASISTREAM* STR, //)
               S32            offset)
{
   //
   // Fetch data from stream until valid header encountered
   //

   if (offset != -1)
      {
      STR->current_offset = offset;
      }

  S32 found_layer = -1;
  S32 start_offset = STR->current_offset;
  S32 result;
  U32 seek_data = 0;
  U32 dest_data = 0;

  again:

   S32 seek_offset = offset;

   while (1)
      {
#ifdef IS_LE
      seek_data >>= 8;
#else
      seek_data <<= 8;
#endif

      if ( ( STR->current_offset - start_offset ) > AIL_MAX_FILE_HEADER_SIZE )
      {
        goto not_found;
      }

      result = STR->fetch_CB(STR->user,
                             ((U8*)&dest_data)+3,
                             1,
                             seek_offset);
      if (result < 1)
         {
         //
         // (Not considered an error)
         //

//       strcpy(ASI_error_text,"End of data reached");
         return 0;
         }

      seek_data |= dest_data;

      seek_offset = -1;
      STR->current_offset++;

#ifdef IS_LE
      if ( (seek_data & 0x0000e0ff ) == 0x0000e0ff )
#else
      if ( (seek_data & 0xffe00000 ) == 0xffe00000 )
#endif
         {
         break;
         }

      }

   //
   // Read rest of header dword
   //

   *((U32*)STR->header_buffer) = seek_data;


   //
   // Parse header, first skipping 11-bit sync field
   //

   STR->hpos = 11;

   STR->MPEG25             = !H(1);
   STR->MPEG1              =  H(1);
   STR->layer              =  H(2);
   STR->protection_bit     =  H(1);
   STR->bitrate_index      =  H(4);
   STR->sampling_frequency =  H(2);
   STR->padding_bit        =  H(1);
   STR->private_bit        =  H(1);
   STR->mode               =  H(2);
   STR->mode_extension     =  H(2);
   STR->copyright          =  H(1);
   STR->original           =  H(1);
   STR->emphasis           =  H(2);

   //
   // Perform sanity check on header, since most encoders seem to be written
   // with complete disregard to the rule against >= 12 consecutive 1 bits
   // in stream....
   //
   // We assume the first header found in the stream is valid, and use its
   // contents to check the fields of all subsequent headers.  The fields
   // tested are those which should not ordinarily change in the course of
   // a single stream.
   //

   if ((STR->bitrate_index      == 0x0f) ||
       (STR->sampling_frequency == 0x03) )
   {
      //
      // Header contains one or more invalid bitfields, so skip it
      //
      // (Note that this will skip a valid frame if it begins within
      // the first 4 bytes of a false header)
      //

      offset = -1;
      goto again;
   }

   // keep searching if we find a non layer 3 block
   if ( STR->layer!= 1)
   {
     found_layer = STR->layer;
     offset = -1;
     goto again;
   }

   if (!STR->check_valid)
      {
      STR->check_valid = 1;

      STR->check_MPEG25             = STR->MPEG25;
      STR->check_MPEG1              = STR->MPEG1;
      STR->check_layer              = STR->layer;
      STR->check_protection_bit     = STR->protection_bit;
      STR->check_sampling_frequency = STR->sampling_frequency;
      STR->check_mode               = STR->mode;
      STR->check_copyright          = STR->copyright;
      STR->check_original           = STR->original;
      }
   else
      {
      if ((STR->MPEG1              != STR->check_MPEG1)              ||
          (STR->MPEG25             != STR->check_MPEG25)             ||
          (STR->layer              != STR->check_layer)              ||

//          (STR->protection_bit     != STR->check_protection_bit)     ||
//          (STR->mode               != STR->check_mode)               ||
//          (STR->copyright          != STR->check_copyright)          ||
//          (STR->original           != STR->check_original)           ||

          (STR->sampling_frequency != STR->check_sampling_frequency) )
          {

          //
          // Header does not match characteristics of first one found in
          // stream -- keep looking
          //
          // (Note that this will skip a valid frame if it begins within
          // the first 4 bytes of a false header)
          //

          offset = -1;
          goto again;
          }

      }

   //
   // Skip CRC word if present
   //

   STR->header_size = 4;

   if (STR->protection_bit == 0)
      {
      result = STR->fetch_CB(STR->user,
                             &STR->header_buffer[4],
                             2,
                             -1);
      if (result < 1)
         {
          not_found:

           if (found_layer == 2)
             {
             strcpy(ASI_error_text,"MPEG Layer 2 files not supported");
             return 0 ; // we don't support layer 2
             }
           else if (found_layer == 3)  // the 3 means layer 1 - cool!
             {
             strcpy(ASI_error_text,"MPEG Layer 1 files not supported");
             return 0 ; // we don't support layer 1
             }
           else  if (found_layer == 0) // reserved layer
             {
             strcpy(ASI_error_text,"Undefined/reserved MPEG layer not supported");
             return 0;
             }

           strcpy(ASI_error_text,"MPEG audio header not found or is badly formatted");
           return 0;
         }

      STR->current_offset += 2;

      STR->hpos += 16;

      STR->header_size = 6;
      }

   STR->frame_function      = L3_frame;
   STR->frame_info_function = L3_frame_info;

   //
   // Call frame info function to fetch stream data
   //

   if (!STR->frame_info_function(STR))
      {
      //
      // End of stream reached, or error occurred
      //

      return 0;
      }

   //
   // Return success
   //

   return 1;
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
      case PROVIDER_NAME:    return (U32) "MSS MPEG Layer 3 Audio Decoder";
      case PROVIDER_VERSION: return 0x100;

      case IN_FTYPES:        return (U32) "MPEG Layer 3 audio files\0*.MP3\0";
      case IN_WTAG:          return 85;
      case OUT_FTYPES:       return (U32) "Raw PCM files\0*.RAW\0";

      case FRAME_SIZE:       return (U32) (STREAM_BUFSIZE / 2);
      }

   return 0;
}

//############################################################################
//#                                                                          #
//# Return ASI decoder error text                                            #
//#                                                                          #
//############################################################################

C8*       AILEXPORT ASI_error       (void)
{
   if (!x_strlen(ASI_error_text))
      {
      return NULL;
      }

   return ASI_error_text;
}


//############################################################################
//#                                                                          #
//# Initialize ASI stream decoder                                            #
//#                                                                          #
//############################################################################

ASIRESULT AILEXPORT ASI_startup     (void)
{
   if (ASI_started++)
      {
      strcpy(ASI_error_text,"Already started");
      return ASI_ALREADY_STARTED;
      }

   //
   // Init static prefs/attributes
   //

   ASI_error_text[0] = 0;

   //
   // Init layer-specific data
   //

   L3_init();

   return ASI_NOERR;
}

//############################################################################
//#                                                                          #
//# Shut down ASI stream decoder                                             #
//#                                                                          #
//############################################################################

ASIRESULT      AILEXPORT ASI_shutdown    (void)
{
   if (!ASI_started)
      {
      strcpy(ASI_error_text,"Not initialized");
      return ASI_NOT_INIT;
      }

   --ASI_started;

   //
   // Destroy layer-specific data
   //

   L3_destroy();

   return ASI_NOERR;
}

//############################################################################
//#                                                                          #
//# Open a stream, returning handle to stream                                #
//#                                                                          #
//############################################################################

HASISTREAM   AILEXPORT ASI_stream_open (U32           user,  //)
                                        AILASIFETCHCB fetch_CB,  
                                        U32           total_size)
{
   //
   // Allocate ASISTREAM descriptor
   //

   ASISTREAM* STR = new ASISTREAM;

   if (STR == NULL)
      {
      strcpy(ASI_error_text,"Out of memory");
      return NULL;
      }

   x_memset(STR, 0, sizeof(ASISTREAM));

   //
   // Init prefs
   //

   STR->requested_rate = -1;
   STR->requested_bits = -1;
   STR->requested_chans = -1;

   //
   // Copy params to descriptor fields
   //

   STR->user        = user;
   STR->fetch_CB    = fetch_CB;
   STR->total_size  = total_size;

   //
   // Alloc frame buffer
   //

   STR->frame_buffer = (U8*)x_malloc( FRAME_BUFSIZE );

   if (STR->frame_buffer == NULL)
      {
      strcpy(ASI_error_text,"Out of memory");
      x_free( STR );
      return NULL;
      }

   //
   // Initialize input buffer
   //

   STR->write_cursor = 0;
   STR->apos         = 0;

   //
   // Initialize output buffer
   //

   STR->frame_size = 0;
   STR->output_cursor = 0;

   //
   // Decode first frame header to obtain stream information
   //

   if (!seek_frame(STR,0))
      {
      x_free( STR );
      return NULL;
      }

   //
   // Force rewind to beginning of stream when first frame decoded
   //

   STR->seek_param = 0;

   x_memset(STR->m_s,0,sizeof(STR->m_s));
   x_memset(STR->m_res,0,sizeof(STR->m_res));

   STR->m_u_start[0] = STR->m_u_start[1] = 0;
   STR->m_u_div  [0] = STR->m_u_div  [1] = 0;

   //
   // Return descriptor address cast to handle
   //

   return (U32) STR;
}
                                            
//############################################################################
//#                                                                          #
//# Close stream, freeing handle and all internally-allocated resources      #
//#                                                                          #
//############################################################################

ASIRESULT      AILEXPORT ASI_stream_close(HASISTREAM stream)
{
   ASISTREAM* STR = (ASISTREAM*)stream;

   if (STR->frame_buffer != NULL)
      {
      x_free( STR->frame_buffer );
      STR->frame_buffer = NULL;
      }

//   x_free( STR );
   delete STR;

   return ASI_NOERR;
}

//############################################################################
//#                                                                          #
//# Decode data from stream, returning # of bytes actually decoded           #
//#                                                                          #
//############################################################################

S32 AILEXPORT ASI_stream_process (HASISTREAM  stream, //)
                                  void*       buffer,
                                  S32         request_size)
{
   ASISTREAM* STR = (ASISTREAM*)stream;

   //
   // Keep track of # of bytes originally requested
   //

   S32 original_request = request_size;

   //
   // Init buffer output offset
   //

   S32 write_cursor = 0;

   U8* dest = (U8*)buffer;

   //
   // If any data from last frame remains in buffer, copy it first
   //
   // Otherwise fetch and decode frame data until request size satisfied
   //

   while (request_size > 0)
      {
      //
      // Copy as much data as possible from currently-buffered frame
      //

      if (STR->output_cursor < STR->frame_size)
         {
         S32 avail = STR->frame_size - STR->output_cursor;

         if (avail > request_size)
            {
            avail = request_size;
            }

         x_memcpy(&dest[write_cursor],
                    &STR->frame_buffer[STR->output_cursor],
                     avail);

         ASSERT( STR->output_cursor+avail <= FRAME_BUFSIZE );

         STR->output_cursor += avail;
         write_cursor       += avail;
         request_size       -= avail;
         }

      //
      // Exit from loop if request completely fulfilled by existing buffer
      // contents
      //

      if (!request_size)
         {
         break;
         }

      //
      // Else initialize output frame buffer and fetch next frame
      //

      STR->frame_size = 0;
      STR->output_cursor = 0;

      //
      // Seek next frame based on current offset
      //

      if (!seek_frame(STR,STR->seek_param))
         {
         //
         // End of stream reached, or error occurred
         //

         break;
         }

      //
      // Set offset parameter to -1 for all subsequent frames, to allow
      // application to perform continuous streaming without explicit seeks
      //

      STR->seek_param = -1;

      //
      // Call frame-processing function
      //

      if (!STR->frame_function(STR))
         {
         //
         // End of stream reached, or error occurred
         //

         break;
         }
      }

   //
   // If source stream exhausted, pad with zeroes to end of buffer
   //

   if (request_size > 0)
      {
      x_memset(&dest[write_cursor],
                  0,
                  request_size);
      }

   //
   // Return # of bytes fetched from source stream
   //

   return original_request - request_size;
}

#define max(a,b)  (((a) > (b)) ? (a) : (b))

//############################################################################
//#                                                                          #
//# Retrieve an ASI stream attribute or preference value by index            #
//#                                                                          #
//############################################################################

S32     AILEXPORT ASI_stream_attribute (HASISTREAM stream, //)
                                        HATTRIB    attribute)
{
   ASISTREAM* STR = (ASISTREAM*)stream;

   //
   // Sample rate in samples/second for [MPEG25][MPEG version][value]
   //

   const S32 sample_rate[2][2][4] =
   {{
      { 22050L,24000L,16000L,22050L },
      { 44100L,48000L,32000L,44100L }
   },
   {
      { 11025L,12000L, 8000L,11025L },
      { 44100L,48000L,32000L,44100L }
   }};

   switch ((ATTRIB) attribute)
      {
      //
      // Attributes
      //

      case INPUT_BIT_RATE:     return STR->bit_rate;
      case INPUT_SAMPLE_RATE:  return sample_rate[STR->MPEG25][STR->MPEG1][STR->sampling_frequency];
      case INPUT_BITS:         return 16 / max(1,((sample_rate[STR->MPEG25][STR->MPEG1][STR->sampling_frequency] * 16 * STR->nch) / STR->bit_rate));
      case INPUT_CHANNELS:     return STR->nch;

      case OUTPUT_BIT_RATE:    return sample_rate[STR->MPEG25][STR->MPEG1][STR->sampling_frequency] * 16 * STR->nch;
      case OUTPUT_SAMPLE_RATE: return sample_rate[STR->MPEG25][STR->MPEG1][STR->sampling_frequency];
      case OUTPUT_BITS:        return 16;
      case OUTPUT_CHANNELS:    return STR->nch;

      case POSITION:      return STR->current_offset;

      case MPEG_VERSION:  return 2 - STR->MPEG1;
      case MPEG_LAYER:    return 4 - STR->layer;

      case MIN_INPUT_BLOCK_SIZE: return (STREAM_BUFSIZE / 2);

      case PERCENT_DONE:

         if ((STR->current_offset == 0) || (STR->total_size == 0))
            {
            return 0;
            }
         else
            {
            F32 percent = F32(STR->current_offset) * 100.0F / F32(STR->total_size);

            return *(U32*)(&percent);
            }

      //
      // Preferences
      //

      case REQUESTED_RATE:  return STR->requested_rate;
      case REQUESTED_BITS:  return STR->requested_bits;
      case REQUESTED_CHANS: return STR->requested_chans;
      }

   return -1;
}

//############################################################################
//#                                                                          #
//# Set stream preference value, returning previous setting                  #
//#                                                                          #
//############################################################################

S32 AILEXPORT ASI_stream_set_preference (HASISTREAM stream, //)
                                         HATTRIB    preference,
                                         void*      value)
{
   ASISTREAM* STR = (ASISTREAM*)stream;

   S32 prev = -1;

   switch ((ATTRIB) preference)
      {
      //
      // Preferences
      //

      case REQUESTED_RATE:  prev = STR->requested_rate;  STR->requested_rate  = *(S32*)value; break;
      case REQUESTED_BITS:  prev = STR->requested_bits;  STR->requested_bits  = *(S32*)value; break;
      case REQUESTED_CHANS: prev = STR->requested_chans; STR->requested_chans = *(S32*)value; break;
      }

   return prev;
}
#if 0

extern "C" S32 MSS_RIB_Main( HPROVIDER provider_handle, U32 up_down );
extern "C" S32 MSS_RIB_Main( HPROVIDER provider_handle, U32 up_down )
{
   const RIB_INTERFACE_ENTRY ASI[] =
      {
      REG_FN(PROVIDER_query_attribute),
      REG_AT("Name",                 PROVIDER_NAME,    RIB_STRING),
      REG_AT("Version",              PROVIDER_VERSION, RIB_HEX),

      REG_AT("Input file types",     IN_FTYPES,  RIB_STRING),
      REG_AT("Input wave tag",       IN_WTAG,    RIB_DEC),
      REG_AT("Output file types",    OUT_FTYPES, RIB_STRING),
      REG_AT("Maximum frame size",   FRAME_SIZE, RIB_DEC),

      REG_FN(ASI_startup),
      REG_FN(ASI_error),
      REG_FN(ASI_shutdown),
      };

   const RIB_INTERFACE_ENTRY ASISTR[] =
      {
      REG_FN(ASI_stream_open),
      REG_FN(ASI_stream_process),
      REG_FN(ASI_stream_attribute),
      REG_FN(ASI_stream_set_preference),
      REG_FN(ASI_stream_seek),
      REG_FN(ASI_stream_close),

      REG_AT("Input bit rate",           INPUT_BIT_RATE,       RIB_DEC),
      REG_AT("Input sample rate",        INPUT_SAMPLE_RATE,    RIB_DEC),
      REG_AT("Input sample width",       INPUT_BITS,           RIB_DEC),
      REG_AT("Input channels",           INPUT_CHANNELS,       RIB_DEC),
      REG_AT("Output bit rate",          OUTPUT_BIT_RATE,      RIB_DEC),
      REG_AT("Output sample rate",       OUTPUT_SAMPLE_RATE,   RIB_DEC),
      REG_AT("Output sample width",      OUTPUT_BITS,          RIB_DEC),
      REG_AT("Output channels",          OUTPUT_CHANNELS,      RIB_DEC),
      REG_AT("Position",                 POSITION,             RIB_DEC),
      REG_AT("Percent done",             PERCENT_DONE,         RIB_PERCENT),
      REG_AT("Minimum input block size", MIN_INPUT_BLOCK_SIZE, RIB_DEC),
      REG_AT("MPEG version",             MPEG_VERSION,         RIB_DEC),
      REG_AT("MPEG layer",               MPEG_LAYER,           RIB_DEC),

      REG_PR("Requested sample rate",    REQUESTED_RATE,       RIB_DEC),
      REG_PR("Requested bit width",      REQUESTED_BITS,       RIB_DEC),
      REG_PR("Requested # of channels",  REQUESTED_CHANS,      RIB_DEC)
      };

   static HPROVIDER self;

   if (up_down)
     {
#ifdef IS_WINDOWS
         self = RIB_provider_library_handle();
#else
         self = provider_handle;
#endif

         RIB_register(self,
                     "ASI codec",
                      ASI);

         RIB_register(self,
                     "ASI stream",
                      ASISTR);
      }
    else
      {
         RIB_unregister_all(self);
      }

   return TRUE;
}
#endif
#ifdef IS_WIN32

//############################################################################
//#                                                                          #
//# DLLMain registers ASI API interface at load time                         #
//#                                                                          #
//############################################################################

BOOL WINAPI DllMain(HINSTANCE hinstDll, //)
                          U32     fdwReason,
                          LPVOID    plvReserved)
{
   switch (fdwReason)
      {
      case DLL_PROCESS_ATTACH:

         DisableThreadLibraryCalls( hinstDll );
         return( MSS_RIB_Main( 0, 1 ) );

      case DLL_PROCESS_DETACH:

         return( MSS_RIB_Main( 0, 0 ) );
      }

   return TRUE;
}

#endif

#ifdef IS_WIN16

BOOL __far __pascal LibMain(HANDLE hInstance, //)
                            WORD   wDataSegment,
                            WORD   wHeapSize,
                            LPSTR  lpszCmdLine )
{
  wDataSegment=wDataSegment;
  wHeapSize=wHeapSize;
  lpszCmdLine=lpszCmdLine;

  if (wHeapSize > 0)
   UnlockData(0);

  MSS_RIB_Main( 0, 1);

  return 1;
}

int CALLBACK WEP(int exit)
{
  MSS_RIB_Main( 0, 0);

  return 1;
}

#endif

#ifdef IS_DOS

#define DLL_PROCESS_ATTACH TRUE
#define DLL_PROCESS_DETACH FALSE

extern "C" S32 AILCALL MPEG_register(S32 fdwReason, HPROVIDER provider_handle)
{
   switch (fdwReason)
      {
      case DLL_PROCESS_ATTACH:

         MSS_RIB_Main( provider_handle, 1);
         break;

      case DLL_PROCESS_DETACH:

         MSS_RIB_Main( provider_handle, 0);
         break;
      }

   return TRUE;

}

#endif

