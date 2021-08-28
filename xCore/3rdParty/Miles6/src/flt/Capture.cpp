//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  API.CPP: FLT module for capturing output data to a wave file          ##
//##                                                                        ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 5-Feb-99: Initial                                     ##
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


//############################################################################
//
// How to use this filter:
//
//   1) Open the filter with AIL_open_filter()
//   2) Set the filter-level preference "Filename" to the output filename
//   3) Turn on the filter by applying to the driver
//
//############################################################################



#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "mss.h"
#include "imssapi.h"

#define CAPTURE_BUFFER_SIZE 65536*16     // 1 meg capture buffer by default

//
// Attribute tokens
//

enum ATTRIB
{
  FILENAME
};

//
// Driver state descriptor
//
// One state descriptor is associated with each HDIGDRIVER
//

struct DRIVERSTATE
{
   //
   // Members common to all pipeline filter providers
   //

   HDIGDRIVER       dig;                  // Driver with which this descriptor is associated

   S32 FAR         *build_buffer;         // Location and size of driver's mixer output buffer
   S32              build_buffer_size;

   //
   // Members associated with specific filter provider
   //

   U8 FAR         *capture_buffer;        // start of capture buffer
   U8 FAR         *capture_end;           // end of capture buffer
   S32             capture_buffer_size;

   U8 FAR         *current_head;          // writing data out of here
   U8 FAR         *current_tail;          // adding data here

   S32            capturing;
   S32            rate;
   S32            format;
   HANDLE         fhand;
   HANDLE         threadhandle,threadwait;
   U32            total_written;
};

//
// Per-sample filter state descriptor
//
// One state descriptor is associated with each HSAMPLE
//

struct SAMPLESTATE
{
   //
   // Members common to all pipeline filter providers
   //

   HSAMPLE          sample;   // HSAMPLE with which this descriptor is associated
   DRIVERSTATE FAR *driver;   // Driver with which this descriptor is associated

   //
   // Members associated with specific filter provider (none)
   //

};

//
// Globals
//

S32 FLT_started = 0;

C8 FLT_error_text[256];

//############################################################################
//#                                                                          #
//# Retrieve a standard RIB provider attribute by index                      #
//#                                                                          #
//############################################################################

static U32 AILEXPORT PROVIDER_query_attribute (HATTRIB index)
{
   switch ((ATTRIB) index)
      {
      case PROVIDER_NAME:      return (U32) "Capture Filter";
      case PROVIDER_VERSION:   return 0x100;
      }

   return 0;
}


//############################################################################
//#                                                                          #
//# Function to write out a wave file header                                 #
//#                                                                          #
//############################################################################

static void write_header(DRIVERSTATE* DRV)
{
   WAVEOUT wo;
   U32 wrote;

   wo.riffmark='FFIR';
   wo.rifflen=DRV->total_written+sizeof(WAVEOUT)-8;

   wo.wavemark='EVAW';
   wo.fmtmark=' tmf';
   wo.fmtlen=16;
   wo.fmttag=WAVE_FORMAT_PCM;
   wo.channels=(S16)((DRV->format&DIG_F_STEREO_MASK)?2:1);
   wo.sampersec=DRV->rate;
   wo.bitspersam=(S16)((DRV->format&DIG_F_16BITS_MASK)?16:8);
   wo.blockalign=(S16)(((S32)wo.bitspersam*(S32)wo.channels) / 8);
   wo.avepersec=(DRV->rate *(S32)wo.bitspersam*(S32)wo.channels) / 8;
   wo.datamark='atad';
   wo.datalen=DRV->total_written;

   SetFilePointer(DRV->fhand,0,0,FILE_BEGIN);
   WriteFile(DRV->fhand,&wo,sizeof(wo),&wrote,0);
}


//############################################################################
//#                                                                          #
//# Function to dump data accumulated in the capture buffer                  #
//#                                                                          #
//############################################################################

static S32 in_do_output=0;

static void do_output(DRIVERSTATE* DRV)
{
  MSSLockedIncrement(in_do_output);

  while (in_do_output!=1)
    Sleep(1);

  while (DRV->current_tail!=DRV->current_head) {
    S32 writeamt;
    U32 wrote;

    writeamt=(DRV->current_tail<DRV->current_head)?(DRV->capture_end-DRV->current_head):(DRV->current_tail-DRV->current_head);
    if (writeamt>32768)
      writeamt=32768;

    WriteFile(DRV->fhand,DRV->current_head,writeamt,&wrote,0);

    Sleep(1);

    DRV->current_head+=writeamt;
    if (DRV->current_head>=DRV->capture_end)
      DRV->current_head-=DRV->capture_buffer_size;

    DRV->total_written+=wrote;
  }

  MSSLockedDecrement(in_do_output);
}


static char filename[256]="default.wav";
static char oldfilename[256]="";

//############################################################################
//#                                                                          #
//# Set provider preference value, returning previous setting                #
//#                                                                          #
//############################################################################

static S32 AILEXPORT FLT_set_provider_preference (HATTRIB    preference, //)
                                           void FAR*  value)
{
   switch ((ATTRIB)preference) {
     case FILENAME:
       AIL_strcpy(oldfilename,filename);
       AIL_strcpy(filename,(char*)value);
       return((S32)oldfilename);
   }

   return -1;
}

//############################################################################
//#                                                                          #
//# Return FLT filter error text                                             #
//#                                                                          #
//############################################################################

static C8 FAR *       AILEXPORT FLT_error       (void)
{
   if (!AIL_strlen(FLT_error_text))
      {
      return NULL;
      }

   return FLT_error_text;
}

//############################################################################
//#                                                                          #
//# Initialize FLT sample filter                                             #
//#                                                                          #
//############################################################################

static FLTRESULT AILEXPORT FLT_startup     (void)
{
   if (FLT_started++)
      {
      strcpy(FLT_error_text,"Already started");
      return FLT_ALREADY_STARTED;
      }

   //
   // Init static prefs/attributes
   //

   FLT_error_text[0]            = 0;

   return FLT_NOERR;
}

//############################################################################
//#                                                                          #
//# Shut down FLT sample filter                                              #
//#                                                                          #
//############################################################################

static FLTRESULT      AILEXPORT FLT_shutdown    (void)
{
   if (!FLT_started)
      {
      strcpy(FLT_error_text,"Not initialized");
      return FLT_NOT_INIT;
      }

   --FLT_started;

   return FLT_NOERR;
}


//############################################################################
//#                                                                          #
//# Background thread that does the writing to disk                          #
//#                                                                          #
//############################################################################

U32 WINAPI write_thread(LPVOID user)
{
  DRIVERSTATE* DRV=(DRIVERSTATE*)user;

  while (WaitForSingleObject(DRV->threadwait,300)==WAIT_TIMEOUT) {

    if (DRV->capturing)
      do_output(DRV);

  }

  return(0);
}

//############################################################################
//#                                                                          #
//# Allocate driver-specific descriptor                                      #
//#                                                                          #
//############################################################################

static HDRIVERSTATE AILEXPORT FLT_open_driver (HDIGDRIVER dig, //)
                                        S32 FAR   *build_buffer,
                                        S32        build_buffer_size)

{
   U32 id;
   DRIVERSTATE FAR *DRV;

   if (*filename==0) {
     strcpy(FLT_error_text,"NULL filename");
     return(0);
   }

   DRV = (DRIVERSTATE *) AIL_mem_alloc_lock(sizeof(DRIVERSTATE));

   if (DRV == NULL)
      {
      strcpy(FLT_error_text,"Out of memory");
      return NULL;
      }

   AIL_memset(DRV,
              0,
              sizeof(DRIVERSTATE));

   //
   // Initialize generic members
   //

   DRV->dig               = dig;
   DRV->build_buffer      = build_buffer;
   DRV->build_buffer_size = build_buffer_size;

   //
   // Initialize provider-specific members to their default values
   //

   DRV->capturing=0;

   DRV->capture_buffer_size=CAPTURE_BUFFER_SIZE;

   DRV->capture_buffer = (U8 FAR *) AIL_mem_alloc_lock(DRV->capture_buffer_size);
   if (DRV->capture_buffer == NULL)
      {
      strcpy(FLT_error_text,"Could not allocate capture buffer");
      AIL_mem_free_lock(DRV);
      return NULL;
      }

   DRV->capture_end=DRV->capture_buffer+DRV->capture_buffer_size;

   DRV->current_head=DRV->capture_buffer;
   DRV->current_tail=DRV->capture_buffer;

   AIL_digital_configuration(dig,&DRV->rate,&DRV->format,0);

   //
   //  Create thread to perform output
   //

   DRV->threadwait=CreateEvent(0,TRUE,0,0);
   if (DRV->threadwait==0) {
     strcpy(FLT_error_text,"Could not create thread event");
     goto err;
   }

   DRV->threadhandle=CreateThread(0,0,write_thread,(LPVOID)DRV,0,(LPDWORD)&id);
   if (DRV->threadhandle==0) {
     strcpy(FLT_error_text,"Could not create IO thread");
     goto err;
   }

   DRV->fhand=CreateFile(filename,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

   if (DRV->fhand==INVALID_HANDLE_VALUE) {
     strcpy(FLT_error_text,"Could not create file");
    err:
     AIL_mem_free_lock(DRV->capture_buffer);
     AIL_mem_free_lock(DRV);
     return(NULL);
   }

   DRV->current_head=DRV->capture_buffer;
   DRV->current_tail=DRV->capture_buffer;
   DRV->total_written=0;

   write_header(DRV);

   DRV->capturing=1;

   //
   // Return descriptor address cast to handle
   //

   return (HSAMPLESTATE) DRV;
}

//############################################################################
//#                                                                          #
//# Close filter driver instance                                             #
//#                                                                          #
//############################################################################

static FLTRESULT     AILEXPORT FLT_close_driver (HDRIVERSTATE state)
{
   DRIVERSTATE FAR *DRV = (DRIVERSTATE FAR *) state;

   // shut down the thread
   SetEvent(DRV->threadwait);
   WaitForSingleObject(DRV->threadhandle,INFINITE);
   CloseHandle(DRV->threadwait);
   CloseHandle(DRV->threadhandle);

   //turn off capturing
   if (DRV->capturing) {
     DRV->capturing=0;
     do_output(DRV);
     write_header(DRV);
     CloseHandle(DRV->fhand);
   }

   if (DRV->capture_buffer != NULL)
      {
      AIL_mem_free_lock(DRV->capture_buffer);
      DRV->capture_buffer = NULL;
      }

   AIL_mem_free_lock(DRV);

   return FLT_NOERR;
}

//############################################################################
//#                                                                          #
//#  Perform any needed processing before per-sample mixing begins           #
//#                                                                          #
//#  Called after the build buffer has been flushed prior to the mixing      #
//#  phase, but before any samples have been mixed into it                   #
//#                                                                          #
//############################################################################

void AILEXPORT FLT_premix_process (HDRIVERSTATE driver)
{
}

//############################################################################
//#                                                                          #
//#  Process data after mixing                                               #
//#                                                                          #
//#  Called after all samples have been mixed into the 32-bit build buffer,  #
//#  prior to copying the build-buffer contents to the driver's output       #
//#  buffer                                                                  #
//#                                                                          #
//############################################################################

void AILEXPORT FLT_postmix_process (HDRIVERSTATE driver)
{
   DRIVERSTATE FAR *DRV = (DRIVERSTATE FAR *) driver;

   //
   // Add capture buffer contents, if any, to build buffer
   //

   S32 FAR *src =  DRV->build_buffer;

   U32 i = DRV->build_buffer_size / sizeof(S32);

   //calculate how much space we have left
   U32 leftincap=(U32)DRV->current_head;
   if (leftincap==(U32)DRV->current_tail)
     leftincap=DRV->capture_buffer_size;
   else if (leftincap>(U32)DRV->current_tail)
     leftincap=(leftincap-(U32)DRV->current_tail)-2;
   else
     leftincap=((DRV->capture_buffer_size+leftincap)-(U32)DRV->current_tail)-2;

   if (DRV->format&DIG_F_16BITS_MASK) {
     // convert to 16-bit output

     leftincap/=2;

     if (i>leftincap)
       i=leftincap;

     while (i--) {
        S32 val;

        val=(*src++>>11);

        if (val>32767)
          val=32767;
        else if (val<-32768)
          val=-32768;

        *((S16*)DRV->current_tail)=(S16)val;

        DRV->current_tail+=2;
        if (DRV->current_tail>=DRV->capture_end)
          DRV->current_tail-=DRV->capture_buffer_size;
      }
   } else {
     // convert to 8-bit output
     if (i>leftincap)
       i=leftincap;

     while (i--) {
        S32 val;

        val=(*src++>>19);

        if (val>127)
          val=127;
        else if (val<-128)
          val=-128;

        *DRV->current_tail++=((U8)val)+128;
        if (DRV->current_tail>=DRV->capture_end)
          DRV->current_tail-=DRV->capture_buffer_size;
     }
   }
}

//############################################################################
//#                                                                          #
//# DLLMain registers FLT API interface at load time                         #
//#                                                                          #
//############################################################################

BOOL WINAPI DllMain(HINSTANCE hinstDll, //)
                          U32     fdwReason,
                          LPVOID    plvReserved)
{
   const RIB_INTERFACE_ENTRY FLT[] =
      {
      REG_AT("Name",                     PROVIDER_NAME,        RIB_STRING),
      REG_AT("Version",                  PROVIDER_VERSION,     RIB_HEX),

      REG_FN(PROVIDER_query_attribute),

      REG_FN(FLT_startup),
      REG_FN(FLT_error),
      REG_FN(FLT_shutdown),

      REG_FN(FLT_open_driver),
      REG_FN(FLT_close_driver),

      REG_FN(FLT_premix_process),
      REG_FN(FLT_postmix_process),

      REG_FN(FLT_set_provider_preference),

      REG_PR("Filename", FILENAME, RIB_STRING),

      };

   static HPROVIDER self;

   switch (fdwReason)
      {
      case DLL_PROCESS_ATTACH:

         self = RIB_provider_library_handle();

         RIB_register(self,
                     "MSS pipeline filter",
                      FLT);

         break;

      case DLL_PROCESS_DETACH:

         RIB_unregister_all(self);
         break;
      }

   return TRUE;
}

