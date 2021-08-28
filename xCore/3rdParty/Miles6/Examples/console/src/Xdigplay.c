//############################################################################
//##                                                                        ##
//##  XDIGPLAY.C                                                            ##
//##                                                                        ##
//##  XMIDI file player with software wave synthesis                        ##
//##                                                                        ##
//##  V1.00 of 18-Sep-94: Initial                                           ##
//##  V1.01 of 18-Jan-95: Set up drivers with .INI files                    ##
//##  V1.02 of  9-Jun-95: Use new HMDIDRIVER/HDIGDRIVER driver handle types ##
//##                      Check driver installation error code              ##
//##                                                                        ##
//##   Author: John Miles                                                   ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "mss.h"

#include "con_util.c"

/***************************************************************/
void MSS_MAIN_DEF main( int argc, char *argv[] )
{
  HMDIDRIVER  mdi;
  HDIGDRIVER  dig;
  HSEQUENCE   S;
  S32         i;
  S32         seq_num;
  void        *wave;
  void        *ptr;

  set_up_console( );

  printf( "XDIGPLAY - Version " MSS_VERSION "             " MSS_COPYRIGHT " \n" );
  printf( "-------------------------------------------------------------------------------\n\n" );

  printf( "This program plays one or more XMIDI sequences simultaneously using\n" );
  printf( "any MSS-compatible .MDI audio driver.\n\n" );

  printf( "XDIGPLAY is similar to XMIPLAY, but it also demonstrates the use of MIDI\n" );
  printf( "events to trigger and control digital sound effects.\n\n" );

  printf( "It may be necessary to use your sound card's control panel to adjust its\n" );
  printf( "digital and FM/MIDI volume settings for the proper balance between music\n" );
  printf( "and digital sound effects.\n\n" );

  printf( "This part of the MSS API is basically obsolete - use the DLS\n" );
  printf( "Services instead.\n\n" );

  if (argc < 3)
  {
    printf( "Usage: XDIGPLAY wavelib.WVL\n" );
    printf( "                file.XMI [seq_number] [file.XMI seq_number ...]\n" );
    exit( 1 );
  }

  //
  // set the redist directory and statup the system
  //

  AIL_set_redist_directory( MSS_DIR_UP_TWO "redist" MSS_DIR_SEP MSS_REDIST_DIR_NAME );

  AIL_startup();

  //
  // set the DOS latency down a bit
  //

  #ifdef IS_DOS
    AIL_set_preference(DIG_LATENCY,              30);
  #endif

  //
  // open a digital driver
  //

  dig=AIL_open_digital_driver( 22050, 16, 2, 0 );

  //
  // Initialize XMIDI sound system
  //

  if (dig == 0)
  {
    printf( AIL_last_error() );

    AIL_shutdown();
    exit( 1 );
  }


  mdi=AIL_open_XMIDI_driver( 0 );

  if (mdi == 0)
  {
    printf( AIL_last_error() );

    AIL_shutdown();
    exit( 1 );
  }

  //
  // Load wave library file, and create a "virtual wavetable synthesizer"
  // with the two previously-loaded drivers
  //
  // Dedicate the maximum number of digital voices to the synthesizer
  //

  wave = AIL_file_read( argv[1], NULL );

  if (wave == NULL)
  {
    printf( "Couldn't read wave library file '%s'\n", argv[1] );

    AIL_shutdown();
    exit(1);
  }

  AIL_create_wave_synthesizer( dig, mdi, wave, MAX_W_VOICES );

  //
  // Load and play all files
  //
  // If sequence number not given, play sequence 0
  //

  for (i = 2; i < argc; i += 2)
  {
    
    //
    // load the file image
    //

    ptr = AIL_file_read( argv[i], NULL );

    if (ptr == NULL)
    {
      printf( "File '%s' not found\n", argv[i] );
      continue;
    }

    //
    // get the sequence number to play
    //

    if (i+1 < argc)
    {
      seq_num = atol( argv[i+1] );
    }
    else
    {
      seq_num = 0;
    }

    //
    // get a sequence to play
    //

    S = AIL_allocate_sequence_handle( mdi );

    if (S == NULL)
    {
      printf( AIL_last_error() );
      continue;
    }

    //
    // initialize the sequence with the loaded file image

    if (AIL_init_sequence( S, ptr, seq_num ) <= 0)
    {
      printf( AIL_last_error() );
      continue;
    }

    printf( "Playing %s ... \n", argv[i] );

    //
    // begin playing the sequence
    //

    AIL_start_sequence( S );
  }

  //
  // Prompt for keypress
  //

  printf( "\nPress any key to halt playback and exit XDIGPLAY\n" );

  while ( AIL_sequence_status( S ) == SEQ_PLAYING )
  {
    //
    // if a key is pressed, then exit
    //

    if (kbhit())
    {
      getch();
      break;
    }
  }

  //
  // Shut down driver, XMIDI services, and process services
  //

  AIL_close_XMIDI_driver( mdi );

  AIL_close_digital_driver( dig );

  AIL_shutdown();

  printf( "\nXDIGPLAY stopped.\n" );
}

