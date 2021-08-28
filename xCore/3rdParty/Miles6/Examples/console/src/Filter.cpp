//############################################################################
//##                                                                        ##
//##  FILTER.CPP: MSS pipeline filter test bed                              ##
//##                                                                        ##
//##  V1.00 of 23-Aug-98: Initial version                                   ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  For technical support, contact RAD Game Tools at 425-893-4300.        ##
//##                                                                        ##
//############################################################################

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "mss.h"

#include "con_util.c"

#define DIGITALCOUNT 16

#define HW_RATE     44100
#define HW_BITS     16
#define HW_CHANNELS 2

#define FILE2D MSS_DIR_UP "media" MSS_DIR_SEP "welcome.wav"


HDIGDRIVER Digital = NULL;

typedef char PREFERENCELIST[128][128];


//############################################################################
//##                                                                        ##
//## shutdown called from an atexit handler                                 ##
//##                                                                        ##
//############################################################################

static void MSS_MAIN_DEF Shutdown( void )
{
  if ( Digital != NULL)
  {
    AIL_close_digital_driver( Digital );
    Digital = 0;
  }

  AIL_shutdown();

  printf("\nFILTER stopped.\n");
}


//############################################################################
//##                                                                        ##
//## enumerate the filter-level and sample-level properties                 ##
//##                                                                        ##
//############################################################################

static U32 Enum_properties( HPROVIDER filter,
                            HSAMPLE last,
                            PREFERENCELIST * preferences )
{
  RIB_INTERFACE_ENTRY attrib;
  HINTENUM next;

  printf("\nFilter provider attributes:\n\n");

  //
  // Dump attributes of filter provider
  //

  next = HINTENUM_FIRST;

  while (AIL_enumerate_filter_attributes( filter,
                                          &next,
                                          &attrib ) )
  {
    S32 value;

    AIL_filter_attribute( filter,
                          attrib.entry_name,
                          &value );

    printf("   %s = %s\n",
            attrib.entry_name,
            RIB_type_string( value, attrib.subtype ) );
  }


  //
  // Dump now the sample level-attributes
  //

  printf( "_______________________________________________________________________________\n\n" );
  printf( "  Enter       : Start a 2D sound\n\n" );

  printf( "   ?          : Show filter properties\n\n" );
  printf( "  a/z         : Adjust sample property up/down (by 1)\n" );
  printf( "  Shift-a/z   : Adjust sample property up/down (by 100)\n" );
  printf( "  Control-a/z : Adjust sample property up/down (by 0.1)\n" );

  U32 preferencecount = 0;

  //
  // list all of the sample attributes
  //

  next = HINTENUM_FIRST;

  while (AIL_enumerate_filter_sample_attributes( filter,
                                                 &next,
                                                 &attrib ) )
  {
    //
    // save the attribute name
    //

    strcpy( (*preferences)[preferencecount], attrib.entry_name );

    ++preferencecount;

    if (last != NULL)
    {
      S32 value;

      //
      // print the current attribute values if they've played a sample
      //

      AIL_filter_sample_attribute( last,
                                   attrib.entry_name,
                                   &value );

      printf( "   %d          : Select \"%s\" (currently = %s)\n" ,
                  preferencecount,
                  attrib.entry_name,
                  RIB_type_string( value, attrib.subtype ) );
    }
    else
    {
      printf( "   %d          : Select \"%s\"\n",
                  preferencecount,
                  attrib.entry_name );
    }
  }

  if (preferencecount > 0)
  {
    printf( "\n" );
  }

  printf( "  SPACE       : Stop all sounds\n" );
  printf( "  ESC         : Exit\n" );
  printf( "_______________________________________________________________________________\n\n" );

  return( preferencecount );
}


//############################################################################
//##                                                                        ##
//## adjust the specified preference                                        ##
//##                                                                        ##
//############################################################################

static void Adjust_preference( HSAMPLE S, char const* preference, F32 adj )
{
  F32 value;

  if ( preference==0 )
  {
    printf( "\r           Choose a provider first.                     " );
    return;
  }

  if ( S==0 )
  {
    printf( "\r           Play a sound first.                          " );
    return;
  }

  //
  // get the current value
  //

  AIL_filter_sample_attribute( S, preference, &value );

  //
  // add in the adjustment
  //

  value += adj;

  //
  // now set the preference
  //

  AIL_set_filter_sample_preference( S, preference, &value );

  //
  // now get the new value
  //

  AIL_filter_sample_attribute( S, preference, &value );

  //
  // and print it
  //

  printf( "\r           %s = %.2f                     ", preference ,value );
}


//############################################################################
//##                                                                        ##
//## ask the user what filter they want to test                             ##
//##                                                                        ##
//############################################################################

static HPROVIDER Ask_user_for_filter()
{
  HPROVIDER avail[256];
  C8   FAR *name;
  U32       n = 0;

  printf("Available filter providers:\n\n");

  HPROENUM next = HPROENUM_FIRST;

  //
  // display each filter and save the provider numbers in "avail"
  //

  while (AIL_enumerate_filters( &next,
                                &avail[n],
                                &name ) )
  {
    printf("   %c: %s\n",'a' + n++,name);
  }

  //
  // wait for the user to choose a filter
  //

  while (1)
  {
    printf("\nEnter choice or ESC to exit: ");

    S32 index = getch();

    if (index == 27)
    {
      exit( 0 );
    }

    printf( "%c\n" ,index);

    if ((U32)(index - 'a') < n)
    {
      return( avail[index-'a'] );
    }
  }
}


//############################################################################
//##                                                                        ##
//## main                                                                   ##
//##                                                                        ##
//############################################################################

void MSS_MAIN_DEF main(S32 argc, C8 **argv)
{
  HPROVIDER      filter;
  HSAMPLE        S[DIGITALCOUNT];
  S32            nsamples;
  U32            nextupdate = 0;
  PREFERENCELIST preferences;

  argc=argc;
  argv=argv;

  set_up_console( 1 );

  printf( "_______________________________________________________________________________\n\n" );
  printf( "MSS Pipeline Filter Test Bed\n");
  printf( "_______________________________________________________________________________\n\n" );

  //
  // set the redist directory and startup the system
  //

  AIL_set_redist_directory( MSS_DIR_UP_TWO "redist" MSS_DIR_SEP MSS_REDIST_DIR_NAME );

  AIL_startup();

  atexit( Shutdown );


  //
  // open a digital driver
  //

  Digital = AIL_open_digital_driver( HW_RATE, HW_BITS, HW_CHANNELS, 0 );

  if (Digital == 0)
  {
    printf( "Error opeing output device.\n");
    exit( 1  );
  }


  //
  // Allocate handles to play normal 2D sound
  //

  for (nsamples=0; nsamples < DIGITALCOUNT; nsamples++)
  {
    S[nsamples] = AIL_allocate_sample_handle( Digital );

    if (S[nsamples] == NULL)
    {
      break;
    }
  }


  //
  // Offer provider selection menu to user
  //

  filter = Ask_user_for_filter();


  //
  // Show provider properties
  //

  S32 maxpreference=Enum_properties( filter, 0, &preferences );

  //
  // Load standard test sample file
  //

  void FAR *data = AIL_file_read( FILE2D, 0 );
  S32       size = AIL_file_size( FILE2D );

  if (data == NULL)
  {
    printf( "Couldn't load %s.\n", FILE2D );
    exit( 1 );
  }

  //
  // starting value
  //

  F32 value = 100.0F;
  HSAMPLE lastsample = 0;
  char* selectedpreference = 0;

  while (1)
  {
    //
    // Give other threads a time slice
    //  

    #ifdef IS_WIN32
    Sleep(10);
    #endif

    //
    // update CPU use
    //

    U32 curtime=AIL_ms_count();

    if (curtime>nextupdate)
    {
      printf( "\rCPU: %d%%", AIL_digital_CPU_percent( Digital ) );
      nextupdate=curtime+1000;
    }

    //
    // Poll keyboard
    //

    if (kbhit())
    {
      S32 i;
      char ch;

      ch = getch();

      switch (ch)
      {
        //
        // 1-9: Select preference to adjust
        //
        case '1': case '2': case '3': case '4': case '5': case '6':
        case '7': case '8': case '9':
          i = ch - '1';
          if (i < maxpreference)
          {
            //
            // update the selected preference
            //

            selectedpreference = preferences[i];
            printf( "\r           %s selected.                  ", selectedpreference);
          }
          break;

        //
        // ESC: Exit
        //
        case 27:
          exit(0);

        //
        // SPC: Stop all sounds
        //
        case ' ':
          for (i=0; i < nsamples; i++)
          {
            AIL_end_sample( S[i] );
          }
          break;

        //
        // ?: Dump provider properties
        //
        case '?':
          printf("  ");
          maxpreference = Enum_properties( filter, lastsample, &preferences );
          break;

        //
        // Enter: Trigger 2D sound
        //
        case 13:
          //
          // find a sample to play
          //

          for (i=0; i < nsamples; i++)
          {
            if (AIL_sample_status( S[i] ) == SMP_DONE)
            {
              AIL_init_sample( S[i] );

              AIL_set_named_sample_file( S[i], FILE2D, data, size, 0 );

              AIL_set_sample_loop_count( S[i], 0 );

              AIL_set_sample_volume( S[i], 127 );

              AIL_set_sample_processor( S[i], DP_FILTER, filter );

              AIL_start_sample( S[i] );

              lastsample= S[i];

              break;
            }
          }
          break;


        //
        // Raise/lower the selected preference
        //
        case 'a':
          Adjust_preference( lastsample, selectedpreference, 1.0F );
          break;

        case 'z':
          Adjust_preference( lastsample, selectedpreference, -1.0F );
          break;

        case 'A':
          Adjust_preference( lastsample, selectedpreference, 100.0F );
          break;

        case 'Z':
          Adjust_preference( lastsample, selectedpreference, -100.0F );
          break;

        case 1:
          Adjust_preference( lastsample, selectedpreference, 0.1F );
          break;

        case 26:
          Adjust_preference( lastsample, selectedpreference, -0.1F );
          break;

      }
    }
  }
}


