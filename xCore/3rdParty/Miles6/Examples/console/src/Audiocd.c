//############################################################################
//##                                                                        ##
//##  AUDIOCD.C                                                             ##
//##                                                                        ##
//##  Red book audio API example program                                    ##
//##                                                                        ##
//##  V1.00 of 28-Jun-96: Initial                                           ##
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

#include "mss.h"

#include "con_util.c"

static U32 LastStatus=(U32)-1;
static S32 DoLoop=0;              // are we in loop mode?
static S32 SetLoopOncePlaying=0;  // should set loop mode once playing?
static U32 LoopStart,LoopEnd;
static S32 Volume;

//
// Function to update the status of redbook (and handle looping if turned on)
//

static void Show_status( HREDBOOK red )
{
  U32 pos, start, track, status;
  
  //
  // get the current status
  //

  status=AIL_redbook_status( red );

  //
  // has the status changed since the last time?
  //

  if (LastStatus!=status)
  {
    printf( "\r                                                                          " );
    LastStatus=status;
  }

  printf( "\rTracks: %i  Status: ", AIL_redbook_tracks( red ) );

  switch (status)
  {
    case REDBOOK_PLAYING:
      
      //
      // if we need to set the loop mode once play, do so
      //

      if (SetLoopOncePlaying)
      {
        DoLoop=1;
        SetLoopOncePlaying=0;
      }

      //
      // get the current track and the current track start
      //

      track=AIL_redbook_track( red );
      AIL_redbook_track_info( red, track, &start, 0 );


      //
      // get the current track position
      //

      pos=AIL_redbook_position( red );
      pos=(pos<start)?0:pos-start;

      //
      // display the playback status
      //

      printf( "Playing (track: %02i at %02i:%02i). %s",
              track, pos/60000, (pos/1000)%60, (DoLoop)?"Loop.":"" );
      break;

    case REDBOOK_STOPPED:

      //
      // if playback has stopped and we're looping, then restart the playback
      if (DoLoop)
      {
        //
        // mark the loop as performed and set the reset loop marker
        //   (so that we don't reloop if the CD takes a second to start playing)
        //

        DoLoop=0;
        SetLoopOncePlaying=1;
        
        //
        // restart the loop
        //

        AIL_redbook_play( red, LoopStart, LoopEnd) ;
        
        printf( "Looping." );
      }
      else
      {
        printf( "Stopped." );
      }
      break;

    case REDBOOK_PAUSED:
      printf( "Paused." );
      break;

    default:
      
      //
      // an error occurred (like no CD present)
      //

      DoLoop=0;
      SetLoopOncePlaying=0;
      printf( "Error." );
      break;
  }
}


void MSS_MAIN_DEF main( void)
{
  HREDBOOK red;

  set_up_console( 1 );

  printf( "Audio CD Player - Version " MSS_VERSION "   " MSS_COPYRIGHT " \n"
          "-------------------------------------------------------------------------------\n"
          "  Press '1' to '0' or 'A' to 'Z' to start a CD-audio track.\n"
          "  Press '*' to play all tracks.\n"
          "  Press '+' to loop whatever is currently playing.\n"
          "  Press '/' to fast forward.\n"
          "  Press '.' to stop the playback.\n"
          "  Press '<' to eject the cd out of the drive.\n"
          "  Press '>' to retract the cd into the drive.\n"
          "  Press ']' to increase the volume.\n"
          "  Press '[' to decrease the volume.\n"
          "  Press the space bar to pause and resume the playback.\n"
          "  Press escape to exit.\n\n" );

  AIL_startup();

  //
  // open the redbook handle
  //

  red=AIL_redbook_open( 0 );

  if ( red )
  {

    //
    // initial the next update counter
    //

    U32 nextupdate=0;

    //
    // get the initial volume
    //

    Volume=AIL_redbook_volume( red );

    while (1)
    {
      U32 start, end, pos;
      char c;

      //
      // loop until a key is hit
      //

      do
      {
        //
        // is it time for a status update
        //

        if (AIL_ms_count()>nextupdate)
        {
          //
          // check the status again in a second
          //

          nextupdate=AIL_ms_count()+1000;
          Show_status( red );
        }
      
      } while ( !kbhit() );

      //
      // get the character that was pressed

      c=getch();

      //
      // convert the CD track ranges all to A to Z
      //

      if ((c>='a') && (c<='z'))       // uppercase
        c-=('a'-'A');
      else if (c=='0')                // convert zero into ten
        c='A'+9;
      else if ((c>='1') && (c<='9'))  // handle one through nine
        c='A'+c-'1';

      //
      // did they want to switch tracks?
      //

      if ((c>='A') && (c<='Z'))       // play a track
      {

        //
        // get the track start and stop
        //

        AIL_redbook_track_info( red, c-'A'+1, &start, &end );

        //
        // now play the track
        //

        AIL_redbook_play( red, start, end );

      }
      else
      {
        switch (c)
        {

          //
          // handle the cd stop command
          //
          case '.':
            DoLoop=0;
            SetLoopOncePlaying=0;
            AIL_redbook_stop( red );
            break;

          //
          // handle the cd eject command
          //
          case '<':
            AIL_redbook_eject( red );
            break;

          //
          // handle the cd retract command
          //
          case '>':
            AIL_redbook_retract( red );
            break;

          //
          // handle the loop current command
          //
          case '+':
            if ( AIL_redbook_status( red )==REDBOOK_PLAYING )
            {
              //
              // turn on or off looping
              //

              DoLoop=!DoLoop;

              if (DoLoop)
              {
                //
                // if turning on looping, save the current start and stop points
                //

                SetLoopOncePlaying=0;
                LoopStart=start;
                LoopEnd=end;
              }

              //
              // mark the status as changed
              //

              LastStatus=(U32)-1;
            }
            break;

          //
          // handle the cd pause command
          //
          case ' ':
            if (AIL_redbook_status( red )==REDBOOK_PLAYING)
            {
              //
              // we're already playing, so pause
              //

              AIL_redbook_pause( red );
            }
            else
            {
              //
              // we're stopped, so unpause
              //

              AIL_redbook_resume( red );
            }
            break;

          //
          // handle the cd fast foreward command
          //
          case '/':

            //
            // are we playing?
            //

            if ( AIL_redbook_status( red )==REDBOOK_PLAYING)
            {
              //
              // get the position in 10 seconds
              //

              pos=AIL_redbook_position( red )+10000;

              if (pos>=end)
              {
                //
                // did we skip past the end, if so stop.
                //

                AIL_redbook_stop( red );
              }
              else
              {
                //
                // Otherwise, move the play cursor
                //

                AIL_redbook_play( red, pos, end );
              }
            }
            break;

          //
          // handle the cd volume up command
          //
          case ']':
            if (Volume<127)
              AIL_redbook_set_volume( red, ++Volume );
            break;

          //
          // handle the cd volume down command
          //
          case '[':
            if (Volume>0)
              AIL_redbook_set_volume( red, --Volume );
            break;

          //
          // handle the play all tracks command
          //
          case '*':
            //
            // get the start of the first track
            //

            AIL_redbook_track_info( red, 1, &start, 0 );

            //
            // get the end of the final track
            //

            AIL_redbook_track_info( red, AIL_redbook_tracks( red ), 0, &end );

            //
            // now play the entire CD
            //

            AIL_redbook_play( red, start, end );
            break;

          //
          // handle the exit command
          //
          case 27:
            goto done;
        }
      }
    }

   done:

    printf( "\n" );

    //
    // stop and close the CD device
    //

    AIL_redbook_stop( red );
    AIL_redbook_close( red );
  }
  else
  {
    printf( "No CD devices found.\n" );
  }

  //
  // shutdown Miles
  //

  AIL_shutdown();
}
