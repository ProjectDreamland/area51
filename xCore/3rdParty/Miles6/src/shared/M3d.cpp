//############################################################################
//##                                                                        ##
//##  M3D.CPP: Wrapper for 3D abstraction providers                         ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 25-Aug-98: Initial                                    ##
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

#include <math.h>

#include "rib.h"
#include "mss.h"
#include "imssapi.h"

//############################################################################
//#                                                                          #
//# AIL_API_enumerate_3D_providers                                           #
//#                                                                          #
//############################################################################

S32        AILCALL AIL_API_enumerate_3D_providers  (HPROENUM  FAR *next, //)
                                                    HPROVIDER FAR *dest,
                                                    C8  FAR * FAR *name)
{
   HATTRIB                  PROVNAME;
   PROVIDER_QUERY_ATTRIBUTE PROVIDER_query_attribute = NULL;

   RIB_INTERFACE_ENTRY M3D[] = 
      { 
      AT("Name",PROVNAME),
      FN(PROVIDER_query_attribute)
      };

   if (RIB_enumerate_providers("MSS 3D audio services",
                               next,
                               dest))
      {
      RIB_request(*dest,"MSS 3D audio services",M3D);

      *name = (C8 FAR *) PROVIDER_query_attribute(PROVNAME);

      return 1;
      }

   return 0;
}

//############################################################################
//#                                                                          #
//# AIL_API_open_3D_provider                                                 #
//#                                                                          #
//############################################################################

M3DRESULT  AILCALL AIL_API_open_3D_provider        (HPROVIDER provider)
{
   //
   // Allocate our provider descriptor
   //

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) AIL_mem_alloc_lock(sizeof(M3DPROVIDER));

   if (M3D == NULL)
      {
      return M3D_OUT_OF_MEM;
      }

   AIL_memset(M3D, 0, sizeof(M3DPROVIDER));

   //
   // Log address in provider's system data element at index 0
   //

   RIB_set_provider_system_data(provider,
                                0,
                          (S32) M3D);

   //
   // Fill in provider members
   //

   RIB_INTERFACE_ENTRY sample_request[] =
      {
      { RIB_FUNCTION, "M3D_3D_sample_query_attribute",(U32) &M3D->sample_query_attribute   ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_set_sample_preference", (U32) &M3D->set_sample_preference    ,RIB_NONE },

      { RIB_FUNCTION, "M3D_3D_sample_cone",           (U32) &M3D->sample_cone              ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_sample_cone",       (U32) &M3D->set_3D_sample_cone       ,RIB_NONE },

      { RIB_FUNCTION, "M3D_set_3D_EOS",               (U32) &M3D->set_3D_EOS               ,RIB_NONE },

      //
      // New functions added for EAX3/A3D2 abstraction
      //

      { RIB_FUNCTION, "M3D_3D_sample_obstruction",       (U32) &M3D->sample_obstruction          ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_sample_occlusion",         (U32) &M3D->sample_occlusion            ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_sample_effects_level",     (U32) &M3D->sample_effects_level        ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_sample_obstruction",   (U32) &M3D->set_3D_sample_obstruction   ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_sample_occlusion",     (U32) &M3D->set_3D_sample_occlusion     ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_sample_effects_level", (U32) &M3D->set_3D_sample_effects_level ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_room_type",            (U32) &M3D->set_3D_room_type            ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_room_type",                (U32) &M3D->room_type                   ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_speaker_type",         (U32) &M3D->set_3D_speaker_type         ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_speaker_type",             (U32) &M3D->speaker_type                ,RIB_NONE },
      
      { RIB_FUNCTION, "M3D_set_3D_rolloff_factor",       (U32) &M3D->set_3D_rolloff_factor       ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_rolloff_factor",           (U32) &M3D->rolloff_factor              ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_doppler_factor",       (U32) &M3D->set_3D_doppler_factor       ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_doppler_factor",           (U32) &M3D->doppler_factor              ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_distance_factor",      (U32) &M3D->set_3D_distance_factor      ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_distance_factor",          (U32) &M3D->distance_factor             ,RIB_NONE },

      };

   RIB_INTERFACE_ENTRY request[] =
      {
      { RIB_FUNCTION, "PROVIDER_query_attribute",       (U32) &M3D->PROVIDER_query_attribute   ,RIB_NONE },
      { RIB_FUNCTION, "M3D_startup",                    (U32) &M3D->startup                    ,RIB_NONE },
      { RIB_FUNCTION, "M3D_error",                      (U32) &M3D->error                      ,RIB_NONE },
      { RIB_FUNCTION, "M3D_shutdown",                   (U32) &M3D->shutdown                   ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_provider_preference",    (U32) &M3D->set_provider_preference    ,RIB_NONE },
      { RIB_FUNCTION, "M3D_activate",                   (U32) &M3D->activate                   ,RIB_NONE },
      { RIB_FUNCTION, "M3D_allocate_3D_sample_handle",  (U32) &M3D->allocate_3D_sample_handle  ,RIB_NONE },
      { RIB_FUNCTION, "M3D_release_3D_sample_handle",   (U32) &M3D->release_3D_sample_handle   ,RIB_NONE },
      { RIB_FUNCTION, "M3D_start_3D_sample",            (U32) &M3D->start_3D_sample            ,RIB_NONE },
      { RIB_FUNCTION, "M3D_stop_3D_sample",             (U32) &M3D->stop_3D_sample             ,RIB_NONE },
      { RIB_FUNCTION, "M3D_resume_3D_sample",           (U32) &M3D->resume_3D_sample           ,RIB_NONE },
      { RIB_FUNCTION, "M3D_end_3D_sample",              (U32) &M3D->end_3D_sample              ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_sample_data",         (U32) &M3D->set_3D_sample_data         ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_sample_volume",       (U32) &M3D->set_3D_sample_volume       ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_sample_playback_rate",(U32) &M3D->set_3D_sample_playback_rate,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_sample_offset",       (U32) &M3D->set_3D_sample_offset       ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_sample_loop_count",   (U32) &M3D->set_3D_sample_loop_count   ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_sample_loop_block",   (U32) &M3D->set_3D_sample_loop_block   ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_sample_status",           (U32) &M3D->sample_status              ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_sample_volume",           (U32) &M3D->sample_volume              ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_sample_playback_rate",    (U32) &M3D->sample_playback_rate       ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_sample_offset",           (U32) &M3D->sample_offset              ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_sample_length",           (U32) &M3D->sample_length              ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_sample_loop_count",       (U32) &M3D->sample_loop_count          ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_sample_distances",    (U32) &M3D->set_3D_sample_distances    ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_sample_distances",        (U32) &M3D->sample_distances           ,RIB_NONE },
      { RIB_FUNCTION, "M3D_active_3D_sample_count",     (U32) &M3D->active_3D_sample_count     ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_open_listener",           (U32) &M3D->open_listener              ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_close_listener",          (U32) &M3D->close_listener             ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_open_object",             (U32) &M3D->open_object                ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_close_object",            (U32) &M3D->close_object               ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_position",            (U32) &M3D->set_3D_position            ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_velocity",            (U32) &M3D->set_3D_velocity            ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_velocity_vector",     (U32) &M3D->set_3D_velocity_vector     ,RIB_NONE },
      { RIB_FUNCTION, "M3D_set_3D_orientation",         (U32) &M3D->set_3D_orientation         ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_position",                (U32) &M3D->position                   ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_velocity",                (U32) &M3D->velocity                   ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_orientation",             (U32) &M3D->orientation                ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_update_position",         (U32) &M3D->update_position            ,RIB_NONE },
      { RIB_FUNCTION, "M3D_3D_auto_update_position",    (U32) &M3D->auto_update_position       ,RIB_NONE },
      };

   RIBRESULT result = RIB_request(provider,"MSS 3D audio services",request);

   if (result != RIB_NOERR)
      {
      return M3D_INTERNAL_ERR;
      }

   //
   // "MSS 3D sample services" is optional, don't fail if it isn't there
   //

   RIB_request(provider,"MSS 3D sample services",sample_request);

   //
   // Activate provider
   //

   return M3D->activate(1);
}

//############################################################################
//#                                                                          #
//# AIL_API_close_3D_provider                                                #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_close_3D_provider       (HPROVIDER    lib)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return;
      }

   //
   // Deactivate provider
   // 

   M3D->activate(0);

   //
   // Free our provider descriptor
   //

   AIL_mem_free_lock(M3D);

   RIB_set_provider_system_data(lib, 0, 0);
}

//############################################################################
//#                                                                          #
//# AIL_API_allocate_3D_sample_handle                                        #
//#                                                                          #
//############################################################################

H3DSAMPLE  AILCALL AIL_API_allocate_3D_sample_handle (HPROVIDER    lib)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return NULL;
      }

   //
   // Allocate our own descriptor for object
   // 
   // This is a wrapper for the actual structure returned by
   // the M3D provider
   //

   H3D FAR *desc = (H3D FAR *) AIL_mem_alloc_lock(sizeof(H3D));

   if (desc == NULL)
      {
      return NULL;
      }

   AIL_memset(desc, 0, sizeof(H3D));

   desc->owner = lib;
   
   desc->actual = M3D->allocate_3D_sample_handle();

   if (desc->actual == NULL)
      {
      return NULL;
      }

   return (H3DSAMPLE) desc;
}

//############################################################################
//#                                                                          #
//# AIL_API_release_3D_sample_handle                                         #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_release_3D_sample_handle (H3DSAMPLE samp)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   //
   // Close provider's representation
   //

   M3D->release_3D_sample_handle(desc->actual);

   //
   // Free our descriptor
   //

   AIL_mem_free_lock(desc);
}

//############################################################################
//#                                                                          #
//# AIL_API_start_3D_sample                                                  #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_start_3D_sample         (H3DSAMPLE samp)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->start_3D_sample(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_API_stop_3D_sample                                                   #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_stop_3D_sample          (H3DSAMPLE samp)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->stop_3D_sample(desc->actual);
}
                                       

//############################################################################
//#                                                                          #
//# AIL_API_resume_3D_sample                                                 #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_resume_3D_sample        (H3DSAMPLE samp)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->resume_3D_sample(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_API_end_3D_sample                                                    #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_end_3D_sample        (H3DSAMPLE samp)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->end_3D_sample(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_info                                               #
//#                                                                          #
//############################################################################

S32        AILCALL AIL_API_set_3D_sample_info      (H3DSAMPLE         samp, //)
                                                    AILSOUNDINFO const FAR *info)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   return M3D->set_3D_sample_data(desc->actual, info);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_file                                               #
//#                                                                          #
//############################################################################

S32        AILCALL AIL_API_set_3D_sample_file      (H3DSAMPLE samp, //)
                                                    void const FAR *file_image)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   AILSOUNDINFO info;

   if (!AIL_WAV_info(file_image, &info))
      {
      AIL_set_error("Not a valid .WAV file");
      return 0;
      }

   return M3D->set_3D_sample_data(desc->actual, &info);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_volume                                             #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_sample_volume    (H3DSAMPLE samp, //)
                                                    S32       volume)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->set_3D_sample_volume(desc->actual, volume);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_playback_rate                                      #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_sample_playback_rate    (H3DSAMPLE samp, //)
                                                           S32       playback_rate)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->set_3D_sample_playback_rate(desc->actual, playback_rate);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_offset                                             #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_sample_offset    (H3DSAMPLE samp, //)
                                                    U32       offset)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->set_3D_sample_offset(desc->actual, offset);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_loop_count                                         #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_sample_loop_count(H3DSAMPLE samp, //)
                                                    U32       loops)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->set_3D_sample_loop_count(desc->actual, loops);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_loop_block                                         #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_sample_loop_block(H3DSAMPLE samp, //)
                                                    S32       loop_start_offset,
                                                    S32       loop_end_offset)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->set_3D_sample_loop_block(desc->actual, 
                                 loop_start_offset,
                                 loop_end_offset);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_sample_status                                                 #
//#                                                                          #
//############################################################################

U32        AILCALL AIL_API_3D_sample_status        (H3DSAMPLE samp)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   return M3D->sample_status(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_sample_volume                                                 #
//#                                                                          #
//############################################################################

S32        AILCALL AIL_API_3D_sample_volume        (H3DSAMPLE samp)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   return M3D->sample_volume(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_sample_playback_rate                                          #
//#                                                                          #
//############################################################################

S32        AILCALL AIL_API_3D_sample_playback_rate        (H3DSAMPLE samp)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   return M3D->sample_playback_rate(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_sample_offset                                                 #
//#                                                                          #
//############################################################################

U32        AILCALL AIL_API_3D_sample_offset        (H3DSAMPLE     samp)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   return M3D->sample_offset(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_sample_length                                                 #
//#                                                                          #
//############################################################################

U32        AILCALL AIL_API_3D_sample_length        (H3DSAMPLE     samp)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   return M3D->sample_length(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_sample_loop_count                                             #
//#                                                                          #
//############################################################################

U32        AILCALL AIL_API_3D_sample_loop_count    (H3DSAMPLE samp)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   return M3D->sample_loop_count(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_distances                                          #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_sample_distances (H3DSAMPLE samp, //)
                                                    F32       max_dist,
                                                    F32       min_dist)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   if (min_dist>max_dist)
   {
     F32 tmp=min_dist;
     min_dist=max_dist;
     max_dist=tmp;
   }

   M3D->set_3D_sample_distances(desc->actual,
                                max_dist,
                                min_dist);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_sample_distances                                              #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_3D_sample_distances     (H3DSAMPLE samp, //)
                                                    F32 FAR * max_dist,
                                                    F32 FAR * min_dist)
{
   H3D FAR *desc = (H3D FAR *) samp;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->sample_distances(desc->actual,
                         max_dist,
                         min_dist);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_user_data                                                 #
//#                                                                          #
//############################################################################

void     AILCALL AIL_API_set_3D_user_data         (H3DPOBJECT obj, //)
                                                   U32        index,
                                                   S32        value)
{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   desc->user_data[index] = value;
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_object_user_data                                              #
//#                                                                          #
//############################################################################

S32      AILCALL AIL_API_3D_user_data             (H3DPOBJECT obj, //)
                                                   U32        index)
{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   return desc->user_data[index];
}

//############################################################################
//#                                                                          #
//# AIL_API_active_3D_sample_count                                           #
//#                                                                          #
//############################################################################

S32      AILCALL AIL_API_active_3D_sample_count   (HPROVIDER lib)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   return M3D->active_3D_sample_count();
}

//############################################################################
//#                                                                          #
//# AIL_API_open_3D_listener                                                 #
//#                                                                          #
//############################################################################

H3DPOBJECT AILCALL AIL_API_open_3D_listener        (HPROVIDER lib)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return NULL;
      }

   //
   // Allocate our own descriptor for object
   //
   // This is a wrapper for the actual structure returned by
   // the M3D provider
   //

   H3D FAR *desc = (H3D FAR *) AIL_mem_alloc_lock(sizeof(H3D));

   if (desc == NULL)
      {
      return NULL;
      }

   AIL_memset(desc, 0, sizeof(H3D));

   desc->owner = lib;

   desc->actual = M3D->open_listener();

   if (desc->actual == NULL)
      {
      return NULL;
      }

   return (H3DPOBJECT) desc;
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_close_listener                                                #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_close_3D_listener       (H3DPOBJECT listener)
{
   H3D FAR *desc = (H3D FAR *) listener;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   //
   // Close provider's representation
   //

   M3D->close_listener(desc->actual);

   //
   // Free our descriptor
   //

   AIL_mem_free_lock(desc);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_open_object                                                   #
//#                                                                          #
//############################################################################

H3DPOBJECT AILCALL AIL_API_open_3D_object          (HPROVIDER lib)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return NULL;
      }

   //
   // Allocate our own descriptor for object
   //
   // This is a wrapper for the actual structure returned by
   // the M3D provider
   //

   H3D FAR *desc = (H3D FAR *) AIL_mem_alloc_lock(sizeof(H3D));

   if (desc == NULL)
      {
      return NULL;
      }

   AIL_memset(desc, 0, sizeof(H3D));

   desc->owner = lib;

   desc->actual = M3D->open_object();

   if (desc->actual == NULL)
      {
      return NULL;
      }

   return (H3DPOBJECT) desc;
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_close_object                                                  #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_close_3D_object         (H3DPOBJECT obj)
{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   //
   // Close provider's representation
   //

   M3D->close_object(desc->actual);

   //
   // Free our descriptor
   // 

   AIL_mem_free_lock(desc);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_position                                                  #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_position         (H3DPOBJECT obj, //)
                                                    F32     X,
                                                    F32     Y,
                                                    F32     Z)
{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->set_3D_position(desc->actual, 
                        X,
                        Y, 
                        Z);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_velocity                                                  #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_velocity         (H3DPOBJECT obj, //)
                                                    F32     dX_per_ms,
                                                    F32     dY_per_ms,
                                                    F32     dZ_per_ms,
                                                    F32     magnitude)
{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->set_3D_velocity(desc->actual,
                        dX_per_ms,
                        dY_per_ms,
                        dZ_per_ms,
                        magnitude);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_velocity_vector                                           #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_velocity_vector  (H3DPOBJECT obj, //)
                                                    F32     dX_per_ms,
                                                    F32     dY_per_ms,
                                                    F32     dZ_per_ms)
{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->set_3D_velocity_vector(desc->actual,
                               dX_per_ms,
                               dY_per_ms,
                               dZ_per_ms);
}


//
// normalize function
//

static void __inline RAD_vector_normalize(F32*x,F32*y,F32*z)
{
  F32 len = (F32) sqrt(((*x) * (*x)) +
                       ((*y) * (*y)) +
                       ((*z) * (*z)));
  if (len>0.00001)
  {
    *x /= len;
    *y /= len;
    *z /= len;
  }
}


//############################################################################
//#                                                                          #
//# AIL_API_set_3D_orientation                                               #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_orientation      (H3DPOBJECT obj, //)
                                                    F32     X_face,
                                                    F32     Y_face,
                                                    F32     Z_face,
                                                    F32     X_up,
                                                    F32     Y_up,
                                                    F32     Z_up)

{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   RAD_vector_normalize(&X_face,&Y_face,&Z_face);
   RAD_vector_normalize(&X_up,&Y_up,&Z_up);

   M3D->set_3D_orientation(desc->actual,
                           X_face,
                           Y_face,
                           Z_face,
                           X_up,
                           Y_up,
                           Z_up);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_position                                                      #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_3D_position             (H3DPOBJECT  obj, //)
                                                    F32 FAR *X,
                                                    F32 FAR *Y,
                                                    F32 FAR *Z)
{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->position(desc->actual,
                 X, 
                 Y, 
                 Z);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_velocity                                                      #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_3D_velocity             (H3DPOBJECT  obj, //)
                                                    F32 FAR *dX_per_ms,
                                                    F32 FAR *dY_per_ms,
                                                    F32 FAR *dZ_per_ms)
{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->velocity(desc->actual,
                 dX_per_ms, 
                 dY_per_ms,
                 dZ_per_ms);
}


//############################################################################
//#                                                                          #
//# AIL_API_3D_orientation                                                   #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_3D_orientation          (H3DPOBJECT  obj, //)
                                                    F32 FAR *X_face,
                                                    F32 FAR *Y_face,
                                                    F32 FAR *Z_face,
                                                    F32 FAR *X_up,
                                                    F32 FAR *Y_up,
                                                    F32 FAR *Z_up)
{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->orientation(desc->actual,
                    X_face,
                    Y_face,
                    Z_face,
                    X_up,
                    Y_up,
                    Z_up);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_update_position                                               #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_update_3D_position      (H3DPOBJECT obj, //)
                                                    F32     dt_milliseconds)
{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->update_position(desc->actual,
                        dt_milliseconds);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_auto_update_position                                          #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_auto_update_3D_position (H3DPOBJECT obj, //)
                                                    S32        enable)
{
   H3D FAR *desc = (H3D FAR *) obj;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   M3D->auto_update_position(desc->actual,
                             enable);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_room_type                                                     #
//#                                                                          #
//############################################################################

S32      AILCALL AIL_API_3D_room_type   (HPROVIDER lib)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   if (M3D->room_type == NULL)
      {
      return -1;
      }

   return M3D->room_type();
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_room_type                                                 #
//#                                                                          #
//############################################################################

void     AILCALL AIL_API_set_3D_room_type   (HPROVIDER lib, //)
                                             S32       EAX_room_type)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return;
      }

   if (M3D->set_3D_room_type == NULL)
      {
      return;
      }

   M3D->set_3D_room_type(EAX_room_type);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_speaker_type                                                  #
//#                                                                          #
//############################################################################

S32      AILCALL AIL_API_3D_speaker_type   (HPROVIDER lib)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   if (M3D->speaker_type == NULL)
      {
      return -1;
      }

   return M3D->speaker_type();
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_speaker_type                                              #
//#                                                                          #
//############################################################################

void     AILCALL AIL_API_set_3D_speaker_type   (HPROVIDER lib, //)
                                               S32       speaker_type)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return;
      }

   if (M3D->set_3D_speaker_type == NULL)
      {
      return;
      }

   M3D->set_3D_speaker_type(speaker_type);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_obstruction                                        #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_sample_obstruction (H3DSAMPLE S, //)
                                                      F32       obstruction)
{
   H3D FAR *desc = (H3D FAR *) S;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   if (M3D->set_3D_sample_obstruction == NULL)
      {
      return;
      }

   M3D->set_3D_sample_obstruction(desc->actual, obstruction);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_occlusion                                          #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_sample_occlusion   (H3DSAMPLE S, //)
                                                      F32       occlusion)
{
   H3D FAR *desc = (H3D FAR *) S;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   if (M3D->set_3D_sample_occlusion == NULL)
      {
      return;
      }

   M3D->set_3D_sample_occlusion(desc->actual, occlusion);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_cone                                               #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_sample_cone        (H3DSAMPLE S, //)
                                                      F32       inner_angle,
                                                      F32       outer_angle,
                                                      S32       outer_volume)
{
   H3D FAR *desc = (H3D FAR *) S;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   if (M3D->set_3D_sample_cone == NULL)
      {
      return;
      }

   M3D->set_3D_sample_cone(desc->actual, inner_angle,outer_angle,outer_volume);
}

//############################################################################
//#                                                                          #
//# AIL_API_set_3D_sample_effects_level                                      #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_set_3D_sample_effects_level   (H3DSAMPLE S, //)
                                                          F32       effects)
{
   H3D FAR *desc = (H3D FAR *) S;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   if (M3D->set_3D_sample_effects_level == NULL)
      {
      return;
      }

   M3D->set_3D_sample_effects_level(desc->actual, effects);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_sample_obstruction                                            #
//#                                                                          #
//############################################################################

F32        AILCALL AIL_API_3D_sample_obstruction (H3DSAMPLE S)
{
   H3D FAR *desc = (H3D FAR *) S;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   if (M3D->sample_obstruction == NULL)
      {
      return -1.0F;
      }

   return M3D->sample_obstruction(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_sample_occlusion                                              #
//#                                                                          #
//############################################################################

F32        AILCALL AIL_API_3D_sample_occlusion   (H3DSAMPLE S)
{
   H3D FAR *desc = (H3D FAR *) S;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   if (M3D->sample_occlusion == NULL)
      {
      return -1.0F;
      }

   return M3D->sample_occlusion(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_sample_cone                                                   #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_3D_sample_cone        (H3DSAMPLE S,
                                                  F32 FAR* inner_angle,
                                                  F32 FAR* outer_angle,
                                                  S32 FAR* outer_volume)
{
   H3D FAR *desc = (H3D FAR *) S;

   if (desc == NULL)
      {
      return;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return;
      }

   if (M3D->sample_cone == NULL)
      {
      
      if (inner_angle)
        *inner_angle=360.0F;
      if (outer_angle)
        *outer_angle=360.0F;
      if (outer_volume)
        *outer_volume=127;

      return;
      }

   M3D->sample_cone(desc->actual,inner_angle,outer_angle,outer_volume);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_sample_effects_level                                          #
//#                                                                          #
//############################################################################

F32        AILCALL AIL_API_3D_sample_effects_level   (H3DSAMPLE S)
{
   H3D FAR *desc = (H3D FAR *) S;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   if (M3D->sample_effects_level == NULL)
      {
      return -1.0F;
      }

   return M3D->sample_effects_level(desc->actual);
}

//############################################################################
//#                                                                          #
//# AIL_register_3D_EOS_callback                                             #
//#                                                                          #
//############################################################################

AIL3DSAMPLECB AILCALL AIL_API_register_3D_EOS_callback (H3DSAMPLE S, //)
                                                        AIL3DSAMPLECB cb)
{
   H3D FAR *desc = (H3D FAR *) S;

   if (desc == NULL)
      {
      return 0;
      }

   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(desc->owner, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   if (M3D->set_3D_EOS == NULL)
      {
      return 0;
      }

   return M3D->set_3D_EOS(S,desc->actual, cb);
}

//############################################################################
//#                                                                          #
//# AIL_set_3D_rolloff_factor                                                #
//#                                                                          #
//############################################################################

void     AILCALL AIL_API_set_3D_rolloff_factor (HPROVIDER lib, //)
                                                F32       factor )
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return;
      }

   if (M3D->set_3D_rolloff_factor == NULL)
      {
      return;
      }

   M3D->set_3D_rolloff_factor(factor);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_rolloff_factor                                                #
//#                                                                          #
//############################################################################

F32      AILCALL AIL_API_3D_rolloff_factor (HPROVIDER lib)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   if (M3D->rolloff_factor == NULL)
      {
      return 1.0F;
      }

   return M3D->rolloff_factor();
}


//############################################################################
//#                                                                          #
//# AIL_set_3D_doppler_factor                                                #
//#                                                                          #
//############################################################################

void     AILCALL AIL_API_set_3D_doppler_factor (HPROVIDER lib, //)
                                                F32       factor )
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return;
      }

   if (M3D->set_3D_doppler_factor == NULL)
      {
      return;
      }

   M3D->set_3D_doppler_factor(factor);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_doppler_factor                                                #
//#                                                                          #
//############################################################################

F32      AILCALL AIL_API_3D_doppler_factor (HPROVIDER lib)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   if (M3D->doppler_factor == NULL)
      {
      return 1.0F;
      }

   return M3D->doppler_factor();
}

//############################################################################
//#                                                                          #
//# AIL_set_3D_distance_factor                                               #
//#                                                                          #
//############################################################################

void     AILCALL AIL_API_set_3D_distance_factor (HPROVIDER lib, //)
                                                 F32       factor )
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return;
      }

   if (M3D->set_3D_distance_factor == NULL)
      {
      return;
      }

   M3D->set_3D_distance_factor(factor);
}

//############################################################################
//#                                                                          #
//# AIL_API_3D_distance_factor                                               #
//#                                                                          #
//############################################################################

F32      AILCALL AIL_API_3D_distance_factor (HPROVIDER lib)
{
   M3DPROVIDER FAR *M3D = (M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0);

   if (M3D == NULL)
      {
      return 0;
      }

   if (M3D->distance_factor == NULL)
      {
      return 1.0;
      }

   return M3D->distance_factor();
}

