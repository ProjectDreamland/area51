//############################################################################
//##                                                                        ##
//##  FLT.CPP: Wrapper for pipeline filter providers                        ##
//##                                                                        ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 7-Feb-99: Initial                                     ##
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

#include "rib.h"
#include "mss.h"
#include "imssapi.h"

static FLTPROVIDER FAR *first = NULL;

//############################################################################
//#                                                                          #
//# FLT_init_list()                                                          #
//#                                                                          #
//############################################################################

void FLT_init_list(void)
{
   first = NULL;
}

//############################################################################
//#                                                                          #
//# FLT_find_provider_instance()                                             #
//#                                                                          #
//############################################################################

FLTPROVIDER FAR *FLT_find_provider_instance(HPROVIDER  provider, //)
                                            HDIGDRIVER dig)
{
   FLTPROVIDER FAR *F = first;

   while (F != NULL)
      {
      if ((F->dig      == dig) &&
          (F->provider == provider))
         {
         return F;
         }

      F = F->next;
      }

   return NULL;
}

//############################################################################
//#                                                                          #
//# FLT_disconnect_driver()                                                  #
//#                                                                          #
//############################################################################

void FLT_disconnect_driver(HDIGDRIVER dig)
{
   //
   // Kill all filter descriptors associated with this driver
   //

   FLTPROVIDER FAR *F = first;

   while (F != NULL)
      {
      if (F->dig == dig)
         {
         AIL_close_filter(HDRIVERSTATE(F));
         F = first;
         continue;
         }

      F = F->next;
      }
}

//############################################################################
//#                                                                          #
//# FLT_call_premix_processors()                                             #
//#                                                                          #
//############################################################################

void FLT_call_premix_processors(HDIGDRIVER dig
#ifdef IS_MAC
                                ,U32 buffer_size
#endif
)
{
   //
   // For each filter associated with this driver...
   //

   FLTPROVIDER FAR *F = first;

   while (F != NULL)
      {
      if (F->dig == dig)
         {
         //
         // Call this filter's premix processor
         //

         F->premix_process(F->driver_state
#ifdef IS_MAC
                                ,buffer_size
#endif
                           );
         }

      F = F->next;
      }
}

//############################################################################
//#                                                                          #
//# FLT_call_postmix_processors()                                            #
//#                                                                          #
//############################################################################

void FLT_call_postmix_processors(HDIGDRIVER dig
#ifdef IS_MAC
                                ,U32 buffer_size
#endif
)
{
   //
   // For each filter associated with this driver...
   //

   FLTPROVIDER FAR *F = first;

   while (F != NULL)
      {
      if (F->dig == dig)
         {
         //
         // Call this filter's postmix processor
         //

         F->postmix_process(F->driver_state
#ifdef IS_MAC
                                ,buffer_size
#endif
                            );
         }

      F = F->next;
      }
}

//############################################################################
//#                                                                          #
//# AIL_API_enumerate_filters()                                              #
//#                                                                          #
//############################################################################

S32        AILCALL AIL_API_enumerate_filters  (HPROENUM  FAR *next, //)
                                               HPROVIDER FAR *dest,
                                               C8  FAR * FAR *name)
{
   HATTRIB                  PROVNAME;
   PROVIDER_QUERY_ATTRIBUTE PROVIDER_query_attribute = NULL;

   RIB_INTERFACE_ENTRY FLT[] = 
      { 
      AT("Name",PROVNAME),
      FN(PROVIDER_query_attribute)
      };

   if (RIB_enumerate_providers("MSS pipeline filter",
                               next,
                               dest))
      {
      RIB_request(*dest,"MSS pipeline filter",FLT);

      *name = (C8 FAR *) PROVIDER_query_attribute(PROVNAME);

      return 1;
      }

   return 0;
}

//############################################################################
//#                                                                          #
//# AIL_API_open_filter()                                                    #
//#                                                                          #
//############################################################################

HDRIVERSTATE AILCALL AIL_API_open_filter (HPROVIDER  provider, //)
                                          HDIGDRIVER dig)
{
   //
   // If filter already associated with driver, return NULL
   //

   FLTPROVIDER FAR *F = first;

   while (F != NULL)
      {
      if ((F->dig      == dig) &&
          (F->provider == provider))
         {
         return NULL;
         }

      F = F->next;
      }

   //
   // Allocate our provider descriptor
   //

   FLTPROVIDER FAR *FLT = (FLTPROVIDER FAR *) AIL_mem_alloc_lock(sizeof(FLTPROVIDER));

   if (FLT == NULL)
      {
      AIL_set_error("Out of memory");
      return NULL;
      }

   AIL_memset(FLT, 0, sizeof(FLTPROVIDER));

   //
   // Fill in provider members
   //

   RIB_INTERFACE_ENTRY FLT_request[] =
      {
      { RIB_FUNCTION, "PROVIDER_query_attribute",       (U32) &FLT->PROVIDER_query_attribute   ,RIB_NONE },
      { RIB_FUNCTION, "FLT_startup",                    (U32) &FLT->startup                    ,RIB_NONE },
      { RIB_FUNCTION, "FLT_error",                      (U32) &FLT->error                      ,RIB_NONE },
      { RIB_FUNCTION, "FLT_shutdown",                   (U32) &FLT->shutdown                   ,RIB_NONE },
      { RIB_FUNCTION, "FLT_set_provider_preference",    (U32) &FLT->set_provider_preference    ,RIB_NONE },
      { RIB_FUNCTION, "FLT_open_driver",                (U32) &FLT->open_driver                ,RIB_NONE },
      { RIB_FUNCTION, "FLT_close_driver",               (U32) &FLT->close_driver               ,RIB_NONE },
      { RIB_FUNCTION, "FLT_premix_process",             (U32) &FLT->premix_process             ,RIB_NONE },
      { RIB_FUNCTION, "FLT_postmix_process",            (U32) &FLT->postmix_process            ,RIB_NONE },
      };

   RIB_INTERFACE_ENTRY SMP_request[] =
      {
      { RIB_FUNCTION, "FLTSMP_open_sample",             (U32) &FLT->open_sample                ,RIB_NONE },
      { RIB_FUNCTION, "FLTSMP_close_sample",            (U32) &FLT->close_sample               ,RIB_NONE },
      { RIB_FUNCTION, "FLTSMP_sample_process",          (U32) &FLT->sample_process             ,RIB_NONE },
      { RIB_FUNCTION, "FLTSMP_sample_attribute",        (U32) &FLT->sample_attribute           ,RIB_NONE },
      { RIB_FUNCTION, "FLTSMP_set_sample_preference",   (U32) &FLT->set_sample_preference      ,RIB_NONE },
      };

   RIBRESULT result = RIB_request(provider,"MSS pipeline filter",FLT_request);

   if (result != RIB_NOERR)
      {
      AIL_set_error("Not a filter provider");
      return NULL;
      }

   result = RIB_request(provider,"Pipeline filter sample services",SMP_request);

/*   if (result != RIB_NOERR)
      {
      AIL_set_error("Not a filter provider");
      return NULL;
      }*/

   //
   // Activate provider, storing HPROVIDER, DRIVERSTATE and original
   // HDIGDRIVER in our wrapper descriptor
   //

   OutMilesMutex();
   FLT->driver_state = FLT->open_driver(dig,
                                        dig->build_buffer,
                                        dig->build_size);
   InMilesMutex();

   if (FLT->driver_state==NULL)
      {
      AIL_mem_free_lock(FLT);
      AIL_set_error("Error loading filter");
      return NULL;
      }

   FLT->dig = dig;

   FLT->provider = provider;

   //
   // Link filter descriptor into list
   //

   FLT->next = first;
   first     = FLT;

   //
   // Return our wrapper descriptor's address as the HDRIVERSTATE from the
   // app's point of view
   //

   return (HDRIVERSTATE) FLT;
}

//############################################################################
//#                                                                          #
//# AIL_API_close_filter()                                                   #
//#                                                                          #
//############################################################################

void       AILCALL AIL_API_close_filter       (HDRIVERSTATE filter)
{
   FLTPROVIDER FAR *FLT = (FLTPROVIDER FAR *) filter;

   if (FLT == NULL)
      {
      return;
      }

   //
   // Deactivate provider
   //

   FLT->close_driver(FLT->driver_state);

   //
   // Unlink descriptor from list
   //

   if (first == FLT)
      {
      first = FLT->next;
      }
   else
      {
      FLTPROVIDER FAR *link = first;
      FLTPROVIDER FAR *prev = NULL;

      while (link != FLT)
         {
         prev = link;
         link = link->next;
         }

      prev->next = FLT->next;
      }

   //
   // Free descriptor structure
   //

   AIL_mem_free_lock(FLT);
}
