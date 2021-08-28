//############################################################################
//##                                                                        ##
//##  STREAM.C                                                              ##
//##                                                                        ##
//##  Streaming API example program                                         ##
//##                                                                        ##
//##  V1.00 of  2-Jul-94: Initial                                           ##
//##                                                                        ##
//##   Author: Jeff Roberts                                                 ##
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

//############################################################################
//##                                                                        ##
//## Streaming example program                                              ##
//##                                                                        ##
//############################################################################

void MSS_MAIN_DEF main(int argc, char *argv[])
{
  HDIGDRIVER  dig;
  HSTREAM  stream;
  S32 size,mem,i,rate,last_per = 0;
    
  U32 paused=0;
  U32 dotdelay=0;

  set_up_console( 0 );

  printf( "STREAM - Version " MSS_VERSION "            " MSS_COPYRIGHT " \n" );
  printf( "-------------------------------------------------------------------------------\n\n" );

  printf( "This program streams a digital audio MP3 or WAV file with the streaming API.\n\n" );

  if (argc < 2)
  { 
    printf("Enter name of file to play (use '" MSS_DIR_UP "media" MSS_DIR_SEP "Wrong.MP3', for example): ");
    get_args( &argc, &argv );
  }

  //
  // set the redist directory and statup the system
  //

  AIL_set_redist_directory( MSS_DIR_UP_TWO "redist" MSS_DIR_SEP MSS_REDIST_DIR_NAME );

  AIL_startup();

  //
  // open a digital driver
  //

  dig=AIL_open_digital_driver( 44100, 16, 2, 0 );

  if (dig==0)
  {
    printf( AIL_last_error() );
    exit(1);
  }

  //
  // open the stream handle
  //

  stream = AIL_open_stream(dig, argv[1], 0  );

  //
  // loop the stream forever
  //

  AIL_set_stream_loop_count( stream, 0 );

  //
  // get the info about the file
  //

  if (stream == 0)
  {
    printf("Could not open file '%s'\n",argv[1]);

    AIL_shutdown();
    exit(1);
  }

  AIL_stream_info( stream, &rate, 0, &size, &mem );

  printf( "Hit Enter to restart the stream.\n"
          "Hit Space to pause and restart the stream.\n"
          "Hit '<' and '>' to skip forwards and backwards in the stream.\n"
          "Hit '[' and ']' to change the playback rate, '\\' to return to normal.\n"
          "Hit ESC to exit.\n\n"
          "Playing a %i byte sound file with %i bytes of RAM.\n\nPress the escape key to stop playback:     ",
          size, mem );

  AIL_start_stream( stream );

  while (1)
  {

    //
    // (You could process other application events here....)
    //



    //
    // process key strokes
    //

    if (kbhit())
    {
      switch ( getch() )
      {
        case 27:
          goto done;         
        
        case ' ':
          AIL_pause_stream( stream, paused^=1 );
          break;

        case 13:
          paused=0;
          AIL_start_stream( stream );
          break;

        case '<':
        case ',':
          i=AIL_stream_position( stream );
          if (i<5000)
            i=5000;
          AIL_set_stream_position( stream, i-5000 );
          break;

        case '>':
        case '.':
          AIL_set_stream_position( stream, AIL_stream_position( stream )+5000 );
          break;

        case '[':
          i=AIL_stream_playback_rate( stream )-1000;
          if (i<(rate/2))
            i=rate/2;
          AIL_set_stream_playback_rate( stream ,i );
          break;

        case ']':
          i=AIL_stream_playback_rate( stream )+1000;
          if (i>(rate*2))
            i=rate*2;
          AIL_set_stream_playback_rate( stream, i );
          break;

        case '\\':
          AIL_set_stream_playback_rate( stream, rate );
          break;
      }
    }

    //
    // Show the percentage complete
    //

    if (AIL_ms_count()>dotdelay)
    {
      S32 per = AIL_stream_position( stream )*100/size ;
      if ( per != last_per)
      {
        printf( "\b\b\b\b%3i%%", per );
        last_per = per;
      }
      dotdelay=AIL_ms_count()+250;
    }

    //
    // Service audio buffers
    //

    AIL_service_stream( stream, 0 );

    //
    // Exit test loop when final buffer stops playing
    //

  }

 done:

  //
  // Clean up
  //

  AIL_close_stream( stream );

  //
  // Shut down driver, digital services, and process services
  //

  AIL_close_digital_driver( dig );

  AIL_shutdown();

  printf( "\n\nSTREAM stopped.\n" );
}

