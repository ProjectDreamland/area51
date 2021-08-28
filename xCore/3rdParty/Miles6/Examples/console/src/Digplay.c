//############################################################################
//##                                                                        ##
//##  DIGPLAY.C                                                             ##
//##                                                                        ##
//##  Digital .WAV / .VOC file player                                       ##
//##                                                                        ##
//##  V1.00 of  4-Jul-94: Initial                                           ##
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
#include <string.h>

#include "mss.h"

#include "con_util.c"

//############################################################################
//##                                                                        ##
//## Digital playback example program                                       ##
//##                                                                        ##
//############################################################################

void MSS_MAIN_DEF main( int argc, char *argv[] )
{
  HDIGDRIVER  dig;
  HSAMPLE     S;
  S32         i;
  U32        *ptr;

  set_up_console( 0 );

  printf( "DIGPLAY - Version " MSS_VERSION "           " MSS_COPYRIGHT " \n" );
  printf( "-------------------------------------------------------------------------------\n\n" );

  printf( "This program plays one or more .WAV or .VOC files simultaneously using\n" );
  printf( "any MSS-compatible digital audio driver.\n\n" );

  if (argc < 2)
  { 
    printf("Enter name of file to play (use '" MSS_DIR_UP "media" MSS_DIR_SEP "Welcome.WAV', for example): ");
    get_args( &argc, &argv );
  }

  //
  // set the redist directory and statup the system
  //

  AIL_set_redist_directory( MSS_DIR_UP_TWO "redist" MSS_DIR_SEP MSS_REDIST_DIR_NAME );

  AIL_startup();

  //
  // Initialize digital sound system
  //

  dig = AIL_open_digital_driver( 44100, 16, 2, 0);

  if ( dig==0 )
  {
    printf( AIL_last_error() );

    AIL_shutdown();
    exit( 1 );
  }

  //
  // Load and play all files
  //

  for (i = 1; i < argc; i++)
  {
    printf( "Loading %s\n", argv[i] );

    //
    // load the file into memory
    //

    ptr = (U32*)AIL_file_read( argv[i], FILE_READ_WITH_SIZE );

    if (ptr == NULL)
    {
      printf( "File '%s' not found\n", argv[i] );
      S = 0;
      continue;
    }

    //
    // allocate an HSAMPLE to use
    //

    S = AIL_allocate_sample_handle( dig );

    if (S == NULL)
    {
      printf( AIL_last_error() );
      continue;
    }

    //
    // initialize the sample
    //

    AIL_init_sample( S );

    //
    // point the sample at the loaded file image (the first dword is the size)
    //

    if (AIL_set_named_sample_file( S, argv[i], ptr+1, ptr[0], -1 ) == 0)
    {
      printf( AIL_last_error() );
      continue;
    }

    //
    // begin playing the sample
    //

    AIL_start_sample( S );
  }

  //
  // Prompt for keypress
  //

  printf( "\nPress any key to halt playback and exit DIGPLAY\n" );

  //
  // wait until the sample is done
  //

  while ( AIL_sample_status( S ) == SMP_PLAYING )
  {
    if ( kbhit() )
    {
      getch();
      break;
    }
  }

  //
  // release the sample handle (not really required in this simple example)
  //

  AIL_release_sample_handle( S );

  //
  // Shut down driver, digital services, and process services
  //

  AIL_close_digital_driver( dig );

  AIL_quick_shutdown();

  printf( "\nDIGPLAY stopped.\n" );
}

