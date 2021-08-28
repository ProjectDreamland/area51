//############################################################################
//##                                                                        ##
//##  PLAY.C                                                                ##
//##                                                                        ##
//##  Quick-integration API test program                                    ##
//##                                                                        ##
//##  V1.00 of 22-Feb-96: Initial                                           ##
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

#include "mss.h"

#include "con_util.c"

void MSS_MAIN_DEF main(int argc, char *argv[])
{
  HAUDIO audio;

  set_up_console( 0 );

  printf( "PLAY - Version " MSS_VERSION "              " MSS_COPYRIGHT " \n" );
  printf( "-------------------------------------------------------------------------------\n\n" );

  printf( "This program plays any .MP3, .WAV, .VOC, .XMI or .MID file\n" );
  printf( "using the currently-selected Miles Sound System driver.\n\n" );

  if (argc != 2)
  { 
    printf("Enter name of file to play (use '" MSS_DIR_UP "media" MSS_DIR_SEP "Welcome.Wav', for example): ");
    get_args( &argc, &argv );
  }

  //
  // set the redist directory
  //

  AIL_set_redist_directory( MSS_DIR_UP_TWO "redist" MSS_DIR_SEP MSS_REDIST_DIR_NAME );

  //
  // Start up Quick Integration API, requesting both MIDI and digital support
  //
  if (!AIL_quick_startup( 1, 1, 22050, 16, 2 ) )
  {
    printf( AIL_last_error() );

    exit( 1 );
  }

  //
  // Load and play the audio data file
  //

  audio = AIL_quick_load_and_play( argv[1],    // Name of file to load and play
                                   1,          // Loop count = single iteration only
                                   0 );        // Return immediately
  if (audio == NULL)
  {
    printf( AIL_last_error() );
    exit(1);
  }

  //
  // Wait for file to finish playing, or until keypress received
  //

  printf( "Press any key to stop playback\n" );

  while ( AIL_quick_status( audio ) != QSTAT_DONE )
  {
    if ( kbhit() )
    {
      getch();
      break;
    }
  }

  //
  // Clean up
  //

  AIL_quick_shutdown();

  printf( "PLAY stopped.\n" );
}

