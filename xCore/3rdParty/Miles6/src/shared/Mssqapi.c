//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  MSSQAPI.C: Quick-integration API module for MSS 3.5                   ##
//##                                                                        ##
//##  Flat-model source compatible with MS VC 9.0                           ##
//##                                                                        ##
//##  Version 1.00 of 20-Feb-96: Initial                                    ##
//##                                                                        ##
//##  Author: John Miles and Jeff Roberts                                   ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#include "mss.h"
#include "imssapi.h"


//
// .WAV file header
//

typedef struct
{
   S8    RIFF_string[4];
   U32   chunk_size;
   S8    ID_string[4];
   U8    data[1];
}
RIFF;


static HMDIDRIVER mdi;
static HDIGDRIVER dig;
static HDLSDEVICE dls;
static HAUDIO audios=0;
static U32 didastartup=0;


#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## Locked code                                                            ##
//##                                                                        ##
//############################################################################

#define LOCK(x)   AIL_vmm_lock  (&(x),sizeof(x))
#define UNLOCK(x) AIL_vmm_unlock(&(x),sizeof(x))

static S32 locked = 0;

void AILQAPI_end(void);

void AILQAPI_start(void)
{
   if (!locked)
      {
      AIL_vmm_lock_range(AILQAPI_start, AILQAPI_end);

      LOCK(mdi);
      LOCK(dig);
      LOCK(dls);
      LOCK(audios);
      LOCK(didastartup);

      locked = 1;
      }
}

#define DOLOCK() AILQAPI_start()

#else

#define DOLOCK()

#endif

//############################################################################
//##                                                                        ##
//## Callback functions to release sample/sequence handle immediately upon  ##
//## completion of playback                                                 ##
//##                                                                        ##
//############################################################################

static void AILLIBCALLBACK end_of_sequence(HSEQUENCE S)
{
   //
   // Release sequence handle to make it available for reuse
   //

   ((HAUDIO) AIL_sequence_user_data(S,0))->handle = NULL;
   ((HAUDIO) AIL_sequence_user_data(S,0))->status = QSTAT_DONE;

   AIL_release_sequence_handle(S);
}

static void AILLIBCALLBACK end_of_sample(HSAMPLE S)
{
   //
   // Release sample handle to make it available for reuse
   //

   ((HAUDIO) AIL_sample_user_data(S,0))->handle = NULL;
   ((HAUDIO) AIL_sample_user_data(S,0))->status = QSTAT_DONE;

   AIL_release_sample_handle(S);
}

//############################################################################
//##                                                                        ##
//## Quick startup of all AIL resources                                     ##
//##                                                                        ##
//############################################################################

#ifdef IS_DOS

typedef S32 (cdecl  *AIL_startup_type)(void);

S32 AILCALL AIL_quick_startup_with_start(void* startup,
#else
S32 AILCALL AIL_API_quick_startup(
#endif
                          S32 use_digital, S32 use_MIDI,
                          U32         output_rate,
                          S32         output_bits,
                          S32         output_channels)

{
#ifdef IS_WINDOWS
   static PCMWAVEFORMAT sPCMWF;
#endif

   //
   // fail if already started
   //
   if (didastartup)
     return(0);

   //
   // Fail if neither digital nor MIDI service requested
   //

   if (!(use_digital || use_MIDI))
      {
      return 0;
      }

   //
   // Start up AIL API
   //

#ifdef IS_DOS
   if (((AIL_startup_type)startup)())
#else
   if (AIL_startup())
#endif
     didastartup=2;
   else
     didastartup=1;

   DOLOCK();

   //
   // Init globals
   //

   mdi=0;
   dig=0;
   dls=0;

   if (use_digital)
   {
#ifdef IS_WIN32
     if (use_digital==AIL_QUICK_USE_WAVEOUT)
       AIL_set_preference(DIG_USE_WAVEOUT,TRUE);
#endif
     dig=AIL_open_digital_driver( output_rate,output_bits,output_channels,
#ifdef IS_WIN32
                                  (use_digital==AIL_QUICK_USE_WAVEOUT)?AIL_OPEN_DIGITAL_FORCE_PREFERENCE:0 );
#else
                                  0 );
#endif

     if (dig==0)
     {
       if (didastartup==2)
         AIL_shutdown();
       didastartup=0;
       return(0);
     }
   }

   if (use_MIDI)
   {
     mdi=AIL_open_XMIDI_driver( ((use_MIDI&255)==AIL_QUICK_DLS_ONLY)?AIL_OPEN_XMIDI_NULL_DRIVER:0 );
     if (mdi==0)
     {
      midierr:
       if (dig)
       {
         AIL_close_digital_driver(dig);
         dig=0;
       }
       if (didastartup==2)
         AIL_shutdown();
       didastartup=0;
       return 0;
     }

     if (((use_MIDI&255)==AIL_QUICK_MIDI_AND_VORTEX_DLS) ||
         ((use_MIDI&255)==AIL_QUICK_MIDI_AND_SONICVIBES_DLS) ||
         ((use_MIDI&255)==AIL_QUICK_MIDI_AND_DLS) ||
         ((use_MIDI&255)==AIL_QUICK_DLS_ONLY))
     {
#ifdef IS_WINDOWS
       char FAR* lib;

       if ((use_MIDI&255)==AIL_QUICK_MIDI_AND_VORTEX_DLS)
         lib="vort_dls.dll";
       else if ((use_MIDI&255)==AIL_QUICK_MIDI_AND_SONICVIBES_DLS)
         lib="s3base.dll";
       else
        lib=0;
#endif
       dls=AIL_DLS_open(mdi,dig,
#ifdef IS_WINDOWS
           lib
#else
           0
#endif
          ,0,output_rate,output_bits,output_channels);

        if (dls==0)
        {
          AIL_close_XMIDI_driver(mdi);
          mdi=0;
          goto midierr;
        }
     }
   }

   //
   // Return success
   //

   return 1;
}

//############################################################################
//##                                                                        ##
//## Quick shutdown of all AIL resources                                    ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_quick_shutdown(void)
{
   if (didastartup) {

     if ((mdi) || (dig)) {

        while (audios)               // free all audio samples
          AIL_quick_unload(audios);

        if (dls) {
          AIL_DLS_close(dls,0);
          dls=0;
        }

        if (mdi) {
          AIL_close_XMIDI_driver( mdi );
          mdi=0;
        }

        if (dig) {
          AIL_close_digital_driver( dig );
          dig=0;
        }
        if (didastartup==2)
          AIL_shutdown();

        didastartup=0;
     }

   }
}

//############################################################################
//##                                                                        ##
//## Return the current digital and midi handles                            ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_quick_handles(HDIGDRIVER FAR* pdig,HMDIDRIVER FAR* pmdi,HDLSDEVICE FAR* pdls)
{
  if (pdig)
    *pdig=dig;

  if (pmdi)
    *pmdi=mdi;

  if (pdls)
    *pdls=dls;
}

//############################################################################
//##                                                                        ##
//## Internal function that sets up the quick audio handle                  ##
//##                                                                        ##
//############################################################################

static HAUDIO setupaudio(HAUDIO audio)
{
   S32 type;

   audio->handle=0;

   if (audio->data == NULL)
      {
      AIL_quick_unload(audio);

      return NULL;
      }

   //
   // Identify audio data type
   //

   type=AIL_file_type(audio->data,audio->size);

   switch (type) {
     case AILFILETYPE_PCM_WAV:
     case AILFILETYPE_ADPCM_WAV:
     case AILFILETYPE_VOC:
       audio->type = AIL_QUICK_DIGITAL_TYPE;
       break;

     case AILFILETYPE_MPEG_L3_AUDIO:
       audio->type = AIL_QUICK_MPEG_DIGITAL_TYPE;
       break;

     case AILFILETYPE_MIDI:
     {
       void FAR* xmidi;
       U32 xmidi_size;

       if (AIL_MIDI_to_XMI(audio->data,audio->size,&xmidi,&xmidi_size,AILMIDITOXMI_TOLERANT)) {

         if (!audio->userdata)
           AIL_mem_free_lock((void FAR*)audio->data);
         audio->data=xmidi;
         audio->size=xmidi_size;
         // fall though to the next case

       } else {
         AIL_quick_unload(audio);
         return(0);
       }
     }

     case AILFILETYPE_XMIDI:
     case AILFILETYPE_XMIDI_DLS:
     case AILFILETYPE_XMIDI_MLS:
       audio->type = AIL_QUICK_XMIDI_TYPE;
       break;

     default:

       //
       // Unknown data type, fail call
       //

       AIL_quick_unload(audio);

       AIL_set_error("Unknown file type.");
       return NULL;

   }

   //
   // Fail if no driver available to play this data type
   //

   if (audio->type == AIL_QUICK_XMIDI_TYPE) {

      if (mdi == NULL)
        {
        AIL_quick_unload(audio);

        AIL_set_error("XMIDI support not enabled.");
        return NULL;
        }

      //
      // check for DLS
      //

      if ((dls) && (audio->dlsmem==0) && (audio->dlsmemunc==0) && ((type==AILFILETYPE_XMIDI_DLS) || (type==AILFILETYPE_XMIDI_MLS))) {

        if (type==AILFILETYPE_XMIDI_MLS) {

          // uncompress DLS data

          AIL_extract_DLS(audio->data,audio->size,0,0,&audio->dlsmem,0,0);

          if (audio->dlsmem) {

            void FAR* ptr;

            if (!audio->userdata) {

              AIL_extract_DLS(audio->data,audio->size,&ptr,0,0,0,0);
              AIL_mem_free_lock((void FAR*)audio->data);
              audio->data=ptr;

            }

          }

        } else {

          // find DLS data

          AIL_find_DLS(audio->data,audio->size,0,0,&audio->dlsmemunc,0);

        }

        if (audio->dlsmem)
          audio->dlsid=AIL_DLS_load_memory(dls,audio->dlsmem,0);
        else if (audio->dlsmemunc)
          audio->dlsid=AIL_DLS_load_memory(dls,audio->dlsmemunc,0);

        AIL_set_preference(DLS_GM_PASSTHROUGH,0);

     } else
       AIL_set_preference(DLS_GM_PASSTHROUGH,1);

   }

   if (((audio->type == AIL_QUICK_DIGITAL_TYPE) || (audio->type == AIL_QUICK_MPEG_DIGITAL_TYPE)) && (dig == NULL))
      {
      AIL_quick_unload(audio);

      AIL_set_error("Digital support not enabled.");
      return NULL;
      }

   //
   // Return success
   //

   audio->status = QSTAT_LOADED;
   audio->speed=-1;
   audio->volume=-1;
   audio->rlevel=-1.0F;
   audio->extravol=-1;

   audio->next = audios;      // insert into linked list
   audios = audio;

   return audio;
}

//############################################################################
//##                                                                        ##
//## Quick load of any audio data type                                      ##
//##                                                                        ##
//############################################################################

HAUDIO AILCALL AIL_API_quick_load(char const FAR*filename)
{
   HAUDIO audio;

   //
   // Allocate memory for AUDIO_TYPE structure
   //

   audio = AIL_mem_alloc_lock(sizeof(AUDIO_TYPE));

   if (audio == NULL)
      {

      return NULL;
      }

   AIL_memset(audio,0,sizeof(AUDIO_TYPE));

   //
   // Load requested file from disk
   //

   audio->data = AIL_file_read(filename, NULL);
   audio->size = AIL_file_size(filename);

   return( setupaudio(audio) );

}

//############################################################################
//##                                                                        ##
//## Quick load of any audio data type                                      ##
//##                                                                        ##
//############################################################################

#ifdef IS_MAC

HAUDIO AILCALL AIL_API_quick_fss_load(FSSpec const FAR*filename)
{
   HAUDIO audio;

   //
   // Allocate memory for AUDIO_TYPE structure
   //

   audio = AIL_mem_alloc_lock(sizeof(AUDIO_TYPE));

   if (audio == NULL)
      {

      return NULL;
      }

   AIL_memset(audio,0,sizeof(AUDIO_TYPE));

   //
   // Load requested file from disk
   //

   audio->data = AIL_file_fss_read(filename, NULL);
   audio->size = AIL_file_fss_size(filename);

   return( setupaudio(audio) );

}

#endif

//############################################################################
//##                                                                        ##
//## Quick load of any audio data type                                      ##
//##                                                                        ##
//############################################################################

HAUDIO AILCALL AIL_API_quick_load_mem(void const FAR*mem,U32 size)
{
   HAUDIO audio;

   if (mem==0)
     return(0);

   //
   // Allocate memory for AUDIO_TYPE structure
   //

   audio = AIL_mem_alloc_lock(sizeof(AUDIO_TYPE));

   if (audio == NULL)
      {

      return NULL;
      }

   AIL_memset(audio,0,sizeof(AUDIO_TYPE));

   //
   // Load requested file from disk
   //

   audio->data = mem;
   audio->size = size;
   audio->userdata=1;

   return( setupaudio(audio) );

}

//############################################################################
//##                                                                        ##
//## Quick copy of an audio handle                                          ##
//##                                                                        ##
//############################################################################

HAUDIO AILCALL AIL_API_quick_copy(HAUDIO hand)
{
   HAUDIO audio;

   //
   // Allocate memory for AUDIO_TYPE structure
   //

   audio = AIL_mem_alloc_lock(sizeof(AUDIO_TYPE));

   if (audio == NULL)
      {

      return NULL;
      }

   AIL_memset(audio,0,sizeof(AUDIO_TYPE));

   //
   // Load requested file from other handle
   //

   audio->data = hand->data;
   audio->size = hand->size;
   audio->userdata=hand->userdata;
   audio->dlsmem=hand->dlsmem;
   audio->dlsmemunc=hand->dlsmemunc;
   audio->dlsid=hand->dlsid;

   return( setupaudio(audio));
}


//############################################################################
//##                                                                        ##
//## Quick "unload" of any audio data type                                  ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_quick_unload(HAUDIO audio)
{
   HAUDIO a;

   if (audio == NULL)
      {
      return;
      }

   //
   // If a handle has been assigned to this audio data, be sure the
   // sound or music has stopped before freeing the data block
   //
   // (Calls EOS callback if sequence/sample not already done)
   //

   AIL_lock();

   if (audio->handle != NULL)
      {
      switch (audio->type)
         {
         case AIL_QUICK_XMIDI_TYPE:

            AIL_end_sequence((HSEQUENCE) audio->handle);
            break;

         case AIL_QUICK_DIGITAL_TYPE:
         case AIL_QUICK_MPEG_DIGITAL_TYPE:

            AIL_end_sample((HSAMPLE) audio->handle);
            break;
         }

      audio->handle = NULL;
      }

   AIL_unlock();

   //
   // Release audio data block
   //

   if (audio->data != NULL)
      {

      a=(HAUDIO)audios;
      while (a) {
        if ((a->data==audio->data) && (a!=audio))   // if this is a copied buffer
          goto nofree;

        a=(HAUDIO)a->next;
      }

      if (!audio->userdata)
        AIL_mem_free_lock((void FAR*)audio->data);

      if (audio->dlsid)
        AIL_DLS_unload(dls, audio->dlsid);

      if (audio->dlsmem)
        AIL_mem_free_lock(audio->dlsmem);

nofree:

      audio->data = NULL;

      }


   //
   // Remove audio from linked list
   //

   if (audio==audios)
     audios=(HAUDIO)audio->next;
   else {
     a=(HAUDIO)audios;
     while (a) {
       if (audio==(HAUDIO)a->next) {
         a->next=((HAUDIO)a->next)->next;
         break;
       }
       a=(HAUDIO)a->next;
     }
   }

   //
   // Release HAUDIO structure itself
   //

   AIL_mem_free_lock(audio);
}

//############################################################################
//##                                                                        ##
//## Quick "play" of loaded audio data                                      ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_quick_play(HAUDIO audio, U32 loop_count)
{
   if (audio == NULL)
      {
      return 0;
      }

   //
   // If this HAUDIO already has a sample or sequence handle, stop it
   //

   if (audio->handle != NULL)
      {
      AIL_quick_halt(audio);
      }

   //
   // Play the data...
   //

   switch (audio->type)
      {
      //
      // .XMI
      //

      case AIL_QUICK_XMIDI_TYPE:

         //
         // Allocate and init a sequence handle
         //

         audio->handle = AIL_allocate_sequence_handle(mdi);

         if (audio->handle == NULL)
            {
            return 0;
            }

         if (AIL_init_sequence((HSEQUENCE) audio->handle,
                                           audio->data,
                                           0) != 1)
            {
            AIL_release_sequence_handle((HSEQUENCE) audio->handle);
            return 0;
            }

         //
         // Set up to release sequence handle when song ends for any reason
         // (either expiration or explicit AIL_end_sequence() call)
         //

         AIL_set_sequence_user_data((HSEQUENCE) audio->handle,
                                                0,
                                        (U32) audio);

         AIL_register_sequence_callback((HSEQUENCE) audio->handle,
                                         end_of_sequence);

         //
         // Set the playback tempo
         //

         if (audio->speed!=-1)
           AIL_set_sequence_tempo((HSEQUENCE)audio->handle,audio->speed,0);


         //
         // Set the volume
         //

         if (audio->volume!=-1)
           AIL_set_sequence_volume((HSEQUENCE)audio->handle,audio->volume,(audio->extravol==-1)?0:audio->extravol);

         //
         // Set the reverb
         //

         if ((audio->rlevel!=-1.0F) && (dls))
           AIL_DLS_set_reverb(dls,audio->rlevel,audio->rrtime,audio->rdtime);

         //
         // Set requested loop count
         //

         AIL_set_sequence_loop_count((HSEQUENCE) audio->handle,
                                                 loop_count);

         //
         // Start sequence
         //

         audio->status = QSTAT_PLAYING;

         if (audio->milliseconds) {

           AIL_set_sequence_ms_position((HSEQUENCE)audio->handle,audio->milliseconds);
           audio->milliseconds=0;
           AIL_resume_sequence((HSEQUENCE) audio->handle);

         } else

           AIL_start_sequence((HSEQUENCE) audio->handle);

         return 1;

      //
      // .VOC / .WAV / ASI types
      //

      case AIL_QUICK_DIGITAL_TYPE:
      case AIL_QUICK_MPEG_DIGITAL_TYPE:

         //
         // Allocate and init a sample handle
         //

         audio->handle = AIL_allocate_sample_handle(dig);

         if (audio->handle == NULL)
            {
            return 0;
            }

         AIL_init_sample((HSAMPLE) audio->handle);

         if (audio->type == AIL_QUICK_DIGITAL_TYPE)
            {
            if (!AIL_set_sample_file((HSAMPLE) audio->handle,
                                               audio->data,
                                              -1))
               {
               AIL_release_sample_handle((HSAMPLE) audio->handle);
               return 0;
               }
            }
         else
            {
            //
            // Otherwise, MP3
            //

            if (!AIL_set_named_sample_file ((HSAMPLE) audio->handle,
                                            ".MP3",
                                            audio->data,
                                            audio->size,
                                            -1))
               {
               AIL_release_sample_handle((HSAMPLE) audio->handle);
               return 0;
               }
            }

         //
         // Set up to release sample handle when sample ends for any reason
         // (either expiration or explicit AIL_end_sample() call)
         //

         AIL_set_sample_user_data((HSAMPLE) audio->handle,
                                            0,
                                    (U32) audio);

         AIL_register_EOS_callback((HSAMPLE) audio->handle,
                                    end_of_sample);

         //
         // Set the playback rate
         //

         if (audio->speed!=-1)
           AIL_set_sample_playback_rate((HSAMPLE)audio->handle,audio->speed);


         //
         // Set the volume
         //

         if (audio->volume!=-1)
           AIL_set_sample_volume((HSAMPLE)audio->handle,audio->volume);


         //
         // Set the reverb
         //

         if (audio->rlevel!=-1.0F)
           AIL_set_sample_reverb((HSAMPLE)audio->handle,audio->rlevel,audio->rrtime,audio->rdtime);

         //
         // Set the pan
         //

         if (audio->extravol!=-1)
           AIL_set_sample_pan((HSAMPLE)audio->handle,audio->extravol);


         //
         // Set requested loop count
         //

         AIL_set_sample_loop_count((HSAMPLE) audio->handle,
                                             loop_count);

         //
         // Start sample
         //

         audio->status = QSTAT_PLAYING;

         if (audio->milliseconds) {

           AIL_set_sample_ms_position((HSAMPLE)audio->handle,audio->milliseconds);
           audio->milliseconds=0;
           AIL_resume_sample((HSAMPLE) audio->handle);

         } else

           AIL_start_sample((HSAMPLE) audio->handle);

         return 1;
      }

   return 0;
}

//############################################################################
//##                                                                        ##
//## Quick "stop" of loaded audio data                                      ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_quick_halt(HAUDIO audio)
{
   if (audio == NULL)
      {
      return;
      }

   //
   // Calls EOS callback if sequence/sample not already done
   //

   AIL_lock();

   if (audio->handle != NULL)
      {
      switch (audio->type)
         {
         case AIL_QUICK_XMIDI_TYPE:

            AIL_end_sequence((HSEQUENCE) audio->handle);
            break;

         case AIL_QUICK_DIGITAL_TYPE:
         case AIL_QUICK_MPEG_DIGITAL_TYPE:

            AIL_end_sample((HSAMPLE) audio->handle);
            break;
         }

      audio->handle = NULL;
      }

   AIL_unlock();
}


static void do_wait( HAUDIO audio )
{
   U32 tim;

  //
  // (Sequence/sample handle automatically released when playback ends)
  //

  switch (audio->type)
  {
    case AIL_QUICK_XMIDI_TYPE:

      tim=AIL_ms_count();

      while (AIL_sequence_status((HSEQUENCE) audio->handle) == SEQ_PLAYING)
      {
  #ifdef IS_WIN32
        Sleep(1);
  #else
    #ifdef IS_WIN16
        if (AIL_ms_count()>tim)
        {
          tim+=30;
          AIL_serve();
        }
    #endif
  #endif
      }
      break;

     case AIL_QUICK_DIGITAL_TYPE:
     case AIL_QUICK_MPEG_DIGITAL_TYPE:

       tim=AIL_ms_count();

       while (AIL_sample_status((HSAMPLE) audio->handle) == SMP_PLAYING)
       {
  #ifdef IS_WIN32
         Sleep(1);
  #else
    #ifdef IS_WIN16
         if (AIL_ms_count()>tim)
         {
           tim+=30;
           AIL_serve();
         }
    #endif
  #endif
        }
        break;
  }
}

//############################################################################
//##                                                                        ##
//## Quick "load and play" of audio data file                               ##
//##                                                                        ##
//############################################################################

HAUDIO AILCALL AIL_API_quick_load_and_play(char const FAR*filename, //)
                                   U32  loop_count,
                                   S32  wait_request)
{
   HAUDIO audio;

   audio = AIL_quick_load(filename);

   if (audio == NULL)
      {
      return NULL;
      }

   if (!AIL_quick_play(audio,loop_count))
      {
      AIL_quick_unload(audio);
      return NULL;
      }


   if ((wait_request) && (loop_count != 0))
   {
      do_wait( audio );
   }

   return audio;
}

//############################################################################
//##                                                                        ##
//## Quick "load and play" of audio data file                               ##
//##                                                                        ##
//############################################################################

#ifdef IS_MAC

HAUDIO AILCALL AIL_API_quick_fss_load_and_play(FSSpec const FAR*filename, //)
                                   U32  loop_count,
                                   S32  wait_request)
{
   HAUDIO audio;

   audio = AIL_quick_fss_load(filename);

   if (audio == NULL)
      {
      return NULL;
      }

   if (!AIL_quick_play(audio,loop_count))
      {
      AIL_quick_unload(audio);
      return NULL;
      }


   if ((wait_request) && (loop_count != 0))
   {
      do_wait( audio );
   }

   return audio;
}

#endif

//############################################################################
//##                                                                        ##
//## Quick status for audio data                                            ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_quick_status(HAUDIO audio)
{
   if (audio == NULL)
      {
      return QSTAT_DONE;
      }

   return audio->status;
}

//############################################################################
//##                                                                        ##
//## Set speed for a handle - tempo for MIDI, playback rate for digital     ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_quick_set_speed(HAUDIO audio,S32 speed)
{
   if (audio)
      {

        audio->speed=speed;

        if ((audio->speed!=-1) && (AIL_quick_status(audio)==QSTAT_PLAYING))
           {
           switch (audio->type)
              {
              case AIL_QUICK_XMIDI_TYPE:
                 //
                 // Set the playback tempo
                 //

                 AIL_set_sequence_tempo((HSEQUENCE)audio->handle,audio->speed,0);

                 break;
              case AIL_QUICK_DIGITAL_TYPE:
              case AIL_QUICK_MPEG_DIGITAL_TYPE:
                 //
                 // Set the playback rate
                 //

                 AIL_set_sample_playback_rate((HSAMPLE)audio->handle,audio->speed);

                 break;
              }
           }
      }
}

//############################################################################
//##                                                                        ##
//## Set volume a handle- extravol is pan for digital, speed in ms for MIDI ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_quick_set_volume(HAUDIO audio,S32 volume,S32 extravol)
{
   if (audio)
      {

        audio->volume=volume;
        audio->extravol=extravol;

        if (AIL_quick_status(audio)==QSTAT_PLAYING)
           {
           switch (audio->type)
              {
              case AIL_QUICK_XMIDI_TYPE:
                 //
                 // Set the volume
                 //

                 if (audio->volume!=-1)
                    AIL_set_sequence_volume((HSEQUENCE)audio->handle,audio->volume,(audio->extravol==-1)?0:audio->extravol);

                 break;
              case AIL_QUICK_DIGITAL_TYPE:
              case AIL_QUICK_MPEG_DIGITAL_TYPE:
                 //
                 // Set the volume
                 //

                 if (audio->volume!=-1)
                    AIL_set_sample_volume((HSAMPLE)audio->handle,audio->volume);


                 //
                 // Set the pan
                 //

                 if (audio->extravol!=-1)
                    AIL_set_sample_pan((HSAMPLE)audio->handle,audio->extravol);

                 break;
              }
           }
      }
}


//############################################################################
//##                                                                        ##
//## Set reverb for a digital sample or a software synth DLS file           ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_quick_set_reverb     (HAUDIO audio,
                                           F32     reverb_level,
                                           F32     reverb_reflect_time,
                                           F32     reverb_decay_time)

{
   if (audio)
      {

        audio->rlevel=reverb_level;
        audio->rrtime=reverb_reflect_time;
        audio->rdtime=reverb_decay_time;

        if (AIL_quick_status(audio)==QSTAT_PLAYING)
           {
           switch (audio->type)
              {
              case AIL_QUICK_XMIDI_TYPE:
                 //
                 // Set the reverb for DLS
                 //

                 if ((audio->rlevel!=-1.0F) && (dls))
                   AIL_DLS_set_reverb(dls,audio->rlevel,audio->rrtime,audio->rdtime);

                 break;
              case AIL_QUICK_DIGITAL_TYPE:
              case AIL_QUICK_MPEG_DIGITAL_TYPE:
                 //
                 // Set the reverb
                 //

                 AIL_set_sample_reverb((HSAMPLE)audio->handle,audio->rlevel,audio->rrtime,audio->rdtime);

                 break;
              }
           }
      }
}


//############################################################################
//##                                                                        ##
//## Set the position in milliseconds for a handle                          ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_quick_set_ms_position(HAUDIO audio,S32 milliseconds)
{
   if (audio)
      {

        audio->milliseconds=milliseconds;

        if (AIL_quick_status(audio)==QSTAT_PLAYING)
           {
           switch (audio->type)
              {
              case AIL_QUICK_XMIDI_TYPE:
                 //
                 // Set the position
                 //

                 AIL_set_sequence_ms_position((HSEQUENCE)audio->handle,milliseconds);

                 break;
              case AIL_QUICK_DIGITAL_TYPE:
              case AIL_QUICK_MPEG_DIGITAL_TYPE:
                 //
                 // Set the position
                 //

                 AIL_set_sample_ms_position((HSAMPLE)audio->handle,milliseconds);

                 break;
              }
           }
      }
}


//############################################################################
//##                                                                        ##
//## Get the position in milliseconds for a handle                          ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_quick_ms_position(HAUDIO audio)
{
   S32 pos;

   if (audio)
      {

        if (AIL_quick_status(audio)==QSTAT_PLAYING)
           {
           switch (audio->type)
              {
              case AIL_QUICK_XMIDI_TYPE:
                 //
                 // Get the position
                 //

                 AIL_sequence_ms_position((HSEQUENCE)audio->handle,0,&pos);

                 return(pos);

              case AIL_QUICK_DIGITAL_TYPE:
              case AIL_QUICK_MPEG_DIGITAL_TYPE:
                 //
                 // Get the position
                 //

                 AIL_sample_ms_position((HSAMPLE)audio->handle,0,&pos);

                 return(pos);

              }

           } else

             return(audio->milliseconds);

      }
  return(0);
}


//############################################################################
//##                                                                        ##
//## Get the length in milliseconds for a handle                            ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_quick_ms_length(HAUDIO audio)
{
   S32 pos;

   if (audio)
      {

        if (AIL_quick_status(audio)==QSTAT_PLAYING)
           {
           switch (audio->type)
              {
              case AIL_QUICK_XMIDI_TYPE:
                 //
                 // Get the position
                 //

                 AIL_sequence_ms_position((HSEQUENCE)audio->handle,&pos,0);

                 return(pos);

              case AIL_QUICK_DIGITAL_TYPE:
              case AIL_QUICK_MPEG_DIGITAL_TYPE:
                 //
                 // Get the position
                 //

                 AIL_sample_ms_position((HSAMPLE)audio->handle,&pos,0);

                 return(pos);
              }

           }
           else
           {
             switch (audio->type)
                {
                //
                // .XMI
                //

                case AIL_QUICK_XMIDI_TYPE:

                   //
                   // Allocate and init a sequence handle
                   //

                   audio->handle = AIL_allocate_sequence_handle(mdi);

                   if (audio->handle == NULL)
                      {
                      return 0;
                      }

                   if (AIL_init_sequence((HSEQUENCE) audio->handle,
                                                     audio->data,
                                                     0) != 1)
                      {
                      AIL_release_sequence_handle((HSEQUENCE) audio->handle);
                      return 0;
                      }

                   AIL_sequence_ms_position((HSEQUENCE)audio->handle,&pos,0);

                   AIL_release_sequence_handle((HSEQUENCE) audio->handle);
                   audio->handle=0;

                   return(pos);

                //
                // .VOC / .WAV / ASI types
                //

                case AIL_QUICK_DIGITAL_TYPE:
                case AIL_QUICK_MPEG_DIGITAL_TYPE:

                   //
                   // Allocate and init a sample handle
                   //

                   audio->handle = AIL_allocate_sample_handle(dig);

                   if (audio->handle == NULL)
                      {
                      return 0;
                      }

                   AIL_init_sample((HSAMPLE) audio->handle);

                   if (audio->type == AIL_QUICK_DIGITAL_TYPE)
                      {
                      if (!AIL_set_sample_file((HSAMPLE) audio->handle,
                                                         audio->data,
                                                        -1))
                         {
                         AIL_release_sample_handle((HSAMPLE) audio->handle);
                         return 0;
                         }
                      }
                   else
                      {
                      //
                      // Otherwise, MP3
                      //

                      if (!AIL_set_named_sample_file ((HSAMPLE) audio->handle,
                                                      ".MP3",
                                                      audio->data,
                                                      audio->size,
                                                      -1))
                         {
                         AIL_release_sample_handle((HSAMPLE) audio->handle);
                         return 0;
                         }
                      }

                   AIL_sample_ms_position((HSAMPLE)audio->handle,&pos,0);

                   AIL_release_sample_handle((HSAMPLE) audio->handle);

                   return(pos);

                }
           }

      }
  return(0);
}

//############################################################################
//##                                                                        ##
//## Get the type for a handle                                              ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_quick_type(HAUDIO audio)
{
   if (audio)
      {

        if ((audio->type==AIL_QUICK_XMIDI_TYPE) && (audio->dlsid))
          return( AIL_QUICK_DLS_XMIDI_TYPE );

        return( audio->type );

      }
  return(0);
}


#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## End of locked code                                                     ##
//##                                                                        ##
//############################################################################

void AILQAPI_end(void)
{
   if (locked)
      {
      AIL_vmm_unlock_range(AILQAPI_start, AILQAPI_end);

      locked = 0;
      }
}

#endif
