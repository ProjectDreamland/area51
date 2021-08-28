//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  SYNTH.CPP: Virtual wavetable synthesizer for XMIDI-triggered          ##
//##             DLS voices                                                 ##
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

#include <limits.h>

#include "mss.h"
#include "imssapi.h"

#include "mssdls.h"
#include "dls1.h"
#include "synth.h"


// ---------------------------------------------------------------------------
// Preference initialization
// ---------------------------------------------------------------------------

#define SHOW_DEBUG_TRACE 0    // Do not show debug text output
#define DEBUG_LOGFILE    0    // Do not write debug output to debug.log file

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

      locked = 1;
      }
}

#define DOLOCK() AIL_lock_start()

#else

#define DOLOCK()

#endif

//############################################################################
//#                                                                          #
//#  Init synthesizer resources                                              #
//#                                                                          #
//############################################################################

DXDEF S32 AILEXPORT DLSMSSOpen      (S32 FAR   *lpdwHandle, //)
                                      S32        dwFlags,
                                      HMDIDRIVER MIDI_driver,        // XMIDI driver handle
                                      S32        output_format,      // DIG_F format for output
                                      S32        output_sample_rate, // Output samples/second
                                      S32        stream_buffer_size, // Size of each buffer segment in bytes
                                      U32        user,               // User value passed to buffer callbacks
                                      AILDLSPCB  stream_poll_CB,     // Buffer polling handler
                                      AILDLSLCB  stream_lock_CB,     // Buffer lock handler
                                      AILDLSUCB  stream_unlock_CB)   // Buffer unlock handler
{
   DOLOCK();

   return DLS_init(lpdwHandle,
                   dwFlags,
                   MIDI_driver,
                   output_format,
                   output_sample_rate,
                   stream_buffer_size,
                   user,
                   stream_poll_CB,
                   stream_lock_CB,
                   stream_unlock_CB);
}

//############################################################################
//#                                                                          #
//#  DLSClose()                                                              #
//#                                                                          #
//############################################################################

DXDEF S32 AILEXPORT DLSClose         (S32      dwDLSHandle,  //)
                                      S32      dwFlags)
{
   return DLS_shutdown(dwDLSHandle,
                       dwFlags);
}

//############################################################################
//#                                                                          #
//#  DLSLoadFile()                                                           #
//#                                                                          #
//############################################################################

DXDEF S32 AILEXPORT DLSLoadFile (S32      dwDLSHandle,  //)
                                 S32      dwFlags,
                                 S32 FAR *lpdwDownloadID,
                                 char const FAR*lpFileName)
{
   dwFlags=dwFlags;

   if (dwDLSHandle != (S32) DLS)
      {
      return DLS_INVALID_HANDLE;
      }

   //
   // Attempt to load file
   //

   void FAR *data = AIL_file_read(lpFileName, NULL);

   if (data == NULL)
      {
      switch (AIL_file_error())
         {
         case AIL_OUT_OF_MEMORY:  return DLS_OUT_OF_MEM;
         case AIL_FILE_NOT_FOUND: return DLS_FILE_NOT_FOUND;
         default:                 return DLS_FILE_NOT_LOADED;
         }
      }

   //
   // Attempt to parse file
   // 

   DLS_FILE FAR*file;

   if ((((CHUNK FAR*)data)->ckID != FOURCC_RIFF) || (((CHUNK FAR*)data)->subID != FOURCC_DLS))
   {
     AIL_mem_free_lock(data);
     return(DLS_INVALID_FILE);
   }

   S32 result = DLSFILE_parse(data, &file, lpFileName, 0);

   if (result != DLS_NOERR)
      {
       AIL_mem_free_lock(data);
       return(result);
      }

   //
   // Return success
   //

   *lpdwDownloadID = file->entry_num;

   return DLS_NOERR;
}


//############################################################################
//#                                                                          #
//#  DLSFSSLoadFile()                                                        #
//#                                                                          #
//############################################################################

#ifdef IS_MAC

DXDEF S32 AILEXPORT DLSFSSLoadFile (S32      dwDLSHandle,  //)
                                 S32      dwFlags,
                                 S32 FAR *lpdwDownloadID,
                                 FSSpec const FAR*lpFileName)
{
   dwFlags=dwFlags;

   if (dwDLSHandle != (S32) DLS)
      {
      return DLS_INVALID_HANDLE;
      }

   //
   // Attempt to load file
   //

   void FAR *data = AIL_file_fss_read(lpFileName, NULL);

   if (data == NULL)
      {
      switch (AIL_file_error())
         {
         case AIL_OUT_OF_MEMORY:  return DLS_OUT_OF_MEM;
         case AIL_FILE_NOT_FOUND: return DLS_FILE_NOT_FOUND;
         default:                 return DLS_FILE_NOT_LOADED;
         }
      }

   //
   // Attempt to parse file
   //

   DLS_FILE FAR*file;

   if ((((CHUNK FAR*)data)->ckID != FOURCC_RIFF) || (((CHUNK FAR*)data)->subID != FOURCC_DLS))
   {
     AIL_mem_free_lock(data);
     return(DLS_INVALID_FILE);
   }

   char str[64];
   memcpy( str, lpFileName->name, lpFileName->name[0] );
   str[lpFileName->name[0]]=0;

   S32 result = DLSFILE_parse(data, &file, str, 0);

   if (result != DLS_NOERR)
      {
       AIL_mem_free_lock(data);
       return(result);
      }

   //
   // Return success
   //

   *lpdwDownloadID = file->entry_num;

   return DLS_NOERR;
}

#endif

//############################################################################
//#                                                                          #
//#  DLSLoadMemFile()                                                        #
//#                                                                          #
//############################################################################

DXDEC S32 AILEXPORT DLSLoadMemFile   (S32      dwDLSHandle,
                                      S32      dwFlags,
                                      S32 FAR *lpdwDownloadID,
                                      void const FAR*lpMemPtr)
{
   dwFlags=dwFlags;

   if (dwDLSHandle != (S32) DLS)
      {
      return DLS_INVALID_HANDLE;
      }

   //
   // Attempt to parse file
   //

   DLS_FILE FAR*file;

   if ((((CHUNK FAR*)lpMemPtr)->ckID != FOURCC_RIFF) || (((CHUNK FAR*)lpMemPtr)->subID != FOURCC_DLS))
   {
     return(DLS_INVALID_FILE);
   }

   S32 result = DLSFILE_parse(lpMemPtr, &file, "(Memory file)", 0);

   if (result != DLS_NOERR)
      {
      return result;
      }

   //
   // Return success
   //

   *lpdwDownloadID = file->entry_num;

   return DLS_NOERR;
}

//############################################################################
//#                                                                          #
//#  DLSGetInfo()                                                            #
//#                                                                          #
//############################################################################

DXDEF S32 AILEXPORT DLSGetInfo       (DLS_INFO_STRUC FAR *lpDlsInfo)
{
   if (lpDlsInfo == NULL)
      {
      return DLS_INVALID_HANDLE;
      }

   lpDlsInfo->dwVersion = (AIL_DLS_VERSION << 8) |
                           AIL_DLS_REVISION;

   lpDlsInfo->dwHwStatus     = DLS_NOERR;
   lpDlsInfo->dwMaxDlsMem    = LONG_MAX;
   lpDlsInfo->dwLargestBuf   = LONG_MAX;
   lpDlsInfo->dwGMBankStatus = 1;
   lpDlsInfo->dwGMBankSize   = LONG_MAX;

   return DLS_NOERR;
}

//############################################################################
//#                                                                          #
//#  DLSCompactMemory()                                                      #
//#                                                                          #
//############################################################################

DXDEF S32 AILEXPORT DLSCompactMemory (S32 dwDLSHandle)
{
   dwDLSHandle=dwDLSHandle;
   return DLS_NOERR;
}

//############################################################################
//#                                                                          #
//#  DLSUnloadAll()                                                          #
//#                                                                          #
//############################################################################

DXDEF S32 AILEXPORT DLSUnloadAll     (S32 dwDLSHandle, //)
                                      S32 dwFlags)
{
   dwFlags=dwFlags;

   if (dwDLSHandle != (S32) DLS)
      {
      return DLS_INVALID_HANDLE;
      }

   //
   // Unload all files in list
   //

   while (file_list->used)
      {
      DLSUnloadFile(dwDLSHandle,
                    file_list->used->entry_num);
      }

   return DLS_NOERR;
}

//############################################################################
//#                                                                          #
//#  DLSUnloadFile()                                                         #
//#                                                                          #
//############################################################################

DXDEF S32 AILEXPORT DLSUnloadFile    (S32 dwDLSHandle,  //)
                                   S32 dwDownloadID)
{
   if (dwDLSHandle != (S32) DLS)
      {
      return DLS_INVALID_HANDLE;
      }

   DLS_FILE FAR*file = &file_list->array[dwDownloadID];

   if (file == NULL)
      {
      return DLS_INVALID_HANDLE;
      }

   //
   // Remove file entry from list and return success
   //

   file_list->dealloc(file);

   return DLS_NOERR;
}

//############################################################################
//#                                                                          #
//#  DLSSetAttribute()                                                       #
//#                                                                          #
//############################################################################

DXDEF S32 AILEXPORT DLSSetAttribute  (S32    dwDLSHandle, //)
                                      S32    dwAttribute,
                                      void FAR*lpDlsParam)
{
   dwDLSHandle=dwDLSHandle;
   dwAttribute=dwAttribute;
   lpDlsParam=lpDlsParam;

   return DLS_ERR_NOT_IMPLEMENTED;
}

//############################################################################
//#                                                                          #
//#  DLSMSSGetCPU()                                                          #
//#                                                                          #
//############################################################################

DXDEF S32 AILEXPORT DLSMSSGetCPU (S32 dwDLSHandle)
{
  S32 time;
  U32 diff;

   if (dwDLSHandle != (S32) DLS)
      {
      return 0;
      }

  if (DLS==0)
    return(0);

  time=AIL_ms_count();

  diff=time-DLS->last_ms_polled;
  if (diff<150)
    return(DLS->last_percent);

  DLS->last_ms_polled=time;

  DLS->ms_count+=(DLS->us_count/1000);
  DLS->us_count=DLS->us_count%1000;

  time=(diff)?((DLS->ms_count*100)/diff):0;
  DLS->ms_count=0;

  DLS->last_percent=time;

  if (time>100)
    time=100;

  return(time);
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

