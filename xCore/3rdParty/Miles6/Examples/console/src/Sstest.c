//############################################################################
//##                                                                        ##
//##  SSTEST.C                                                              ##
//##                                                                        ##
//##  Digital audio mixer test facility                                     ##
//##                                                                        ##
//##  V1.00 of 13-Jun-94: Initial                                           ##
//##  V1.01 of 28-Nov-94: Check for voice overflow in test loop             ##
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
#include <string.h>
#include <ctype.h>

#include "mss.h"

#include "con_util.c"

#define N_FILES  8         // # of .RAW file samples to load and play
#define N_VOICES 32        // # of mixer channels to allocate

char *filenames[N_FILES] =   // data files to load
   {
   "Welcome.wav"  ,
   "Horn.wav"   ,
   "Charge.wav"   ,
   "Wrong.mp3" ,
   "She_bop.wav"  ,
   "Guitar.wav"    ,
   "Glass.wav"  ,
   "Shot.wav"
   };


//############################################################################
//##                                                                        ##
//## Digital playback example program                                       ##
//##                                                                        ##
//############################################################################

void MSS_MAIN_DEF main( int argc, char *argv[] )
{
  HDIGDRIVER dig;             // Handle to digital audio driver
  U32        *snd[N_FILES];   // Pointer to beginning of audio data for each .RAW
  HSAMPLE    smp[N_VOICES];   // Sample handles for each mixer channel
  S32        num_voices;      // Actual number of mixer channels available

  char       ch;
  S32        i,n;
  U32        nextupdate;
  U32        rate, bits, chans;

  set_up_console( 1 ); 

  printf( "SSTEST - Version " MSS_VERSION "            " MSS_COPYRIGHT " \n" );
  printf( "-------------------------------------------------------------------------------\n\n" );

  printf( "This program plays multiple sound files.\n\n" );

  if ((argc > 1) && (!isdigit( argv[1][0] )))
  {
    printf( "Usage: SSTEST [Frequency] [8 or 16 bit] [1 or 2 channels] \n" );
    exit( 1 );
  }

  //
  // set the redist directory and startup the system
  //

  AIL_set_redist_directory( MSS_DIR_UP_TWO "redist" MSS_DIR_SEP MSS_REDIST_DIR_NAME );

  AIL_startup();

  //
  // get the initialization parameters
  //

  rate=44100;
  bits=16;
  chans=2;

  if (argc > 1)
    rate=atoi( argv[1] );

  if (argc > 2)
    bits=atoi( argv[2] );

  if (argc > 3)
    chans=atoi( argv[3]);

  //
  // Initialize digital sound system
  //

  dig = AIL_open_digital_driver( rate, bits, chans, 0 );
 
  if (dig==0)
  {
    printf( AIL_last_error() );

    AIL_shutdown();
    exit( 1 );
  }

  //
  // Allocate 16 digital mixer channels, or as many as available
  //

  for (i=0; i < N_VOICES; i++)
  {
    smp[i] = AIL_allocate_sample_handle( dig );

    if (smp[i] == NULL)
    {
      break;
    }
  }

  num_voices = i;

  //
  // Load samples and show help screen
  //

  for (i=0; i < N_FILES; i++)
  {
    char filename[260];
    
    //
    // load each file image into memory
    //

    strcpy( filename, MSS_DIR_UP "media" MSS_DIR_SEP );
    strcat( filename, filenames[i] );
    
    snd[i]=(U32*)AIL_file_read( filename, FILE_READ_WITH_SIZE );

    printf( "    %d: %s\n", i+1, filenames[i] );
  }

  printf( "\n");
  printf( " + - : Adjust the volume of current sample\n" );
  printf( " [ ] : Adjust the pan position of current sample\n" );
  printf( " < > : Adjust the pitch of current sample\n\n" );

  printf( "ENTER: Loop current sample\n" );
  printf( "SPACE: Stop all samples\n" );
  printf( " ESC : Quit\n\n" );

  //
  // Fetch and interpret keypresses
  //  

  i = -1; 

  nextupdate=AIL_ms_count()+1000;

  while (1)
  {
    if (AIL_ms_count()>nextupdate)
    {
      //
      // only update the CPU percentage every second
      //

      printf( "\r  CPU: %i %% (%i samples playing)   ", AIL_digital_CPU_percent( dig ),AIL_active_sample_count(dig) );
      nextupdate=AIL_ms_count()+1000;
    }

    if (kbhit())
    {
      ch=getch();
 
      switch (ch)
      {
        //
        // Start playback of sample
        //
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8':

          //
          // Get file # and find an HSAMPLE to play it
          //
          // If no samples available, ignore request
          // 

          n = ch-'1';

          for (i=0; i < num_voices; i++)
          {
            if ( AIL_sample_status( smp[i] ) == SMP_DONE)
            {
              break;
            }
          }

          if ( ( i == num_voices ) || ( snd[ n ] == 0 ) )
          {
            i = -1;
            break;
          }

          //
          // Initialize sample
          //

          AIL_init_sample( smp[i] );

          //
          // point the sample at the file image (the size is in the first dword)
          //

          if ( AIL_set_named_sample_file( smp[i], 
                                          filenames[n], 
                                          snd[n]+1, 
                                          snd[n][0], 
                                          0 )==0)
          {
            //
            // if the file couldn't set, then assume it is a RAW file
            //

            AIL_set_sample_type( smp[i], DIG_F_MONO_8, 0 );
            AIL_set_sample_address( smp[i], snd[n]+1, snd[n][0] );
            AIL_set_sample_playback_rate( smp[i], 44100 );

         }

          //
          // Finally, activate sample
          //

          AIL_start_sample( smp[i] );
          break;

        //
        // Stop all samples
        //
        case ' ':
          for (n=0; n < num_voices; n++)
          {
            AIL_end_sample( smp[n] );
          }
          break;

        //
        // Loop sample indefinitely
        //
        case 13:
          if (i != -1)
          {
            AIL_set_sample_loop_count( smp[i], 0 );
          }
          break;

        //
        // Pitch-shift sample by 1 kHz increments
        //

        case '.': case '>':
          if (i != -1)
          {
            //
            // get the playback rate and then speed it up
            //

            n = AIL_sample_playback_rate( smp[i] );

            AIL_set_sample_playback_rate( smp[i], n + 1000 );
          }
          break;

        case ',': case '<':
          if (i != -1)
          {
            //
            // get the playback rate and then slow it down
            //

            n = AIL_sample_playback_rate( smp[i] );

            AIL_set_sample_playback_rate( smp[i], n - 1000 );
          }
          break;

        //
        // Change volume/pan of sample
        //

        case '+': case '=':
          //
          // get the volume and then turn it up
          //
          n = AIL_sample_volume( smp[i] );
          n += 10;
          if (n > 127)
            n = 127;

          AIL_set_sample_volume( smp[i], n );
          break;

        case '-': case '_':
          //
          // get the volume and then turn it down
          //

          n = AIL_sample_volume( smp[i] );
          n -= 10;
          if (n < 0)
            n = 0;

          AIL_set_sample_volume( smp[i] ,n );
          break;

        //
        // Pan left/right
        //
 
        case ']': case '}':
          //
          // get the pan and then shift it right
          //

          n = AIL_sample_pan( smp[i] );
          n += 10;
          if (n > 127)
            n = 127;

          AIL_set_sample_pan( smp[i], n );
          break;

        case '[': case '{':
          //
          // get the pan and then shift it left
          //

          n = AIL_sample_pan( smp[i] );
          n -= 10;
          if (n < 0)
            n = 0;

          AIL_set_sample_pan( smp[i], n );
          break;
  
        //
        // exit on escape
        //

        case 27:
          goto done;
      }
    }
  }

 done:

  //
  // Shut down AIL system and exit
  //

  AIL_quick_shutdown();

  printf( "\n\nSSTEST stopped.\n" );
}

