//############################################################################
//##                                                                        ##
//##  XMIPLAY.C                                                             ##
//##                                                                        ##
//##  XMIDI file player                                                     ##
//##                                                                        ##
//##  V1.00 of 13-Jul-94: Initial                                           ##
//##  V1.01 of 18-Jan-95: Set up driver with .INI file                      ##
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

#include "mss.h" 

#include "con_util.c"

/***************************************************************/
void MSS_MAIN_DEF main(int argc, char *argv[])
{
  HMDIDRIVER  mdi;
  HSEQUENCE   S;
  S32         i;
  S32         seq_num;
  void        *ptr;

  set_up_console( 0 );

  printf( "XMIPLAY - Version " MSS_VERSION "           " MSS_COPYRIGHT " \n" );
  printf( "-------------------------------------------------------------------------------\n\n" );

  printf( "This program plays one or more XMIDI sequences simultaneously using\n" );
  printf( "any MSS MIDI driver.\n\n" );

  if (argc < 2)
  { 
    printf("Enter name of file to play (use '" MSS_DIR_UP "media" MSS_DIR_SEP "Demo.XMI', for example): ");
    get_args( &argc, &argv );
  }

  //
  // set the redist directory and statup the system
  //

  AIL_set_redist_directory( MSS_DIR_UP_TWO "redist" MSS_DIR_SEP MSS_REDIST_DIR_NAME );

  AIL_startup();

  //
  // Initialize MIDI sound system
  //

  mdi = AIL_open_XMIDI_driver( 0 );

  if ( mdi == 0)
  {
    printf( AIL_last_error() );

    AIL_shutdown();
    exit( 1 );
  }

  //
  // Load and play all files
  //
  // If sequence number not given, play sequence 0
  //

  for (i = 1; i < argc; i += 2)
  {

    //
    // load the file into memory
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

    if ( (i+1) < argc)
    {
      seq_num = atol( argv[i+1] );
    }
    else
    {
      seq_num = 0;
    }

    //
    // allocate a sequence to use
    //

    S = AIL_allocate_sequence_handle( mdi );

    if (S == NULL)
    {
      printf( AIL_last_error() );
      continue;
    }

    //
    // point the sequence at the loaded file image
    //

    if ( AIL_init_sequence( S, ptr, seq_num ) <= 0 )
    {
      printf( AIL_last_error() );
      continue;
    }

    printf( "Playing %s ... \n", argv[i] );

    //
    // start the sequence playback
    //

    AIL_start_sequence( S );
  }

  //
  // Prompt for keypress
  //

  printf( "\nPress any key to halt playback and exit XMIPLAY\n" );

  while ( AIL_sequence_status( S ) == SEQ_PLAYING )
  {
    //
    // if a key is pressed, then exit
    //

    if ( kbhit() )
    {
      getch();
      break;
    }
  }

  //
  // Shut down driver, XMIDI services, and process services
  //

  AIL_close_XMIDI_driver( mdi );

  AIL_startup();

  printf( "\nXMIPLAY stopped.\n" );
}

