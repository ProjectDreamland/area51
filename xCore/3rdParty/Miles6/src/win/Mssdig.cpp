//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  mssdig.C: Digital Sound module for waveOut and DirectSound            ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 9.0                  ##
//##                                                                        ##
//##  Version 1.00 of 15-Feb-95: Derived from AILSS.C V1.03                 ##
//##          1.01 of 19-Jun-95: Stereo tracks panned for mono output       ##
//##                             Use multiply/shift for 16-bit scaling      ##
//##                             Digital master volume added                ##
//##                             AIL_resume_sample() restarts driver        ##
//##          1.02 of 16-Jul-95: Win95 thread synchronization added         ##
//##          1.03 of 21-Nov-95: API brought up to DOS 3.03C level          ##
//##                             Changed synchronization methods            ##
//##          1.04 of 15-Feb-96: Fixes for optimization and multiple        ##
//##                             16 bit loads (JKR)                         ##
//##          1.05 of 11-Apr-96: Added background thread checking (JKR)     ##
//##          1.06 of 11-May-97: Added IMA ADPCM support (Serge Plagnol)    ##
//##          1.10 of 10-Jun-98: Adapted for use with new mixer, many       ##
//##                             changes (JM)                               ##
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

#ifdef WIN32
#include <dsound.h>
#endif

#include "mss.h"
#include "imssapi.h"

#include <stdlib.h>

#ifndef IS_WIN32
#define WAVEFORMATEX WAVEFORMAT
#define LPWAVEFORMATEX LPWAVEFORMAT
#endif

//
// Assembly-language functions
//

extern "C"
{
#ifdef IS_WIN32

#define SETTAG(var,tag) AIL_memcpy(var,tag,4);

#else

#define SETTAG(var,tag) AIL_memcpy(var,tag,4);

void Win32sToBeCalled();

#endif
}

//
// Macros to hide DirectSound typecasting
//

#define ASDSBUF(ptr) ((LPDIRECTSOUNDBUFFER)ptr)
#define ASDS(ptr)    ((LPDIRECTSOUND)ptr)

//
// SS_calculate_volume_scalars()
//

static U8 pan_graph[128] =
   {
   0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,
   32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,
   64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,
   96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,128,
   128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,
   128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,
   128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,
   128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128
   };

//
// HDR_USER structure and associated flags
//

typedef struct                      // Descriptor appended to WAVEHDRs
{
   HDIGDRIVER owner;                // DIG_DRIVER associated with this WAVEHDR
   LPWAVEHDR  next;                 // Next WAVEHDR in circular chain
   U32        flags;                // Misc. flags (FORCE_POST, etc.)
}
HDR_USER;

#define HDR_FORCE_POST 0x00000001   // Force this WAVEHDR to be posted

//
// Macro for easy access to WAVEHDR.dwUser member as pointer to HDR_USER
//

#define CURRENT_USER ((HDR_USER FAR *) current->dwUser)

//
// Flag to disable background and foreground callbacks
//

static volatile S32 disable_callbacks = 0;


//
// Flag to keep SS_serve from reentering
//

static volatile S32 SS_servicing = 0;

//
// Primary digital driver (the one used with 3D providers and other
// future single-driver extensions)
//

extern "C"
{
HDIGDRIVER primary_digital_driver = NULL;
}

static void low_serve();

//############################################################################
//##                                                                        ##
//## Boolean macro: is sample 'done' or not yet started?                    ##
//##                                                                        ##
//## Returns TRUE if WHDR_DONE bit is set, or if HDR_FORCE_POST bit is set  ##
//## (used to indicate virgin sample that needs to be posted for the first  ##
//## time)                                                                  ##
//##                                                                        ##
//############################################################################

#define NEED_POST(x) (((x)->dwFlags & WHDR_DONE) || \
                     (((HDR_USER FAR *) (x)->dwUser)->flags & HDR_FORCE_POST))

//############################################################################
//#                                                                          #
//# waveOut emulation routines for DirectSound                               #
//#                                                                          #
//############################################################################

#ifdef IS_WIN32

//
// DirectSound DLL loaded count
//

static S32 DSDLLcount = 0;


//
// DirectSound DLL handle
//

static HINSTANCE DSDLLinstance=0;


//
// Pointer to the DirectSoundCreate function
//

typedef HRESULT (WINAPI *DirectSoundCreateType)(void* lpGUID,void* lpds,void* pUnkOuter);
static DirectSoundCreateType DirectSoundCreatePtr=0;


//
// Pointer to the DirectSoundEnumerate function
//

typedef BOOL (WINAPI *DirectSoundEnumerateType)(LPVOID enumcb,LPVOID context);
static DirectSoundEnumerateType DirectSoundEnumeratePtr=0;

//############################################################################
//##                                                                        ##
//## Load the DirectSound DLL                                               ##
//##                                                                        ##
//############################################################################

S8 DirectSoundLoad()
{
  UINT err;

  if (DSDLLcount++==0) {

    // load the DLL without an error message popping up if it's not there

    OutMilesMutex();
    err=SetErrorMode(0x8000);
    DSDLLinstance=LoadLibrary("DSOUND.DLL");
    SetErrorMode(err);

    Sleep(1);
    InMilesMutex();

    if (DSDLLinstance==0) {
      DSDLLcount--;
      return(0);
    }

    // get the function addresses

    DirectSoundCreatePtr=(DirectSoundCreateType)GetProcAddress(DSDLLinstance,"DirectSoundCreate");
    DirectSoundEnumeratePtr=(DirectSoundEnumerateType)GetProcAddress(DSDLLinstance,"DirectSoundEnumerateA");

    if ((DirectSoundCreatePtr==0) || (DirectSoundEnumeratePtr==0)) {
      FreeLibrary(DSDLLinstance);
      DSDLLcount--;
      return(0);
    }
  }
  return(1);
}


//############################################################################
//##                                                                        ##
//## Unload the DirectSound DLL                                             ##
//##                                                                        ##
//############################################################################

void DirectSoundUnload()
{
  OutMilesMutex();
  if ((DSDLLcount) && (--DSDLLcount==0)) {
    DirectSoundCreatePtr=0;
    FreeLibrary(DSDLLinstance);
    DSDLLinstance=0;
  }
  Sleep(1);
  InMilesMutex();
}


static S32 clear_dsbuf(void* dsbuf, S32 bits)
{
  //
  // fill the buffer with silence and get it looping
  //

  DSBCAPS bcaps;

  AIL_memset(&bcaps,0,sizeof(bcaps));
  bcaps.dwSize = sizeof(bcaps);

  ASDSBUF(dsbuf)->GetCaps(&bcaps);

  void FAR *p1,FAR *p2;
  U32 l1,l2;

  MMRESULT result;

 again:
  result = ASDSBUF(dsbuf)->Lock(0,
                                bcaps.dwBufferBytes,
                                &p1,
                                &l1,
                                &p2,
                                &l2,
                                0 );

  if (result == DSERR_BUFFERLOST)
  {
    //
    // Restore, and try again later
    //

    if (SUCCEEDED(ASDSBUF(dsbuf)->Restore()))
      goto again;

  }
  else if (result == MMSYSERR_NOERROR)
  {
    //
    // Mix data into locked region
    //

    if ((p1) && (l1))
      AIL_memset(p1,(bits == 16)?0:0x80,l1);

    if ((p2) && (l2))
      AIL_memset(p2,(bits == 16)?0:0x80,l2);

    //
    // Release locked region
    //

    ASDSBUF(dsbuf)->Unlock(p1,
                           l1,
                           p2,
                           l2);

    return(1);
  }
  return(0);
}

//############################################################################
//##                                                                        ##
//## Setup DirectSound based on the settings in the DIGDRIVER structure     ##
//##                                                                        ##
//############################################################################


S32 setup_directsound(HDIGDRIVER dig)
{
   DSBUFFERDESC pdsbdesc;
   S32          setformat;
   DSCAPS       caps;
   U32          using_default;
   WAVEFORMATEX form;
   U32          requested_priority;

   //
   // Get window handle for application owning this thread
   //

   if ((dig->dsHwnd==0) || (!IsWindow((HWND)dig->dsHwnd)))
      {
      dig->dsHwnd=(U32) AIL_HWND();
      }

   //
   // First, set cooperative level to DSSCL_NORMAL, to determine default
   // format of primary buffer
   //
   // (If, however, we're mixing into the primary buffer, we've got to
   // go ahead and request DSSCL_WRITEPRIMARY access now)
   //

   if (AIL_preference[DIG_DS_USE_PRIMARY])
      {
      setformat = !dig->DS_use_default_format;
      requested_priority = DSSCL_WRITEPRIMARY;
      }
   else
      {
      setformat = 0;
      requested_priority = DSSCL_NORMAL;
      }

   //
   // Iterate until primary buffer object created with desired format
   //

   while (1)
      {
      //
      // Release previously-created objects
      //

      if (dig->lppdsb)
         {
         ASDSBUF(dig->lppdsb)->Release();
         dig->lppdsb = NULL;
         }

      if (dig->pDS)
         {
         ASDS(dig->pDS)->Release();
         dig->pDS = NULL;
         }

      //
      // Attempt to instantiate DirectSound object
      //
      // If DirectSound object cannot be instantiated, fail
      //

      MMRESULT res;

      OutMilesMutex();
      DirectSoundCreateType DS_CREATE = (DirectSoundCreateType) AIL_preference[DIG_DS_CREATION_HANDLER];
      InMilesMutex();

      if (DS_CREATE == NULL)
         {
         //
         // No specific DirectSoundCreate() handler provided -- use
         // default
         //

         OutMilesMutex();
         if (!SUCCEEDED((res=DirectSoundCreatePtr((LPGUID) dig->guid,
                                    (LPDIRECTSOUND FAR *) &dig->pDS,
                                                           NULL))))
         {
            InMilesMutex();
            AIL_set_error("DirectSoundCreate() failed.");
            return 0;
          }
          InMilesMutex();
         }
      else
      {
         //
         // Call application-provided DirectSoundCreate() handler (e.g., for
         // Aureal 3D)
         //

         OutMilesMutex();
         if (!SUCCEEDED((res=DS_CREATE((LPGUID) dig->guid,
                         (LPDIRECTSOUND FAR *) &dig->pDS,
                                                NULL))))
         {
            InMilesMutex();
            AIL_set_error("DirectSoundCreate() custom handler failed.");
            return 0;
         }
         InMilesMutex();
      }

      //
      // Figure out if we're running in emulation mode
      //

      dig->emulated_ds = 0;

      AIL_memset(&caps,0,sizeof(caps));
      caps.dwSize = sizeof(caps);

      if (ASDS(dig->pDS)->GetCaps(&caps) == DS_OK)
         {
         dig->emulated_ds = (caps.dwFlags & DSCAPS_EMULDRIVER) ? 1 : 0;
         }

      //
      // If DS is emulated, we can't request WRITEPRIMARY access
      //

      if ((dig->emulated_ds) && (requested_priority == DSSCL_WRITEPRIMARY))
         {
         requested_priority = DSSCL_PRIORITY;
         }

      //
      // Set requested priority level
      //

      OutMilesMutex();
      if (!SUCCEEDED(ASDS(dig->pDS)->SetCooperativeLevel(
           (HWND)dig->dsHwnd,
                 requested_priority)))
         {
         ASDS(dig->pDS)->Release();
         InMilesMutex();
         AIL_set_error("SetCooperativeLevel() failed.");
         return 0;
         }
      InMilesMutex();
      
      dig->ds_priority=requested_priority;

      //
      // Set up DSBUFFERDESC structure for primary buffer
      //
      // lpwxFormat must be NULL for primary buffers at setup time
      //

      AIL_memset(&pdsbdesc, 0, sizeof(DSBUFFERDESC));

      pdsbdesc.dwSize        = sizeof(DSBUFFERDESC);
      pdsbdesc.dwFlags       = DSBCAPS_PRIMARYBUFFER;

      if (AIL_preference[DIG_DS_DSBCAPS_CTRL3D])
         {
         pdsbdesc.dwFlags   |= DSBCAPS_CTRL3D;
         }

      pdsbdesc.dwBufferBytes = 0;
      pdsbdesc.lpwfxFormat   = NULL;

      //
      // Try to create primary sound buffer
      //

      OutMilesMutex();
      if (!SUCCEEDED(ASDS(dig->pDS)->CreateSoundBuffer(&pdsbdesc,
                                                      (LPDIRECTSOUNDBUFFER FAR *) &dig->lppdsb,
                                                       NULL)))
         {
         ASDS(dig->pDS)->Release();
         InMilesMutex();
         AIL_set_error("CreateSoundBuffer() failed on primary buffer.");
         return 0;
         }
      InMilesMutex();

      //
      // Set format of buffer, if requested
      //

      if (setformat)
         {
         dig->wformat.wf.wFormatTag          = WAVE_FORMAT_PCM;
         dig->wformat.wf.nAvgBytesPerSec  = dig->wformat.wf.nSamplesPerSec *
                                            dig->wformat.wf.nBlockAlign;

         OutMilesMutex();
         if (!SUCCEEDED(ASDSBUF(dig->lppdsb)->SetFormat(
                                                       (LPWAVEFORMATEX)&dig->wformat)))
            {
            ASDS(dig->pDS)->Release();
            InMilesMutex();
            AIL_set_error("SetFormat() failed on primary buffer.");
            return 0;
            }
         InMilesMutex();
         }

      //
      // Get format of primary sound buffer
      //

      ASDSBUF(dig->lppdsb)->GetFormat( &form, sizeof(form), NULL);

      using_default = dig->DS_use_default_format;

      if ((form.nChannels      == dig->wformat.wf.nChannels)      &&
          (form.nSamplesPerSec == dig->wformat.wf.nSamplesPerSec) &&
          (form.nBlockAlign    == dig->wformat.wf.nBlockAlign)    &&
          (form.wBitsPerSample == dig->wformat.wBitsPerSample)    &&
          (form.wFormatTag     == dig->wformat.wf.wFormatTag))
         {
         using_default = 1;
         }

      //
      // If we can use the current format, or if a specific format was just
      // set, exit from the loop
      //

      if ((using_default) || (setformat))
         {
         dig->wformat.wf.nChannels        = form.nChannels;
         dig->wformat.wf.nSamplesPerSec   = form.nSamplesPerSec;
         dig->wformat.wf.nBlockAlign      = form.nBlockAlign;
         dig->wformat.wBitsPerSample      = form.wBitsPerSample;
         dig->wformat.wf.wFormatTag       = form.wFormatTag;

         dig->wformat.wf.wFormatTag       = WAVE_FORMAT_PCM;
         dig->wformat.wf.nBlockAlign      = ((dig->wformat.wf.nChannels   == 2 ) + 1) *
                                            ((dig->wformat.wBitsPerSample == 16) + 1);

         dig->wformat.wf.nAvgBytesPerSec  = dig->wformat.wf.nSamplesPerSec *
                                            dig->wformat.wf.nBlockAlign;
         break;
         }

      //
      // Otherwise, set flag to try specified format, and continue
      //

      setformat = 1;
      requested_priority = DSSCL_PRIORITY;
      }


     //
     // fill the buffer with silence and get it looping
     //

     if (requested_priority == DSSCL_WRITEPRIMARY)
       clear_dsbuf(dig->lppdsb,dig->wformat.wBitsPerSample);

     //
     // Set primary buffer to "play"
     //
     OutMilesMutex();
     ASDSBUF(dig->lppdsb)->Play( 0, 0, DSBPLAY_LOOPING);
     InMilesMutex();

   //
   // Return success
   //

   return 1;
}

//############################################################################
//##                                                                        ##
//## Set the DirectSound HWND to use                                        ##
//##                                                                        ##
//############################################################################

extern "C" extern HWND MSShWnd;

S32 AILCALL AIL_API_set_DirectSound_HWND(HDIGDRIVER dig, HWND wnd)
{
   if (dig == NULL)
      {
      return 0;
      }

   dig->dsHwnd = (U32) wnd;

   if (dig->pDS)
   {
     OutMilesMutex();
     if (!SUCCEEDED(ASDS(dig->pDS)->SetCooperativeLevel(
                                      (HWND)dig->dsHwnd,
                                      dig->ds_priority)))
     {
       InMilesMutex();
       AIL_set_error("SetCooperativeLevel() failed.");
       return 0;
     }
     InMilesMutex();
   }

   MSShWnd=wnd;

   return 1;
}

//############################################################################
//##                                                                        ##
//##  Get DirectSound driver and secondary buffer associated with sample    ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_\
get_DirectSound_info(HSAMPLE              S,//)
                     AILLPDIRECTSOUND       *lplpDS,
                     AILLPDIRECTSOUNDBUFFER *lplpDSB)
{
   HDIGDRIVER dig;
   S32        n;

   if (S == NULL)
      {
      if (DIG_first == NULL)
         {
         return;
         }

      //
      // Return first driver's DirectSound and primary buffer objects
      // if no specific sample handle provided
      //

      if (lplpDS != NULL)
         {
         *lplpDS = (LPDIRECTSOUND) DIG_first->pDS;
         }

      if (lplpDSB != NULL)
         {
         *lplpDSB = ASDSBUF(DIG_first->lppdsb);
         }

      return;
      }

   //
   // Get driver and buffer associated with sample
   //

   dig = S->driver;
   n   = S->secondary_buffer;

   //
   // Return requested values
   //

   if (lplpDS != NULL)
      {
      *lplpDS = (LPDIRECTSOUND) dig->pDS;
      }

   if ((lplpDSB != NULL) && (n != -1))
      {
      *lplpDSB = ASDSBUF(dig->lpbufflist[n]);
      }
}

void AILCALL AIL_LP_lock()
{
  if (AIL_preference[AIL_LOCK_PROTECTION])
  {
    AIL_lock();
  }
}


void AILCALL AIL_LP_unlock()
{
  if (AIL_preference[AIL_LOCK_PROTECTION])
  {
    AIL_unlock();
  }
}


static void SS_start_DIG_driver_playback(HDIGDRIVER dig);

// ------------------------------------------------------------------
// DS_shutdown
// ------------------------------------------------------------------

void DS_shutdown(HDIGDRIVER dig)
{
   S32 rel;
   AIL_LP_lock();    // Lock to avoid DirectSound deadlocks with SS_serve

   MSSLockedIncrement(disable_callbacks);

   rel=dig->released;

   dig->playing=0;
   dig->released=1;
   dig->hWaveOut=0;

   if (!rel)
      {
      //
      // Release secondary buffer, if created
      //

      if (dig->DS_sec_buff != NULL)
         {
         ASDSBUF(dig->DS_sec_buff)->Release();
         dig->DS_sec_buff = NULL;
         }

      //
      // Release the primary buffer
      //

      if (dig->lppdsb != NULL)
         {
         ASDSBUF(dig->lppdsb)->Release();
         dig->lppdsb = NULL;
         }

      //
      // Shut down DirectSound driver
      //

      if (dig->pDS != NULL)
         {
         ASDS(dig->pDS)->Release();
         dig->pDS = NULL;
         }
      }

   //
   // Unload DS DLL
   //

   DirectSoundUnload();

   MSSLockedDecrement(disable_callbacks);

   AIL_LP_unlock();
}

// ------------------------------------------------------------------
// DIG_waveOutPrepareHeader
// ------------------------------------------------------------------

MMRESULT DIG_waveOutPrepareHeader(HDIGDRIVER dig, //)
                                  HWAVEOUT   hwo,
                                  LPWAVEHDR  pwh,
                                  UINT       cbwh)
{
   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      return waveOutPrepareHeader(hwo, pwh, cbwh);
      }

   return MMSYSERR_NOERROR;
}

// ------------------------------------------------------------------
// DIG_waveOutUnprepareHeader
// ------------------------------------------------------------------

MMRESULT DIG_waveOutUnprepareHeader(HDIGDRIVER dig, //)
                                    HWAVEOUT  hwo,
                                    LPWAVEHDR pwh,
                                    UINT      cbwh)
{
   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      return waveOutUnprepareHeader(hwo, pwh, cbwh);
      }

   return MMSYSERR_NOERROR;
}

// ------------------------------------------------------------------
// DIG_waveOutGetDevCaps
// ------------------------------------------------------------------

MMRESULT DIG_waveOutGetDevCaps(HDIGDRIVER dig, //)
                               UINT          uDeviceID,
                               LPWAVEOUTCAPS lpCaps,
                               UINT          wSize)
{
   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      return waveOutGetDevCaps(uDeviceID, lpCaps, wSize);
      }

   AIL_strcpy(lpCaps->szPname, dig->emulated_ds?"Emulated DirectSound - MSS Mixer":"DirectSound - MSS Mixer");

   return MMSYSERR_NOERROR;
}

// ------------------------------------------------------------------
// DIG_waveOutGetID
// ------------------------------------------------------------------

MMRESULT DIG_waveOutGetID(HDIGDRIVER dig, //)
                          HWAVEOUT hWaveOut,
                          PUINT    lpuDeviceID)
{
   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      return waveOutGetID(hWaveOut, lpuDeviceID);
      }

   *lpuDeviceID = 0;

   return MMSYSERR_NOERROR;
}

// ------------------------------------------------------------------
// DIG_waveOutOpen
// ------------------------------------------------------------------

MMRESULT DIG_waveOutOpen(HDIGDRIVER      dig, //)
                         LPHWAVEOUT      lphWaveOut,
                         U32             dwDeviceID,
                         LPWAVEFORMATEX  lpFormat,
                         DWORD           dwCallback,
                         DWORD           dwCallbackInstance,
                         DWORD           fdwOpen)
{
   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      OutMilesMutex();
      MMRESULT res=waveOutOpen(lphWaveOut, dwDeviceID, lpFormat, dwCallback, dwCallbackInstance, fdwOpen);
      InMilesMutex();
      return(res);
      }

   //
   // Fail if already open
   //

   if (dig->DS_initialized)
      {
      AIL_set_error("DirectSound already initialized.");
      return MMSYSERR_ERROR;
      }

   AIL_LP_lock();    // Lock to avoid DirectSound deadlocks with SS_serve

   //
   // Init DS vars
   //

   dig->DS_sec_buff = NULL;
   dig->DS_out_buff = NULL;
   dig->lppdsb = NULL;
   dig->pDS    = NULL;

   //
   // Load the DirectSound dll
   //
   
   if (!DirectSoundLoad())
      {
      AIL_set_error("The DirectSound DLL could not be loaded.");
      AIL_LP_unlock();

      return MMSYSERR_ERROR;
      }

   //
   // If WAVE_MAPPER is passed in, convert to zero GUID
   //

   if ((dwDeviceID==WAVE_MAPPER) || (dwDeviceID==(WAVE_MAPPER&0xffff)))
     dwDeviceID=0;

   dig->guid = dwDeviceID;

   //
   // Set up DirectSound, setting primary buffer to specified format
   //

   if (!setup_directsound(dig))
      {
      DS_shutdown(dig);
      AIL_LP_unlock();
      return MMSYSERR_ERROR;
      }

   if ((!AIL_preference[DIG_DS_USE_PRIMARY]) || (dig->emulated_ds))
      {
      //
      // Calculate size of secondary buffer based on preferences
      //

      dig->DS_frag_cnt = AIL_preference[DIG_DS_FRAGMENT_CNT];

      // make sure we have at least triple the mix ahead
      if (dig->DS_frag_cnt<(AIL_preference[DIG_DS_MIX_FRAGMENT_CNT]*3))
        dig->DS_frag_cnt = AIL_preference[DIG_DS_MIX_FRAGMENT_CNT]*3;

      dig->DS_frag_size = ((dig->wformat.wf.nAvgBytesPerSec *
                           AIL_preference[DIG_DS_FRAGMENT_SIZE]) / 1000L);

      //hack to give emulated mode a little more breathing room
      if (dig->emulated_ds)
        dig->DS_frag_size*=2;

      dig->DS_frag_size = (dig->DS_frag_size + 15) & ~0x0f;

      dig->DS_buffer_size = dig->DS_frag_size * dig->DS_frag_cnt;

      //
      // Allocate secondary buffer configured for specified format
      //

      DSBUFFERDESC dsbdesc;

      AIL_memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));

      dsbdesc.dwSize           =  sizeof(DSBUFFERDESC);

      dsbdesc.dwFlags          =  DSBCAPS_GLOBALFOCUS   |
                                  DSBCAPS_GETCURRENTPOSITION2 |
                                  DSBCAPS_CTRLFREQUENCY |
                                  DSBCAPS_CTRLPAN       |
                                  DSBCAPS_CTRLVOLUME;

      dsbdesc.dwBufferBytes    =  dig->DS_buffer_size;
      dsbdesc.lpwfxFormat      = (LPWAVEFORMATEX) &dig->wformat;

      //
      // Try to create secondary sound buffer
      //

      OutMilesMutex();
      if (!(SUCCEEDED (ASDS(dig->pDS)->CreateSoundBuffer(&dsbdesc,
                                                         (LPDIRECTSOUNDBUFFER FAR *) &dig->DS_sec_buff,
                                                          NULL))))
         {
         InMilesMutex();
         DS_shutdown(dig);

         AIL_set_error("Could not create secondary buffer");
         AIL_LP_unlock();
         return MMSYSERR_ERROR;
         }
      InMilesMutex();

      //
      // Start it
      //

      dig->DS_out_buff = dig->DS_sec_buff;

      SS_start_DIG_driver_playback(dig);
      }
   else
      {
      //
      // Set up fragment info for writing to primary buffer
      //

      dig->DS_out_buff = dig->lppdsb;

      DSBCAPS caps;

      AIL_memset(&caps,0,sizeof(caps));
      caps.dwSize = sizeof(caps);

      ASDSBUF(dig->DS_out_buff)->GetCaps(&caps);

      dig->DS_buffer_size = caps.dwBufferBytes;

      //
      // First, approximate the fragment size by the requested fragment
      // playback period
      //

      dig->DS_frag_size = (dig->wformat.wf.nAvgBytesPerSec *
                           AIL_preference[DIG_DS_FRAGMENT_SIZE]) / 1000L;

      dig->DS_frag_cnt = dig->DS_buffer_size / dig->DS_frag_size;

      //
      // Accept the next larger fragment size which evenly divides the
      // primary buffer, and which consists of an integral number of
      // samples
      //
      // We can't use fewer than 2 fragments
      //

      while (dig->DS_frag_cnt >= 2)
         {
         dig->DS_frag_size = dig->DS_buffer_size / dig->DS_frag_cnt;

         if (dig->DS_frag_size & (dig->wformat.wf.nBlockAlign - 1))
            {
            --dig->DS_frag_cnt;
            continue;
            }

         if ((dig->DS_frag_size * dig->DS_frag_cnt) != dig->DS_buffer_size)
            {
            --dig->DS_frag_cnt;
            continue;
            }

         break;
         }

      if (dig->DS_frag_cnt < 2)
         {
         dig->DS_frag_cnt = 2;
         dig->DS_frag_size = dig->DS_buffer_size / 2;
         }
      }

   //
   // Get initial play position
   //

   U32 p,w;

   if (!(SUCCEEDED (ASDSBUF(dig->DS_out_buff)->GetCurrentPosition(
         &p,
         &w))))
      {
      DS_shutdown(dig);

      AIL_set_error("Could not get buffer position");
      AIL_LP_unlock();
      return MMSYSERR_ERROR;
      }

   dig->DS_last_frag  = (w / dig->DS_frag_size)+1;
   dig->DS_last_write = dig->DS_last_frag;

   dig->DS_skip_time = AIL_preference[DIG_DS_FRAGMENT_CNT] *
                       AIL_preference[DIG_DS_FRAGMENT_SIZE];

   //
   // Return OK
   //

   dig->DS_initialized = 1;

   AIL_LP_unlock();
   return MMSYSERR_NOERROR;
}

// ------------------------------------------------------------------
// DIG_waveOutClose
// ------------------------------------------------------------------

MMRESULT DIG_waveOutClose(HDIGDRIVER dig, HWAVEOUT hwo)
{
   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      MMRESULT result=waveOutClose(hwo);
      return result;
      }

   if (!dig->DS_initialized)
      {
      return MMSYSERR_ERROR;
      }

   DS_shutdown(dig);

   dig->DS_initialized = 0;

   return MMSYSERR_NOERROR;
}

// ------------------------------------------------------------------
// DIG_waveOutReset
// ------------------------------------------------------------------

MMRESULT DIG_waveOutReset(HDIGDRIVER dig, HWAVEOUT hWaveOut)
{
   MMRESULT result;

   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      result=waveOutReset(hWaveOut);
      dig->wom_done_buffers=0;
      return result;
      }

   AIL_LP_lock();    // Lock to avoid DirectSound deadlocks with SS_serve

   do
      {
      result = ASDSBUF(dig->DS_out_buff)->Stop();

      if (result == DSERR_BUFFERLOST)
         {
         ASDSBUF(dig->DS_out_buff)->Restore();
         OutMilesMutex();
         Sleep(10);
         InMilesMutex();
         }
      }
   while (result == DSERR_BUFFERLOST);

   AIL_LP_unlock();

   return MMSYSERR_NOERROR;
}

// ------------------------------------------------------------------
// DIG_waveOutWrite
// ------------------------------------------------------------------

MMRESULT DIG_waveOutWrite(HDIGDRIVER dig,  //)
                          HWAVEOUT   hwo,
                          LPWAVEHDR  pwh,
                          UINT       cbwh)
{
   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      MMRESULT result=waveOutWrite(hwo, pwh, cbwh);
      return(result);
      }

   return MMSYSERR_NOERROR;
}

#else

#define DIG_waveOutClose(a,b) waveOutClose(b)
#define DIG_waveOutGetDevCaps(a,b,c,d) waveOutGetDevCaps(b,c,d)
#define DIG_waveOutGetID(a,b,c) waveOutGetID(b,c)
#define DIG_waveOutOpen(a,b,c,d,e,f,g) waveOutOpen(b,c,d,e,f,g)
#define DIG_waveOutPrepareHeader(a,b,c,d) waveOutPrepareHeader(b,c,d)
#define DIG_waveOutReset(a,b) waveOutReset(b)
#define DIG_waveOutUnprepareHeader(a,b,c,d) waveOutUnprepareHeader(b,c,d)
#define DIG_waveOutWrite(a,b,c,d) waveOutWrite(b,c,d)

#endif

//############################################################################
//##                                                                        ##
//## REVERB_process()                                                       ##
//##                                                                        ##
//############################################################################

void REVERB_process(HSAMPLE   S, //)
                    void const FAR *orig_src,
                    U32       orig_src_fract,
                    void FAR *orig_src_end,
                    S32  FAR *build_dest,
                    void FAR *build_dest_end,
                    S32       playback_ratio,
                    U32       op)
{
   //
   // Reverb algorithm details:
   //
   // Apply reflections to contents of this buffer segment, until
   // either (a) the volume scalar has dropped below 60 dB down (< 2
   // on a 2048-unit linear square); or (b) the end of the reverb buffer
   // is reached via wraparound to the reverb buffer's write cursor.
   //
   // The period of each reflection is determined by the reverb voice's
   // reflection time.
   //
   // The initial volume is determined by scaling the base sample volume
   // by the reverb voice's level.
   //
   // Prior to each reflection, the 0-2048 volume is dropped by a factor 
   // which will reduce it to 2 in the number of reflection intervals which
   // equals the period specified by the reverb buffer's decay time.
   //
   // The initial dest buffer offset for the first reflection is determined
   // by adding the reflection time to the reverb buffer's current_offset.
   // Subsequent reflection offsets are determined by adding the reflection
   // period to the last reflection's offset.
   //
   // The reverb buffer's write cursor, however, is advanced only when the
   // reverb buffer data is added to the build buffer contents in the postmix
   // stage.  In the postmix stage, the contents of the reverb buffer are
   // added into the output build buffer, and the buffer's write cursor
   // is advanced accordingly.
   //

   HDIGDRIVER dig = S->driver;

   //
   // Fail if reverb parms invalid or disabled
   //

   if ((S->reverb_reflect_time < 0.0001F) ||
       (S->reverb_level        < 0.001F))
      {
      return;
      }

   //
   // Get initial destination offset
   //
   // The size of the reverb buffer is a multiple of the build buffer size,
   // and this position is guaranteed to be aligned to a 
   // build-buffer-size boundary
   //

   S32 dest_offset = dig->reverb_buffer_position;

   //
   // Adjust destination offset by difference between build_dest pointer and
   // beginning of build buffer
   //
   // This handles the cases where buffer-switching, looping, etc.
   // require multiple calls to the mixer to fill the build buffer
   // completely
   //

   dest_offset += AIL_ptr_dif(build_dest,
                              dig->build_buffer);

   S32 FAR *write_cursor = (S32 FAR *) AIL_ptr_add(dig->reverb_buffer,
                                                   dest_offset);

   //
   // Calculate # of bytes in reverb buffer that correspond to 
   // reflection time interval
   //

   F32 adv = F32(S->playback_rate) * S->reverb_reflect_time;

   S32 reflection_advance = S32(adv) * 
                           ((op & M_DEST_STEREO) ? 8 : 4);

   //
   // Calculate factor by which to decrease amplitude each reflection
   //
   // We want the volume to fall to 2/2048 (1/1024=-60 dB) after
   // reverb_decay_time seconds elapses.  Each reflection interval is
   // reverb_reflect_time seconds long, so the d/r quotient is the total
   // number of reflection periods over which the volume decay should occur.
   //

   S32 decay_intervals = S32(S->reverb_decay_time / S->reverb_reflect_time);

   if (decay_intervals == 0)
      {
      decay_intervals = 1;
      }

   S32 volume_decay = (S->base_scale - 2) / decay_intervals;

   if (volume_decay == 0)
      {
      volume_decay = 1;
      }

   //
   // Get (potential) amount of destination data to write to reverb 
   // buffer for each reflection interval
   //

   S32 max_dest_bytes = AIL_ptr_dif(build_dest_end, build_dest);

   S32 FAR *reverb_end = (S32 FAR *) AIL_ptr_add(dig->reverb_buffer,
                                                 dig->reverb_buffer_size);

   //
   // Modify mixer operation code to support reverb (always use volume
   // scaling, never do filtering)
   //

   op |=  M_VOL_SCALING;
   op &= ~M_FILTER;

   //
   // Generate reflections in reverb buffer until buffer full or sound
   // dies out
   // 

   S32 left_scale  = (S32) (((F32) S->left_scale)  * S->reverb_level);
   S32 right_scale = (S32) (((F32) S->right_scale) * S->reverb_level);
   S32 base_scale  = (S32) (((F32) S->base_scale)  * S->reverb_level);

   S32  FAR *dest;
   void FAR *dest_end;

   S32 wrapped = 0;
   
   while (1)
      {
#if 0
      S32 tolerance = reflection_advance / 1000;

      dest_offset -= tolerance;

      dest_offset += (rand() % (2 * tolerance));

      dest_offset &= ~7;
#endif

      //
      // Add one reflection period to dest buffer offset
      //

      dest_offset += reflection_advance;

      if (dest_offset >= dig->reverb_buffer_size)
         {
         dest_offset -= dig->reverb_buffer_size;
         }

      //
      // Decrease amplitude by decay value; exit if -60 dB point reached
      //

      base_scale  = max(base_scale  - volume_decay, 0);
      left_scale  = max(left_scale  - volume_decay, 0);
      right_scale = max(right_scale - volume_decay, 0);

      if (base_scale <= 2)
         {
         break;
         }

      --decay_intervals;

      //
      // Add reflection to buffer, handling buffer wrap as needed
      //

      S32 left_val  = S->left_val;
      S32 right_val = S->right_val;

      dest     = (S32 FAR *) AIL_ptr_add(dig->reverb_buffer, dest_offset);
      dest_end =             AIL_ptr_add(dest,               max_dest_bytes);

      void const FAR *src        = orig_src;
      U32       src_fract  = orig_src_fract;
      void FAR *src_end    = orig_src_end;

      while (1)
         {
         if (AIL_ptr_dif(dest, reverb_end) == 0)
            {
            S32 size = AIL_ptr_dif(dest_end, dest);
            dest     = dig->reverb_buffer;
            dest_end = AIL_ptr_add(dest, size);
#if 0
            AIL_debug_printf("!");
#endif
            continue;
            }

         S32 remnant = AIL_ptr_dif(dest_end, reverb_end);

         if (remnant > 0)
            {
#if 0
            AIL_debug_printf("W");
#endif
            dest_end = reverb_end;
            }

         if ((AIL_ptr_dif(write_cursor, dest)     > 0) &&
             (AIL_ptr_dif(dest_end, write_cursor) > 0))
            {
            dest_end = write_cursor;
            remnant  = 0;
#if 0
            AIL_debug_printf("T");  // truncated write because of wrap
#endif
            }

#ifdef IS_WIN32
         
         S->pipeline[DP_MERGE].TYPE.MIX.MSS_mixer_merge(&src,
                                                        &src_fract,
                                                         src_end,
                                      (S32 FAR * FAR *) &dest,
                                                         dest_end,
                                                        &left_val,
                                                        &right_val,
                                                         playback_ratio,
                                                         left_scale,
                                                         right_scale,
                                                         op,
                                                         S->driver->use_MMX);
#else

         U32 src_offset  = LOWORD(src);
         U32 dest_offset = LOWORD(dest);

         U32 src_end_offset  = src_offset  + AIL_ptr_dif(src_end, src);
         U32 dest_end_offset = dest_offset + AIL_ptr_dif(dest_end,dest);

         S->pipeline[DP_MERGE].TYPE.MIX.MSS_mixer_merge(HIWORD(src),
                                                        HIWORD(dest),
                                                       &src_fract,
                                                       &src_offset,
                                                       &dest_offset,
                                                        src_end_offset,
                                                        dest_end_offset,
                                                       &left_val,
                                                       &right_val,
                                                        playback_ratio,
                                                        (left_scale << 16) | right_scale,
                                                        op);

         src  =             AIL_ptr_add(src,  src_offset  - LOWORD(src));
         dest = (S32 FAR *) AIL_ptr_add(dest, dest_offset - LOWORD(dest));

#endif

         if (remnant > 0)
            {
            dest     = dig->reverb_buffer;
            dest_end = AIL_ptr_add(dest, remnant);
            }
         else
            {
            break;
            }
         }
      }
}

//############################################################################
//##                                                                        ##
//## ASI callback routine to fetch encoded source data from sample          ##
//##                                                                        ##
//## Used by both MSSWO.CPP and MSSDS.CPP                                   ##
//##                                                                        ##
//############################################################################

S32 AILCALLBACK DP_ASI_DECODER_callback(U32       user, //)
                                        void FAR *dest,
                                        S32       bytes_requested,
                                        S32       offset)
{
   HSAMPLE S = (HSAMPLE) user;

   S32 total = 0;
   S32 orig = bytes_requested;

   S32 n = S->current_buffer;

   if (offset != -1)
      {
      S->pos[n] = offset;
      }

   //
   // Fulfill as much of request as possible from current buffer
   //

   S32 amount = bytes_requested;

   if ((S->pos[n] + amount) > S->len[n])
      {
      amount = S->len[n] - S->pos[n];
      }

   AIL_memcpy(dest,
              AIL_ptr_add(S->start[n], S->pos[n]),
              amount);

   dest = AIL_ptr_add(dest, amount);

   S->pos[n] += amount;

   total += amount;
   bytes_requested -= amount;

   //
   // If end of buffer reached with samples left to fetch, try to
   // switch buffers
   //

   if (bytes_requested > 0)
      {
      if (S->EOB != NULL)
         ++S->doeob;

      if (S->service_type == 2)
         {

         // reset the ASI, if requested
         if (S->reset_ASI[n])
         {
           // return without switching buffers if we were requested to do a reset
           return(total);
         }

         n ^= 1;

         //
         // If explicit 0-length buffer was posted, fall through to allow
         // the sample to terminate
         //

         if (S->done[n])
            {
            return total;
            }
         else
            {
            //
            // If alternate buffer not yet initialized, break out of loop
            // to allow application to supply data as soon as it can
            //

            if (!S->len[n])
               {
               S->starved = 1;
               return total;
               }

            //
            // If alternate buffer already played, break out of loop to
            // allow application to refresh it as soon as possible
            //

            if (S->pos[n])
               {
               S->starved = 1;
               return total;
               }

            //
            // Otherwise, alternate buffer is ready to play -- switch to
            // it and keep filling output buffer
            //

            S->current_buffer = n;

            amount = bytes_requested;

            if ((S->pos[n] + amount) > S->len[n])
               {
               amount = S->len[n] - S->pos[n];
               }

            AIL_memcpy(dest,
                       AIL_ptr_add(S->start[n], S->pos[n]),
                       amount);

            S->pos[n] += amount;

            total += amount;
            }
         }
      }

   return total;
}

//############################################################################
//##                                                                        ##
//## Return # of bytes per sample                                           ##
//##                                                                        ##
//############################################################################

U32 SS_granularity(U32 format)
{
   switch (format)
      {
      case DIG_F_MONO_16:
      case DIG_F_STEREO_8:
         return(2);

      case DIG_F_STEREO_16:
         return(4);

      default:
         return(1);
      }
}

//############################################################################
//##                                                                        ##
//## Start driver-based DMA buffer playback                                 ##
//##                                                                        ##
//############################################################################

static void SS_start_DIG_driver_playback(HDIGDRIVER dig)
{
   LPWAVEHDR current;
   S32       i;
   
   //
   // Always clear any reset request when starting playback
   //

   dig->request_reset = 0;

   //
   // Return if playback already active
   //

   if (dig->playing)
      {
      return;
      }

   //
   // Set playing flag
   //

   dig->playing = 1;

   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      //
      // Set up to force all buffers to be posted
      //

      current = dig->first;

      for (i=0; i < dig->n_buffers; i++)
         {
         CURRENT_USER->flags |= HDR_FORCE_POST;

         current = CURRENT_USER->next;
         }

      //
      // Manually force foreground timer service to begin playing sound
      // immediately
      //

      if (AIL_bkgnd_flag == 0)
         {
         low_serve();
         }
      }
   else
      {
#ifdef IS_WIN32
      AIL_LP_lock();    // Lock to avoid DirectSound deadlocks with SS_serve

      MMRESULT result;

      clear_dsbuf(dig->DS_out_buff,dig->wformat.wBitsPerSample);

      do
         {
         result = ASDSBUF(dig->DS_out_buff)->Play(0,
                                                  0,
                                                  DSBPLAY_LOOPING);

         if (result == DSERR_BUFFERLOST)
            {
            ASDSBUF(dig->DS_out_buff)->Restore();
            OutMilesMutex();
            Sleep(10);
            InMilesMutex();
            }
         }
      while (result == DSERR_BUFFERLOST);

      if (result)
         {
         AIL_set_error("Could not start output buffer");
         }

      AIL_LP_unlock();
#endif
      }
}

//############################################################################
//##                                                                        ##
//## Stop driver-based DMA buffer playback                                  ##
//##                                                                        ##
//## Warning: if !dig->reset_works, do NOT call this function except at     ##
//## shutdown time!                                                         ##
//##                                                                        ##
//############################################################################

static void SS_stop_DIG_driver_playback(HDIGDRIVER dig)
{
   if (!dig->playing)
      {
      return;
      }

   //
   // Stop playback ASAP and return all buffers
   //
   // If we're not in background mode, we can reset immediately --
   // otherwise, post a request for the next foreground timer call
   //

   if (!AIL_background())
      {
      dig->playing = 0;
      DIG_waveOutReset(dig,dig->hWaveOut);
      }
   else
      {
      dig->request_reset = 1;
      }
}

//############################################################################
//##                                                                        ##
//## Flush mixer buffer                                                     ##
//##                                                                        ##
//############################################################################

static void SS_flush (HDIGDRIVER dig)
{
   //
   // Initialize the build buffer by flushing with 0
   //

   dig->pipeline[DP_FLUSH].TYPE.MIX.MSS_mixer_flush(dig->build_buffer,
                                                    dig->build_size,
                                                    NULL,
                                                    0,
                                                    dig->use_MMX);

#ifdef IS_WIN32

   //
   // Invoke installable filters
   //

   FLT_call_premix_processors(dig);

#endif
}

//############################################################################
//##                                                                        ##
//## Merge source data into output buffer by calling processor and mixer    ##
//## handlers                                                               ##
//##                                                                        ##
//############################################################################

static S32 SS_merge (HSAMPLE S, //)
                     S32     in_len,
                     S32     out_pos)
{
   //
   // Set mixer operation code
   //

   S32 op = 0;

   if (S->driver->hw_format & DIG_F_STEREO_MASK)
      {
      op |= M_DEST_STEREO;

      //
      // Reverse left/right channels if DIG_PCM_ORDER specified
      //

      if (S->flags & DIG_PCM_ORDER)
         {
         op |= M_ORDER;
         }
      }

   if (S->format & DIG_F_STEREO_MASK)
      {
      op |= M_SRC_STEREO;
      }

   if ((S->format & DIG_F_16BITS_MASK) || (S->format & DIG_F_ADPCM_MASK))
      {
      op |= M_SRC_16;
      }

   // 
   // Set M_SRC_SCALE
   // 
   // Force M_SRC_SCALE if stereo source mixed into mono output
   //

   if ((op & M_SRC_STEREO) && (!(op & M_DEST_STEREO)))
      {
      op |= M_VOL_SCALING;
      }
   else
      {
      if ((S->left_scale  != 2047) || 
          (S->right_scale != 2047))
         {
         op |= M_VOL_SCALING;
         }
      }

   //
   // Calculate sampling fraction
   //

   S32 playback_ratio = (S32) ((((F32) S->playback_rate) * 65536.0F) /
                                 (F32) S->driver->DMA_rate);

   if ((playback_ratio > (65536 + AIL_preference[DIG_RESAMPLING_TOLERANCE])) ||
       (playback_ratio < (65536 - AIL_preference[DIG_RESAMPLING_TOLERANCE])))
      {
      op |= M_RESAMPLE;
      }

   //
   // Calculate # of bytes needed from ASI or ADPCM decoder to fill
   // build buffer, rounded up
   //
   // This calculation takes resampling into account, as well as the
   // difference in sample sizes between the output of the decode stage
   // ("source sample") and the mixer build buffer.  The resulting sample
   // count reflects the amount of data needed to fill the build buffer
   // from its current out_pos write position.
   // 
   // Output from source ADPCM or ASI decoder is assumed to be 16-bit,
   // so src_size is either 2 or 4 depending on the number of channels per
   // sample
   // 

   S32 src_size = (S->format & DIG_F_STEREO_MASK) ? 4 : 2;

   src_size *= S->playback_rate;

   S32 dest_size = SS_granularity(S->driver->hw_format) * S->driver->DMA_rate;

   S32 k = S->driver->build_size / S->driver->buffer_size;

   S32 needed = S32(F32(S->driver->buffer_size - (out_pos / k)) * F32(src_size) / F32(dest_size));

   needed = (needed + 3) & ~3;

   if (needed == 0)
      {
      needed = 4;
      }

   needed = min(S->driver->decode_buffer_size, needed);

   //
   // If ASI or ADPCM compression is in use, unpack data into
   // temporary buffer
   //

   void const FAR *src;
   void FAR *src_end;

   if (S->pipeline[DP_ASI_DECODER].active)
      {
      //
      // ASI decoder in use -- source data must be obtained from
      // DP_ASI_DECODER_callback() via ASI_stream_process()
      //

      ASISTAGE *ASI = &S->pipeline[DP_ASI_DECODER].TYPE.ASI;

      S32 nbytes = ASI->ASI_stream_process(ASI->stream,
                                           S->driver->decode_buffer,
                                           needed);

      src = S->driver->decode_buffer;

      src_end = AIL_ptr_add(src, nbytes);
      }
   else
      {
      //
      // No ASI decoder in use -- source data is available in its entirety
      // from the current buffer
      //

      src = AIL_ptr_add(S->start[S->current_buffer],
                        S->pos  [S->current_buffer]);

      src_end = AIL_ptr_add(src,
                            in_len);

      //
      // If ADPCM compression in use, decompress the data into the decode
      // buffer
      //

      if (S->format & DIG_F_ADPCM_MASK)
         {
         void const FAR *in  = src;
         void FAR *out = S->driver->decode_buffer;

         //
         // Decode block of data from source sample to decode buffer
         //
         // Size of block to decode is determined by decoder
         // given smaller amount of available and needed data
         //

         if (S->format & DIG_F_STEREO_MASK)
            {
            DecodeADPCM_STEREO(&out, &in, needed, in_len, &S->adpcm);
            }
         else
            {
            DecodeADPCM_MONO(&out, &in, needed, in_len, &S->adpcm);
            }

         //
         // Update source sample position index
         //
         // Note: In Win16, *in and *out are normalized upon return from
         // the ADPCM decoder.  Their selectors may not be the same as the
         // original selectors passed to *in and *out.  Since we are writing
         // to the (small) decode buffer, *out will not be renormalized in
         // this application -- but *in often will.
         //

         src = S->driver->decode_buffer;

         src_end = out;

         S->pos[S->current_buffer] = AIL_ptr_dif(in,
                                                 S->start[S->current_buffer]);
         }
      }

   //
   // Get dest address range
   //

   void FAR *dest = AIL_ptr_add(S->driver->build_buffer,
                                out_pos);

   void FAR *dest_end = AIL_ptr_add(S->driver->build_buffer,
                                    S->driver->build_size);

   //
   // If empty source or dest range, return build buffer full
   //

   if ((AIL_ptr_dif(dest_end, dest) <= 0) ||
       (AIL_ptr_dif(src_end, src) <= 0))
      {
      return S->driver->build_size;
      }

   //
   // Enable filtering if preference set
   //

   if (AIL_preference[DIG_ENABLE_RESAMPLE_FILTER])
      {
      op |= M_FILTER;
      }

#if 0
   //
   // For diagnostic purposes only
   //

   if ((GetAsyncKeyState(VK_SHIFT) & 0x8000))
      {
      op &= ~M_FILTER;
      ++*(char *) 0xb0000;
      }

   if ((GetAsyncKeyState(VK_MENU) & 0x8000))
      {
      op |= M_ORDER;
      ++*(char *) 0xb0002;
      }
#endif

   //
   // If source and dest rates are exactly identical, disable resample
   // filtering
   //

   if (S->playback_rate == S->driver->DMA_rate)
      {
      op &= ~M_FILTER;
      }

   //
   // Call reverb processor, if appropriate
   //

   if (S->driver->reverb_buffer_size != 0)
      {
      REVERB_process(S,
                     src,
                     S->src_fract,
                     src_end,
         (S32 FAR *) dest,
                     dest_end,
                     playback_ratio,
                     op);
      }

   //
   // Call mixer provider
   //

#ifdef IS_WIN32

   //
   // Call installable filter first, skipping mixer call altogether if
   // requested
   //

   S32 call_mixer = 1;

   void const FAR *filter_src       = src;
   U32       filter_src_fract = S->src_fract;
   void FAR *filter_dest      = dest;
   S32       filter_left_val  = S->left_val;
   S32       filter_right_val = S->right_val;

   if (S->pipeline[DP_FILTER].active)
      {
      FLTPROVIDER FAR *F = S->pipeline[DP_FILTER].TYPE.FLT.provider;

      call_mixer = F->sample_process(S->pipeline[DP_FILTER].TYPE.FLT.sample_state,
                                    &filter_src,
                                    &filter_src_fract,
                                     src_end,
                  (S32 FAR * FAR *) &filter_dest,
                                     dest_end,
                                    &filter_left_val,
                                    &filter_right_val,
                                     playback_ratio,
                                     S->left_scale,
                                     S->right_scale,
                                     S->base_scale,
                                    &S->pipeline[DP_MERGE].TYPE.MIX,
                                     op);
      }

   if (call_mixer)
      {
      //
      // Execute mixer call normally with original src and dest pointers,
      // discarding pointers returned by filter
      //
      
      S->pipeline[DP_MERGE].TYPE.MIX.MSS_mixer_merge(&src,
                                                     &S->src_fract,
                                                      src_end,
                                   (S32 FAR * FAR *) &dest,
                                                      dest_end,
                                                     &S->left_val,
                                                     &S->right_val,
                                                      playback_ratio,
                                                      S->left_scale,
                                                      S->right_scale,
                                                      op,
                                                      S->driver->use_MMX);
      }
   else
      {
      //
      // Skip mixer call, and update source and dest pointers based on
      // values returned from filter
      //

      src          = filter_src;
      S->src_fract = filter_src_fract;
      dest         = filter_dest;
      S->left_val  = filter_left_val;
      S->right_val = filter_right_val;
      }

#else

   U32 src_offset  = LOWORD(src);
   U32 dest_offset = LOWORD(dest);

   U32 src_end_offset  = src_offset  + AIL_ptr_dif(src_end, src);
   U32 dest_end_offset = dest_offset + AIL_ptr_dif(dest_end,dest);

   S->pipeline[DP_MERGE].TYPE.MIX.MSS_mixer_merge(HIWORD(src),
                                                  HIWORD(dest),
                                                 &S->src_fract,
                                                 &src_offset,
                                                 &dest_offset,
                                                  src_end_offset,
                                                  dest_end_offset,
                                                 &S->left_val,
                                                 &S->right_val,
                                                  playback_ratio,
                                                  (S->left_scale << 16) | S->right_scale,
                                                  op);

   src  =             AIL_ptr_add(src,  src_offset  - LOWORD(src));
   dest = (S32 FAR *) AIL_ptr_add(dest, dest_offset - LOWORD(dest));

#endif

   //
   // If not decoding via buffer, update source pointer normally
   //

   if ((!S->pipeline[DP_ASI_DECODER].active) &&
       (!(S->format & DIG_F_ADPCM_MASK)))
      {
      S->pos[S->current_buffer] = AIL_ptr_dif(src,
                                              S->start[S->current_buffer]);
      }

   //
   // Return updated build buffer output offset
   //

   return AIL_ptr_dif(dest,
                      S->driver->build_buffer);
}

//############################################################################
//##                                                                        ##
//## Copy mixer buffer to DMA buffer                                        ##
//##                                                                        ##
//############################################################################

static void SS_copy (HDIGDRIVER dig, void FAR *lpWaveAddr)
{
   //
   // Add reverb buffer contents, if any, to build buffer
   //

   if (dig->reverb_buffer_size != 0)
      {
      S32 FAR *src  = (S32 FAR *) AIL_ptr_add(dig->reverb_buffer,
                                              dig->reverb_buffer_position);
      S32 FAR *dest =  dig->build_buffer;

#ifdef IS_WIN32

      S32 i = dig->build_size / sizeof(S32);

      while (i--)
         {
         dest[i] += src[i];
         }

#else

      MSS_reverb_merge(src,
                       dest,
                       dig->build_size);

#endif

      //
      // Clear this segment of reverb buffer and advance write pointer
      //

      AIL_memset(src,
                 0,
                 dig->build_size);

      dig->reverb_buffer_position += dig->build_size;

      if (dig->reverb_buffer_position >= dig->reverb_buffer_size)
         {
         dig->reverb_buffer_position = 0;
         }
      }


#ifdef IS_WIN32

   //
   // Invoke installable filters
   //

   FLT_call_postmix_processors(dig);

#endif

   //
   // Copy build buffer contents to output buffer
   //

   dig->pipeline[DP_COPY].TYPE.MIX.MSS_mixer_copy(dig->build_buffer,
                                                  dig->build_size,
                                                  lpWaveAddr,
                                                  dig->hw_format,
                                                  dig->use_MMX);
}


//############################################################################
//##                                                                        ##
//## Copy data from source sample to build buffer, with mixing and ASI      ##
//## decompression                                                          ##
//##                                                                        ##
//############################################################################

void SS_stream_to_buffer(HSAMPLE S)
{
   S32 out_pos;
   S32 in_len;
   S32 next_buffer;

   S->dosob=0;
   S->doeob=0;
   S->doeos=0;

   //
   // Bail if source stream ended in previous call
   //

   if (S->status != SMP_PLAYING)
      {
      return;
      }

   //
   // Init output buffer position to beginning of build buffer
   //

   out_pos = 0;

   //
   // Merge source data into output buffer until source exhausted or output full
   //
   // Loop can be exited under the following conditions:
   //
   // 1) Output buffer full (normal condition)
   //
   // 2) Source sample ended (normal condition)
   //
   // 3) Source stream "starved" (abnormal condition)
   //

   while (out_pos < S->driver->build_size)
      {
      //
      // Set input len = size of source block to merge, based on
      // size and playback position of source sample
      //

      in_len = S->len[S->current_buffer] - S->pos[S->current_buffer];

      //
      // Initial block may terminate before end of source buffer, if loop
      // endpoint has been declared
      //

      if (S->loop_count != 1)
         {
         if ((S->loop_end != -1) && ((U32) S->loop_end < S->len[S->current_buffer]))
            {
            in_len = S->loop_end - S->pos[S->current_buffer];
            }
         }

      //
      // If no input data left, check for buffer switching and loop iteration
      //
      // (Note: in_len may fall below 0 if source data is undersampled!)
      //

      if (in_len <= 0)
         {
         //
         // Set end-of-buffer function, if any, to signal end of loop block
         // or buffer
         //
         // Note that this function will be called repeatedly if buffer
         // starvation occurs
         //

         if (S->EOB != NULL)
           ++S->doeob;

         // reset the ASI, if requested
         if (S->reset_ASI[S->current_buffer])
         {
           // if requested, do a reset now that the ASI decoder has the data
           ASISTAGE *ASI = &S->pipeline[DP_ASI_DECODER].TYPE.ASI;
           ASI->ASI_stream_seek(ASI->stream, -2);
           S->reset_ASI[S->current_buffer]=0;
         }

         //
         // If looping active, go back to beginning of loop to fetch next
         // source data block
         //

         if (S->loop_count != 1)
            {
            //
            // Reset source sample position to beginning of loop
            //

            S->pos[S->current_buffer] = S->loop_start;

            //
            // Reset ADPCM offset to the end of decode buffer
            // to force a decode buffer refill
            //

            if (S->format & DIG_F_ADPCM_MASK)
               {
               S->adpcm.blockleft = 0;
               S->adpcm.extrasamples = 0;
               }

            //
            // Reset ASI state, if applicable
            //

            if (S->pipeline[DP_ASI_DECODER].active)
               {
               ASISTAGE *ASI = &S->pipeline[DP_ASI_DECODER].TYPE.ASI;
               ASI->ASI_stream_seek(ASI->stream, -2);
               }

            //
            // Decrement loop count if not infinite
            //

            if (S->loop_count != 0)
               {
               --S->loop_count;
               }

            //
            // Recalculate size and address of source block
            //

            continue;
            }


         //
         // If streaming sample and alternate buffer available,
         // set up to fetch next source data block from it
         //
         // Typically, buffer-switching takes place in the ASI fetch
         // callback function if an ASI decoder is in use.  We must
         // still be able to switch here, though, in case the callback
         // function returns at the end of its current buffer.
         //
         // Note that control of ASI data source length (e.g., for loop
         // termination) is limited to the decode-buffer granularity
         // specified by the DIG_DECODE_BUFFER_SIZE preference.
         //

         if (S->service_type == 2)
            {

            next_buffer = S->current_buffer ^ 1;

            //
            // If explicit 0-length buffer was posted, fall through to allow
            // the sample to terminate
            //

            if (!S->done[next_buffer])
               {
               //
               // If alternate buffer not yet initialized, break out of loop
               // to allow application to supply data as soon as it can
               //

               if (!S->len[next_buffer])
                  {
                  S->starved=1;
                  goto docbs;
                  }

               //
               // If alternate buffer already played, break out of loop to
               // allow application to refresh it as soon as possible
               //

               if (S->pos[next_buffer])
                  {
                  S->starved=1;
                  goto docbs;
                  }

               //
               // Otherwise, alternate buffer is ready to play -- switch to
               // it and keep filling output buffer
               //

               S->current_buffer = next_buffer;
               continue;
               }
            }

         //
         // If new input data is still not available after looping
         // and/or switching buffers, terminate the sample
         //

         S->status = SMP_DONE;
         S->starved = 1;

         if (S->EOS != NULL)
            ++S->doeos;

         goto docbs;
         }

      //
      // Set to perform start-of-block callback
      //

      if (S->SOB != NULL)
         ++S->dosob;

      //
      // Merge source block with contents of output buffer, updating output
      // buffer position counter
      //
      // Note that buffer switching may also take place within this routine if
      // an ASI decoder is in use!
      //

      out_pos = SS_merge(S,         // sample handle
                         in_len,    // max len of source block to copy
                         out_pos);  // dest position in build buffer

      }

docbs:

  //
  // make the callbacks after everything's been merged
  //

  while (S->dosob--)
     {
     MSS_do_cb1( (AILSAMPLECB) ,S->SOB,S->driver->callingDS,S->SOB_IsWin32s,
     S);
     }

  while (S->doeob--)
     {
     MSS_do_cb1( (AILSAMPLECB) ,S->EOB,S->driver->callingDS,S->EOB_IsWin32s,
     S);
     }

  while (S->doeos--)
     {
     MSS_do_cb1( (AILSAMPLECB) ,S->EOS,S->driver->callingDS,S->EOS_IsWin32s,
     S);
     }
}

//############################################################################
//##                                                                        ##
//## Fill output buffer with mixed data and/or silence                      ##
//##                                                                        ##
//############################################################################

static void SS_fill(HDIGDRIVER dig, void FAR *lpData)
{
   static S32        cnt,n;
   static HSAMPLE    S;

   U32 start_us = AIL_us_count();
   S32 current_ms = start_us/1000;

   //
   // Flush build buffer with silence
   //

   SS_flush(dig);

   //
   // Merge active samples (if any) into build buffer
   //

   cnt = 0;

   for (n = dig->n_samples,S = &dig->samples[0]; n; --n,++S)
      {
      //
      // Skip sample if stopped, finished, or not allocated
      //

      if (S->status != SMP_PLAYING)
         {
         continue;
         }

      ++cnt;

      //
      // Convert sample to 16-bit signed format and mix with
      // contents of build buffer
      //
      // Note that SS_stream_to_buffer() may invoke user callback functions
      // which may free or otherwise alter the sample being merged
      //
      // If ASI codec is in use, buffer maintenance can take place within
      // either SS_stream_to_buffer() or the ASI fetch callback
      //

      SS_stream_to_buffer(S);
      }

   //
   // Set number of active samples
   //

   dig->n_active_samples = cnt;

   //
   // Copy build buffer contents to DMA buffer
   //

   SS_copy(dig, lpData);

   //
   // If no samples active for two consecutive interrupts (three in Win32),
   // request DMA halt
   //

   if (dig->n_active_samples)
      {
      dig->quiet = 0;
      }
   else
      {
#ifdef IS_WIN32
      if (dig->quiet++ == 3)
#else
      if (dig->quiet++ == 2)
#endif
         {
         if (dig->reset_works)
            {
            //
            // Don't stop the DirectSound secondary buffer -- we only need to
            // stop the waveOut buffer to reduce subsequent sample latency,
            // which isn't a problem with DirectSound
            //

            if (AIL_preference[DIG_USE_WAVEOUT])
               {
               SS_stop_DIG_driver_playback(dig);
               }
            }
         }
      }

   //
   // keep the profiling information
   //

   U32 end_us=AIL_us_count();

   start_us=(end_us<start_us)?(end_us+(0xffffffff-start_us)):(end_us-start_us);

   dig->us_count+=start_us;
   if (dig->us_count>10000000) {
     dig->ms_count+=(dig->us_count/1000);
     dig->us_count=dig->us_count%1000;
   }
}

//############################################################################
//##                                                                        ##
//## Calculate 11-bit sample volume scalars according to SAMPLE volume and  ##
//## panpot settings                                                        ##
//##                                                                        ##
//############################################################################

static void SS_calculate_volume_scalars(HSAMPLE S)
{
   if (S->volume > 127)
      {
      S->volume = 127;
      }
   else if (S->volume < 0)
      {
      S->volume = 0;
      }

   if (S->pan > 127)
      {
      S->pan = 127;
      }
   else if (S->pan < 0)
      {
      S->pan = 0;
      }

   S32 v = S->volume;
   S32 p = S->pan;

   //
   // Scale volume by master volume
   //

   v = (v * S->driver->master_volume) / 127;

   if (v < 0)
      {
      v = 0;
      }

   if (v > 127)
      {
      v = 127;
      }

   //
   // Set left and right scalars for sample mixing
   //
   // Revised mixer kernel uses 11-bit volume scalars
   //

   S->left_scale  = (v * pan_graph[127-p]) / 8;
   S->right_scale = (v * pan_graph[    p]) / 8;

   if (S->left_scale >= 2032)
      {
      S->left_scale = 2047;
      }

   if (S->right_scale >= 2032)
      {
      S->right_scale = 2047;
      }

   //
   // Store base volume for use in reverb calculations
   //

   S->base_scale = v * 16;

   if (S->base_scale >= 2032)
      {
      S->base_scale = 2047;
      }
}

//############################################################################
//##                                                                        ##
//## Foreground callback function to keep buffer chain in queue             ##
//##                                                                        ##
//## Causes all DONE WAVEHDRS in chain to be sent to the driver, in order,  ##
//## via waveOutWrite()                                                     ##
//##                                                                        ##
//############################################################################

static void SS_foreground_service()
{
   HDIGDRIVER dig;
   LPWAVEHDR  current,start;

   //
   // Return immediately if callbacks disabled
   //

   if (disable_callbacks)
      {
      return;
      }

   //
   // Skip foreground processing if DirectSound in use
   //

   if (!AIL_preference[DIG_USE_WAVEOUT])
   {
     return;
   }

   MSSLockedIncrement(SS_servicing);

   if (SS_servicing==1)
   {

     //
     // Disallow timer service
     //

     AIL_lock();

     //
     // Iterate digital driver list
     //

     dig = DIG_first;

     while (dig != NULL)
        {
        //
        // Perform waveOutReset(), if requested
        //

        if (dig->request_reset)
           {
           dig->request_reset = 0;
           dig->playing       = 0;

           DIG_waveOutReset(dig,dig->hWaveOut);
           }

        //
        // Skip drivers which are not playing
        //

        if (!dig->playing)
           {
           dig = dig->next;
           continue;
           }

        //
        // Begin search at first queue entry
        //

        current = dig->first;

        //
        // Because buffer list is circular and contains two subqueues of
        // entries, the subqueue of DONE buffers begins immediately
        // "after" the subqueue of pending buffers, if any.
        //
        // First, locate the subgroup of pending (posted but unplayed) buffers.
        // If no pending buffers exist, then the entire queue is full of
        // DONE buffers that need to be reposted.
        //

        start = current;

        while (NEED_POST(current))
           {
           current = CURRENT_USER->next;

           if (current == start)
              {
              break;
              }
           }

        //
        // If "starvation" occurred, or if this is an initial call to start
        // playback, re-prime queue from beginning
        //

        if (NEED_POST(current))
           {
           //
           // Prime queue buffers
           //

           start = dig->first;
           SS_fill(dig, start->lpData);

           start = ((HDR_USER FAR *) start->dwUser)->next;
           SS_fill(dig, start->lpData);

           start = ((HDR_USER FAR *) start->dwUser)->next;
           SS_fill(dig, start->lpData);

  #ifdef IS_WIN32

           start = ((HDR_USER FAR *) start->dwUser)->next;
           SS_fill(dig, start->lpData);

  #endif
           current = dig->first;
           }

        //
        // If we've hit the subqueue of pending buffers, find the first
        // DONE buffer after it and start posting.
        //
        // If no pending buffers found, fall through and start posting.
        //

        start = current;

        while (!NEED_POST(current))
           {
           current = CURRENT_USER->next;

           if (current == start)
              {
              break;
              }
           }

        //
        // Post buffers until starting point reached or already-pending buffer
        // encountered
        //

        start = current;

        while NEED_POST(current)
           {
           //
           // Clear "virgin" bit
           //

           CURRENT_USER->flags &= ~HDR_FORCE_POST;

           //
           // Do waveOutWrite() call to post this header
           //

           DIG_waveOutWrite(dig,
                            dig->hWaveOut,
                            current,
                            sizeof(WAVEHDR));

           //
           // Iterate through rest of "done" chain
           //

           current = CURRENT_USER->next;

           if (current == start)
              {
              break;
              }
           }

        //
        // Move on to next HDIGDRIVER
        //

        dig = dig->next;
        }

     AIL_unlock();
  }

  MSSLockedDecrement(SS_servicing);
}

//############################################################################
//##                                                                        ##
//## SetTimer timer to periodically call the foreground servicing routine   ##
//##                                                                        ##
//############################################################################

extern "C" void stream_background(void); // background service for streaming

void AILEXPORT Timer_foreground_service(HWND  hwnd,       //)
                                    UINT  message,
                                    UINT  nIDEvent,
                                    DWORD dwTime)
{
   Only16Push32s();

   InMilesMutex();
   low_serve();
   OutMilesMutex();

   Only16Pop32s();
}

#ifdef IS_WIN32

extern "C"
{
static U32 checkforeground=0;
}

#else

extern "C"
{
HTASK Win16_thread_task=0;

S32 Win16_pump_mess=0;

//############################################################################
//##                                                                        ##
//## SetTimer timer to periodically call the foreground servicing routine   ##
//##                                                                        ##
//############################################################################

void AILEXPORT Timer_othertask_service(HWND  hwnd,       //)
                                       UINT  message,
                                       UINT  nIDEvent,
                                       DWORD dwTime)
{
   Only16Push32s();

   SS_foreground_service();

   Only16Pop32s();
}

//############################################################################
//##                                                                        ##
//## Called from the other task to do background processing                 ##
//##                                                                        ##
//############################################################################

WORD AILEXPORT SS_Win16_thread()
{
  MSG msg;

  DWORD timer = (DWORD) SetTimer((HWND) NULL,0,75, Timer_othertask_service);

  Win16_thread_task=GetCurrentTask();

  while (GetMessage(&msg,0,0,0)) {
    switch (msg.message) {
      case WM_USER+230:
        PostQuitMessage(0);
        continue;
      case WM_USER+300:
        --Win16_pump_mess;
        break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    Win32sToBeCalled();
  }

  KillTimer((HWND) NULL, (UINT) timer);

  return msg.message;
}

}
#endif

//############################################################################
//##                                                                        ##
//## Timer callback function to mix data into output buffers                ##
//##                                                                        ##
//## Called from callback handler under Win16 to improve latency;           ##
//## called from timeSetEvent() callback thread under Win32 to avoid        ##
//## deadlocks                                                              ##
//##                                                                        ##
//## If DirectSound is being used to emulate waveOut functionality, this    ##
//## routine fills the next available DirectSound buffer fragment.          ##
//## Otherwise, it writes the data to a waveOut buffer which is several     ##
//## buffers ahead of the one most recently returned.                       ##
//##                                                                        ##
//############################################################################

void WINAPI SS_serve(HDIGDRIVER dig)
{
   static LPWAVEHDR  current;
   static S32        i;

   //
   // Return immediately if callbacks disabled or driver not actively playing
   //

   if (disable_callbacks)
      {
      return;
      }

   if (!dig->playing)
      {
      return;
      }

   //
   // Increment background count so callback functions will run in
   // background
   //

   MSSLockedIncrement(SS_servicing);

   if (SS_servicing==1)
   {

      MSSLockedIncrement(AIL_bkgnd_flag);

      if (AIL_preference[DIG_USE_WAVEOUT])
         {
         //
         // Loop through all returned buffers (if any)
         //
         // WARNING: SS_WOM_DONE() may add entry to circular list asynchronously!
         //

         while (dig->return_tail != dig->return_head)
            {
            //
            // Fetch WAVEHDR at tail pointer
            //

            i = dig->return_tail;

            current = dig->return_list[i];

            i = (i + 1) % dig->n_buffers;

            dig->return_tail = i;

            //
            // Fill subsequent output buffer with mixed audio
            //

            current = CURRENT_USER->next;
            current = CURRENT_USER->next;
            current = CURRENT_USER->next;

      #ifdef IS_WIN32
            current = CURRENT_USER->next;
      #endif

            SS_fill(dig, current->lpData);
            }
         }
      else
         {
   #ifdef IS_WIN32
         //
         // Get current buffer fragment
         //

         U32 p,w;

         AIL_LP_lock();
         ASDSBUF(dig->DS_out_buff)->GetCurrentPosition(&p,&w);
         AIL_LP_unlock();

         S32 cur_frag = (w / dig->DS_frag_size)+1;
         S32 frags_filled;


         // check for a wrap
         if (cur_frag<dig->DS_last_write)
         {
           dig->DS_last_frag%=dig->DS_frag_cnt;
         }
         dig->DS_last_write=cur_frag;

         S32 timer=AIL_ms_count();

         // compute the number of currently filled frags
         if (((timer-dig->DS_last_timer)>dig->DS_skip_time) || (dig->DS_last_frag<cur_frag))
         {
           // bad news - we dropped out
           frags_filled=0;
           dig->DS_last_frag=cur_frag;
         }
         else
         {
           frags_filled=dig->DS_last_frag-cur_frag;
         }

         dig->DS_last_timer=timer;

         // get the number of fragments to mix ahead
         S32 DS_frag_mix_ahead = AIL_preference[DIG_DS_MIX_FRAGMENT_CNT];
         if (DS_frag_mix_ahead >= dig->DS_frag_cnt)
           DS_frag_mix_ahead=dig->DS_frag_cnt-1;

         //
         // Keep at least minimum number of fragments ahead
         //

         while (frags_filled<DS_frag_mix_ahead)
            {

            S32 fill_frag = (dig->DS_last_frag + dig->DS_frag_cnt) % dig->DS_frag_cnt;

            //
            // Calculate offset and size of fragment to lock
            //

            S32 offset = fill_frag * dig->DS_frag_size;
            S32 size   = dig->DS_frag_size;

            //
            // Lock the buffer
            //

            void FAR *p1,FAR *p2;
            U32 l1,l2;

            MMRESULT result;

            AIL_LP_lock();
            result = ASDSBUF(dig->DS_out_buff)->Lock(offset,
                                                     size,
                                                    &p1,
                                                    &l1,
                                                    &p2,
                                                    &l2,
                                                     0);
            AIL_LP_unlock();

            if (result == DSERR_BUFFERLOST)
               {
               //
               // Restore, and try again later
               //

               AIL_LP_lock();
               ASDSBUF(dig->DS_out_buff)->Restore();
               AIL_LP_unlock();
               }
            else if (result == MMSYSERR_NOERROR)
               {
               //
               // Mix data into locked region
               //

               SS_fill(dig, p1);

               //
               // Release locked region
               //

               AIL_LP_lock();
               ASDSBUF(dig->DS_out_buff)->Unlock(p1,
                                                 l1,
                                                 p2,
                                                 l2);
               AIL_LP_unlock();
               }

            // we don't wrap this variable until cur_frag wraps
            dig->DS_last_frag = dig->DS_last_frag + 1;
            ++frags_filled;
            }
   #endif
         }

      //
      // Clear entry flags and return
      //

      MSSLockedDecrement(AIL_bkgnd_flag);

   }

   MSSLockedDecrement(SS_servicing);

#ifdef IS_WIN32

   if ((checkforeground++&7)==0)
   {
     SS_foreground_service();
   }

#endif
}


//############################################################################
//##                                                                        ##
//## Background callback function for output buffer return                  ##
//##                                                                        ##
//## Not called if DirectSound in use                                       ##
//##                                                                        ##
//############################################################################

void

#ifndef IS_WIN32
__export
#endif

FAR PASCAL SS_WOM_DONE(HWAVEOUT hWaveOut, //)
                            UINT     wMsg,
                            DWORD    dwInstance,
                            DWORD    dwParam1,
                            DWORD    dwParam2)
{
   LPWAVEHDR  current;
   HDIGDRIVER dig;
   U32        i;

   //
   // Return if message not WOM_DONE
   //

   if (wMsg != MM_WOM_DONE)
      {
      return;
      }

   //
   // Return if no HDR_USER information available
   //

   current = (LPWAVEHDR) dwParam1;

   if (current->dwUser == 0)
   {
      return;
   }

   dig = CURRENT_USER->owner;

   if ( dig->no_wom_done )
   {
     return;
   }

   Only16Push32s();

   //
   // We want the WOM_DONE callback function to behave like a DOS interrupt
   // handler; i.e., the application thread, once interrupted, should not
   // execute again until the callback function has returned.  So we'll
   // suspend the application thread (the one that called AIL_startup())
   // until we're done.
   //

   AIL_lock();

   MSSLockedIncrement(SS_servicing);

   if (SS_servicing!=1)
   {
     dig->wom_done_buffers++;
   }
   else
   {
     InMilesMutex();

  #ifdef IS_WIN32
     if (AIL_preference[AIL_LOCK_PROTECTION])
       if (SuspendThread(hAppThread) == 0xffffffff)
          {
          goto justexit;
          }
  #endif

     //
     // Return if not playing
     //

     if (!dig->playing)
        {
        goto resumethreadandexit;
        }

     dig->wom_done_buffers++;

     while ( dig->wom_done_buffers )
     {
       dig->wom_done_buffers--;

       //
       // Add buffer to circular list at head pointer
       //
       // Temporary variable "i" is used below because list is scanned
       // asynchronously by SS_serve.  A fault could occur if the compiler
       // stores the incremented head pointer before taking its modulus.
       //

       i = dig->return_head;

       dig->return_list[i] = current;

       i = (i + 1) % dig->n_buffers;

       //
       // New head entry becomes readable by SS_serve after atomic write below
       //

       dig->return_head = i;
     }

     //
     // Win16 requires an extra buffer of lead time if the queue is not
     // serviced within the WOM_DONE callback -- so we'll explicitly call
     // the SS_serve function to mix the buffer if Win32 is not in use
     //

  #ifndef IS_WIN32
     SS_serve(dig);
  #endif

     //
     // Resume foreground thread and return
     //

    resumethreadandexit:
  #ifdef IS_WIN32
     if (AIL_preference[AIL_LOCK_PROTECTION])
       ResumeThread(hAppThread);
  #endif
    justexit:

     OutMilesMutex();
   }

   MSSLockedDecrement(SS_servicing);
   AIL_unlock();

   Only16Pop32s();
}

//############################################################################
//##                                                                        ##
//## Initialize Windows waveOut driver and allocate output buffers          ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_waveOutOpen(HDIGDRIVER   FAR *drvr,       //)
                                LPHWAVEOUT FAR *lphWaveOut,
                                U32               dwDeviceID,
                                LPWAVEFORMAT      lpFormat)
{
   S32                i;
   U32                test_sample;
   HDIGDRIVER         dig;
   U32                result;
   volatile LPWAVEHDR current,temp;
   LPWAVEHDR FAR     *prev_next;
   DWORD              ticks;
   static char        string[128];

   //
   // Fail if not conventional 8-bit or 16-bit, mono or stereo PCM format
   //

   if (lpFormat) 
      {
      if (lpFormat->wFormatTag != WAVE_FORMAT_PCM)
         {
         AIL_set_error("Non-PCM wave data type not supported.");

         return MMSYSERR_ERROR;
         }

      if ((((LPPCMWAVEFORMAT) lpFormat)->wf.nChannels < 1) ||
          (((LPPCMWAVEFORMAT) lpFormat)->wf.nChannels > 2) ||

         ((((LPPCMWAVEFORMAT) lpFormat)->wBitsPerSample != 8 ) &&
          (((LPPCMWAVEFORMAT) lpFormat)->wBitsPerSample != 16)))
         {
         AIL_set_error("Unsupported PCM data format.");

         return MMSYSERR_ERROR;
         }
      }

   //
   // Allocate memory for DIG_DRIVER structure
   //

   dig = (HDIGDRIVER) AIL_mem_alloc_lock(sizeof(struct _DIG_DRIVER));

   if (dig == NULL)
      {
      AIL_set_error("Could not allocate memory for driver descriptor.");

      return MMSYSERR_NOMEM;
      }

   //
   // Explicitly initialize all DIG_DRIVER fields to NULL/0
   //

   AIL_memset(dig,
          0,
          sizeof(*dig));

   SETTAG(dig->tag,"HDIG");

   *drvr = dig;

   //
   // Check for MMX support if enabled
   //

   dig->use_MMX = AIL_MMX_available();

   //
   // Attempt to open wave output device
   //

   if (lpFormat)
      {
      AIL_memcpy(&dig->wformat,lpFormat,sizeof(PCMWAVEFORMAT));

      dig->DS_use_default_format = 0;
      }
   else
      {
      //
      // Choose a default format for waveOut, and set flag to force use of
      // default primary format for DirectSound
      //

      dig->wformat.wf.wFormatTag     = WAVE_FORMAT_PCM;
      dig->wformat.wf.nChannels      = 2;
      dig->wformat.wf.nSamplesPerSec = 22050;
      dig->wformat.wf.nBlockAlign    = 2;
      dig->wformat.wBitsPerSample    = 8;
      dig->wformat.wf.nAvgBytesPerSec = 44100;

      dig->DS_use_default_format = 1;
      }

   dig->deviceid = dwDeviceID;

   dig = dig;

   result = DIG_waveOutOpen( dig,
                            &dig->hWaveOut,
              (unsigned int) dig->deviceid,
       (WAVEFORMATEX FAR *) &dig->wformat,
                     (DWORD) SS_WOM_DONE,
                             0,
                             CALLBACK_FUNCTION);

   if (result)
      {
      AIL_set_error("waveOutOpen() failed.");

      AIL_mem_free_lock(dig);

      return result;
      }

   if (lphWaveOut)
      {
      *lphWaveOut = &dig->hWaveOut;
      }

   //
   // Init miscellaneous buffer variables
   //

   dig->request_reset    = 0;
   dig->playing          = 0;
   dig->released         = 0;
   dig->quiet            = 0;

   //
   // Some Windows drivers may not properly implement waveOutReset().  Allow
   // end users to insert a [MSS] / Reset=0 statement in WIN.INI to keep
   // MSS from calling waveOutReset() if trouble occurs.
   //

   //
   // Assume waveOutReset() does work...
   //

   dig->reset_works      = 1;

   //
   // "Reset=1" in WIN.INI forces use of waveOutReset()
   // "Reset=0" in WIN.INI disables use of waveOutReset()
   //

   GetProfileString("MSS",
                    "Reset",
                    "None",
                     string, sizeof(string));

   if (!AIL_strnicmp(string, "1", 1))
      {
      dig->reset_works   = 1;
      }

   if (!AIL_strnicmp(string, "0", 1))
      {
      dig->reset_works   = 0;
      }

   //
   // Set sample rate and size values and calling params
   //

   #ifndef IS_WIN32
     MSSGetTaskInfo(&dig->callingCT,&dig->callingDS);
   #endif
   
   dig->DMA_rate            = dig->wformat.wf.nSamplesPerSec;
   dig->channels_per_sample = dig->wformat.wf.nChannels;
   dig->bytes_per_channel   = dig->wformat.wBitsPerSample / 8;

   if (dig->bytes_per_channel == 1)
      {
      test_sample = 0x80808080;

      dig->hw_mode_flags = 0;

      if (dig->channels_per_sample == 1)
         {
         dig->hw_format = DIG_F_MONO_8;
         }
      else
         {
         dig->hw_format = DIG_F_STEREO_8;
         }
      }
   else
      {
      test_sample = 0;

      dig->hw_mode_flags = DIG_PCM_SIGN;

      if (dig->channels_per_sample == 1)
         {
         dig->hw_format = DIG_F_MONO_16;
         }
      else
         {
         dig->hw_format = DIG_F_STEREO_16;
         }
      }

   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      //
      // Determine build buffer size by measuring actual playback duration
      // of sample known to be smaller than DMA buffer (i.e., force silence
      // padding)
      //

      current = (LPWAVEHDR) AIL_mem_alloc_lock(sizeof(WAVEHDR));

      if (current == NULL)
         {
         AIL_set_error("Could not allocate memory for test WAVEHDR.");

         DIG_waveOutClose(dig,dig->hWaveOut);
         AIL_mem_free_lock(dig);

         return MMSYSERR_NOMEM;
         }

      current->lpData         = (LPSTR) &test_sample;
      current->dwBufferLength =          4;
      current->dwFlags        =          0;
      current->dwLoops        =          0;
      current->dwUser         =          0;

      DIG_waveOutPrepareHeader(dig,
                               dig->hWaveOut,
                               current,
                               sizeof(WAVEHDR));

      //
      // Disable timer callbacks
      //

      MSSLockedIncrement(disable_callbacks);

      dig->no_wom_done = 1;

      DIG_waveOutWrite(dig,
                       dig->hWaveOut,
                       current,
                       sizeof(WAVEHDR));

      while (!(current->dwFlags & WHDR_DONE)) ;

      #ifdef IS_WIN32
      OutMilesMutex();
      Sleep(1);
      #endif

      DIG_waveOutReset(dig, dig->hWaveOut);

      ticks = AIL_ms_count();

      DIG_waveOutWrite(dig,
                       dig->hWaveOut,
                       current,
                       sizeof(WAVEHDR));

      //
      // Profile the buffer
      //

      while (!(current->dwFlags & WHDR_DONE)) ;

      ticks = AIL_ms_count() - ticks;

      // if way long time, setup a reasonable default
      #ifdef IS_WIN32
      if (ticks>75)
        ticks=75;
      #else
      if (ticks>100)
        ticks=100;
      #endif
      else
      {
        // if impossibly small time, setup a reasonable start
        if (ticks<20)
          ticks=20;
      }

      DIG_waveOutUnprepareHeader(dig,
                                 dig->hWaveOut,
                                 current,
                                 sizeof(WAVEHDR));

      //
      // Give WOM_DONE callback thread a chance to execute before permitting
      // callback processing again
      //

      DIG_waveOutReset(dig, dig->hWaveOut);

   #ifdef IS_WIN32
      Sleep(1);
      InMilesMutex();
   #endif

      dig->no_wom_done = 0;

      MSSLockedDecrement(disable_callbacks);

      AIL_mem_free_lock(current);

      S32 min_chain_element_size = ( ( dig->wformat.wf.nAvgBytesPerSec * AIL_preference[DIG_MIN_CHAIN_ELEMENT_TIME] ) / 1000 ) & ~1023;

      if ( min_chain_element_size > (S32) AIL_preference[DIG_MAX_CHAIN_ELEMENT_SIZE] )
      {
        min_chain_element_size = AIL_preference[DIG_MAX_CHAIN_ELEMENT_SIZE];
      }

      do
         {
         //
         // Add 5 ms to duration in milliseconds to provide safety margin
         //
         // Ensure that build buffers are at least DIG_MIN_CHAIN_ELEMENT_SIZE
         // bytes in size
         //

         ticks += 5;

         //
         // Calculate size of DMA buffer in samples
         //
         // Ensure that buffer is multiple of 4 samples in size
         //

         dig->samples_per_buffer  = dig->wformat.wf.nSamplesPerSec * ticks / 1000L;

         dig->samples_per_buffer  = (dig->samples_per_buffer + 3) & ~3;

         dig->channels_per_buffer = dig->samples_per_buffer  * dig->channels_per_sample;

         dig->buffer_size         = dig->channels_per_buffer * dig->bytes_per_channel;
         }
      while (dig->buffer_size < min_chain_element_size );

      }
   else
      {
      //
      // Use DSound to emulate waveOut -- exact buffer size is already known
      //

      dig->buffer_size = dig->DS_frag_size;

      dig->channels_per_buffer = dig->buffer_size / dig->bytes_per_channel;

      dig->samples_per_buffer = dig->channels_per_buffer / dig->channels_per_sample;
      }

   //
   // Allocate build buffer
   //

   dig->build_size = sizeof(S32) * dig->channels_per_buffer;

   dig->build_buffer = (S32 FAR *) AIL_mem_alloc_lock(dig->build_size);

   if (dig->build_buffer == NULL)
      {
      AIL_set_error("Could not allocate build buffer.");

      DIG_waveOutClose(dig, dig->hWaveOut);
      AIL_mem_free_lock(dig);

      return MMSYSERR_NOMEM;
      }

   //
   // Allocate decode buffer
   //

   dig->decode_buffer_size = AIL_preference[DIG_DECODE_BUFFER_SIZE];

   dig->decode_buffer = (void FAR *) AIL_mem_alloc_lock(dig->decode_buffer_size);

   if (dig->decode_buffer == NULL)
      {
      AIL_set_error("Could not allocate decode buffer.");

      DIG_waveOutClose(dig, dig->hWaveOut);
      AIL_mem_free_lock(dig->build_buffer);
      AIL_mem_free_lock(dig);

      return MMSYSERR_NOMEM;
      }

   //
   // Allocate reverb buffer, if desired
   //

   dig->reverb_buffer_size     = AIL_preference[DIG_REVERB_BUFFER_SIZE];
   dig->reverb_buffer_position = 0;

   if (dig->reverb_buffer_size == 0)
      {
      dig->reverb_buffer = NULL;
      }
   else
      {
      //
      // Reverb buffer size must be a multiple of build buffer size
      //

      S32 s = (dig->reverb_buffer_size + (dig->build_size - 1)) / dig->build_size;

      dig->reverb_buffer_size = s * dig->build_size;

      dig->reverb_buffer = (S32 FAR *) AIL_mem_alloc_lock(dig->reverb_buffer_size);

      if (dig->reverb_buffer == NULL)
         {
         AIL_set_error("Could not allocate reverb buffer.");

         DIG_waveOutClose(dig, dig->hWaveOut);
         AIL_mem_free_lock(dig->build_buffer);
         AIL_mem_free_lock(dig->decode_buffer);
         AIL_mem_free_lock(dig);

         return MMSYSERR_NOMEM;
         }

      //
      // Wipe reverb buffer
      //

      AIL_memset(dig->reverb_buffer,
                 0,
                 dig->reverb_buffer_size);
      }

   //
   // Allocate physical SAMPLE structures for driver
   //

   dig->n_samples        = AIL_get_preference(DIG_MIXER_CHANNELS);
   dig->n_active_samples = 0;

   dig->master_volume    = 127;

   dig->samples = (HSAMPLE) AIL_mem_alloc_lock(sizeof(struct _SAMPLE) * dig->n_samples);

   if (dig->samples == NULL)
      {
      AIL_set_error("Could not allocate SAMPLE structures.");

      AIL_mem_free_lock(dig->build_buffer);
      AIL_mem_free_lock(dig->decode_buffer);
      if (dig->reverb_buffer_size != 0)
         {
         AIL_mem_free_lock(dig->reverb_buffer);
         }
      DIG_waveOutClose(dig, dig->hWaveOut);
      AIL_mem_free_lock(dig);

      return MMSYSERR_NOMEM;
      }

   for (i=0; i < dig->n_samples; i++)
      {
      AIL_memset(&dig->samples[i],
                  0,
                  sizeof(struct _SAMPLE));

      SETTAG(dig->samples[i].tag,"HSAM");

      dig->samples[i].status = SMP_FREE;
      dig->samples[i].driver = dig;
      }

   //
   // Allocate waveOut-specific data structures
   // These are not needed for DirectSound support
   //

   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      //
      // Allocate and page-lock approximately DIG_OUTPUT_BUFFER_SIZE bytes'
      // worth of output WAVEHDR buffers -- or at least 4 buffers
      //

      dig->n_buffers = (AIL_get_preference(DIG_OUTPUT_BUFFER_SIZE) /
                        dig->buffer_size);

      if (dig->n_buffers < 4)
         {
         dig->n_buffers = 4;
         }

      //
      // Allocate and page-lock circular "return list" used to
      // communicate between SS_WOM_DONE() and SS_serve
      //

      dig->return_list = (LPWAVEHDR FAR *) AIL_mem_alloc_lock(sizeof(LPWAVEHDR) * dig->n_buffers);

      dig->return_head = 0;
      dig->return_tail = 0;

      //
      // Allocate circular chain of WAVEHDR structures and their associated
      // HDR_USER structures
      //

      prev_next = &dig->first;

      for (i=0; i < dig->n_buffers; i++)
         {
         current = (LPWAVEHDR) AIL_mem_alloc_lock(sizeof(WAVEHDR));

         *prev_next = current;

         if (current == NULL)
            {
            AIL_set_error("Could not allocate WAVEHDR structures.");

            i = -1;
            break;
            }

         current->dwUser = (DWORD) AIL_mem_alloc_lock(sizeof(HDR_USER));

         if (current->dwUser == 0)
            {
            AIL_set_error("Could not allocate HDR_USER structures.");

            i = -1;
            break;
            }

         CURRENT_USER->owner = dig;

         prev_next = &(CURRENT_USER->next);
         }

      //
      // Exit if memory allocation error occurred
      //

      if (i == -1)
         {
         current = dig->first;

         while (current != NULL)
            {
            if (current->dwUser == 0)
               {
               AIL_mem_free_lock(current);
               break;
               }

            temp = CURRENT_USER->next;

            AIL_mem_free_lock(CURRENT_USER);
            AIL_mem_free_lock(current);

            current = temp;
            }

         AIL_mem_free_lock((void FAR *) dig->return_list);
         AIL_mem_free_lock((void FAR *) dig->samples);
         AIL_mem_free_lock((void FAR *) dig->build_buffer);
         AIL_mem_free_lock((void FAR *) dig->decode_buffer);
         if (dig->reverb_buffer_size != 0)
            {
            AIL_mem_free_lock(dig->reverb_buffer);
            }
         DIG_waveOutClose(dig, dig->hWaveOut);
         AIL_mem_free_lock(dig);

         return MMSYSERR_NOMEM;
         }

      //
      // Otherwise, make list circular
      //

      *prev_next = dig->first;

      //
      // Allocate buffer memory and page-lock it with waveOutPrepareHdr()
      //
      // Init flag bytes to WHDR_DONE, to force foreground timer to perform
      // waveOutWrite calls
      //
      // Bit 0 of dwBufferLength is used as "virgin" bit to force posting of
      // newly-initialized buffers
      //

      current = dig->first;

      for (i=0; i < dig->n_buffers; i++)
         {
         current->lpData = (C8 FAR *) AIL_mem_alloc_lock(dig->buffer_size);

         if (current->lpData == NULL)
            {
            AIL_set_error("Could not allocate output buffer.");

            i = -1;
            break;
            }

         current->dwBufferLength = dig->buffer_size;
         current->dwFlags        = 0;
         current->dwLoops        = 0;

         DIG_waveOutPrepareHeader(dig,
                                 dig->hWaveOut,
                                 current,
                                 sizeof(WAVEHDR));

         current = CURRENT_USER->next;
         }

      //
      // Exit if memory allocation error occurred
      //

      if (i == -1)
         {
         current = dig->first;

         for (i=0; i < dig->n_buffers; i++)
            {
            if (current->lpData != NULL)
               {
               DIG_waveOutUnprepareHeader(dig,
                                          dig->hWaveOut,
                                          current,
                                          sizeof(WAVEHDR));

               AIL_mem_free_lock(current->lpData);
               }

            temp = CURRENT_USER->next;

            AIL_mem_free_lock(CURRENT_USER);
            AIL_mem_free_lock(current);

            current = temp;
            }

         AIL_mem_free_lock((void FAR *) dig->return_list);
         AIL_mem_free_lock((void FAR *) dig->samples);
         AIL_mem_free_lock((void FAR *) dig->build_buffer);
         AIL_mem_free_lock((void FAR *) dig->decode_buffer);
         if (dig->reverb_buffer_size != 0)
            {
            AIL_mem_free_lock(dig->reverb_buffer);
            }
         DIG_waveOutClose(dig,dig->hWaveOut);
         AIL_mem_free_lock(dig);

         return MMSYSERR_NOMEM;
         }
      }

      //
      // Link HDIGDRIVER into chain used by timers
      //

      if (DIG_first!=NULL)
        {
        dig->next=DIG_first;
        }
      else
        {

        dig->next = NULL;

#ifdef IS_WIN32
        AIL_primary_digital_driver(dig);
#endif

      //
      // If this is the first driver initialized, set up
      // 54 ms foreground timer to maintain output buffer queue
      //

      dig->foreground_timer = (DWORD) SetTimer((HWND) NULL,
                                              0,
                                              50,
                                              (TIMERPROC) Timer_foreground_service);

      //
      // If timer allocation failed, release all resources and return
      // error
      //

      if (dig->foreground_timer == 0)
         {
#ifdef IS_WIN32
         if (DIG_first == NULL)
            {
            AIL_release_timer_handle(dig->backgroundtimer);
            }
#endif

         if (AIL_preference[DIG_USE_WAVEOUT])
            {
            current = dig->first;

            for (i=0; i < dig->n_buffers; i++)
               {
               DIG_waveOutUnprepareHeader(dig,
                                          dig->hWaveOut,
                                          current,
                                          sizeof(WAVEHDR));

               AIL_mem_free_lock(current->lpData);

               temp = CURRENT_USER->next;

               AIL_mem_free_lock(CURRENT_USER);
               AIL_mem_free_lock(current);

               current = temp;
               }

            AIL_mem_free_lock((void FAR *) dig->return_list);
            }

         AIL_mem_free_lock((void FAR *) dig->samples);
         AIL_mem_free_lock((void FAR *) dig->build_buffer);
         AIL_mem_free_lock((void FAR *) dig->decode_buffer);
         if (dig->reverb_buffer_size != 0)
            {
            AIL_mem_free_lock(dig->reverb_buffer);
            }
         DIG_waveOutClose(dig,dig->hWaveOut);
         AIL_mem_free_lock(dig);

         return MMSYSERR_ERROR;
         }
      }

   DIG_first = dig;

   //
   // Background timer used only for Win32 -- SS_serve called
   // explicitly from SS_WOM_DONE if Win16 is in use, for improved
   // latency performance
   //

#ifdef IS_WIN32

   dig->backgroundtimer = AIL_register_timer((AILTIMERCB)SS_serve);

   //
   // If timer allocation failed, release all resources and return
   // error
   //

   if (dig->backgroundtimer == -1)
      {
      AIL_set_error("Out of timer handles.");

      if (AIL_preference[DIG_USE_WAVEOUT])
         {
         current = dig->first;

         for (i=0; i < dig->n_buffers; i++)
            {
            DIG_waveOutUnprepareHeader(dig,
                                       dig->hWaveOut,
                                       current,
                                       sizeof(WAVEHDR));

            AIL_mem_free_lock(current->lpData);

            temp = CURRENT_USER->next;

            AIL_mem_free_lock(CURRENT_USER);
            AIL_mem_free_lock(current);

            current = temp;
            }

         AIL_mem_free_lock((void FAR *) dig->return_list);
         }

      KillTimer((HWND) NULL, (UINT) dig->foreground_timer);
      AIL_mem_free_lock((void FAR *) dig->samples);
      AIL_mem_free_lock((void FAR *) dig->build_buffer);
      AIL_mem_free_lock((void FAR *) dig->decode_buffer);
      if (dig->reverb_buffer_size != 0)
         {
         AIL_mem_free_lock(dig->reverb_buffer);
         }
      DIG_waveOutClose(dig,dig->hWaveOut);
      AIL_mem_free_lock(dig);

      return MMSYSERR_ERROR;
      }
#endif

   //
   // Init driver pipeline stages
   //

   for (i=0; i < N_DIGDRV_STAGES; i++)
      {
      dig->pipeline[i].active = 0;
      }

   //
   // Select default mixer flush/copy providers
   //

   HPROVIDER HP;
   
   RIB_enumerate_providers("MSS mixer services",
                            NULL,
                           &HP);

   AIL_set_digital_driver_processor(dig,
                                    DP_DEFAULT_FILTER,
                                    0);

   AIL_set_digital_driver_processor(dig,
                                    DP_DEFAULT_MERGE,
                                    HP);

   AIL_set_digital_driver_processor(dig,
                                    DP_FLUSH,
                                    HP);

   AIL_set_digital_driver_processor(dig,
                                    DP_COPY,
                                    HP);

   //
   // Start 100 Hz background service timer to process WOM_DONE events
   //

#ifdef IS_WIN32

   AIL_set_timer_frequency(dig->backgroundtimer,         100);
   AIL_set_timer_user     (dig->backgroundtimer, (DWORD) dig);
   AIL_start_timer        (dig->backgroundtimer             );

#else

  if (dig->next == NULL) {
    char name[128];    // load the background task exe
    char str[32];
    U16 show[2];
    struct _lp {
       WORD   segEnv;                  /* child environment  */
       LPSTR  lpszCmdLine;             /* child command tail */
       LPWORD lpwShow;                 /* how to show child  */
       LPWORD lpwReserved;             /* must be NULL       */
    } lp;

    lp.segEnv=0;
    lp.lpszCmdLine=(LPSTR)str;
    show[0]=2;
    show[1]=SW_SHOWNORMAL;
    lp.lpwShow=show;
    lp.lpwReserved=0;

    str[0]=(U8)AIL_sprintf(str+1,"!%li",(U32)(LPVOID)SS_Win16_thread);

    AIL_strcpy(name,MSS_Directory);
    AIL_strcat(name,"\\");

#ifdef MSSXTRA
    AIL_strcat(name,"MSSBX16.TSK");  // set up the name of the task
#else
    AIL_strcat(name,"MSSB16.TSK");  // set up the name of the task
#endif
    {
      WORD err=SetErrorMode(0x8000);
      if ((U16)LoadModule(name,&lp)<32) {  // Start the background task

        //now try in the redist directory
        if (*AIL_redist_directory)
        {
          if ((AIL_redist_directory[0]=='\\') || (AIL_redist_directory[1]==':'))
          {
            AIL_strcpy(name,AIL_redist_directory);
            AIL_strcat(name,"\\");
          }
          else
          {
            // relative path
            AIL_strcpy(name,MSS_Directory);
            AIL_strcat(name,"\\");
            AIL_strcat(name,AIL_redist_directory);
          }

          #ifdef MSSXTRA
            AIL_strcat(name,"MSSBX16.TSK");  // set up the name of the task
          #else
            AIL_strcat(name,"MSSB16.TSK");  // set up the name of the task
          #endif

          if ((U16)LoadModule(name,&lp)>=32)  // Start the background task
            goto cont;
        }
        OutputDebugString("Could not load MSS background task: ");
        OutputDebugString(name);
        OutputDebugString("\n");
      }
     cont:
      SetErrorMode(err);
    }
  }

#endif

   //
   // Return normally
   //

   return 0;
}

//############################################################################
//##                                                                        ##
//## Shut down Windows waveOut driver and free output buffers               ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_waveOutClose(HDIGDRIVER dig)
{
   HDIGDRIVER cur,prev;
   LPWAVEHDR  current,temp;
   S32        i;

   if ((dig == NULL) || (DIG_first == NULL))
      {
      return;
      }

   AILSTRM_shutdown(dig);

   MSSLockedIncrement( SS_servicing );

   while ( SS_servicing != 1 )
   {
     Sleep( 1 );
   }

   InMilesMutex();

   //
   // Disable callback processing
   //

   MSSLockedIncrement(disable_callbacks);

   dig->no_wom_done = 1;

   //
   // Stop playback
   //

   if (!dig->released)
     SS_stop_DIG_driver_playback(dig);

   //
   // Release any open sample handles (to ensure that pipeline resources
   // are deallocated properly)
   //

   for (i=0; i < dig->n_samples; i++)
      {
      if (dig->samples[i].status != SMP_FREE)
         {
         AIL_release_sample_handle(&dig->samples[i]);
         }
      }


#ifdef IS_WIN32

   //
   // Release any filters associated with this driver
   //

   FLT_disconnect_driver(dig);

   //
   // Kill buffer maintenance timer
   //

    AIL_release_timer_handle(dig->backgroundtimer);

#endif

   //
   // Unlink from foreground service chain
   //

   if (dig == DIG_first)
      {
      DIG_first = dig->next;

      if (DIG_first == NULL)
         {
         //
         // If no more drivers left, kill foreground timer
         //

#ifndef IS_WIN32

         if (Win16_thread_task) {
           PostAppMessage(Win16_thread_task,WM_USER+230,0,0);
           DirectedYield(Win16_thread_task);  // run the task specifically
           Win16_thread_task=0;
         }

#endif
         KillTimer((HWND) NULL, (UINT) dig->foreground_timer);
         }
      }
   else
      {
      prev = DIG_first;
      cur  = DIG_first->next;

      while (cur != dig)
         {
         if (cur == NULL)
            {
            MSSLockedDecrement(disable_callbacks);
            return;
            }

         prev = cur;
         cur  = cur->next;
         }

      prev->next = cur->next;
      }

   //
   // Reset driver to force return of all structures
   //
   // In Win32, ensure that all buffer-return threads have a chance to run
   //

#ifdef IS_WIN32
   OutMilesMutex();
#endif

   if (!dig->released)
     DIG_waveOutReset(dig,dig->hWaveOut);

#ifdef IS_WIN32
   Sleep(1);
   InMilesMutex();
#endif

   //
   // Release all resources allocated by this driver
   //

   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      current = dig->first;

      for (i=0; i < dig->n_buffers; i++)
         {
         if (!dig->released)
         DIG_waveOutUnprepareHeader(dig,
                                    dig->hWaveOut,
                                    current,
                                    sizeof(WAVEHDR));

         AIL_mem_free_lock(current->lpData);

         temp = CURRENT_USER->next;

         AIL_mem_free_lock(CURRENT_USER);
         AIL_mem_free_lock(current);

         current = temp;
         }
      }

   //
   // Close driver and free resources
   //

   if (!dig->released)
     DIG_waveOutClose(dig,dig->hWaveOut);

   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      AIL_mem_free_lock((void FAR *) dig->return_list);
      }

   AIL_mem_free_lock((void FAR *) dig->samples);
   AIL_mem_free_lock((void FAR *) dig->build_buffer);
   AIL_mem_free_lock((void FAR *) dig->decode_buffer);

   if (dig->reverb_buffer_size != 0)
      {
      AIL_mem_free_lock(dig->reverb_buffer);
      }

   MSSLockedDecrement( SS_servicing );

   MSSLockedDecrement(disable_callbacks);

#ifdef IS_WIN32
   OutMilesMutex();
   Sleep(1);
#endif

   AIL_mem_free_lock(dig);
}

//############################################################################
//##                                                                        ##
//## Temporarily release the Windows HWAVEOUT device                        ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_digital_handle_release(HDIGDRIVER dig)
{
   LPWAVEHDR  current;
   S32        i;

   if (dig == NULL)
      {
      dig = primary_digital_driver;

      if (dig == NULL)
         {
         dig = DIG_first;
         }
      }

   if ((dig == NULL) || (DIG_first == NULL))
      {
      return(0);
      }

   if (dig->released)
     return(1);

   //
   // Disable callback processing
   //

   MSSLockedIncrement(disable_callbacks);

   dig->no_wom_done = 1;

   //
   // Stop playback
   //

   SS_stop_DIG_driver_playback(dig);

   //
   // Reset driver to force return of all structures
   //
   // In Win32, ensure that all buffer-return threads have a chance to run
   //

#ifdef IS_WIN32
   OutMilesMutex();
#endif

   DIG_waveOutReset(dig,dig->hWaveOut);

#ifdef IS_WIN32
   Sleep(1);
   InMilesMutex();
#endif

   //
   // Unprepare the headers
   //

   for (i=0; i < dig->n_samples; i++)
      {
      if (dig->samples[i].status == SMP_PLAYING)
         {
         dig->samples[i].status=SMP_PLAYINGBUTRELEASED;
         }
      }

   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      current = dig->first;

      for (i=0; i < dig->n_buffers; i++)
         {
         DIG_waveOutUnprepareHeader(dig,
                                    dig->hWaveOut,
                                    current,
                                    sizeof(WAVEHDR));

         current = CURRENT_USER->next;
         }
      }

   //
   // Close driver
   //

   DIG_waveOutClose(dig,dig->hWaveOut);

   dig->playing=0;

   dig->released=1;

   dig->hWaveOut=0;

   dig->no_wom_done = 0;

   MSSLockedDecrement(disable_callbacks);

#ifdef IS_WIN32
   OutMilesMutex();
   Sleep(1);
   InMilesMutex();
#endif

   return(1);
}

//############################################################################
//##                                                                        ##
//## Reacquire the Windows HWAVEOUT device                                  ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_digital_handle_reacquire(HDIGDRIVER dig)
{
   LPWAVEHDR  current;
   U32 result;
   S32 i;
   S32 playing=0;

   if (dig == NULL)
      {
      dig = primary_digital_driver;

      if (dig == NULL)
         {
         dig = DIG_first;
         }
      }

   if ((dig == NULL) || (DIG_first == NULL))
      {
      return(0);
      }

   if (!dig->released)
     return(1);

   //
   // Disable callback processing
   // 

   MSSLockedIncrement(disable_callbacks);

   result = DIG_waveOutOpen( dig,
                            &dig->hWaveOut,
              (unsigned int) dig->deviceid,
           (LPWAVEFORMATEX) &dig->wformat,
                     (DWORD) SS_WOM_DONE,
                             0,
                             CALLBACK_FUNCTION);

   if (result)
      {
      AIL_set_error("waveOutOpen() failed.");

      MSSLockedDecrement(disable_callbacks);

      return(0);
      }

   //
   // Reprepare the headers
   //

   for (i=0; i < dig->n_samples; i++)
      {
      if (dig->samples[i].status == SMP_PLAYINGBUTRELEASED)
         {
         dig->samples[i].status=SMP_PLAYING;
         playing=1;
         }
      }

   if (AIL_preference[DIG_USE_WAVEOUT])
      {
      current = dig->first;

      for (i=0; i < dig->n_buffers; i++)
         {
         DIG_waveOutPrepareHeader(dig,
                                 dig->hWaveOut,
                                 current,
                                 sizeof(WAVEHDR));

         current = CURRENT_USER->next;
         }
      }

  dig->released=0;

  if (playing)
    SS_start_DIG_driver_playback(dig);

   MSSLockedDecrement(disable_callbacks);

  return(1);
}

//############################################################################
//##                                                                        ##
//## Externally-callable service function for foreground timer              ##
//##                                                                        ##
//############################################################################

static S32 only_one_serve=0;
static U32 last_serve=0;

static void low_serve()
{
   last_serve=AIL_ms_count();

   MSSLockedIncrement(only_one_serve);

   if (only_one_serve==1)
   {
     //
     // If Win32 in use, call foreground service only if DirectSound not in use
     //

     #ifdef IS_WIN32
     if (AIL_preference[DIG_USE_WAVEOUT] == YES)
     #endif
     {
       SS_foreground_service();
     }

     OutMilesMutex();
     stream_background();
     InMilesMutex();

#ifndef IS_WIN32
     Win32sToBeCalled();
#endif
   }

   MSSLockedDecrement(only_one_serve);

   // check all the buffer status's
   HDIGDRIVER dig=DIG_first;
   while (dig)
   {
     SS_serve(dig);
     dig=dig->next;
   }
}


void AILCALL AIL_API_serve()
{
  U32 t=AIL_ms_count();
  if ((t-last_serve)>10)
  {
    InMilesMutex();
    low_serve();
    OutMilesMutex();
  }
}


//############################################################################
//##                                                                        ##
//## Return number of actively playing samples for given driver             ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_active_sample_count(HDIGDRIVER dig)
{
   S32 i,n;

   n = 0;

   for (i=0; i < dig->n_samples; i++)
      {
      if ((dig->samples[i].status == SMP_PLAYING) || (dig->samples[i].status == SMP_PLAYINGBUTRELEASED))
         {
         ++n;
         }
      }

   return n;
}

//############################################################################
//##                                                                        ##
//## Allocate a SAMPLE structure for use with a given driver                ##
//##                                                                        ##
//############################################################################

HSAMPLE AILCALL AIL_API_allocate_sample_handle(HDIGDRIVER dig)
{
   S32    i;
   HSAMPLE S;

   //
   // Exit if driver invalid
   //

   if (dig == NULL)
      {
      return NULL;
      }

   //
   // Lock timer services to prevent reentry
   //

   AIL_lock();

   //
   // Look for an unallocated sample structure
   //

   for (i=0; i < dig->n_samples; i++)
      {
      if (dig->samples[i].status == SMP_FREE)
         break;
      }

   //
   // If all structures in use, return NULL
   //

   if (i == dig->n_samples)
      {
      AIL_set_error("Out of sample handles.");

      AIL_unlock();
      return NULL;
      }

   S = &dig->samples[i];

   //
   // Initialize sample to default (SMP_DONE) status with nominal
   // sample attributes
   //

   AIL_init_sample(S);

   AIL_unlock();
   return S;
}

//############################################################################
//##                                                                        ##
//## Free a SAMPLE structure for later allocation                           ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_release_sample_handle(HSAMPLE S)
{
   if (S == NULL)
      {
      return;
      }

   S->status = SMP_FREE;

   //
   // Shut down any installed pipeline providers
   //

   AIL_set_sample_processor(S,
                            SAMPLE_ALL_STAGES,
                            NULL);
}

//############################################################################
//##                                                                        ##
//## Initialize a SAMPLE structure to baseline values                       ##
//##                                                                        ##
//## Sample is allocated (not free), done playing, and stopped              ##
//##                                                                        ##
//## By default, sample source data is assumed to be 11.025 kHz 8-bit mono  ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_init_sample(HSAMPLE S)
{
   if (S == NULL)
      {
      return;
      }

   //
   // Set status to FREE while manipulating vars, to keep callback thread
   // from reading invalid data
   //

   S->status = SMP_FREE;

   //
   // Shut down any previously-active pipeline providers
   //   and init pipeline stages
   //

   AIL_set_sample_processor(S,
                            SAMPLE_ALL_STAGES,
                            NULL);

   //
   // Initialize sample vars
   //

   S->start[0]       =  NULL;
   S->len  [0]       =  0;
   S->pos  [0]       =  0;
   S->done [0]       =  0;

   S->start[1]       =  NULL;
   S->len  [1]       =  0;
   S->pos  [1]       =  0;
   S->done [1]       =  1;

   S->left_val       =  0;
   S->right_val      =  0;
   S->src_fract      =  0;

   S->current_buffer =  0;
   S->last_buffer    = -2;
   S->starved        =  1;

   S->loop_count     =  1;
   S->loop_start     =  0;
   S->loop_end       = -1;

   S->format         =  DIG_F_MONO_8;
   S->flags          =  0;

   S->playback_rate  =  11025;

   S->volume         =  AIL_preference[DIG_DEFAULT_VOLUME];

   // Set default ADPCM data
   S->adpcm.blocksize=  256;
   S->adpcm.blockleft = 0;
   S->adpcm.extrasamples = 0;

   //
   // If stereo sample played with mono driver, set pan control to play
   // left source channel by default
   //
   // This preserves behavior of earlier (pre-3.03) AIL releases, while
   // allowing the application to pan stereo source channels to mono output
   // if desired
   //

   if ((S->driver->hw_format == DIG_F_MONO_8) ||
       (S->driver->hw_format == DIG_F_MONO_16))
      {
      S->pan = 0;
      }
   else
      {
      S->pan = 64;
      }

   S->SOB =  NULL;
   S->EOB =  NULL;
   S->EOS =  NULL;
   S->dosob = 0;
   S->doeob = 0;
   S->doeos = 0;

   S->reverb_level        = 0.0F;
   S->reverb_reflect_time = 0.030F;
   S->reverb_decay_time   = 1.493F;    // equivalent to EAX_PRESET_GENERIC

   SS_calculate_volume_scalars(S);

   //
   // Select default mixer merge provider
   //

   AIL_set_sample_processor(S,
                            DP_MERGE,
                            S->driver->pipeline[DP_DEFAULT_MERGE].provider);

   AIL_set_sample_processor(S,
                            DP_FILTER,
                            S->driver->pipeline[DP_DEFAULT_FILTER].provider);

   //
   // Mark sample initialized
   //

   S->status =  SMP_DONE;
}

//############################################################################
//##                                                                        ##
//## Get status of sample                                                   ##
//##                                                                        ##
//############################################################################

U32 AILCALL AIL_API_sample_status(HSAMPLE S)
{
   if (S == NULL)
      {
      return 0;
      }

   return S->status;
}

//############################################################################
//##                                                                        ##
//## Set adpcm sample block size                                            ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_adpcm_block_size(HSAMPLE S, U32 blocksize)
{
   if (S == NULL)
      {
      return;
      }

   S->adpcm.blocksize = blocksize;
}

//############################################################################
//##                                                                        ##
//## Set starting address and length of sample                              ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_address(HSAMPLE S, void const FAR *start, U32 len)
{
   if (S == NULL)
      {
      return;
      }

   S->start[0] = start;
   S->len[0]   = len;

   S->start[1] = NULL;
   S->len[1]   = 0;
}

//############################################################################
//##                                                                        ##
//## Set sample data format and flags                                       ##
//##                                                                        ##
//## Available formats:                                                     ##
//##                                                                        ##
//##   DIG_F_MONO_8                                                         ##
//##   DIG_F_MONO_16                                                        ##
//##   DIG_F_STEREO_8                                                       ##
//##   DIG_F_STEREO_16                                                      ##
//##   DIG_F_ADPCM_MONO_16                                                  ##
//##   DIG_F_ADPCM_STEREO_16                                                ##
//##                                                                        ##
//## Available flags:                                                       ##
//##                                                                        ##
//##   DIG_PCM_SIGN                                                         ##
//##   DIG_PCM_ORDER (stereo formats only)                                  ##
//##                                                                        ##
//## Note: 16-bit sample data must be in Intel byte order                   ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_type(HSAMPLE S, S32 format, U32 flags)
{
   if (S == NULL)
      {
      return;
      }

   if ((format != S->format) ||
       (flags  != S->flags))
      {
      S->format = format;
      S->flags  = flags;

      // Reset ADPCM offset to the end of decode buffer
      // to force a decode buffer refill
      if(format & DIG_F_ADPCM_MASK)
         {

         S->adpcm.blockleft = 0;
         S->adpcm.extrasamples = 0;

         }

      SS_calculate_volume_scalars(S);
      }
}

//############################################################################
//##                                                                        ##
//## Get sample playback rate in hertz                                      ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_sample_playback_rate(HSAMPLE S)
{
   if (S == NULL)
      {
      return 0;
      }

   return S->playback_rate;
}

//############################################################################
//##                                                                        ##
//## Set sample playback rate in hertz                                      ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_playback_rate(HSAMPLE S, S32 playback_rate)
{
   if (S == NULL)
      {
      return;
      }

   S->playback_rate = playback_rate;
}

//############################################################################
//##                                                                        ##
//## Get sample volume (0-127)                                              ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_sample_volume(HSAMPLE S)
{
   if (S == NULL)
      {
      return 0;
      }

   return S->volume;
}

//############################################################################
//##                                                                        ##
//## Set sample volume (0-127)                                              ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_volume(HSAMPLE S, S32 volume)
{
   if (S == NULL)
      {
      return;
      }

   if (volume != S->volume)
      {
      S->volume = volume;

      SS_calculate_volume_scalars(S);
      }
}

//############################################################################
//##                                                                        ##
//## Get sample panpot / stereo balance setting (0=left, 127=right)         ##
//##                                                                        ##
//## Value determines panning position for mono samples, and balance for    ##
//## stereo samples                                                         ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_sample_pan(HSAMPLE S)
{
   if (S == NULL)
      {
      return 0;
      }

   return S->pan;
}

//############################################################################
//##                                                                        ##
//## Set sample panpot / stereo balance (0=left, 127=right)                 ##
//##                                                                        ##
//## Value determines panning position for mono samples, and balance for    ##
//## stereo samples                                                         ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_pan(HSAMPLE S, S32 pan)
{
   if (S == NULL)
      {
      return;
      }

   if (pan != S->pan)   
      {
      S->pan = pan;

      SS_calculate_volume_scalars(S);
      }
}

//############################################################################
//##                                                                        ##
//## Get the granularity of sample type (1=8/m,2=8/s,16/m,4=16/m, or adpcm) ##
//##                                                                        ##
//## Value is returned in bytes                                             ##
//##                                                                        ##
//############################################################################

U32 AILCALL AIL_API_sample_granularity(HSAMPLE S)
{
   if (S == NULL)
      {
      return 0;
      }

   if (S->pipeline[DP_ASI_DECODER].active)
      {
      //
      // All ASI compressed formats are assumed to have 1-byte granularity --
      // the ASI layer will handle any granularity issues associated with
      // seeks, etc. internally
      //

      return 1;
      }

   if ((S->format == DIG_F_ADPCM_MONO_16) ||
       (S->format == DIG_F_ADPCM_STEREO_16))
      {
      return S->adpcm.blocksize;
      }

   return SS_granularity(S->format);
}

//############################################################################
//##                                                                        ##
//## Get position of next sample block to be transferred                    ##
//##                                                                        ##
//## Value is returned in bytes relative to start of sample, and may be     ##
//## treated as the sample's approximate playback position                  ##
//##                                                                        ##
//############################################################################

U32 AILCALL AIL_API_sample_position(HSAMPLE S)
{
   if (S == NULL)
      {
      return 0;
      }

   return S->pos[S->current_buffer];
}

//############################################################################
//##                                                                        ##
//## Set position of next sample block to be transferred                    ##
//##                                                                        ##
//## Value is specified in bytes relative to start of sample, and will      ##
//## determine the precise location of the next block of data which is      ##
//## fetched from the sample                                                ##
//##                                                                        ##
//## AIL_set_sample_position(0) "rewinds" the sample to its beginning       ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_position(HSAMPLE S, U32 pos)
{
   U32 gran;

   if (S == NULL)
      {
      return;
      }

   gran=AIL_sample_granularity(S);

   pos = ((pos+gran/2) / gran)*gran;

   // Reset ADPCM offset to the end of decode buffer
   // to force a decode buffer refill

   if(S->format & DIG_F_ADPCM_MASK)
     {
       S->adpcm.blockleft = 0;
       S->adpcm.extrasamples = 0;
     }

   //
   // If this is an ASI-compressed format, perform a seek with offset -1 
   // to allow the decoder to reinitialize its state
   //
   // We could also implement this by seeking to the specified offset, but
   // we need to change the position here anyway so it will be reported
   // correctly in subsequent AIL_set_sample_position() calls.  The ASI
   // decoder itself does not care about the seek position -- it only needs
   // to be informed when we change it
   //

   if (S->pipeline[DP_ASI_DECODER].active)
      {
      ASISTAGE *ASI = &S->pipeline[DP_ASI_DECODER].TYPE.ASI;

      ASI->ASI_stream_seek(ASI->stream, -1);
      }

   S->pos[S->current_buffer] = pos;
}

//############################################################################
//##                                                                        ##
//## Get number of sample loops remaining                                   ##
//##                                                                        ##
//## 1 indicates that the sample is on its last iteration                   ##
//## 0 indicates that the sample is looping indefinitely                    ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_sample_loop_count(HSAMPLE S)
{
   if (S == NULL)
      {
      return -1;
      }

   return S->loop_count;
}

//############################################################################
//##                                                                        ##
//## Set sample loop count                                                  ##
//##                                                                        ##
//##  1: Single iteration, no looping                                       ##
//##  0: Loop indefinitely                                                  ##
//##  n: Play sample n times                                                ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_loop_count(HSAMPLE S, S32 loop_count)
{
   if (S == NULL)
      {
      return;
      }

   S->loop_count = loop_count;
}

//############################################################################
//##                                                                        ##
//## Set optional subblock for looping                                      ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_loop_block(HSAMPLE S, //)
                                          S32     loop_start_offset,
                                          S32     loop_end_offset)
{
   U32 gran;

   if (S == NULL)
      {
      return;
      }

   gran=AIL_sample_granularity(S);

   S->loop_start = ((loop_start_offset+gran/2) / gran)*gran;
   S->loop_end   = loop_end_offset;
}

//############################################################################
//##                                                                        ##
//## Start playback of sample from beginning                                ##
//##                                                                        ##
//## At a minimum, sample must first have been initialized with prior calls ##
//## to AIL_init_sample() and AIL_set_sample_address()                      ##
//##                                                                        ##
//## Playback will begin at the next DMA half-buffer transition             ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_start_sample(HSAMPLE S)
{
   if (S == NULL)
      {
      return;
      }

   //
   // Make sure sample has been allocated
   //

   if (S->status == SMP_FREE)
      {
      return;
      }

   //
   // Make sure valid sample data exists
   //

   if ((S->len  [S->current_buffer] == 0) ||
       (S->start[S->current_buffer] == NULL))
      {
      return;
      }

   // Reset ADPCM data
   S->adpcm.blockleft = 0;
   S->adpcm.extrasamples = 0;

   //
   // Rewind sample to beginning
   //

   S->pos[S->current_buffer] = 0;

   //
   // Mark as single-buffered sample
   //

   S->service_type = 1;


   if (S->driver->released)

     S->status = SMP_PLAYINGBUTRELEASED;

   else {

     //
     // Set 'playing' status
     //

     S->status = SMP_PLAYING;

     //
     // If sample's driver is not already transmitting data, start it
     //

     SS_start_DIG_driver_playback(S->driver);
   }
}

//############################################################################
//##                                                                        ##
//## Stop playback of sample                                                ##
//##                                                                        ##
//## Sample playback may be resumed with AIL_resume_sample(), or restarted  ##
//## from the beginning with AIL_start_sample()                             ##
//##                                                                        ##
//## Playback will stop at the next DMA half-buffer transition              ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_stop_sample(HSAMPLE S)
{
   if (S == NULL)
      {
      return;
      }

   //
   // Make sure sample is currently playing
   //

   if (S->status != SMP_PLAYING)
      {
      return;
      }

   //
   // Mask 'playing' status
   //

   S->status = SMP_STOPPED;
}

//############################################################################
//##                                                                        ##
//## Resume playback of sample from current position                        ##
//##                                                                        ##
//## Playback will resume at the next DMA half-buffer transition            ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_resume_sample(HSAMPLE S)
{
   if (S == NULL)
      {
      return;
      }

   //
   // See if sample has been previously stopped
   //

   if (S->status == SMP_STOPPED) {

      goto startsound;

   } else if (S->status == SMP_DONE) {

     //
     // this means they called resume before start (good for non-zero
     //   start offsets)
     //

     //
     // Make sure valid sample data exists
     //

     if ((S->len  [S->current_buffer] == 0) ||
         (S->start[S->current_buffer] == NULL))
       {
       return;
       }

     //
     // Mark as single-buffered sample
     //

     S->service_type = 1;

    startsound:

     if (S->driver->released)

       S->status = SMP_PLAYINGBUTRELEASED;

     else {

       //
       // Set 'playing' status
       //

       S->status = SMP_PLAYING;

       //
       // If sample's driver is not already transmitting data, start it
       //

       SS_start_DIG_driver_playback(S->driver);
     }

   }

}

//############################################################################
//##                                                                        ##
//## Terminate playback of sample, setting sample status to SMP_DONE        ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_end_sample(HSAMPLE S)
{
   if (S == NULL)
      {
      return;
      }

   //
   // Make sure sample has been allocated
   //

   if (S->status == SMP_FREE)
      {
      return;
      }

   //
   // If sample is not already done, halt it and invoke its end-of-buffer
   // and end-of-sample callback functions
   //

   if (S->status != SMP_DONE)
      {
      //
      // Set 'done' status
      //

      S->status = SMP_DONE;
      S->starved = 1;

      //
      // Call EOB and EOS functions
      //

      if (S->EOB != NULL)
         {
         MSS_do_cb1( (AILSAMPLECB) ,S->EOB,S->driver->callingDS,S->EOB_IsWin32s,
           S);
         }

      if (S->EOS != NULL)
         {
         MSS_do_cb1( (AILSAMPLECB) ,S->EOS,S->driver->callingDS,S->EOS_IsWin32s,
           S);
         }
      }
}


//############################################################################
//##                                                                        ##
//## Set start-of-block (SOB) callback function for sample                  ##
//##                                                                        ##
//## Callback function will be invoked prior to playback of each sample     ##
//## data block, whose position within the sample can be determined by      ##
//## calling AIL_sample_position()                                          ##
//##                                                                        ##
//## The rate at which this function is called is determined by the DMA     ##
//## half-buffer transition period for the driver in use; e.g., a 1K half-  ##
//## buffer being played at 22 kHz will trigger start-of-block callbacks    ##
//## at a rate of 22 per second                                             ##
//##                                                                        ##
//## This function returns the sample's previous SOB callback handler       ##
//## address, or NULL if no SOB callback handler was registered             ##
//##                                                                        ##
//############################################################################

AILSAMPLECB AILCALL AIL_API_register_SOB_callback(HSAMPLE S, AILSAMPLECB SOB)
{
   AILSAMPLECB old;

   if (S == NULL)
      {
      return NULL;
      }

   old = S->SOB;
   
   #ifndef IS_WIN32
     CheckWin32sCB(S->SOB_IsWin32s);
   #endif

   S->SOB = SOB;

   return old;
}

//############################################################################
//##                                                                        ##
//## Set end-of-buffer (EOB) callback function for sample                   ##
//##                                                                        ##
//## Callback function will be invoked when either of two sample buffers    ##
//## finish playing                                                         ##
//##                                                                        ##
//## When not double-buffering, the end-of-sample (EOS) callback, if any,   ##
//## will be triggered immediately after the end of buffer 0                ##
//##                                                                        ##
//## This function returns the sample's previous EOB callback handler       ##
//## address, or NULL if no EOB callback handler was registered             ##
//##                                                                        ##
//############################################################################

AILSAMPLECB AILCALL AIL_API_register_EOB_callback(HSAMPLE S, AILSAMPLECB EOB)
{
   AILSAMPLECB old;

   if (S == NULL)
      {
      return NULL;
      }

   old = S->EOB;

   #ifndef IS_WIN32
     CheckWin32sCB(S->EOB_IsWin32s);
   #endif

   S->EOB = EOB;

   return old;
}

//############################################################################
//##                                                                        ##
//## Set end-of-sample (EOS) callback function for sample                   ##
//##                                                                        ##
//## Callback function will be invoked when all valid sample buffers have   ##
//## finished playing                                                       ##
//##                                                                        ##
//## When not double-buffering, the end-of-sample (EOS) callback will be    ##
//## triggered immediately after the end of buffer 0                        ##
//##                                                                        ##
//## This function returns the sample's previous EOS callback handler       ##
//## address, or NULL if no EOS callback handler was registered             ##
//##                                                                        ##
//############################################################################

AILSAMPLECB AILCALL AIL_API_register_EOS_callback(HSAMPLE S, AILSAMPLECB EOS)
{
   AILSAMPLECB old;

   if (S == NULL)
      {
      return NULL;
      }

   old = S->EOS;

   #ifndef IS_WIN32
     CheckWin32sCB(S->EOS_IsWin32s);
   #endif

   S->EOS = EOS;

   return old;
}

//############################################################################
//##                                                                        ##
//## Set sample user data value at specified index                          ##
//##                                                                        ##
//## Any desired 32-bit value may be stored at one of eight user data words ##
//## associated with a given SAMPLE                                         ##
//##                                                                        ##
//## Callback functions may access the user data array at interrupt time    ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_user_data(HSAMPLE S, U32 index, S32 value)
{
   if (S == NULL)
      {
      return;
      }

   S->user_data[index] = value;
}

//############################################################################
//##                                                                        ##
//## Get sample user data value at specified index                          ##
//##                                                                        ##
//## Any desired 32-bit value may be stored at one of eight user data words ##
//## associated with a given SAMPLE                                         ##
//##                                                                        ##
//## Callback functions may access the user data array at interrupt time    ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_sample_user_data(HSAMPLE S, U32 index)
{
   if (S == NULL)
      {
      return 0;
      }

   return S->user_data[index];
}

//############################################################################
//##                                                                        ##
//## Set master volume for all samples                                      ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_digital_master_volume(HDIGDRIVER dig, S32 master_volume)
{
   S32 i;

   dig->master_volume = master_volume;

   //
   // Update all sample volume settings
   //

   for (i=0; i < dig->n_samples; i++)
      {
      if (dig->samples[i].status == SMP_FREE)
         {
         continue;
         }

      SS_calculate_volume_scalars(&dig->samples[i]);
      }
}

//############################################################################
//##                                                                        ##
//## Return current master digital volume setting                           ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_digital_master_volume(HDIGDRIVER dig)
{
   return dig->master_volume;
}


static S32 nibbles_per_sample(S32 format)
{
   switch (format)
      {
      case DIG_F_ADPCM_MONO_16:
         return(1);
      case DIG_F_ADPCM_STEREO_16:
      case DIG_F_MONO_8:
         return(2);
      case DIG_F_STEREO_8:
      case DIG_F_MONO_16:
         return(4);
      default:
         return(8);
      }
}


//############################################################################
//##                                                                        ##
//## Return minimum buffer size for dual-buffer playback                    ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_minimum_sample_buffer_size(HDIGDRIVER dig, //)
                                              S32       playback_rate,
                                              S32       format)
{
   S32  app_nibbles_per_sample;
   S32  hw_nibbles_per_sample;
   S32  n;

   //
   // Get # of application nibbles per sample unit
   //

   app_nibbles_per_sample = nibbles_per_sample(format);

   //
   // Get # of hardware nibbles per sample unit
   //

   hw_nibbles_per_sample = dig->channels_per_sample * dig->bytes_per_channel * 2;

   //
   // Multiply size of hardware half-buffer by ratio of logical-to-physical
   // sample size
   //

   n = dig->buffer_size * app_nibbles_per_sample / hw_nibbles_per_sample ;

   //
   // Scale n by resampling ratio
   //

   n = n * playback_rate / dig->DMA_rate;

   //
   // Scale n by 2X resampling tolerance to provide safety margin
   //

   n = n + ((n * AIL_preference[DIG_RESAMPLING_TOLERANCE]) / 32768);

   //
   // If DMA rate is not 1X, 2X, or 4X times playback rate, round buffer
   // size up 1 sample to avoid possible truncation errors
   //

   if ((dig->DMA_rate != (1 * playback_rate)) &&
       (dig->DMA_rate != (2 * playback_rate)) &&
       (dig->DMA_rate != (4 * playback_rate)))
      {
      n += 4;
      }

   //
   // Round n up to nearest multiple of 256 bytes
   //

   n = (n + 255) & ~255;

   //
   // Return size *3 (*4 in Win32) to ensure enough data is
   // available for initial buffer fill
   //

#ifdef IS_WIN32
   if (AIL_preference[DIG_USE_WAVEOUT])
   {
     n = n * 4;
   }
   else
   {
     // get the number of fragments to mix ahead
     S32 DS_frag_mix_ahead = AIL_preference[DIG_DS_MIX_FRAGMENT_CNT];
     if (DS_frag_mix_ahead >= dig->DS_frag_cnt)
       DS_frag_mix_ahead=dig->DS_frag_cnt-1;

     n = n * DS_frag_mix_ahead;
   }
#else
   n = n * 3;
#endif

   return n;
}

//############################################################################
//##                                                                        ##
//## Set address and length for one of two double-buffered sample buffers   ##
//##                                                                        ##
//## Start playback of sample if not already in progress                    ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_load_sample_buffer(HSAMPLE S, S32 buff_num, void const FAR *buffer, U32 len)
{
   if (S == NULL)
      {
      return;
      }

   S->done [buff_num] = (len == 0);
   S->start[buff_num] = buffer;
   S->len  [buff_num] = len;
   S->pos  [buff_num] = 0;

   S->starved = 0;

   if (len)
      {
      if (S->status != SMP_PLAYING)
         {
         //
         // Mark as double-buffered sample
         //

         S->service_type = 2;

#if 0
         //
         // Don't start driver playback until both buffers primed with
         // stream data
         //

         if (S->last_buffer < 0)
            {
            return;
            }
#endif

         //
         // Set 'playing' status
         //

         if (S->driver->released)
            {
            S->status = SMP_PLAYINGBUTRELEASED;
            }
         else
            {
            S->status = SMP_PLAYING;

            //
            // Start driver playback
            //

            SS_start_DIG_driver_playback(S->driver);
            }
         }
      }
}

//############################################################################
//##                                                                        ##
//## Get double-buffer playback status for sample                           ##
//##                                                                        ##
//##  0: Buffer 0 is ready to be filled (buffer 1 or neither buffer playing)##
//##  1: Buffer 1 is ready to be filled (buffer 0 playing)                  ##
//## -1: Both buffers are already full                                      ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_sample_buffer_ready(HSAMPLE S)
{
   if (S == NULL)
      {
      return -1;
      }

   //
   // For first two calls after sample initialization, return 0 and 1,
   // respectively
   //
   // This allows the application to "prime" the buffers for continued
   // playback
   //

   switch (S->last_buffer)
      {
      case -2:

         //
         // First call after AIL_init_sample() must clear second buffer's
         // "done" flag to permit buffer-switching
         //

         S->done[1] = 0;

         //
         // Set up to load buffer 0 this call, and "bootstrap" buffer 1 at
         // next call
         //

         S->last_buffer = -1;
         return 0;

      case -1:

         //
         // Return 1 to force load of second buffer immediately
         // Subsequent calls should operate on alternating buffers
         //

         S->last_buffer = S->current_buffer;
         return 1;
      }

   //
   // If buffer has not switched since last call, return -1
   //

   if (S->last_buffer == S->current_buffer)
      {
      return -1;
      }

   //
   // New current_buffer exists -- set last_buffer equal to
   // current_buffer and return exhausted buffer
   //

   S->last_buffer = S->current_buffer;

   return S->current_buffer ^ 1;
}

//############################################################################
//##                                                                        ##
//## Get digital driver configuration                                       ##
//##                                                                        ##
//############################################################################

void     AILCALL AIL_API_digital_configuration     (HDIGDRIVER dig, //)
                                            S32    FAR*rate,
                                            S32    FAR*format,
                                            char   FAR*config)
{
   UINT id;
   WAVEOUTCAPS CAPS;

   if (dig==NULL)
     return;

   if (rate != NULL)
      {
      *rate = dig->DMA_rate;
      }

   if (format != NULL)
      {
      *format = dig->hw_format;
      }
   if (config != NULL)
      {

      DIG_waveOutGetID(dig,dig->hWaveOut,&id);
      DIG_waveOutGetDevCaps(dig,id, &CAPS, sizeof(CAPS));

      *config=0;

      if (AIL_preference[DIG_USE_WAVEOUT])
         {
         lstrcat(config,"WaveOut - ");
         }

      lstrcat(config,CAPS.szPname);
      }
}

//############################################################################
//##                                                                        ##
//## Get information about status of streaming buffer pair                  ##
//##                                                                        ##
//############################################################################

S32  AILCALL AIL_API_sample_buffer_info (HSAMPLE    S, //)
                                 U32     FAR* pos0,
                                 U32     FAR* len0,
                                 U32     FAR* pos1,
                                 U32     FAR* len1)
{
   if (pos0 != NULL)
      {
      *pos0 = S->pos[0];
      }

   if (pos1 != NULL)
      {
      *pos1 = S->pos[1];
      }

   if (len0 != NULL)
      {
      *len0 = S->len[0];
      }

   if (len1 != NULL)
      {
      *len1 = S->len[1];
      }

   return S->starved;
}

//############################################################################
//##                                                                        ##
//## Get size and current position of sample in milliseconds                ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_sample_ms_position(HSAMPLE  S, //)
                                        S32 FAR *total_milliseconds,
                                        S32 FAR *current_milliseconds)
{
   U32 datarate;

   if (S==NULL)
      {
      return;
      }

   //
   // Get data rate in bytes per second
   //

   if (S->format & DIG_F_ADPCM_MASK) 
      {
      //
      // ADPCM compression
      //

      U32 samples_per_block = 4 << ((S->format&DIG_F_STEREO_MASK)?1:0);
      samples_per_block = 1 + (S->adpcm.blocksize-samples_per_block)*8 / samples_per_block;

      datarate=(S->playback_rate * S->adpcm.blocksize)/samples_per_block;
      } 
   else
      {
      if (S->pipeline[DP_ASI_DECODER].active)
         {
         //
         // ASI compression
         //

         ASISTAGE *ASI = &S->pipeline[DP_ASI_DECODER].TYPE.ASI;

         datarate = ASI->ASI_stream_attribute(ASI->stream,
                                              ASI->INPUT_BIT_RATE) / 8;
         }
      else
         {
         //
         // Straight PCM
         //

         datarate=(S->playback_rate * nibbles_per_sample(S->format))/2;
         }
      }

   //
   // Return requested values
   //

   if (total_milliseconds)
      {
      *total_milliseconds=(S32)(((float)S->len[S->current_buffer]*1000.0)/(float)datarate);
      }

   if (current_milliseconds)
      {
      *current_milliseconds=(S32)(((float)S->pos[S->current_buffer]*1000.0)/(float)datarate);
      }
}

//############################################################################
//##                                                                        ##
//## Seek to a specified millisecond within a sample                        ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_ms_position  (HSAMPLE S, //)
                                              S32     milliseconds)
{
   U32 datarate;

   if (S==NULL) 
      {
      return;
      }

   //
   // Get data rate in bytes per second
   //

   if (S->format & DIG_F_ADPCM_MASK) 
      {
      //
      // ADPCM compression
      //

      U32 samples_per_block = 4 << ((S->format&DIG_F_STEREO_MASK)?1:0);
      samples_per_block = 1 + (S->adpcm.blocksize-samples_per_block)*8 / samples_per_block;

      datarate=(S->playback_rate * S->adpcm.blocksize)/samples_per_block;
      } 
   else
      {
      if (S->pipeline[DP_ASI_DECODER].active)
         {
         //
         // ASI compression
         //

         ASISTAGE *ASI = &S->pipeline[DP_ASI_DECODER].TYPE.ASI;

         datarate = ASI->ASI_stream_attribute(ASI->stream,
                                              ASI->INPUT_BIT_RATE) / 8;
         }
      else
         {
         //
         // Straight PCM
         //

         datarate=(S->playback_rate * nibbles_per_sample(S->format))/2;
         }
      }

   //
   // Set requested position
   //

   AIL_set_sample_position(S, (S32)(((float)datarate*(float)milliseconds)/1000.0));
}

//############################################################################
//##                                                                        ##
//## Set reverb parms                                                       ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_set_sample_reverb  (HSAMPLE S, //)
                                               F32     reverb_level,
                                               F32     reverb_reflect_time,
                                               F32     reverb_decay_time)
{
   if (S==NULL)
      {
      return;
      }

   S->reverb_level        = reverb_level;
   S->reverb_reflect_time = reverb_reflect_time;
   S->reverb_decay_time   = reverb_decay_time;
}

//############################################################################
//##                                                                        ##
//## Get reverb parms                                                       ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_sample_reverb  (HSAMPLE  S, //)
                                           F32 FAR *reverb_level,
                                           F32 FAR *reverb_reflect_time,
                                           F32 FAR *reverb_decay_time)
{
   if (S==NULL)
      {
      return;
      }

   if (reverb_level)        *reverb_level        = S->reverb_level;
   if (reverb_reflect_time) *reverb_reflect_time = S->reverb_reflect_time;
   if (reverb_decay_time)   *reverb_decay_time   = S->reverb_decay_time;
}

static U32 mult64anddiv(U32 a,U32 b, U32 c)
{
   return( (U32) ( (((F64)a)*((F64)b)) / ((F64)c)) );
}

S32 AILCALL AIL_API_digital_latency(HDIGDRIVER dig)
{
  if (dig)
  {
    if (AIL_preference[DIG_USE_WAVEOUT])
    {
      if (dig->playing)
      {
        return( 10+mult64anddiv(dig->buffer_size*
#ifdef IS_WIN32
                4
#else
                3
#endif
                ,1000,dig->wformat.wf.nAvgBytesPerSec));

      }
      else
      {
        return(10);
      }
    }
    else
    {
      if (dig->emulated_ds)
      {
        return( 200+mult64anddiv(dig->DS_frag_size,1000,dig->wformat.wf.nAvgBytesPerSec)*
                    AIL_preference[DIG_DS_MIX_FRAGMENT_CNT] );
      }
      else
      {
        if (AIL_preference[DIG_DS_USE_PRIMARY])
        {
          return( mult64anddiv(dig->DS_frag_size,1000,dig->wformat.wf.nAvgBytesPerSec)*
                  AIL_preference[DIG_DS_MIX_FRAGMENT_CNT] );
        }
        else
        {
          return( 20+mult64anddiv(dig->DS_frag_size,1000,dig->wformat.wf.nAvgBytesPerSec)*
                     AIL_preference[DIG_DS_MIX_FRAGMENT_CNT] );
        }
      }
    }
  }
  return(0);
}

