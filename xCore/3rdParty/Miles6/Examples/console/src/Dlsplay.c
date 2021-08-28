//############################################################################
//##                                                                        ##
//##  DLSPLAY.C                                                             ##
//##                                                                        ##
//##  Quick-integration API test program                                    ##
//##                                                                        ##
//##  V1.00 of 22-Feb-96: Initial                                           ##
//##                                                                        ##
//##  Project: MSS 3.5                                                      ##
//##   Author: Jeff Roberts                                                 ##
//##                                                                        ##
//##  C source compatible with 32-bit ANSI C                                ##
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

static void Show_error( char const* err )
{
  printf( "%s", (err==0)?AIL_last_error():err );

  AIL_quick_shutdown();

  exit( 1 );
}


void MSS_MAIN_DEF main(int argc, char *argv[])
{
  HAUDIO     audio;
  HDLSDEVICE dls;
  U32        nextupdate;

  set_up_console( 0 );

  printf( "DLSPLAY - Version " MSS_VERSION "           " MSS_COPYRIGHT " \n" );
  printf( "-------------------------------------------------------------------------------\n\n" );

  printf( "This program plays an XMI with embedded DLS data using the Miles DLS software\nsynthesizer.\n\n" );

  if (argc < 2)
  {
    printf("Enter name of file to play (use '" MSS_DIR_UP "media" MSS_DIR_SEP "Demo.XMI', for example): ");
    get_args( &argc, &argv );
  }

  //
  // set the redist directory and statup the system
  //

  AIL_set_redist_directory( MSS_DIR_UP_TWO "redist" MSS_DIR_SEP MSS_REDIST_DIR_NAME );
  
  //
  // Start up Quick Integration API, requesting both MIDI and digital support
  //
  if ( !AIL_quick_startup( 1, AIL_QUICK_DLS_ONLY, 44100, 16, 2 ) )
  {
    Show_error(0);
  }

  AIL_quick_handles( 0, 0, &dls );

  //
  // Load and play the audio data file
  //

  audio = AIL_quick_load( argv[1] );    // Name of file to load and play

  if (audio == NULL)
  {
    Show_error(0);
  }

  //
  // identify the file type and prepare it to be played
  //

  switch ( AIL_quick_type( audio ) )
  {
    case AIL_QUICK_DLS_XMIDI_TYPE:
      break;

    case AIL_QUICK_XMIDI_TYPE:

      //
      // XMIDI with DLS data - we need DLS data to play it
      //

      if (argc!=3) 
      {
        Show_error("To play an MIDI without embedded DLS data, you must also specify a DLS file.\n");
      } 
      else 
      {

        //
        // load the DLS file and get the DLS data
        //

        void* dlsfile;
        U32*  dlsdata;

        dlsdata=(U32*)AIL_file_read( argv[2], FILE_READ_WITH_SIZE );

        if (dlsdata==0)
        {
          Show_error(0);
        }

        //
        // now see what kind of DLS file we have
        //

        switch ( AIL_file_type( dlsdata+1, dlsdata[0] ) )
        {

          //
          // uncompressed DLS file - just use it
          //

          case AILFILETYPE_DLS:
            dlsfile=dlsdata+1;  // skip the size
            break;

          //
          // uncompressed DLS data in an XMI file - just find it
          //

          case AILFILETYPE_XMIDI_DLS:
            if ( AIL_find_DLS( dlsdata+1, dlsdata[0], 0, 0, &dlsfile, 0 )==0)
            {
              Show_error(0);
            }
            break;

          //
          // compressed DLS data standalone or in an XMI file - decompress it
          //

          case AILFILETYPE_MLS:
          case AILFILETYPE_XMIDI_MLS:
            if ( AIL_extract_DLS( dlsdata+1, dlsdata[0], 0, 0, &dlsfile, 0, 0 )==0)
            {
              Show_error(0);
            }

            //
            // free the compressed dls data (now that we uncompressed it)
            //

            AIL_mem_free_lock( dlsdata );
            break;
        }

        //
        // load the file into the DLS synth
        //

        if ( AIL_DLS_load_memory( dls, dlsfile, 0 )==0)
        {
          Show_error(0);
        }
      }
      break;

    default:
      printf( "Not an MIDI file.\n" );
      Show_error(0);
      break;
  }

  AIL_quick_play( audio, 1 );

  //
  // Wait for file to finish playing, or until keypress received
  //

  printf( "Press any key to stop playback.\n\n" );

  nextupdate=AIL_ms_count()-1;

  while ( AIL_quick_status( audio ) != QSTAT_DONE)
  {

    //
    // display the percent CPU used every two seconds
    //

    U32 curtime=AIL_ms_count();

    if (curtime>nextupdate)
    {
      S32 percent;

      AIL_DLS_get_info( dls, 0, &percent );
      printf( "\rCPU: %i%%", percent );

      nextupdate=curtime+1000;
    }

    //
    // check for key hit
    //

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

  printf( "\n\nDLSPLAY stopped.\n" );
}

