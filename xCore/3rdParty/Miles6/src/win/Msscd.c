//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  MSSCD.C: Red Book CD-audio support for mss                           ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 9.0                  ##
//##                                                                        ##
//##  Version 1.00 of 02-Feb-96: Originally written.                        ##
//##  Version 1.01 of 11-May-97: New AIL_redbook_open_drive (Serge Plagnol) ##
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


static S32 voldeviceid=-1;  // holds an aux device or an HMIXER
static S32 volopened=0;     // times the volume control has been opened
static S32 mixerapi=0;      // are we using the mixer API?

#ifdef IS_WIN32

static MIXERLINE lineinfo;
static MIXERLINECONTROLS linecontrols;
static MIXERCONTROL volumecontrol;
static MIXERCONTROLDETAILS volumedetails;
static MIXERCONTROLDETAILS_UNSIGNED volumevalue[2];

#endif

//############################################################################
//##                                                                        ##
//## Complete redbook_open(_drive) and get mixer                            ##
//##                                                                        ##
//############################################################################

static HREDBOOK rbcompleteopen(U32 which, U32 flags, MCI_OPEN_PARMS *pmciopen)
{
  HREDBOOK r;
  AUXCAPS Caps;
  MCI_SET_PARMS mciset;
  int i;

  AIL_serve();
  if (mciSendCommand((WORD)which,MCI_OPEN,flags,(DWORD)(LPVOID)pmciopen)) {
    AIL_serve();
    if (mciSendCommand((WORD)which,MCI_OPEN,flags|MCI_OPEN_SHAREABLE,(DWORD)(LPVOID)pmciopen))
      return(0);
  }

  AIL_serve();
  mciset.dwTimeFormat=MCI_FORMAT_MILLISECONDS;
  mciSendCommand(pmciopen->wDeviceID,MCI_SET,MCI_SET_TIME_FORMAT,(DWORD)(LPVOID)&mciset);
  AIL_serve();
  r=(HREDBOOK)AIL_mem_alloc_lock(sizeof(REDBOOK));
  r->paused=0;
  r->DeviceID=pmciopen->wDeviceID;

  if (++volopened==1) {

#ifdef IS_WIN32

    // try the mixer api for volume control first

    for(i=mixerGetNumDevs();i;) {

      if (mixerOpen((HMIXER*)&voldeviceid,--i,0,0,0)==MMSYSERR_NOERROR) {
        AIL_memset(&lineinfo,0,sizeof(lineinfo));
        lineinfo.cbStruct=sizeof(lineinfo);
        lineinfo.dwComponentType=MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;

        if (mixerGetLineInfo((HMIXEROBJ)voldeviceid,&lineinfo,MIXER_GETLINEINFOF_COMPONENTTYPE)==MMSYSERR_NOERROR) {
          AIL_memset(&linecontrols,0,sizeof(linecontrols));
          AIL_memset(&volumecontrol,0,sizeof(volumecontrol));
          linecontrols.cbStruct=sizeof(linecontrols);
          linecontrols.dwLineID=lineinfo.dwLineID;
          linecontrols.dwControlType=MIXERCONTROL_CONTROLTYPE_VOLUME;
          linecontrols.cControls=1;
          linecontrols.cbmxctrl=sizeof(volumecontrol);
          linecontrols.pamxctrl=&volumecontrol;

          if (mixerGetLineControls((HMIXEROBJ)voldeviceid,&linecontrols,MIXER_GETLINECONTROLSF_ONEBYTYPE)==MMSYSERR_NOERROR) {
            AIL_memset(&volumedetails,0,sizeof(volumedetails));
            AIL_memset(volumevalue,0,sizeof(volumevalue[0])*2);
            volumedetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
            volumedetails.dwControlID=volumecontrol.dwControlID;
            volumedetails.cChannels=lineinfo.cChannels;
            volumedetails.cbDetails=sizeof(volumevalue[0]);
            volumedetails.paDetails=volumevalue;

            if (mixerGetControlDetails((HMIXEROBJ)voldeviceid,&volumedetails,MIXER_SETCONTROLDETAILSF_VALUE)==MMSYSERR_NOERROR) {
              mixerapi=1;
              break;
            }
          }

        }

        mixerClose((HMIXER)voldeviceid);
        voldeviceid=-1;

      }
    }


    // if that didn't work, try to get the aux volume handle

    if (voldeviceid==-1)

#endif

    {

      // find the auxif that didn't work, get the volume control id

      for(i=auxGetNumDevs();i;) {
        auxGetDevCaps(--i,&Caps,sizeof(Caps));
        if ((Caps.wTechnology==AUXCAPS_CDAUDIO) && (Caps.dwSupport&AUXCAPS_VOLUME)) {
          voldeviceid=i;
          mixerapi=0;
          break;
        }
      }

    }

  }

  return(r);
}


//############################################################################
//##                                                                        ##
//## Open a handle to a Red Book Device.                                    ##
//##                                                                        ##
//##    drive is the CD-ROM drive letter				    ##
//##                                                                        ##
//############################################################################

HREDBOOK AILCALL AIL_API_redbook_open_drive(S32 drive)
{
  MCI_OPEN_PARMS mciopen;
#ifdef IS_WIN32
  CHAR szElementName[4];
#else
  char szElementName[4];
#endif

  mciopen.lpstrDeviceType=(LPCSTR)MCI_DEVTYPE_CD_AUDIO;
#ifdef IS_WIN32
  AIL_sprintf( szElementName, TEXT("%c:"), drive);
#else
  AIL_sprintf( szElementName, "%c:", drive);
#endif
  mciopen.lpstrElementName = szElementName ;

  return rbcompleteopen(0, MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID|MCI_OPEN_ELEMENT, &mciopen);
}


//############################################################################
//##                                                                        ##
//## Open a handle to a Red Book Device.                                    ##
//##                                                                        ##
//##    which is the number of the CD-ROM to use (0=auto).                  ##
//##                                                                        ##
//############################################################################

HREDBOOK AILCALL AIL_API_redbook_open(U32 which)
{

  MCI_OPEN_PARMS mciopen;

  mciopen.lpstrDeviceType=(LPCSTR)MCI_DEVTYPE_CD_AUDIO;

  return rbcompleteopen(which, MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID, &mciopen);
}


//############################################################################
//##                                                                        ##
//## Close the handle to the Red Book device (free the memory, etc).        ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_redbook_close(HREDBOOK hand)
{
  if (hand==0)
    return;
  
  AIL_serve();
  mciSendCommand((UINT)hand->DeviceID,MCI_CLOSE,0,0);
  AIL_serve();

  if (--volopened==0) {

#ifdef IS_WIN32

    if (mixerapi)
      mixerClose((HMIXER)voldeviceid);

#endif

    voldeviceid=-1;

  }

  AIL_mem_free_lock(hand);
}


//############################################################################
//##                                                                        ##
//## Eject a CD from a Red Book device.                                     ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_redbook_eject(HREDBOOK hand)
{
  if (hand==0)
    return;
  
  AIL_serve();
  mciSendCommand((UINT)hand->DeviceID,MCI_SET,MCI_SET_DOOR_OPEN,0);
  AIL_serve();
  hand->paused=0;
}


//############################################################################
//##                                                                        ##
//## Retract a CD back into a Red Book device.                              ##
//##                                                                        ##
//############################################################################

void AILCALL AIL_API_redbook_retract(HREDBOOK hand)
{
  if (hand==0)
    return;
  
  AIL_serve();
  mciSendCommand((UINT)hand->DeviceID,MCI_SET,MCI_SET_DOOR_CLOSED,0);
  AIL_serve();
  hand->paused=0;
}


//############################################################################
//##                                                                        ##
//## Returns the current status of a Red book device.  Possibilities are:   ##
//##                                                                        ##
//##    REDBOOK_ERROR     CD is a data disk or was removed or is bad.       ##
//##    REDBOOK_STOPPED   CD is stopped.                                    ##
//##    REDBOOK_PLAYING   CD is playing.                                    ##
//##    REDBOOK_PAUSED    CD is playing, but is paused.                     ##
//##                                                                        ##
//############################################################################

U32 AILCALL AIL_API_redbook_status(HREDBOOK hand)
{
  MCI_STATUS_PARMS mcistatus;
  if (hand) {
    mcistatus.dwItem=MCI_STATUS_MODE;

    AIL_serve();
    mciSendCommand((UINT)hand->DeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_WAIT,(DWORD)(LPVOID)&mcistatus);
    AIL_serve();

    switch (mcistatus.dwReturn) {
      case MCI_MODE_PLAY:
        return(REDBOOK_PLAYING);
      case MCI_MODE_STOP:
        return(hand->paused?REDBOOK_PAUSED:REDBOOK_STOPPED);
    }
    hand->paused=0;     // reset paused status if there is an error
  }

  return(REDBOOK_ERROR);
}


//############################################################################
//##                                                                        ##
//## Return the number of tracks on the Red book device.                    ##
//##                                                                        ##
//############################################################################

U32 AILCALL AIL_API_redbook_tracks(HREDBOOK hand)
{
  MCI_STATUS_PARMS mcistatus;
  if (AIL_redbook_status(hand)==REDBOOK_ERROR)
    return(0);
  
  mcistatus.dwItem=MCI_STATUS_NUMBER_OF_TRACKS;
  mciSendCommand((UINT)hand->DeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_WAIT,(DWORD)(LPVOID)&mcistatus);
  AIL_serve();
  return(mcistatus.dwReturn);
}


//##############################################################################
//##                                                                          ##
//## Returns the starting and ending millisec counts for the specified track. ##
//##                                                                          ##
//##############################################################################

void AILCALL AIL_API_redbook_track_info(HREDBOOK hand,U32 tracknum,U32 FAR* startmsec,U32 FAR* endmsec)
{
  U32 start;
  MCI_STATUS_PARMS mcistatus;

  if (AIL_redbook_status(hand)==REDBOOK_ERROR) {
    if (startmsec)
      *startmsec=0;
    if (endmsec)
      *endmsec=0;
    return;
  }

  mcistatus.dwItem = MCI_STATUS_POSITION;
  mcistatus.dwTrack = tracknum;
  mciSendCommand((UINT)hand->DeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_TRACK|MCI_WAIT,(DWORD)(LPVOID)&mcistatus);
  AIL_serve();
  start=mcistatus.dwReturn;
  if (startmsec)
    *startmsec=start;

  mcistatus.dwItem = MCI_STATUS_LENGTH;
  mciSendCommand((UINT)hand->DeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_TRACK|MCI_WAIT,(DWORD)(LPVOID)&mcistatus);
  AIL_serve();
  if (endmsec) {
    if (tracknum==AIL_redbook_tracks(hand))
      mcistatus.dwReturn-=15;
    *endmsec=start+mcistatus.dwReturn;
  }
}


//############################################################################
//##                                                                        ##
//## Returns a special hashed value that will uniquely identify the CD.     ##
//##                                                                        ##
//############################################################################

static U32 stupidhash[4]={0xb16eade1L,0x471f295aL,0x38bca4d5L,0xe41926fcL};
#define rotate(val) ((((U32)(val))<<8L)|(((U32)(val))>>24L))

U32 AILCALL AIL_API_redbook_id(HREDBOOK hand)
{
  U32 count,hash,tracks,starts,ends,i;
  if (AIL_redbook_status(hand)==REDBOOK_ERROR)
    return(0);

  count=0;
  hash=0;

  tracks=AIL_redbook_tracks(hand);
  hash=hash^stupidhash[(++count&3)]^tracks;
  hash=rotate(hash);

  for(i=0;i<tracks;i++) {
    AIL_redbook_track_info(hand,i+1,&starts,&ends);
    hash=hash^stupidhash[(++count&3)]^starts;
    hash=rotate(hash);
    hash=hash^stupidhash[(++count&3)]^ends;
    hash=rotate(hash);
  }

  return(hash);
}


//############################################################################
//##                                                                        ##
//## Returns the current position of a Red book device in seconds.          ##
//##                                                                        ##
//############################################################################

U32 AILCALL AIL_API_redbook_position(HREDBOOK hand)
{
  MCI_STATUS_PARMS mcistatus;
  if (AIL_redbook_status(hand)==REDBOOK_PLAYING) {
    mcistatus.dwItem=MCI_STATUS_POSITION;
    mciSendCommand((UINT)hand->DeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_WAIT,(DWORD)(LPVOID)&mcistatus);
    AIL_serve();
    return(mcistatus.dwReturn);
  }
  return(0);
}


//############################################################################
//##                                                                        ##
//## Returns the current track of a Red book device.                        ##
//##                                                                        ##
//############################################################################

U32 AILCALL AIL_API_redbook_track(HREDBOOK hand)
{
  MCI_STATUS_PARMS mcistatus;
  if (AIL_redbook_status(hand)==REDBOOK_PLAYING) {
    mcistatus.dwItem=MCI_STATUS_CURRENT_TRACK;
    mciSendCommand((UINT)hand->DeviceID,MCI_STATUS,MCI_STATUS_ITEM|MCI_WAIT,(DWORD)(LPVOID)&mcistatus);
    AIL_serve();
    return(mcistatus.dwReturn);
  }
  return(0);
}


//############################################################################
//##                                                                        ##
//## Starts a Red Book Device playing.  Returns the new Red book status.    ##
//##                                                                        ##
//##  startsec and endsec specify the starting and ending millisec count.   ##
//##                                                                        ##
//############################################################################

U32 AILCALL AIL_API_redbook_play(HREDBOOK hand,U32 startmsec, U32 endmsec)
{
  MCI_PLAY_PARMS mciplay;
  if (AIL_redbook_status(hand)!=REDBOOK_ERROR) {
    mciplay.dwFrom=startmsec;
    mciplay.dwTo=endmsec;
    hand->lastendsec=endmsec;
    mciSendCommand((UINT)hand->DeviceID,MCI_PLAY,MCI_FROM|MCI_TO,(DWORD)(LPVOID)&mciplay);
  } else {
    return(REDBOOK_ERROR);
  }
  return(AIL_redbook_status(hand));
}


//############################################################################
//##                                                                        ##
//## Stops a playing Red Book Device.  Returns the new Red book status.     ##
//##                                                                        ##
//############################################################################

U32 AILCALL AIL_API_redbook_stop(HREDBOOK hand)
{
  if (AIL_redbook_status(hand)!=REDBOOK_ERROR) {
    hand->paused=0;
    mciSendCommand((UINT)hand->DeviceID,MCI_STOP,0,0);
  } else {
    return(REDBOOK_ERROR);
  }
  return(AIL_redbook_status(hand));
}


//################################################################################
//##                                                                            ##
//## Pauses the playing of a Red Book Device.  Returns the new Red book status. ##
//##                                                                            ##
//################################################################################

U32 AILCALL AIL_API_redbook_pause(HREDBOOK hand)
{
  U32 s=AIL_redbook_status(hand);

  if (s==REDBOOK_ERROR)
    return(REDBOOK_ERROR);

  if (s==REDBOOK_PLAYING) {
    hand->pausedsec=AIL_redbook_position(hand);
    hand->paused=1;
    mciSendCommand((UINT)hand->DeviceID,MCI_STOP,0,0);
  }
  return(AIL_redbook_status(hand));
}


//############################################################################
//##                                                                        ##
//## Resumes a paused Red Book Device.  Returns the new red book status.    ##
//##                                                                        ##
//############################################################################

U32 AILCALL AIL_API_redbook_resume(HREDBOOK hand)
{
  U32 s=AIL_redbook_status(hand);

  if (s==REDBOOK_ERROR)
    return(REDBOOK_ERROR);

  if (s==REDBOOK_PAUSED) {
    hand->paused=0;
    AIL_redbook_play(hand,hand->pausedsec,hand->lastendsec);
  }
  return(AIL_redbook_status(hand));
}


//############################################################################
//##                                                                        ##
//## Gets the current volume of the CD device.  Returns 0 to 127.           ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_redbook_volume(HREDBOOK hand)
{
  U32 volume;

  if (hand)
    if (voldeviceid!=-1) {

#ifdef IS_WIN32

      if (mixerapi) {

        mixerGetControlDetails((HMIXEROBJ)voldeviceid,&volumedetails,MIXER_SETCONTROLDETAILSF_VALUE);
        return((volumevalue[0].dwValue&0xffff)>>9);

      } else

#endif

      {
        if (auxGetVolume((UINT)voldeviceid,&volume)==0)
          return((volume&0xffff)>>9);
      }

    }

  return(-1);
}

//############################################################################
//##                                                                        ##
//## Sets the current volume of the CD device.  Returns 0 to 127.           ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_redbook_set_volume(HREDBOOK hand, S32 volume)
{
  U32 v;

  if ((hand) && (volume>=0) && (volume<=127))
    if (voldeviceid!=-1)  {

#ifdef IS_WIN32

      if (mixerapi) {

        volumevalue[0].dwValue=volume*516;
        volumevalue[1].dwValue=volume*516;
        mixerSetControlDetails((HMIXEROBJ)voldeviceid,&volumedetails,MIXER_SETCONTROLDETAILSF_VALUE);

      } else

#endif

      {
        v=volume*516;
        v|=(v<<16);
        auxSetVolume((UINT)voldeviceid,v);
      }

    }

  return(AIL_redbook_volume(hand));
}

