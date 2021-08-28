//############################################################################
//##                                                                        ##
//##  DBTEST.C                                                              ##
//##                                                                        ##
//##  Double-buffered digital audio test facility                           ##
//##                                                                        ##
//##  V1.00 of  2-Jul-94: Initial                                           ##
//##  V1.01 of 20-Nov-94: Use minimum possible buffer size                  ##
//##  V1.02 of 18-Jan-95: Set up driver with .INI file                      ##
//##  V1.03 of  9-Jun-95: Use new HMDIDRIVER/HDIGDRIVER driver handle types ##
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
#include <fcntl.h>
#include <sys\stat.h>
#include <io.h>

#include "mss.h"

//############################################################################
//##                                                                        ##
//## Check to see if either of the sample's buffers need to be filled with  ##
//## audio data, and, if necessary, refresh it                              ##
//##                                                                        ##
//############################################################################

static void Serve_sample( HSAMPLE sample, S8 **buf, S32 size, S32 file )
{
  S32 n,len;

  n = AIL_sample_buffer_ready(sample);

  if (n != -1)
  {
    printf(".");

    //
    // a buffer is available - do the IO
    //

    len = read( file, buf[n], size );

    //
    // pass it into Miles
    //

    AIL_load_sample_buffer( sample,
                            n,
                            buf[n],
                            len );
  }
}

//############################################################################
//##                                                                        ##
//## Double-buffering example program                                       ##
//##                                                                        ##
//############################################################################

void MSS_MAIN_DEF main( int argc, char *argv[] )
{
  HDIGDRIVER  dig;
  HSAMPLE     sample;
  S32         file;
  S8          *buf[2];
  S32         buffer_size;

  setbuf( stdout, NULL );

  printf( "DBTEST - Version " MSS_VERSION "               " MSS_COPYRIGHT " \n" );
  printf( "-------------------------------------------------------------------------------\n\n" );

  printf( "This program plays a raw 16-bit mono 44-kHz audio data file using\n" );
  printf( "the low-level double-buffering calls.\n\n" );

  if (argc != 2)
  {
    printf( "Usage: DBTEST filename.RAW\n" );
    exit( 1 );
  }

  //
  // set the redist directory and statup the system
  //

  AIL_set_redist_directory( "..\\..\\redist\\" MSS_REDIST_DIR_NAME );

  AIL_startup();

  //
  // Initialize digital sound system
  //

  dig = AIL_open_digital_driver( 44100, 16, 1, 0);

  if (dig == 0)
  {
    printf( AIL_last_error() );
    printf( "Run SETSOUND to select an appropriate digital audio driver\n" );

    AIL_shutdown();
    exit( 1 );
  }

  //
  // ask Miles for the size of the buffer that we need
  //

  buffer_size = AIL_minimum_sample_buffer_size( dig, 44100, DIG_F_MONO_16 );

  //
  // Allocate two buffers for double-buffering
  //

  buf[0] = AIL_mem_alloc_lock( buffer_size );
  buf[1] = AIL_mem_alloc_lock( buffer_size );

  //
  // Allocate sample for double-buffered output
  //

  sample = AIL_allocate_sample_handle( dig );

  //
  // Configure sample for 16-bit mono 44 kHz playback
  //

  AIL_init_sample( sample );

  AIL_set_sample_type( sample, DIG_F_MONO_16, 0 );

  AIL_set_sample_playback_rate( sample, 44100 );

  //
  // Example of application's main event loop
  //
  // Read data from file in buffer_size chunks, passing each chunk to the
  // MSS API when requested
  //

  file = open( argv[1], O_RDONLY | O_BINARY );

  if (file == -1)
  {
    printf( "Could not open file '%s'\n", argv[1] );

    AIL_shutdown();
    exit( 1 );
  }

  printf( "Press any key to stop playback " );

  while ( !kbhit() )
  {
    //
    // (Process application events here....)
    //

    //
    // Service audio buffers
    //

    Serve_sample( sample, buf, buffer_size, file );

    //
    // Exit test loop when final buffer stops playing
    //

    if (AIL_sample_status( sample ) != SMP_PLAYING)
    {
      break;
    }
  }

  if (kbhit())
    getch();

  //
  // Clean up
  //

  AIL_release_sample_handle( sample );

  close( file );

  AIL_mem_free_lock( buf[0] );
  AIL_mem_free_lock( buf[1] );

  //
  // Shut down driver, digital services, and process services
  //

  AIL_close_digital_driver( dig );

  AIL_shutdown();

  printf( "\n\nDBTEST stopped.\n" );
}

