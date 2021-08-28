//############################################################################
//##                                                                        ##
//##  CON3D.CPP                                                             ##
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

#include <stdio.h>
#include <stdlib.h>

#include "mss.h"

#include "con_util.c"

#define HW_FORMAT  DIG_F_STEREO_16
#define HW_RATE    44100

#define FILE2D MSS_DIR_UP "media" MSS_DIR_SEP "Welcome.wav"
#define FILE3D MSS_DIR_UP "media" MSS_DIR_SEP "Shot.wav"

//
// AIL handles
//

HDIGDRIVER dig = NULL;

HSAMPLE normal_sample;

HPROVIDER provider = NULL;

H3DSAMPLE h3D[16];
S32 n_3D = 0;

HSAMPLE h2D[16];
S32 n_2D = 0;

H3DSAMPLE last_3D = NULL;
HSAMPLE   last_2D = NULL;

// ---------------------------------------------------------------------------
// shutdown
// ---------------------------------------------------------------------------

static void MSS_MAIN_DEF shutdown(void)
{
   S32 i;

   for (i=0; i < n_2D; i++)
   {
      AIL_end_sample(h2D[i]);
   }

   for (i=0; i < n_3D; i++)
   {
      AIL_end_3D_sample(h3D[i]);
   }

   if (provider)
   {
      AIL_close_3D_provider(provider);
      provider = NULL;
   }

   if (dig != NULL)
   {
      AIL_close_digital_driver(dig);
      dig = NULL;
   }

   AIL_shutdown();

   printf("\nCON3D stopped.\n");
}

// ---------------------------------------------------------------------------
// enum_attribs()
// ---------------------------------------------------------------------------

static void enum_properties(void)
{
   printf("\nProvider attributes:\n\n");

   HINTENUM next = HINTENUM_FIRST;

   RIB_INTERFACE_ENTRY attrib;

   while (AIL_enumerate_3D_provider_attributes(provider,
                                              &next,
                                              &attrib))
   {
      S32 value;

      AIL_3D_provider_attribute(provider,
                                attrib.entry_name,
                               &value);
      printf("   %s = %s\n",
             attrib.entry_name,
             RIB_type_string(value, attrib.subtype));
   }

   if (last_3D != NULL)
   {
      printf("\nSample attributes:\n\n");

      HINTENUM next = HINTENUM_FIRST;

      RIB_INTERFACE_ENTRY attrib;

      while (AIL_enumerate_3D_sample_attributes(provider,
                                             &next,
                                             &attrib))
      {
         S32 value;

         AIL_3D_sample_attribute(last_3D,
                                 attrib.entry_name,
                                &value);

         printf("   %s = %s\n",
               attrib.entry_name,
               RIB_type_string(value, attrib.subtype));
      }
   }
}

//############################################################################
//##                                                                        ##
//## Load a sample file into memory (decompress it if necessary)            ##
//##                                                                        ##
//############################################################################

static void* load_sample_file(char* name,U32* size)
{
   U32* s;
   void* d;
   S32 type;
   AILSOUNDINFO info;

   s=(U32*)AIL_file_read(name,FILE_READ_WITH_SIZE);

   if (s==0)
     return(0);

   type=AIL_file_type(s+1,s[0]);

   switch (type) 
   {
     case AILFILETYPE_PCM_WAV:
       *size=s[0];
       return(s+1);

     case AILFILETYPE_ADPCM_WAV:
       AIL_WAV_info(s+1,&info);
       AIL_decompress_ADPCM(&info,&d,size);
       AIL_mem_free_lock(s);
       return(d);

     default:
       AIL_mem_free_lock(s);
       return(0);
   }
}


static void Update_3D_position( H3DSAMPLE sample, F32 dx, F32 dy, F32 dz )
{
  if ( sample == NULL)
  {
    return;
  }

  F32 X,Y,Z;

  AIL_3D_position( sample, &X, &Y, &Z );

  X += dx; Y += dy; Z += dz;

  AIL_set_3D_position( sample, X, Y, Z );

  printf("\r    %d,%d,%d      ", (S32)(X), (S32)(Y), (S32)(Z));
}


static void Update_3D_volume( H3DSAMPLE sample, S32 dv )
{
  if ( sample == NULL )
  {
    return;
  }

  S32 i = AIL_3D_sample_volume( sample );

  i += dv;

  if ( i < 0 )
    i = 0;
  else if ( i > 127 )
    i=127;

  AIL_set_3D_sample_volume( sample, i );
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

void MSS_MAIN_DEF main(S32 argc, C8 **argv)
{
   S32 i;

   argc=argc;
   argv=argv;

   set_up_console( 1 );

   printf("_______________________________________________________________________________\n\n");
   printf("MSS 3D Test Bed\n");
   printf("_______________________________________________________________________________\n\n");

   //
   // set the redist directory
   //

   AIL_set_redist_directory( MSS_DIR_UP_TWO "redist" MSS_DIR_SEP MSS_REDIST_DIR_NAME );

   AIL_startup();
   atexit(shutdown);

   dig = NULL;

   dig = AIL_open_digital_driver( HW_RATE,
                                  (HW_FORMAT & DIG_F_16BITS_MASK) ? 16 : 8,
                                  (HW_FORMAT & DIG_F_16BITS_MASK) ? 2 : 1,
                                  0 );


   //
   // Allocate handles to play normal 2D sound
   //

   for (n_2D=0; n_2D < 16; n_2D++)
   {
      h2D[n_2D] = AIL_allocate_sample_handle(dig);

      if (h2D[n_2D] == NULL)
      {
         break;
      }
   }

   //
   // Offer provider selection menu to user
   //

   printf("Available 3D audio providers:\n\n");

   HPROVIDER avail[256];
   C8   FAR *name;
   U32       n = 0;

   HPROENUM next = HPROENUM_FIRST;

   while (AIL_enumerate_3D_providers(&next,
                                     &avail[n],
                                     &name))

   {
      printf("   %c: %s\n",(++n)+64,name);
   }

   printf("\nEnter choice or ESC to exit: ");

   S32 index = getch();

   if ( ( index >= 'a' ) && ( index <= 'z' ) )
     index = index - 'a' + 'A';

   printf("%c\n",index);

   if ((U32)(index - 'A') >= n)
   {
      exit(0);
   }

   //
   // Activate provider
   //

   provider = avail[index-'A'];

   S32 result = AIL_open_3D_provider(provider);

   if (result != M3D_NOERR)
   {
      printf("Error opening specified provider, code %d\n",result);
      exit(1);
   }

   enum_properties();

   //
   // Allocate handles to play 3D sound
   //

   for (n_3D=0; n_3D < 16; n_3D++)
   {
      h3D[n_3D] = AIL_allocate_3D_sample_handle(provider);

      if (h3D[n_3D] == NULL)
      {
         break;
      }
   }

   //
   // Load standard test sample file
   //

   void FAR *w1 = AIL_file_read(FILE2D,FILE_READ_WITH_SIZE);

   if (w1 == NULL)
   {
      printf("Couldn't load "FILE2D"\n");
      exit(1);
   }

   // the size is store in the first dword of the pointer address
   U32       l1 = *((U32*)w1);
   w1           =  ((U32*)w1)+1;


   U32       l2;
   void FAR *w2 = load_sample_file(FILE3D,&l2);

   if (w2 == NULL)
   {
      printf("Couldn't load "FILE3D"\n");
      exit(1);
   }

   printf("_______________________________________________________________________________\n\n");
   printf("S     : Start a 3D sound\n");
   printf("D     : Start a 2D sound\n\n");

   printf("I/K   : 3D sound forward/back\n");
   printf("J/L   : 3D sound left/right\n");
   printf("Y/H   : 3D sound elevation\n");

   printf("-/+   : 3D sound volume\n");

   printf("P/R   : 3D sound pause/resume\n\n");

   printf(" ?    : Show provider properties\n\n");

   printf("B/Q   : Bathroom / rock quarry environment (Providers with reverb only)\n");
   printf("1/9   : 0.1 / 0.9 effect volume            (Providers with reverb only)\n");

   printf("SPACE : Stop all sounds\n");
   printf("ESC   : Exit\n");
   printf("_______________________________________________________________________________\n\n");

   //
   // Get listener handle and position it at (0,0,0), looking straight
   // ahead in positive Z
   //

   H3DPOBJECT listener = AIL_open_3D_listener(provider);

   AIL_set_3D_position(listener,
                       0.0F,
                       0.0F,
                       0.0F);

   AIL_set_3D_orientation(listener,
                          0.0F, 0.0F, 1.0F,
                          0.0F, 1.0F, 0.0F);

   while (1)
   {
      //
      // Give other threads a time slice
      //

      #ifdef IS_WIN32
      Sleep(10);
      #endif

      //
      // Poll keyboard
      //

      if (kbhit())
      {
         S32 ch = getch();

         switch (ch)
         {
            //
            // ESC: Exit
            //

            case 27:
               exit(0);

            //
            // Arrows: change sound distance/azimuth
            //
            // We assume a left-handed coordinate system is in use, with
            // positive X to the right, positive Z ahead, and positive Y up
            //

            case 'i': case 'I':
              Update_3D_position( last_3D, 0.0F, 0.0F, 3.0F );
              break;

            case 'k': case 'K':
              Update_3D_position( last_3D, 0.0F, 0.0F, -3.0F );
              break;

            case 'j': case 'J':
              Update_3D_position( last_3D, -3.0F, 0.0F, 0.0F );
              break;

            case 'l': case 'L':
              Update_3D_position( last_3D, 3.0F, 0.0F, 0.0F );
              break;

            case 'y': case 'Y':
              Update_3D_position( last_3D, 0.0F, 3.0F, 0.0F );
              break;

            case 'h': case 'H':
              Update_3D_position( last_3D, 0.0F, -3.0F, 0.0F );
              break;


            //
            // -/+: Volume
            //

            case '-': case '_':
               Update_3D_volume( last_3D, -5 );
               break;

            case '=': case '+':
               Update_3D_volume( last_3D, 5 );
               break;

            //
            // P/R: Pause/resume
            //

            case 'p': case 'P':

               if (last_3D != NULL)
               {
                  AIL_stop_3D_sample(last_3D);
               }
               break;

            case 'r': case 'R':

               if (last_3D != NULL)
               {
                  AIL_resume_3D_sample(last_3D);
               }
               break;

            //
            // Test EAX controls
            //

            case 'b': case 'B':
               AIL_set_3D_room_type         (provider,
                                             ENVIRONMENT_BATHROOM);
              break;

            case 'q': case 'Q':
               AIL_set_3D_room_type         (provider,
                                             ENVIRONMENT_QUARRY);
               break;

            case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '0':
               
               if (last_3D != NULL)
               {
                 AIL_set_3D_sample_effects_level( last_3D, ( (F32) (ch-'0') ) / 9.0F );
               }
               break;

            //
            // SPC: Stop all sounds
            //

            case ' ':

               for (i=0; i < n_2D; i++)
               {
                  AIL_end_sample( h2D[i] );
               }

               for (i=0; i < n_3D; i++)
               {
                  AIL_end_3D_sample( h3D[i] );
               }
               break;

            //
            // ?: Dump provider properties
            //

            case '?':

               enum_properties();
               break;

            //
            // 2: Trigger 2D sound
            //

             case 'd': case 'D':

               for (i=0; i < n_2D; i++)
               {
                  if (AIL_sample_status(h2D[i]) == SMP_DONE)
                  {
                     break;
                  }
               }

               if (i == n_2D)
               {
                  break;
               }

               AIL_init_sample(h2D[i]);

               AIL_set_named_sample_file(h2D[i],
                                         FILE2D,
                                         w1,
                                         l1,
                                         0);

               AIL_set_sample_loop_count(h2D[i],
                                         0);

               AIL_set_sample_volume(h2D[i],
                                     127);

               AIL_start_sample(h2D[i]);
               last_2D = h2D[i];
               break;

            case 's': case 'S':

               for (i=0; i < n_3D; i++)
               {
                  if (AIL_3D_sample_status(h3D[i]) == SMP_DONE)
                  {
                     break;
                  }
               }

               if (i == n_3D)
               {
                  break;
               }

               AIL_set_3D_sample_file(h3D[i],
                                      w2);


               AIL_set_3D_sample_distances(h3D[i],
                                           2048.0,
                                           16.0);

               AIL_set_3D_position(h3D[i],
                                   0.0F,
                                   0.0F,
                                   0.0F);

               AIL_set_3D_orientation(h3D[i],
                                      0.0F, 0.0F, 1.0F,
                                      0.0F, 1.0F, 0.0F);

               AIL_set_3D_sample_loop_count(h3D[i],
                                            0);

               AIL_start_3D_sample(h3D[i]);

               last_3D = h3D[i];
               break;

         }
      }
   }
}
