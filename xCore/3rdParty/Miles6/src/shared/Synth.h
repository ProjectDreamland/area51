//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  SYNTH.H                                                               ##
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

#ifndef SYNTH_H
#define SYNTH_H

#include "stdio.h"

#pragma pack(1)

#define N_BUILD_BUFFERS 3            // 3 build buffers in chain
#define MQ_SIZE         4096         // 4K message slots in MIDI queue

#ifndef F32
#define F32 float
#endif

#define FOURCC_MLS  mmioFOURCC('M','L','S',' ')
#define FOURCC_FMT  mmioFOURCC('f','m','t',' ')
#define FOURCC_PCM  mmioFOURCC('p','c','m',' ')
#define FOURCC_JUNK mmioFOURCC('j','u','n','k')
#define FOURCC_DATA mmioFOURCC('d','a','t','a')
#define FOURCC_FACT mmioFOURCC('f','a','c','t')
#define FOURCC_INFO mmioFOURCC('I','N','F','O')
#define FOURCC_INAM mmioFOURCC('I','N','A','M')

#define WAVE_FORMAT_PCM         1
#define WAVE_FORMAT_IMA_ADPCM   0x0011

//
// DLL instance handle
//

//
// Single global instance of synthesizer device
//

extern struct SYNTHDEVICE FAR *DLS;


//
// Misc. definitions
//

struct CHUNK
{
   FOURCC ckID;
   S32    ckSize;
   FOURCC subID;
   U8     data[1];
};

struct SUBCHUNK
{
   FOURCC ckID;
   S32    ckSize;
   U8     data[1];
};

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
}
WAV_FMT;

//
// .WAV IMA ADPCM file format chunk
//

typedef struct
{
   S8  FMT_string[4];
   U32 chunk_size;

   S16 format_tag;
   S16 channels;
   S32 sample_rate;
   S32 average_data_rate;
   S16 alignment;
   S16 bits_per_sample;
   S16 extra;
   S16 samples_per_block;
}
IMA_FMT;

//
// ASI MPEG Audio file format chunk
//
// This is a proprietary WAV extension, used to wrap DLS sample files
// created by ASI encoders.  (ASI encoders always output raw data, which
// needs a RIFF header in order to be navigable by DLS readers.)
//
// The IMA_FACT and IMA_DATA chunks are reused by the ASI format to
// store the total sample count and data stream, respectively.
//

typedef struct
{
   S8 FMT_string[4];
   U32 chunk_size;

   S16 format_tag;               // WAVE_FORMAT_UNKNOWN
   S16 channels;                 // # of channels
   S32 sample_rate;              // Sample rate in samples/second
   S32 average_data_rate;        // Stream rate in bytes per second
   S16 alignment;                // Always 1 (actual alignment constraints are determined by ASI decoder)
   S16 bits_per_sample;          // Bits/sample value from encoder output
   S16 extra;                    // Always 4

   C8 original_file_suffix[4];   // Example: "MP3" with zero terminator
}
ASI_FMT;

//
// .WAV FACT chunk
//

typedef struct
{
   S8  FACT_string[4];
   U32 chunk_size;
   U32 samples;
}
IMA_FACT;

//
// .WAV file data chunk
//

typedef struct
{
   S8  DATA_string[4];
   U32 chunk_size;
   S8  data[1];
}
IMA_DATA;


//
// .WAV file data chunk
//

typedef struct
{
   S8  DATA_string[4];
   U32 chunk_size;
   U8  data[1];
}
WAV_DATA;

//
// Articulation data
//

struct CONN
{
   C8 FAR *name;
   U16 val;
};

extern const CONN src[12];

extern const CONN dest[18];

struct CID
{
   U16 usSource;
   U16 usControl;
   U16 usDestination;
   U16 usTransform;
};

#define CONN_SRC_RPN0 -1
#define CONN_SRC_RPN1 -1
#define CONN_SRC_RPN2 -1

extern const CID CID_list[29];

enum ARTNAME
{
   LFO_FREQUENCY = 0,
   LFO_START_DELAY,
   LFO_ATTENUATION_SCALE,
   LFO_PITCH_SCALE,
   LFO_MODW_TO_ATTENUATION,
   LFO_MODW_TO_PITCH,
   VOL_EG_ATTACK_TIME,
   VOL_EG_DECAY_TIME,
   VOL_EG_SUSTAIN_LEVEL,
   VOL_EG_RELEASE_TIME,
   VOL_EG_VELOCITY_TO_ATTACK,
   VOL_EG_KEY_TO_DECAY,
   PITCH_EG_ATTACK_TIME,
   PITCH_EG_DECAY_TIME,
   PITCH_EG_SUSTAIN_LEVEL,
   PITCH_EG_RELEASE_TIME,
   PITCH_EG_VELOCITY_TO_ATTACK,
   PITCH_EG_KEY_TO_DECAY,
   DEFAULT_PAN,
   EG1_TO_ATTENUATION,
   EG2_TO_PITCH,
   KEY_ON_VELOCITY_TO_ATTENUATION,
   PITCH_WHEEL_TO_PITCH,
   KEY_NUMBER_TO_PITCH,
   MIDI_CONTROLLER_7_TO_ATTENUATION,
   MIDI_CONTROLLER_10_TO_PAN,
   MIDI_CONTROLLER_11_TO_ATTENUATION,
   RPN1_TO_PITCH,
   RPN2_TO_PITCH
};

struct ARTICULATION
{
   S32 lScale[29];
};

#define IMPLICIT -1              // This connection is fixed and cannot be overridden
#define ABSOLUTE_ZERO 0x80000000 // 32-bit absolute-zero value

#define F_PI  3.141592654F
#define F_2PI 6.283185308F

//
// Default ARTICULATION members used where no articulator values are
// explicitly defined in region
//

extern const ARTICULATION ARTDEFAULT;

//
// Default WSMPL -- used if neither WAVE nor RGN contains a valid WSMP chunk
//

extern const WSMPL WSMPLDEFAULT;

//
// Build buffer descriptor: status = FULL, EMPTY, or index of next chunk
// to be filled
//

#define BUILD_FULL  (-1)
#define BUILD_EMPTY (0)

struct BUILD_BUFFER
{
   S32 FAR *block;
   S32      status;
};

//
// MIDI channel status descriptor
//

struct MIDI_CHANNEL
{
   S32 bank_LSB;
   S32 bank_MSB;
   S32 patch;
   S32 instrument_index;

   S8     control[128];
   S32    coarse_tuning;
   F32 fine_tuning;
   F32 pitch_bend_range;
   F32 pitch;
};

struct MIDI_MESSAGE
{
   U8  status;
   U8  channel;
   U8  data_1;
   U8  data_2;
   S32 instrument_index;
};

//
// DLS voice descriptor
//

#define ATTACK_PHASE  0
#define DECAY_PHASE   1
#define SUSTAIN_PHASE 2
#define RELEASE_PHASE 3

struct REGION
{
   RGNHEADER           header;
   WSMPL               sample;
   WLOOP               loop;
   WAVELINK            wave;
   ARTICULATION FAR   *connection;
   CONNECTIONLIST FAR *art1;        // Original ART1 chunk data, for diagnostics
};

struct WAVEDESC
{
   S32       valid;

   S32       format;
   F32    rate;
   void FAR *data;
   S32       len;

   S32       WSMP_valid;
   WSMPL     sample;
   WLOOP     loop;
};

struct VOICE
{
   S32                  active;
   S32                  key;  
   S32                  release_request;

   struct WAVEDESC FAR *wave;

   MIDI_MESSAGE         trigger;
   struct REGION FAR   *region;

   S32                  mixer_operation;

   S32                  play_cursor;
   U32                  src_fract;
   S32                  left_val;
   S32                  right_val;

   S32                  loop_start;
   S32                  loop_end;
   S32                  loop_size;

   F32               static_pitch;
   F32               static_atten;

   S32                  BPS;

   S32                  LFO_holdoff;
   F32               LFO_radians_per_interval;
   F32               LFO_phase_accumulator;
   F32               LFO_atten_scale;
   F32               LFO_pitch_scale;
   F32               LFO_CC1_to_atten;
   F32               LFO_CC1_to_pitch;

   S32                  EG1_active;
   S32                  EG1_phase;
   F32               EG1_scale;
   F32               EG1_atten;
   F32               EG1_attack_dB_per_interval;
   S32                  EG1_attack_intervals;
   F32               EG1_decay_dB_per_interval;
   F32               EG1_sustain_atten;
   F32               EG1_release_dB_per_interval;
   S32                  EG1_release_intervals;

   S32                  EG2_active;
   S32                  EG2_phase;
   F32               EG2_scale;
   F32               EG2_pitch;
   F32               EG2_attack_per_interval;
   S32                  EG2_attack_intervals;
   F32               EG2_decay_per_interval;
   F32               EG2_sustain_pitch;
   F32               EG2_release_per_interval;
   S32                  EG2_release_intervals;

   S32                  default_pan;
};

//
// DLS synthesizer type definitions
//

struct SYNTHDEVICE
{
   HMDIDRIVER    mdi;

   U32           output_format;
   F32        output_sample_rate;
   S32           channels_per_sample;
   S32           bytes_per_channel;
   S32           bytes_per_sample;

   S32           stream_buffer_size;
   U32           user;
   AILDLSPCB     stream_poll_CB;
   AILDLSLCB     stream_lock_CB;
   AILDLSUCB     stream_unlock_CB;

   F32           service_rate_uS;
   U32           last_interval_us_count;
   U32           last_total;
   HTIMER        timer_handle;

   S32           bytes_to_write;

   BUILD_BUFFER  build[N_BUILD_BUFFERS];
   S32           bytes_per_buffer;
               
   S32 FAR * FAR *reverb_buffers;
   S32            n_reverb_buffers;

   S32           current_build_buffer;

   BUILD_BUFFER FAR *build_queue[N_BUILD_BUFFERS+1];
   S32               build_queue_head;
   S32               build_queue_tail;
   S32               buffers_filled;

   AILEVENTCB    prev_event_fn;
   AILTIMBRECB   prev_timb_fn; 

   MIDI_CHANNEL  channel[16];

   MIDI_MESSAGE  MIDI_queue[MQ_SIZE];
   S32           MIDI_queue_head;
   S32           MIDI_queue_tail;

   VOICE    FAR *voice_list;
   S32           n_voices;

   U32           use_MMX;

   S32           enabled;

   U32           ms_count;
   U32           us_count;
   U32           last_ms_polled;
   U32           last_percent;

   MIXER_FLUSH   mixer_flush;
   MIXER_MERGE   mixer_merge;
   MIXER_COPY    mixer_copy;
};

struct INSTRUMENT
{
   INSTRUMENT FAR *next;
   INSTRUMENT FAR *prev;
   INSTRUMENT FAR *next_in_set;
   INSTRUMENT FAR *prev_in_set;
   S32         set_key;
   S32         entry_num;

   void init(void const FAR *data)
      {
      data=data;
      DLS_file    = -1;
      region_list = NULL;
      art_list    = NULL;
      }

   void cleanup(void)
      {
      if (region_list != NULL)
         {
         AIL_mem_free_lock(region_list);
         region_list = NULL;
         }

      if (art_list != NULL)
         {
         AIL_mem_free_lock(art_list);
         art_list = NULL;
         }
      }

   S32 is_match(void FAR *data)
      {
      data=data;
      return 0;
      }

   static U32 set(void const FAR *data)
      {
      data=data;
      return 0;
      }

   S32                 DLS_file;    // DLS file containing instrument

   INSTHEADER          header;      // Bank/patch#/region count

   ARTICULATION   FAR *art_list;    // Articulation set(s)
   S32                 art_cnt;     // # of articulation set(s)

   REGION         FAR *region_list; // Instrument definitions by keyboard region
};

// ---------------------------------------------------------------------------
// Definitions/prototypes
// ---------------------------------------------------------------------------

#define ARYSIZE(x) (sizeof((x)) / sizeof((x)[0]))

//
// Instrument list manager class
//

#define INS_SET_DIM    1
#define INS_BLOCK_SIZE 128

class InsListMgr
{
public:
   S32 current_size;
   INSTRUMENT FAR*used;
   INSTRUMENT FAR*avail;
   INSTRUMENT FAR*table[INS_SET_DIM];
   INSTRUMENT FAR*array;

   void InsListMgr_construct(void)
      {
      current_size = INS_BLOCK_SIZE;

      array = (INSTRUMENT FAR*) AIL_mem_alloc_lock(sizeof(INSTRUMENT) * (size_t) current_size);

      S32 i;
      for (i=0; i < current_size; i++)
         {
         array[i].prev = &array[i-1];
         array[i].next = &array[i+1];
         array[i].entry_num = -1;
         }

      array[0].prev = NULL;
      array[current_size-1].next = NULL;

      used = NULL;
      avail = &array[0];

      for (i=0; i < INS_SET_DIM; i++) table[i] = NULL;
      }

   ~InsListMgr(void)
      {
      cleanup();
      }

   void cleanup(void)
      {
      if (array)
         {
		   while (used) dealloc(used);

		   current_size = 0;
		   AIL_mem_free_lock(array);
		   array = NULL;
         }
      }

   INSTRUMENT FAR *lookup(void FAR *seed)
      {
      INSTRUMENT FAR *found = table[INSTRUMENT::set(seed)];

      while (found)
         {
         if (found->is_match(seed))
            break;

         found = found->next_in_set;
         }

      return found;
      }

   INSTRUMENT FAR *alloc(void const FAR *seed=NULL)
      {
      U32 key = INSTRUMENT::set(seed);
      INSTRUMENT FAR *entry;

      if (avail == NULL)
         {
         S32 prev_size = current_size;
         S32 new_size = prev_size + INS_BLOCK_SIZE;

         INSTRUMENT FAR *old_array = array;
         INSTRUMENT FAR *new_array = (INSTRUMENT FAR *) AIL_mem_alloc_lock(sizeof(INSTRUMENT) * (size_t) new_size);
         if (new_array == NULL) return NULL;

         U32 adjust = ((U32) new_array) - ((U32) old_array);

         S32 i;
         
         for (i=0; i < prev_size; i++)
            {
            new_array[i] = old_array[i];

            if (new_array[i].next_in_set) new_array[i].next_in_set = (INSTRUMENT FAR *) (((U32) new_array[i].next_in_set) + adjust);
            if (new_array[i].prev_in_set) new_array[i].prev_in_set = (INSTRUMENT FAR *) (((U32) new_array[i].prev_in_set) + adjust);

            if (new_array[i].next) new_array[i].next = (INSTRUMENT FAR *) (((U32) new_array[i].next) + adjust);
            if (new_array[i].prev) new_array[i].prev = (INSTRUMENT FAR *) (((U32) new_array[i].prev) + adjust);
            }

         for (i=0; i < INS_SET_DIM; i++)
            if (table[i]) table[i] = (INSTRUMENT FAR *) (((U32) table[i]) + adjust);

         for (i=prev_size; i < new_size; i++)
            {
            new_array[i].entry_num = -1;
            new_array[i].prev = &new_array[i-1];
            new_array[i].next = &new_array[i+1];
            }

         new_array[prev_size].prev = NULL;
         new_array[new_size-1].next = NULL;
         used = (INSTRUMENT FAR *) (((U32) used) + adjust);
         avail = &new_array[prev_size];
         current_size = new_size;

         AIL_mem_free_lock(array);
         array = new_array;
         }

      entry = avail;
      avail = entry->next;
      entry->next = used;
      entry->prev = NULL;

      if (used) used->prev = entry;
      used = entry;

      entry->set_key = key;
      entry->next_in_set = table[key];
      entry->prev_in_set = NULL;

      if (table[key])
         table[key]->prev_in_set = entry;

      table[key] = entry;
      entry->entry_num = (S32) ((((U32)(entry)) - ((U32)(array))) / sizeof(INSTRUMENT));

      entry->init(seed);

      return entry;
      }

   void dealloc(INSTRUMENT FAR *entry)
      {
      AIL_lock();
      entry->cleanup();

      if (entry->next_in_set) entry->next_in_set->prev_in_set = entry->prev_in_set;
      if (entry->prev_in_set) entry->prev_in_set->next_in_set = entry->next_in_set;

      entry->entry_num = -1;

      if (entry->next) entry->next->prev = entry->prev;
      if (entry->prev) entry->prev->next = entry->next;

      if (used == entry) used = entry->next;

      if (table[entry->set_key] == entry)
         table[entry->set_key] = entry->next_in_set;

      entry->next = avail;
      entry->prev = NULL;

      if (avail) avail->prev = entry;
      avail = entry;
      AIL_unlock();
      }

   void dealloc(S32 entry_num)
      {
      if ((entry_num != -1) && (array[entry_num].entry_num != -1)) dealloc(&array[entry_num]);
      }
};

extern InsListMgr FAR *instrument_list;

struct DLS_FILE
{
   DLS_FILE FAR *next;
   DLS_FILE FAR *prev;
   DLS_FILE FAR *next_in_set;
   DLS_FILE FAR *prev_in_set;
   S32       set_key;
   S32       entry_num;

   void init(void const FAR *data)
      {
      data=data;
      filename[0] = 0;
      image       = NULL;
      ptbl        = NULL;
      cues        = NULL;
      WAVE_list   = NULL;
      }

   void cleanup(void)
      {
      //
      // Terminate any voices which use an instrument defined in this file,
      // before freeing file resources
      //

      S32 killed = 0;

      if (DLS) {
      
        for (S32 vn=0; vn < DLS->n_voices; vn++)
           {
           VOICE FAR *V = &DLS->voice_list[vn];

           if ((V == NULL) || (!V->active))
              {
              continue;
              }

           if (instrument_list->array[V->trigger.instrument_index].DLS_file == entry_num)
              {
              V->active = FALSE;
              ++killed;
              }
           }
      }

      //
      // If we terminated any voices, pause briefly to ensure
      // any background timer tick in progress has a chance to finish
      // executing before freeing instrument data
      //

      if (killed)
         {
         AIL_delay(1);
         }

      //
      // Free all instrument entries which refer to file
      //

      INSTRUMENT FAR *ptr = instrument_list->used;

      while (ptr != NULL)
         {
         INSTRUMENT FAR *next = ptr->next;

         if (ptr->DLS_file == entry_num)
            {
            instrument_list->dealloc(ptr);
            }

         ptr = next;
         }

      //
      // Free cloned selectors
      //

      if (ptbl != NULL)
         {
         AIL_ptr_free_clone(ptbl);
         ptbl = NULL;
         }

      if (cues != NULL)
         {
         AIL_ptr_free_clone(cues);
         cues = NULL;
         }

      //
      // Free file image and other data
      //

      if (image != NULL)
         {
         //
         // Free file data image only if we loaded it (as opposed to the
         // app having passed it to us with DLSLoadMemFile)
         //

         if (AIL_strcmp(filename,"(Memory file)"))
            {
            AIL_mem_free_lock((void FAR*)image);
            image = NULL;
            }
         }

      if (WAVE_list != NULL)
         {
         AIL_mem_free_lock(WAVE_list);
         WAVE_list = NULL;
         }

      }

   S32 is_match(void const FAR *data)
      {
      data=data;
      return 0;
      }

   static U32 set(void const FAR *data)
      {
      data=data;
      return 0;
      }

   //
   // File data
   //

   C8        filename[256];
   void const FAR *image;

   //
   // COLH data
   //

   S32 cInstruments;

   //
   // PTBL data
   //

   POOLTABLE FAR *ptbl;
   POOLCUE   FAR *cues;

   //
   // WAVE data
   //

   WAVEDESC FAR *WAVE_list;
};

//
// File list manager
//

#define FILE_SET_DIM    1
#define FILE_BLOCK_SIZE 4

class FileListMgr
{
public:
   S32 current_size;    
   DLS_FILE FAR*used;           
   DLS_FILE FAR*avail;
   DLS_FILE FAR*table[FILE_SET_DIM];
   DLS_FILE FAR*array;

   void FileListMgr_construct(void)
      {
      current_size = FILE_BLOCK_SIZE;

      array = (DLS_FILE FAR *) AIL_mem_alloc_lock(sizeof(DLS_FILE) * (size_t) current_size);

      S32 i;
      for (i=0; i < current_size; i++)
         {
         array[i].prev = &array[i-1];
         array[i].next = &array[i+1];
         array[i].entry_num = -1;
         }

      array[0].prev = NULL;
      array[current_size-1].next = NULL;

      used = NULL;
      avail = &array[0];

      for (i=0; i < FILE_SET_DIM; i++) table[i] = NULL;
      }

   ~FileListMgr(void)
      {
      cleanup();
      }

   void cleanup(void)
      {
      if (array)
         {
		   while (used) dealloc(used);

		   current_size = 0;
		   AIL_mem_free_lock(array);
		   array = NULL;
         }
      }

   DLS_FILE FAR *lookup(void const FAR *seed)
      {
      DLS_FILE FAR *found = table[DLS_FILE::set(seed)];

      while (found)
         {
         if (found->is_match(seed))
            break;

         found = found->next_in_set;
         }

      return found;
      }

   DLS_FILE FAR *alloc(void const FAR *seed=NULL)
      {
      U32 key = DLS_FILE::set(seed);
      DLS_FILE FAR *entry;

      if (avail == NULL)
         {
         S32 prev_size = current_size;
         S32 new_size = prev_size + FILE_BLOCK_SIZE;

         DLS_FILE FAR *old_array = array;
         DLS_FILE FAR *new_array = (DLS_FILE FAR *) AIL_mem_alloc_lock(sizeof(DLS_FILE) * (size_t) new_size);
         if (new_array == NULL) return NULL;

         U32 adjust = ((U32) new_array) - ((U32) old_array);

         S32 i;
         for (i=0; i < prev_size; i++)
            {
            new_array[i] = old_array[i];

            if (new_array[i].next_in_set) new_array[i].next_in_set = (DLS_FILE FAR *) (((U32) new_array[i].next_in_set) + adjust);
            if (new_array[i].prev_in_set) new_array[i].prev_in_set = (DLS_FILE FAR *) (((U32) new_array[i].prev_in_set) + adjust);

            if (new_array[i].next) new_array[i].next = (DLS_FILE FAR *) (((U32) new_array[i].next) + adjust);
            if (new_array[i].prev) new_array[i].prev = (DLS_FILE FAR *) (((U32) new_array[i].prev) + adjust);
            }

         for (i=0; i < FILE_SET_DIM; i++)
            if (table[i]) table[i] = (DLS_FILE FAR *) (((U32) table[i]) + adjust);

         for (i=prev_size; i < new_size; i++)
            {
            new_array[i].entry_num = -1;
            new_array[i].prev = &new_array[i-1];
            new_array[i].next = &new_array[i+1];
            }

         new_array[prev_size].prev = NULL;
         new_array[new_size-1].next = NULL;
         used = (DLS_FILE FAR *) (((U32) used) + adjust);
         avail = &new_array[prev_size];
         current_size = new_size;

         AIL_mem_free_lock(array);
         array = new_array;
         }

      entry = avail;
      avail = entry->next;
      entry->next = used;
      entry->prev = NULL;

      if (used) used->prev = entry;
      used = entry;

      entry->set_key = key;
      entry->next_in_set = table[key];
      entry->prev_in_set = NULL;

      if (table[key])
         table[key]->prev_in_set = entry;

      table[key] = entry;
      entry->entry_num = (S32) ((((U32) entry) - ((U32) array)) / sizeof(DLS_FILE));

      entry->init(seed);

      return entry;
      }

   void dealloc(DLS_FILE FAR *entry)
      {
      entry->cleanup();

      if (entry->next_in_set) entry->next_in_set->prev_in_set = entry->prev_in_set;
      if (entry->prev_in_set) entry->prev_in_set->next_in_set = entry->next_in_set;

      entry->entry_num = -1;

      if (entry->next) entry->next->prev = entry->prev;
      if (entry->prev) entry->prev->next = entry->next;

      if (used == entry) used = entry->next;

      if (table[entry->set_key] == entry) 
         table[entry->set_key] = entry->next_in_set;

      entry->next = avail;
      entry->prev = NULL;

      if (avail) avail->prev = entry;
      avail = entry;
      }

   void dealloc(S32 entry_num)
      {
      if ((entry_num != -1) && (array[entry_num].entry_num != -1)) dealloc(&array[entry_num]);
      }
};

extern FileListMgr FAR *file_list;

extern S32 InstrumentsInit();
extern void InstrumentsDeinit();

extern S32 DLSFILE_parse(void const FAR *data, DLS_FILE FAR * FAR *file_var, C8 const FAR *lpFileName, U32 flags);

extern S32 DLS_init     (S32 FAR   *lpdwHandle,
                         S32        dwFlags,
                         HMDIDRIVER MIDI_driver,
                         S32        output_format,
                         S32        output_sample_rate,
                         S32        buffer_size,
                         U32        user,
                         AILDLSPCB  buffer_poll_CB,
                         AILDLSLCB  buffer_lock_CB,
                         AILDLSUCB  buffer_unlock_CB);

extern S32 DLS_shutdown (S32        dwDLSHandle,
                         S32        dwFlags);

#pragma pack()

#endif

