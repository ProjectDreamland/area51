//############################################################################
//##                                                                        ##
//##  Miles Sound System Vortex DLS driver                                  ##
//##                                                                        ##
//##  VORTEX.C API module and support routines                              ##
//##                                                                        ##
//##  32-bit protected-mode source compatible with MSC 9.0                  ##
//##                                                                        ##
//##  Version 1.00 of 22-Sep-98: Initial version                            ##
//##                                                                        ##
//##  Author: Jeff Roberts                                                  ##
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

#include "mssdls.h"

static HANDLE v=INVALID_HANDLE_VALUE;
static S32 loaded=0;

#define ASP4_W32_WAVETABLE_DOWNLOAD		0x39950000
#define ASP4_W32_WAVETABLE_OPEN			0x39960000
#define ASP4_W32_WAVETABLE_CLOSE		0x39970000
#define ASP4WT_PRESET_CHANGE	3

DXDEF S32 AILEXPORT DLSOpen          (S32 FAR *lpdwHandle,
                                      S32      dwFlags)
{

  if (v==INVALID_HANDLE_VALUE) {

		v = CreateFile("\\\\.\\Au88Mid0", GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (v==INVALID_HANDLE_VALUE) {
      // AU8830
      v = CreateFile("\\\\.\\au30wt.vxd",0,0,NULL,0,FILE_FLAG_DELETE_ON_CLOSE,NULL);
      if (v==INVALID_HANDLE_VALUE) {
        // AU8820
        v = CreateFile("\\\\.\\asp4wt.vxd",0,0,NULL,0,FILE_FLAG_DELETE_ON_CLOSE,NULL);
      }
    }
    if (v==INVALID_HANDLE_VALUE)
      return(DLS_NOT_FOUND);
  }

  if (!DeviceIoControl(v, ASP4_W32_WAVETABLE_OPEN, NULL, 0, NULL, 0, NULL, NULL))
		{
			return( DLS_NOT_ENABLED );
		}

  *lpdwHandle=(S32)v;

  loaded=0;
  return( DLS_NOERR );
}


static void reloadGM()
{
  char buf[256];
  if (loaded==0)
    return;
  
  GetSystemDirectory(buf,sizeof(buf));
  AIL_strcat(buf,"\\aurealgm");
  
  if (DLSLoadFile((S32)v,0,0,buf)!=DLS_NOERR) {
    GetWindowsDirectory(buf,sizeof(buf));
    AIL_strcat(buf,"\\aurealgm");

    if (DLSLoadFile((S32)v,0,0,buf)!=DLS_NOERR) {
      GetWindowsDirectory(buf,sizeof(buf));
      AIL_strcat(buf,"\\system\\aurealgm");
      DLSLoadFile((S32)v,0,0,buf);
    }
  }
  loaded=0;
}


DXDEF S32 AILEXPORT DLSClose         (S32      dwDLSHandle,
                                      S32      dwFlags)
{
	if ((dwDLSHandle!=(S32)v) || (v==INVALID_HANDLE_VALUE))
    return(DLS_INVALID_HANDLE);

  if (v) {
    reloadGM();
//    if (!DeviceIoControl(v, ASP4_W32_WAVETABLE_CLOSE, NULL, 0, NULL, 0, NULL, NULL))
//			return( DLS_NOT_ENABLED );
		CloseHandle(v);
    v=INVALID_HANDLE_VALUE;
	}
  return( DLS_NOERR );
}


DXDEF S32 AILEXPORT DLSLoadFile      (S32      dwDLSHandle,
                                      S32      dwFlags,
                                      S32 FAR *lpdwDownloadID,
                                      char const FAR*lpFileName)
{
	if ((dwDLSHandle!=(S32)v) || (v==INVALID_HANDLE_VALUE))
    return(DLS_INVALID_HANDLE);

  if (!DeviceIoControl(v, ASP4_W32_WAVETABLE_DOWNLOAD, (LPVOID)lpFileName, strlen(lpFileName)+1, NULL, 0, NULL, NULL))
    return( DLS_FILE_ERR );

  loaded=1;

  return( DLS_NOERR);
}

typedef struct
{
    char		bank_name[256];
    DWORD		preset;
    char    sndfontcrap[10];
    DWORD		level;
    DWORD		layerNdx;
    DWORD		splitNdx;
    DWORD		dwParam;
  	void*		pBank;
    char  temp[1024];
} Asp4wtDevIoParams;

DXDEF S32 AILEXPORT DLSLoadMemFile   (S32      dwDLSHandle,
                                      S32      dwFlags,
                                      S32 FAR *lpdwDownloadID,
                                      void FAR*lpMemPtr)
{
	Asp4wtDevIoParams params;

  if ((dwDLSHandle!=(S32)v) || (v==INVALID_HANDLE_VALUE))
    return(DLS_INVALID_HANDLE);

	memset(&params,0,sizeof(params));

  params.dwParam = 1;		      //Track number for Program change after download
	params.preset = 1;	        //Program number for Program change after download

	params.pBank = lpMemPtr;		//Pointer to DLS file in memory

	if (!DeviceIoControl(v,ASP4WT_PRESET_CHANGE,&params,sizeof(Asp4wtDevIoParams),NULL,0,NULL,NULL))
    return( DLS_FILE_ERR );

  loaded=1;

  return( DLS_NOERR);
}



DXDEF S32 AILEXPORT DLSCompactMemory (S32      dwDLSHandle)
{
  return( DLS_NOERR );
}


DXDEF S32 AILEXPORT DLSUnloadAll     (S32      dwDLSHandle,
                                      S32      dwFlags)
{
  reloadGM();
  return( DLS_NOERR );
}



DXDEF S32 AILEXPORT DLSUnloadFile    (S32      dwDLSHandle,
                                      S32      dwDownloadID)
{
  reloadGM();
  return( DLS_NOERR );
}



