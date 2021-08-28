//############################################################################
//##                                                                        ##
//##  DEMO.C                                                                ##
//##                                                                        ##
//##  MSS demo program                                                      ##
//##                                                                        ##
//##  V1.00 of  3 - Apr - 95: Initial V3.0 release                          ##
//##  V1.01 of  4 - Feb - 96: Minor clean - up stuff (JKR)                  ##
//##  V1.02 of 24 - Feb - 96: Lots of polish and the red book stuff (JKR)   ##
//##  V1.03 of 10 - Nov - 00: Prettied up drastically (JKR)                 ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  For technical support, contact RAD Game Tools at 425 - 893 - 4300.    ##
//##                                                                        ##
//############################################################################

#include <windows.h>
#include <stdlib.h>

#include "mss.h"


//
// Windows constants
//

#define DEVICESTRING   0x0078
#define DIGITALBUTTON1 0x0065
#define DIGITALBUTTON2 0x0066
#define DIGITALBUTTON3 0x0067
#define DIGITALBUTTON4 0x0068
#define STREAMBUTTON   0x0069
#define DIGITALCPU     0x00c8

#define VOLUMESLIDER   0x006E
#define PITCHSLIDER    0x0070
#define VOLUMETEXT     0x0071
#define PITCHTEXT      0x006a

#define CDPLAYBUTTON   0x0080
#define CDBACKBUTTON   0x0081
#define CDFOREBUTTON   0x0082
#define CDSTOPBUTTON   0x0083
#define CDPAUSEBUTTON  0x0084
#define CDEJECTBUTTON  0x0085

#define MIDIBUTTON     0x006C
#define DLSBUTTON      0x0077

#define ABOUTBUTTON    0x006B
#define QUIETBUTTON    0x006D
#define CLOSEBUTTON    0x006f

#ifdef _WIN32
  #define APPNAME "Demo32"
#else
  #define APPNAME "Demo16"
#endif

static char szAppName[] = APPNAME;



//
// Application constants
//

#define DIGITALFILECOUNT  4     // How many digital files to load
#define DIGITALPOOLCOUNT  16    // How many samples to play at once

#define VOLUMEMIN         0
#define VOLUMEMAX         127

#define PITCHMIN          0
#define PITCHMID          64
#define PITCHMAX          127

#define DIGITALRATE       22050
#define DIGITALBITS       16
#define DIGITALCHANNELS   2


//
// Application globals
//

HWND        Window = 0;          // Main Hwnd of the dialog box
HWND        DigitalPitchWindow;  // Hwnd of pitch slider
HWND        DigitalCPUWindow;    // Hwnd of the CPU indicator


HDIGDRIVER  Digital = 0;           // Digital playback device
HSTREAM     DigitalStream = 0;     // Digital stream handle
char        StreamName[] =         // Name of the stream to play
             "..\\media\\Wrong.mp3";

char FAR*   DigitalNames[DIGITALFILECOUNT] = // Names of the samples to play
            {"..\\media\\Welcome.wav",
             "..\\media\\Horn.wav",
             "..\\media\\Glass.wav",
             "..\\media\\Charge.wav"};

void FAR*   DigitalData[DIGITALFILECOUNT] = // File images of the sample files
            {0,0,0,0};

U32         DigitalSize[DIGITALFILECOUNT] = // File lengths of the sample files
            {0,0,0,0};

HSAMPLE     DigitalSamples[DIGITALPOOLCOUNT]; // Pool of digital samples
U32         DigitalCount;                     // How many samples are current
S32         DigitalVolume = 100;              // Current volume
S32         DigitalPitch = PITCHMID;          // Current pitch
S32         DigitalCPUHidden = 0;            // Is the cpu window hidden?


HMDIDRIVER  MIDI = 0;              // MIDI playback device
HMDIDRIVER  DLSMIDI = 0;           // DLS MIDI playback device
HDLSDEVICE  DLSDevice = 0;         // DLS synth device
HDLSFILEID  DLSId = 0;             // DLS loaded file ID
HSEQUENCE   MIDISequence = 0;      // Sequence for plain MIDI
HSEQUENCE   DLSSequence = 0;       // Sequence for MIDI with DLS
void FAR*   MIDIData = 0;          // File image of the XMIDI data
void FAR*   DLSData = 0;           // File image of the DLS data
char        MIDIName[] =           // Name of the XMIDI with DLS file
             "..\\media\\Demo.xmi";


HREDBOOK    RedBook = 0;           // Redbook device
U32         RedBookTrack = 1;      // Current CD track
S32         RedBookPlayed = 0;     // Started a CD track?


S32         trytoacquireDIG = 0;
S32         trytoacquireMDI = 0;




//############################################################################
//##                                                                        ##
//## Open the Miles Sound System for use                                    ##
//##                                                                        ##
//############################################################################

static void Init_Miles( HDIGDRIVER* dig,
                        HMDIDRIVER* mdi,
                        HMDIDRIVER* dls,
                        HDLSDEVICE* dlsdev,
                        HREDBOOK* red )
{
  //
  // Initialize the Miles Sound System
  //

  AIL_set_redist_directory( "..\\..\\redist\\" MSS_REDIST_DIR_NAME );

  AIL_startup();

  //
  // Initialize digital driver
  //

  *dig = AIL_open_digital_driver( DIGITALRATE, DIGITALBITS, DIGITALCHANNELS, 0 );

  //
  // Initialize MIDI driver
  //

  *mdi = AIL_open_XMIDI_driver( 0 );

  //
  // Initialize DLS MIDI driver
  //

  *dls = AIL_open_XMIDI_driver( AIL_OPEN_XMIDI_NULL_DRIVER );

  //
  // Initialize DLS MIDI driver
  //

  if (*dls)
  {
    //
    // Bump down the volume for our sample midi file
    //

    AIL_set_preference( DLS_VOLUME_BOOST, 0 );

    *dlsdev = AIL_DLS_open( *dls, *dig, 0, 0,
                           DIGITALRATE, DIGITALBITS, DIGITALCHANNELS );

  }

  //
  // Initialize the red book interface
  //

  *red = AIL_redbook_open( 0 );

}


//############################################################################
//##                                                                        ##
//## Close the Miles Sound System                                           ##
//##                                                                        ##
//############################################################################

static void Uninit_Miles( HDIGDRIVER* dig,
                          HMDIDRIVER* mdi,
                          HMDIDRIVER* dls,
                          HDLSDEVICE* dlsdev,
                          HREDBOOK* red )
{
   //
   // Close the DLS device
   //

   if (*dlsdev)
   {
     AIL_DLS_close( *dlsdev, RETURN_TO_GM_ONLY_STATE );
     *dlsdev = 0;
   }

   //
   // Close the DLS MIDI driver
   //

   if (*dls)
   {
     AIL_close_XMIDI_driver( *dls );
     *dls = 0;
   }

   //
   // Close the MIDI driver
   //

   if (*mdi)
   {
     AIL_close_XMIDI_driver( *mdi );
     *mdi = 0;
   }

   //
   // Close the digital driver
   //

   if (*dig)
   {
     AIL_close_digital_driver( *dig );
     *dig = 0;
   }


   //
   // Close redbook audio
   //

   if (*red)
   {
     AIL_redbook_stop( *red );
     AIL_redbook_close( *red );
     *red = 0;
   }


   //
   // Now shutdown Miles completely
   //

   AIL_shutdown();
}


//############################################################################
//##                                                                        ##
//## Load one or more data files into an array of file images               ##
//##                                                                        ##
//############################################################################

static void Load_data_files( U32 count,
                             char const FAR* const FAR* filenames,
                             void FAR* FAR* datas,
                             U32  FAR* sizes)
{
  U32 i;

  for (i = 0;i < count;i++)
  {
    U32 size;

    //
    // Get the size of the file
    //

    size = AIL_file_size( filenames[i] );

    //
    // Does the caller want the data?
    //

    if (datas)
    {
      
      //
      // Read the data if there is any
      //

      if (size)
      {
        datas[i] = AIL_file_read( filenames[i], 0 );
      }
      else
      {
        datas[i] = 0;
      }
    }

    //
    // Does the caller want the sizes?
    //

    if (sizes)
    {
      sizes[i] = size;
    }
  }
}


//############################################################################
//##                                                                        ##
//## Free a list of memory pointers                                         ##
//##                                                                        ##
//############################################################################

static void Free_data_files( U32 count,
                             void FAR* FAR* datas )
{
  U32 i;

  for (i = 0;i < count;i++)
  {
    if (datas[i])
    {
      //
      // Free the memory
      //

      AIL_mem_free_lock( datas[i] );
      datas[i] = 0;

    }
  }
}


//############################################################################
//##                                                                        ##
//## Allocate an array of samples, return the number actually allocated     ##
//##                                                                        ##
//############################################################################

static U32 Allocate_sample_pool( HDIGDRIVER dig,
                                 U32 count,
                                 HSAMPLE FAR* samples)
{
  U32 i;

  if (dig == 0)
    return(0);

  for (i = 0;i < count;i++)
  {
    //
    // Allocate a sample
    //

    samples[i] = AIL_allocate_sample_handle( dig );

    if (samples[i] == 0)
    {
      //
      // The allocation failed, so we're out of samples, return the total we did get
      //

      return( i );
    }
  }

  return( count );
}


//############################################################################
//##                                                                        ##
//## Free an array of samples                                               ##
//##                                                                        ##
//############################################################################

static void Release_sample_pool( U32 count,
                                 HSAMPLE FAR* samples)
{
  U32 i;

  for (i = 0;i < count;i++)
  {
    //
    // Free a sample
    //

    AIL_release_sample_handle( samples[i] );

    samples[i] = 0;
  }
}


//############################################################################
//##                                                                        ##
//## Load an MSS file and then split out the XMI and the DLS portion        ##
//##                                                                        ##
//############################################################################

static void Load_MIDI_and_DLS_data( char const FAR* filename,
                                    void FAR* FAR* midi,
                                    void FAR* FAR* dls )
{
  void FAR * image;
  U32 size;

  //
  // Load the input file
  //

  Load_data_files( 1, &filename, &image, &size );

  if (image)
  {
    //
    // Split out the XMI and DLS chunks
    //

    AIL_extract_DLS( image, size, midi, 0, dls, 0, 0 );

    //
    // Now free the original file image (extract_DLS allocates new memory)
    //

    Free_data_files( 1, &image );

  }
  else
  {
    *midi = 0;
    *dls = 0;
  }
}


//############################################################################
//##                                                                        ##
//## Allocate and initialize a sequence to an XMIDI file image              ##
//##                                                                        ##
//############################################################################

static HSEQUENCE Init_sequence( HMDIDRIVER mdi, void FAR* image )
{
  HSEQUENCE seq = 0;

  if (mdi)
  {

    //
    // Allocate a sequence to use
    //

    seq = AIL_allocate_sequence_handle( mdi );
    if (seq)
    {

      //
      // Try to point the sequence at the XMIDI image
      //

      if (!AIL_init_sequence( seq, image, 0 ))
      {

        //
        // Init failed, so clean up and return
        //
        
        AIL_release_sequence_handle(seq);
        seq = 0;

      }
    }
  }

  return(seq);
}


//############################################################################
//##                                                                        ##
//## Open the resources used by this simple demo application                ##
//##                                                                        ##
//############################################################################

static void Open_my_MSS_resources()
{
   //
   // Initialize all the MIDI stuff
   //

   //
   // Load the MIDI and DLS data
   //

   Load_MIDI_and_DLS_data( MIDIName, &MIDIData, &DLSData );

   //
   // Load the DLS instruments into the DLS device
   //

   DLSId = AIL_DLS_load_memory( DLSDevice, DLSData, 0 );

   //
   // Allocate the MIDI and DLS sequences
   //

   MIDISequence = Init_sequence( MIDI, MIDIData );
   DLSSequence = Init_sequence( DLSMIDI, MIDIData );

   //
   // Crank down the volume a little on the DLS sequence
   //

   if (DLSSequence)
     AIL_set_sequence_volume( DLSSequence, 115 ,0 );


   //////////////////////////////////////


   //
   // Initialize all the digital stuff
   //

   //
   // Open the digital stream
   //

   DigitalStream = AIL_open_stream( Digital, StreamName, 0 );
   
   //
   // Store the original stream playback rate in user data slot 0
   //

   if (DigitalStream)
   {
     AIL_set_stream_user_data( DigitalStream,
                               0,
                               AIL_stream_playback_rate( DigitalStream ) );
   }

   //
   // Load each of the digital sound files
   //

   Load_data_files( DIGITALFILECOUNT, DigitalNames, DigitalData, DigitalSize );

   //
   // Allocate a pool of HSAMPLES to use
   //

   DigitalCount = Allocate_sample_pool( Digital, DIGITALPOOLCOUNT, DigitalSamples);
}



//############################################################################
//##                                                                        ##
//## Close the resources opened by this simple demo application             ##
//##                                                                        ##
//############################################################################

static void Close_my_MSS_resources()
{
   //
   // Close the digital stream
   //

   if (DigitalStream)
   {
     AIL_close_stream( DigitalStream );
     DigitalStream = 0;
   }

   //
   // Unload the DLS file ID
   //

   if (DLSId)
   {
     AIL_DLS_unload(DLSDevice,DLSId);
     DLSId = 0;
   }

   //
   // Release the MIDI sequence
   //

   if (DLSSequence)
   {
     AIL_release_sequence_handle( DLSSequence );
     DLSSequence = 0;
   }

   //
   // Release the MIDI sequence
   //

   if (MIDISequence)
   {
     AIL_release_sequence_handle( MIDISequence );
     MIDISequence = 0;
   }

   //
   // Release the sample handles
   //

   Release_sample_pool( DigitalCount, DigitalSamples );

   //
   // Free all of the memory chunks
   //

   if (DLSData)
   {
     Free_data_files( 1, &DLSData );
   }

   if (MIDIData)
   {
     Free_data_files( 1, &MIDIData );
   }

   Free_data_files( DIGITALFILECOUNT, DigitalData );
}


//############################################################################
//##                                                                        ##
//## Start playback of desired digital sample                               ##
//##                                                                        ##
//############################################################################

static void Start_digital_sound( U32 which )
{
  U32 i;

  //
  // Return if no digital driver available
  //

  if (Digital == 0)
  {
    return;
  }

  //
  // Find a HSAMPLE that isn't doing anything
  //

  for (i = 0; i < DigitalCount; i++)
  {
    if (AIL_sample_status( DigitalSamples[i] ) == SMP_DONE)
    {
      //
      // We found a sample to use - set it all up!
      //

      //
      // Initialize sample
      //

      AIL_init_sample( DigitalSamples[i] );

      //
      // Point the sample handle at the loaded file image
      //

      if (!AIL_set_named_sample_file( DigitalSamples[i],
                                      DigitalNames[which],
                                      DigitalData[which],
                                      DigitalSize[which],
                                      0))
      {
        MessageBox( Window,
                    "Sorry, no ASI provider available for this file type.",
                    "Error.",
                    MB_OK);
        return;
      }

      //
      // Set initial volume and rate
      //

      AIL_set_sample_volume( DigitalSamples[i], DigitalVolume );

      //
      // Save the original playback rate in a user_data slot
      //

      AIL_set_sample_user_data( DigitalSamples[i],
                                0,
                                AIL_sample_playback_rate( DigitalSamples[i]) );

      //
      // Now adjust the playback rate to the current slider setting
      //

      AIL_set_sample_playback_rate( DigitalSamples[i],
                                    ((DigitalPitch + 1L)
                                    * AIL_sample_user_data( DigitalSamples[i], 0 ))
                                    / 64L);

      //
      // Finally, start sample and break out of the search loop
      //

      AIL_start_sample( DigitalSamples[i] );

      break;
    }
  }
}


//############################################################################
//##                                                                        ##
//## Updates the pause status of the redbook window controls                ##
//##                                                                        ##
//############################################################################

static void Update_redbook_pause_status( HWND wnd )
{
  U32 status;

  status = AIL_redbook_status( RedBook );

  //
  // Have we played or are we playing a track?
  //

  RedBookPlayed = ((status == REDBOOK_PLAYING) || (status == REDBOOK_PAUSED))?1:0;


  //
  // Update the window status
  //

  SetWindowText( GetDlgItem( wnd, CDPAUSEBUTTON ),
                 status == REDBOOK_PAUSED?"Res&ume":"Pa&use" );
}


//############################################################################
//##                                                                        ##
//## Play a track on the CD                                                 ##
//##                                                                        ##
//############################################################################

static Play_CD_track( S32 offset )
{
  U32 start, end;

  RedBookTrack += offset;

  //
  // Handle the CD tracks wrapping around
  //

  if (RedBookTrack == 0)
  {
    RedBookTrack = AIL_redbook_tracks( RedBook );
  }
  else
  {
    if (RedBookTrack > AIL_redbook_tracks( RedBook ))
      RedBookTrack = 1;
  }

  //
  // Get the beginning and ending of the track and then start it
  //

  AIL_redbook_track_info( RedBook, RedBookTrack, &start, &end );
  AIL_redbook_play( RedBook, start + 1, end );
  Update_redbook_pause_status( Window );
}


//############################################################################
//##                                                                        ##
//## Stops all MIDI and digital sound                                       ##
//##                                                                        ##
//############################################################################

static void Stop_all_sounds()
{
  U32 i;

  //
  // Stop the digital samples
  //

  for (i = 0;i < DigitalCount;i++)
  {
    AIL_end_sample( DigitalSamples[i] );
  }

  //
  // Stop the digital stream
  //

  if ( DigitalStream )
  {
    AIL_pause_stream( DigitalStream, 1 );
  }

  //
  // Stop the MIDI sequence
  //

  if (MIDISequence)
  {
    AIL_end_sequence( MIDISequence );
  }

  //
  // Stop the DLS sequence
  //

  if (DLSSequence)
  {
    AIL_end_sequence( DLSSequence );
  }
}



//############################################################################
//##                                                                        ##
//## Enable or disable a dialog window control                              ##
//##                                                                        ##
//############################################################################

static void Enable_control( HWND wnd, U32 control, S32 state )
{
   EnableWindow( GetDlgItem( wnd, (WORD)control ), (BOOL)state );
}


//############################################################################
//##                                                                        ##
//## Show the about window                                                  ##
//##                                                                        ##
//############################################################################

void Show_about( HWND wnd )
{
   char text[1024];
   char version[8];

   AIL_MSS_version(version,8);
   lstrcpy(text,"Version ");
   lstrcat(text,version);
   lstrcat(text," " MSS_COPYRIGHT "\n\n"
               APPNAME ": Miles Sound System demonstration program.\n\n"
               "For questions or comments, please contact RAD Game Tools at:\n\n"
               "\tRAD Game Tools\n"
               "\t335 Park Place - Suite G109\n"
               "\tKirkland, WA  98033\n"
               "\t425 - 893 - 4300\n"
               "\tFAX: 425 - 893 - 9111\n\n"
               "\tWeb: www.radgametools.com\n"
               "\tE - mail: sales@radgametools.com\n\n"
               "MPEG audio clip is from The Terminator by Orion Pictures.\n"
               "(C) MCMLXXXIV Cinema 84, A Greenberg Bros. Partnership.\n"
               "Obtained from http://www.iocon.com/mp3/mp3audio.html?term");

   MessageBox( wnd, text, "About " APPNAME, MB_OK );
}


//############################################################################
//##                                                                        ##
//## Handle a button being clicked                                          ##
//##                                                                        ##
//############################################################################

static void Handle_button_click( HWND wnd, U32 button )
{
  switch (button)
  {
    //
    // Handle the digital buttons
    //

    case DIGITALBUTTON1:
    case DIGITALBUTTON2:
    case DIGITALBUTTON3:
    case DIGITALBUTTON4:
      Start_digital_sound( button - DIGITALBUTTON1 );
      break;

    //
    // Handle the stream button
    //

    case STREAMBUTTON:
      AIL_service_stream( DigitalStream, 1 );
      AIL_start_stream( DigitalStream );
      break;

    //
    // Handle the MIDI button
    //

    case MIDIBUTTON:
      AIL_start_sequence( MIDISequence );
      break;

    //
    // Handle the DLS button
    //

    case DLSBUTTON:
      AIL_start_sequence( DLSSequence );
      break;

    //
    // Handle the quiet button
    //

    case QUIETBUTTON:
      Stop_all_sounds();             
      // NOTE: falls through to CD stop next!!!

    //
    // Handle the CD player buttons

    case CDSTOPBUTTON:
      if ((RedBook) && (RedBookPlayed))
      {
        AIL_redbook_stop( RedBook );
        Update_redbook_pause_status( wnd );
      }
      break;

    case CDBACKBUTTON:
      Play_CD_track( -1 );
      break;

    case CDFOREBUTTON:
      Play_CD_track( 1 );
      break;

    case CDPLAYBUTTON:
      Play_CD_track( 0 );
      break;

    case CDPAUSEBUTTON:
      if (AIL_redbook_status( RedBook ) == REDBOOK_PAUSED )
      {
        AIL_redbook_resume( RedBook );
      }
      else
      {
        AIL_redbook_pause( RedBook );
      }
      Update_redbook_pause_status( wnd );
      break;

    case CDEJECTBUTTON:
      AIL_redbook_eject( RedBook );
      Update_redbook_pause_status( wnd );
      break;

    //
    // Handle the system buttons
    //

    case ABOUTBUTTON:
      Show_about( wnd );
      break;

    case IDCANCEL:
    case CLOSEBUTTON:
      DestroyWindow( wnd );
      break;
  }
}


//############################################################################
//##                                                                        ##
//## Handle the timer tick by updating the CPU indicator (if necessary)     ##
//##                                                                        ##
//############################################################################

static void Handle_timer_tick( void )
{
  S32 percent = 0;

  //
  // Get the CPU hit of the DLS software synth
  //

  if ( AIL_sequence_status( DLSSequence ) == SEQ_PLAYING)
  {
    S32 dlspercent;
    AIL_DLS_get_info( DLSDevice, 0, &dlspercent );
    percent += dlspercent;
  }

  //
  // Add in the digital subsystem CPU hit
  //

  if (Digital)
  {
    percent += AIL_digital_CPU_percent( Digital );
  }


  //
  // Should we show the percentage?
  //

  if ((percent) || (AIL_active_sample_count( Digital ) > 1))
  {
    char buf[16];

    //
    // Set the Window text to the current perctange
    //
    wsprintf( buf, " CPU: %i%% ", percent );
    SetWindowText( DigitalCPUWindow, buf );

    //
    // Is the window hidden, if so show it!
    //

    if (DigitalCPUHidden)
    {
      ShowWindow( DigitalCPUWindow, SW_SHOW );
      DigitalCPUHidden = 0;
    }
  }
  else
  {

    //
    // No percentage to show (hide the window if it isn't already)
    //

    if (DigitalCPUHidden == 0)
    {
      ShowWindow(DigitalCPUWindow,SW_HIDE);
      DigitalCPUHidden = 1;
    }
  }
}


//############################################################################
//##                                                                        ##
//## Update the playback rate of the currently playing samples and streams  ##
//##                                                                        ##
//############################################################################

static S32 Update_pitch( S32 newpos )
{
  U32 i;

  //
  // Clip the pitch to the possible range
  //

  if (newpos < PITCHMIN)
    newpos = PITCHMIN;
  else if (newpos > PITCHMAX)
    newpos = PITCHMAX;


  //
  // If the pitch hasn't changed, then just return
  //

  if (DigitalPitch == newpos)
    return( -1 );

  DigitalPitch = newpos;

  //
  // Apply the pitch to each of the playing digital samples
  //

  for (i = 0;i < DigitalCount;i++)
  {

    //
    // Is the sample currently playing?
    //

    if (AIL_sample_status( DigitalSamples[i] ) == SMP_PLAYING)
    {
      //
      // We store the original playback rate in a user data, so use
      //   it to adjust the playback rate based on the slider value
      //

      AIL_set_sample_playback_rate( DigitalSamples[i],
                              (((U32)DigitalPitch + 1L)
                              * AIL_sample_user_data( DigitalSamples[i], 0 ))
                              / 64L);
    }
  }

  //
  // Adjust the stream rate as well
  //

  if (DigitalStream)
    AIL_set_stream_playback_rate( DigitalStream,
                                  (((U32)DigitalPitch + 1L)
                                  * AIL_stream_user_data( DigitalStream, 0 ))
                                  / 64L);
  return( DigitalPitch );
}


//############################################################################
//##                                                                        ##
//## Update the volume of the currently playing samples and streams         ##
//##                                                                        ##
//############################################################################

static S32 Update_volume( S32 newpos )
{
  U32 i;

  //
  // Clip the volume to the possible range
  //

  if (newpos < VOLUMEMIN)
    newpos = VOLUMEMIN;
  else if (newpos > VOLUMEMAX)
    newpos = VOLUMEMAX;

  //
  // If the volume hasn't changed, then just return
  //

  if (DigitalVolume == newpos)
    return( -1 );

  DigitalVolume = newpos;

  //
  // Apply the volume to each of the playing digital samples
  //

  for (i = 0;i < DigitalCount;i++)
  {
    //
    // Adjust the volume of all playing samples
    //

    if (AIL_sample_status( DigitalSamples[i] ) == SMP_PLAYING)
    {
      //
      // Adjust the volume
      //

      AIL_set_sample_volume( DigitalSamples[i], DigitalVolume );
    }
  }

  //
  // Adjust the stream volume as well
  //

  if (DigitalStream)
    AIL_set_stream_volume( DigitalStream, DigitalVolume );

  return( DigitalVolume );
}


//############################################################################
//##                                                                        ##
//## defines to handle scrollbar reading on Win16 and Win32                 ##
//##                                                                        ##
//############################################################################

#ifdef _WIN32
  #define GetSBHWND(lp) ((HWND)(lp))
  #define GetSBPos()   (HIWORD(wparam))
#else
  #define GetSBHWND(lp) ((HWND)(HIWORD(lp)))
  #define GetSBPos() (LOWORD(lparam))
#endif


//############################################################################
//##                                                                        ##
//## Main window procedure                                                  ##
//##                                                                        ##
//############################################################################

LRESULT AILEXPORT Window_proc( HWND wnd,
                               UINT message,
                               WPARAM wparam,
                               LPARAM lparam )
{
  S32 i;
  HWND scroll;

  switch (message)
  {
    case WM_HSCROLL:

      scroll = GetSBHWND( lparam );

      //
      // What did the user do?
      //

      i = GetScrollPos( scroll, SB_CTL );
      switch ( LOWORD( wparam ) )
      {
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
          i = GetSBPos();
          break;

        case SB_BOTTOM:
        case SB_PAGEDOWN:
        case SB_LINEDOWN:
          i++;
          break;

        case SB_TOP:
        case SB_PAGEUP:
        case SB_LINEUP:
          i-- ;
          break;
      }

      //
      // Perform what the user did
      //

      if (scroll == DigitalPitchWindow)
        i = Update_pitch( i );
      else
        i = Update_volume( i );

      //
      // Now update the screen (if the value changed)
      //

      if (i != -1)
        SetScrollPos( scroll, SB_CTL, (WORD)i, TRUE );
      break;

    case WM_COMMAND:
      Handle_button_click( wnd, LOWORD( wparam ) );
      return 0;

    case WM_TIMER:
      Handle_timer_tick();
      break;

    case WM_DESTROY:
      PostQuitMessage( 0 );
      return 0;

    case WM_ENDSESSION:  // This is for 16 - bit windows
      Stop_all_sounds();
      AIL_shutdown();
      break;
  }

  return( DefWindowProc( wnd, message, wparam, lparam ) );
}


//############################################################################
//##                                                                        ##
//## Initialize the window to match the loaded state                        ##
//##                                                                        ##
//############################################################################

static void Init_window( HWND wnd,
                         HDIGDRIVER     dig,
                         U32            digcount,
                         void FAR* FAR* digdata,
                         HSTREAM        stream,
                         HMDIDRIVER     mdi,
                         HSEQUENCE      midiseq,
                         HMDIDRIVER     dls,
                         HDLSDEVICE     dlsdev,
                         HDLSFILEID     dlsid,
                         HSEQUENCE      dlsseq,
                         HREDBOOK       red )
{
  HWND ctrl;
  U32 i;
  char buf[256];

  //
  // Set the volume and pitch controls to their default values
  //

  ctrl = GetDlgItem( wnd, VOLUMESLIDER );
  SetScrollRange( ctrl, SB_CTL, VOLUMEMIN, VOLUMEMAX, FALSE );
  SetScrollPos( ctrl, SB_CTL, (WORD)DigitalVolume, TRUE );

  DigitalPitchWindow = GetDlgItem( wnd, PITCHSLIDER );
  SetScrollRange( DigitalPitchWindow, SB_CTL, PITCHMIN, PITCHMAX, FALSE );
  SetScrollPos( DigitalPitchWindow, SB_CTL, (WORD)DigitalPitch, TRUE);


  //
  // Set the description of the digital device
  //

  if (dig)
  {
    strcpy(buf,"Device: ");
    AIL_digital_configuration( dig, 0, 0, buf + strlen( buf ) );
  }
  else
  {
    strcpy(buf,"Unable to open any digital device.");
    Enable_control( wnd, VOLUMESLIDER, 0 );
    Enable_control( wnd, PITCHSLIDER, 0 );
    Enable_control( wnd, VOLUMETEXT, 0 );
    Enable_control( wnd, PITCHTEXT, 0 );
  }
  SetWindowText( GetDlgItem( wnd, DEVICESTRING ), buf );

  //
  // Disable digital controls if digital had any init problems
  //

  for (i = 0;i < DIGITALFILECOUNT;i++)
  {
    if ((dig == 0) || (digcount == 0) || (digdata[i] == 0))
      Enable_control( wnd, DIGITALBUTTON1 + i, 0 );
  }

  //
  // Turn off the stream if it couldn't be opened
  //

  if (stream == 0)
     Enable_control( wnd, STREAMBUTTON, 0 );

  //
  // Disable MIDI controls if MIDI had trouble
  //

  if ((mdi == 0) || (midiseq == 0))
  {
    Enable_control( wnd, MIDIBUTTON, 0);
  }

  //
  // Disable DLS controls if DLS had trouble
  //

  if ((dig == 0) || (dls == 0) || (dlsdev == 0) || (dlsid == 0) || (dlsseq == 0))
  {
    Enable_control( wnd, DLSBUTTON, 0);
  }

  //
  // Disable the CD buttons if redbook didn't open
  //

  if (red == 0)
  {
    Enable_control( wnd, CDPLAYBUTTON, 0 );
    Enable_control( wnd, CDBACKBUTTON, 0 );
    Enable_control( wnd, CDFOREBUTTON, 0 );
    Enable_control( wnd, CDSTOPBUTTON, 0 );
    Enable_control( wnd, CDPAUSEBUTTON, 0 );
    Enable_control( wnd, CDEJECTBUTTON, 0 );
  }

  //
  // Hide the CPU indicator
  //

  DigitalCPUWindow = GetDlgItem( wnd, DIGITALCPU );
  ShowWindow( DigitalCPUWindow, SW_HIDE );
  DigitalCPUHidden = 1;

  //
  // Setup a timer to display CPU hit
  //

   SetTimer( wnd, 1, 2000, 0 );
}


//############################################################################
//##                                                                        ##
//## Create the application window                                          ##
//##                                                                        ##
//############################################################################


static HWND Create_window( HINSTANCE inst, HINSTANCE previnst )
{
  
  //
  // If this is the first instance (always true under Win32), create the class
  //

  if (!previnst)
  {
    WNDCLASS wndclass;

    wndclass.lpszClassName = szAppName;
    wndclass.lpfnWndProc = (WNDPROC) Window_proc;
    wndclass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndclass.hInstance = inst;
    wndclass.hIcon = LoadIcon( inst, "Demo" );
    wndclass.hCursor = LoadCursor( NULL, IDC_ARROW );

#ifdef _WIN32
    wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
#else
    wndclass.hbrBackground = GetStockObject( LTGRAY_BRUSH );
#endif

    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = DLGWINDOWEXTRA;
    wndclass.lpszMenuName = NULL;

    RegisterClass( &wndclass );
  }

  //
  // Now attempt to create the window
  //

  return( CreateDialog( inst, szAppName, 0, NULL ) );
}


//############################################################################
//##                                                                        ##
//## WinMain()                                                              ##
//##                                                                        ##
//############################################################################

int PASCAL WinMain( HINSTANCE instance,
                    HINSTANCE previnstance,
                    LPSTR cmdline,
                    int cmdshow )
{
   MSG msg;

   //
   // Create the main window
   //

   Window = Create_window( instance, previnstance );
   if (!Window)
   {
     MessageBox( 0, "Unable to create window.", "Error", MB_OK|MB_ICONSTOP );
     return( 0 );
   }

   //
   // Initialize Miles
   //

   Init_Miles( &Digital,
               &MIDI,
               &DLSMIDI,
               &DLSDevice,
               &RedBook );


   //
   // Open the resources that this simple demo uses
   //

   Open_my_MSS_resources();


   //
   // Initialize all of the controls to match what was successfully loaded
   //

   Init_window( Window,
                Digital,
                DigitalCount,
                DigitalData,
                DigitalStream,
                MIDI,
                MIDISequence,
                DLSMIDI,
                DLSDevice,
                DLSId,
                DLSSequence,
                RedBook );

   //
   // Begin the main message loop
   //

   ShowWindow( Window, cmdshow );

   while ( GetMessage( &msg, 0, 0, 0 ) )
   {

     if (!IsDialogMessage( Window, &msg ) )
     {
       TranslateMessage(&msg);
       DispatchMessage(&msg);
     }

   }


   //
   // All done - shut everything down!
   //


   //
   // Stop all the currently playing sounds
   //

   Stop_all_sounds();


   //
   // Close this simple demo's resources (you aren't required to do this
   //    AIL_shutdown will clean it all up, but it is listed here for completeness
   //

   Close_my_MSS_resources();


   //
   // Uninitialize Miles
   //

   Uninit_Miles( &Digital,
                 &MIDI,
                 &DLSMIDI,
                 &DLSDevice,
                 &RedBook );

   return msg.wParam;
}

