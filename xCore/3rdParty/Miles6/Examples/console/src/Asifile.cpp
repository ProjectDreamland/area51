//############################################################################
//##                                                                        ##
//##  ASIFILE.CPP                                                           ##
//##                                                                        ##
//##  V1.00 of 5-Jul-98: Initial version                                    ##
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
#include <string.h>

#include "mss.h"

#include "con_util.c"

#pragma pack(1)

struct WAVEOUT
{
  U32 riffmark;
  U32 rifflen;
  char wavemark[4];
  U32 fmtmark;
  U32 fmtlen;
  U16 fmttag;
  U16 channels;
  U32 sampersec;
  U32 avepersec;
  U16 blockalign;
  S16 bitspersam;
  U32 datamark;
  U32 datalen;
  C8  data[1];
};

#pragma pack()

// ---------------------------------------------------------------------------
// enum_attribs()
// ---------------------------------------------------------------------------

static void Enum_properties( HPROVIDER ASI,
                             ASI_STREAM_ATTRIBUTE ASI_stream_attribute,
                             HASISTREAM stream,
                             RIB_ENTRY_TYPE type )
{
  HINTENUM next;
  RIB_INTERFACE_ENTRY attrib;

  //
  // enumerate the properties
  //

  next = HINTENUM_FIRST;

  while (RIB_enumerate_interface( ASI,
                                  "ASI stream",
                                  type,
                                  &next,
                                  &attrib ) )
  {
    printf("   %s = %s\n",
               attrib.entry_name,
               RIB_type_string( ASI_stream_attribute( stream, attrib.token ),
               attrib.subtype) );
  }
}


// ---------------------------------------------------------------------------
// stream_CB()
// ---------------------------------------------------------------------------

static U8 FAR* Source=0;
static U32 SourceLength=0;
static U32 SourcePosition=0;

static S32 AILCALLBACK Stream_CB( U32       user,
                                  void FAR *dest,
                                  S32       bytesrequested,
                                  S32       offset )
{
  user=user;
  
  //
  // new position, if so, change the source position
  //
  
  if (offset != -1)
  {
    SourcePosition = offset;
  }

  //
  // make sure we don't read too much
  //

  if ((SourcePosition + bytesrequested) > SourceLength)
  {
    bytesrequested = SourceLength - SourcePosition;
  }

  //
  // copy the data to the destination
  //

  memcpy( dest, &Source[SourcePosition], bytesrequested );

  //
  // update our position
  //

  SourcePosition += bytesrequested;

  //
  // return how many bytes we actually copied
  //

  return bytesrequested;
}


// ---------------------------------------------------------------------------
// shutdown
// ---------------------------------------------------------------------------

static void MSS_MAIN_DEF Shutdown(void)
{
  AIL_shutdown();

  printf( "\nASIFILE stopped.\n" );
}


// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

void MSS_MAIN_DEF main( int argc, char** argv )
{
  HPROVIDER ASI;
  HASISTREAM stream;

  set_up_console( 0 );

  printf( "_______________________________________________________________________________\n\n" );
  printf( "ASI file test bed\n" );
  printf( "_______________________________________________________________________________\n\n" );

  if (argc < 2)
  {
    printf( "Usage asifile infile [outfile]\n\n" );
    printf( "If infile is a .WAV or .MP3 file, source parms will automatically be determined\n" );
    printf( "If infile is a .RAW file, ASIFILE.CPP must be edited to supply parameters\n" );

    printf( "\nEnter filenames: ");
    get_args( &argc, &argv );
  }

  //
  // set the redist directory and start up the system
  //

  AIL_set_redist_directory( MSS_DIR_UP_TWO "redist" MSS_DIR_SEP MSS_REDIST_DIR_NAME );


  AIL_startup();

  atexit( Shutdown );

  //
  // get the filenames to use
  //

  char infile[256];
  char outfile[256];

  strcpy( infile, argv[1] );
  strcpy( outfile, (argc==3)?argv[2]:"null.mp3" );

  printf( "Loading input file, please wait...\n" );

  //
  // Load source data
  //

  Source = (U8 FAR *) AIL_file_read( infile, 0 );

  if (Source == NULL)
  {
    printf( AIL_last_error() );
    exit( 1 );
  }

  WAVEOUT FAR *wav = (WAVEOUT FAR *) Source;

  //
  // Parameters to be used for raw file input...
  //

  S32 source_rate     = 44100;
  S32 source_bits     = 16;
  S32 source_channels = 2;

  if ( !memcmp( &wav->wavemark, "WAVE", 4 ))
  {
    //
    // this is a wave file

    SourceLength = wav->datalen;
    SourcePosition = (U8*)wav->data-(U8*)wav;

    if (wav->fmttag != WAVE_FORMAT_PCM)
    {
      printf( "Cannot process compressed wave files.\n" );
      exit( 1 );
    }

    source_rate     = wav->sampersec;
    source_bits     = wav->bitspersam;
    source_channels = wav->channels;

    printf( "Done.  Source identified as .WAV file, " );
    printf( "%d Hz %d-bit %s\n\n",
            source_rate,
            source_bits,
            (source_channels == 2) ? "stereo" : "mono" );

    //
    // Find ASI decoder provider which understands the input wave type
    //

    ASI = RIB_find_file_dec_provider( "ASI codec",
                                      "Input wave tag",
                                      wav->fmttag,
                                      "Output file types",
                                      outfile );

  }
  else
  {
    printf( "Done.  Source is assumed to be raw data file." );

    SourceLength = AIL_file_size(infile);
    SourcePosition = 0;

    //
    // Find ASI decoder provider which understands the suffixs
    //

    ASI = RIB_find_files_provider( "ASI codec",
                                   "Input file types",
                                   infile,
                                   "Output file types",
                                   outfile );


  }

  if (ASI == 0)
  {
    printf( "No provider available for specified output file type.\n" );
    exit( 1 );
  }

  //
  // function pointers to ASI functions
  //

  ASI_ERROR                 ASI_error;

  ASI_STREAM_OPEN           ASI_stream_open;
  ASI_STREAM_PROCESS        ASI_stream_process;
  ASI_STREAM_SEEK           ASI_stream_seek;
  ASI_STREAM_CLOSE          ASI_stream_close;
  ASI_STREAM_ATTRIBUTE      ASI_stream_attribute;
  ASI_STREAM_SET_PREFERENCE ASI_stream_set_preference;

  //
  // data values to ASI preferences
  //

  HATTRIB INPUT_BIT_RATE;
  HATTRIB INPUT_SAMPLE_RATE;
  HATTRIB INPUT_BITS;
  HATTRIB INPUT_CHANNELS;
  HATTRIB OUTPUT_BIT_RATE;
  HATTRIB OUTPUT_SAMPLE_RATE;
  HATTRIB OUTPUT_BITS;
  HATTRIB OUTPUT_CHANNELS;
  HATTRIB POSITION;
  HATTRIB PERCENT_DONE;
  HATTRIB MIN_INPUT_BLOCK_SIZE;
  HATTRIB RAW_RATE;
  HATTRIB RAW_BITS;
  HATTRIB RAW_CHANNELS;
  HATTRIB REQUESTED_BIT_RATE;
  HATTRIB REQUESTED_RATE;
  HATTRIB REQUESTED_BITS;
  HATTRIB REQUESTED_CHANS;
  HATTRIB REQUESTED_LAYER;

  //
  // structure used to lookup the codec functions
  //

  RIB_INTERFACE_ENTRY ASICODEC[] =
  {
    FN( ASI_error )
  };

  //
  // structure used to lookup the stream functions and the stream preferences
  //

  RIB_INTERFACE_ENTRY ASISTR[] =
  {
    FN( ASI_stream_attribute ),
    FN( ASI_stream_open ),
    FN( ASI_stream_seek ),
    FN( ASI_stream_close ),
    FN( ASI_stream_process ),
    FN( ASI_stream_set_preference ),

    AT( "Input bit rate",           INPUT_BIT_RATE ),
    AT( "Input sample rate",        INPUT_SAMPLE_RATE ),
    AT( "Input sample width",       INPUT_BITS ),
    AT( "Input channels",           INPUT_CHANNELS ),
    AT( "Output bit rate",          OUTPUT_BIT_RATE ),
    AT( "Output sample rate",       OUTPUT_SAMPLE_RATE ),
    AT( "Output sample width",      OUTPUT_BITS ),
    AT( "Output channels",          OUTPUT_CHANNELS ),
    AT( "Position",                 POSITION ),
    AT( "Percent done",             PERCENT_DONE ),
    AT( "Minimum input block size", MIN_INPUT_BLOCK_SIZE ),
    PR( "Raw source sample rate",   RAW_RATE ),
    PR( "Raw source sample width",  RAW_BITS ),
    PR( "Raw source channels",      RAW_CHANNELS ),
    PR( "Requested bit rate",       REQUESTED_BIT_RATE ),
    PR( "Requested sample rate",    REQUESTED_RATE ),
    PR( "Requested sample width",   REQUESTED_BITS ),
    PR( "Requested channels",       REQUESTED_CHANS ),
    PR( "Layer",                    REQUESTED_LAYER )
  };

  //
  // the function calls to do the actual lookups
  //

  RIB_request( ASI, "ASI codec", ASICODEC );
  RIB_request( ASI, "ASI stream", ASISTR );

  //
  // Open stream with codec, registering callback function
  //

  stream = ASI_stream_open( 0,
                            Stream_CB,
                            SourceLength );

  if (stream == NULL)
  {
    printf( "Could not open stream (%s)\n", ASI_error() );
  }

  //
  // Set stream parameters
  //

  S32 req_layer = 3;

  ASI_stream_set_preference( stream,
                             REQUESTED_LAYER,
                             &req_layer );

  ASI_stream_set_preference( stream,
                             RAW_RATE,
                             &source_rate );

  ASI_stream_set_preference( stream,
                             RAW_BITS,
                             &source_bits );

  ASI_stream_set_preference( stream,
                             RAW_CHANNELS,
                             &source_channels );

  //
  // open the output file (if one is specified)
  //

  FILE* file = 0;

  if (argc == 3)
  {
    file = fopen( outfile, "wb" );

    if (file == 0)
    {
      printf( "Could not open output file\n" );
      exit( 1 );
    }
  }

  S32 len;

  U8* buffer = (U8*)AIL_mem_alloc_lock(32768);
  if ( buffer == 0 )
  {
     printf( "Out of memory.\n" );
     exit( 1 );
  }

  printf( "Converting, please wait...\n" );
  U32 timer=AIL_ms_count();

  //
  // Keep reading the file until done
  //

  while ((len = ASI_stream_process( stream,
                                    buffer,
                                    sizeof( buffer ) ) ) != 0)
  {
    if (file != 0)
    {
      U32 nbytes;

      //
      // write the converted bytes
      //

      nbytes = fwrite( buffer, 1, len, file );
    
    }

    //
    // print the percentage if we are printing to a file
    //

    if (argc == 3)
    {
      if ((PERCENT_DONE != -1) &&
          (ASI_stream_attribute( stream, PERCENT_DONE ) != -1))
      {
        printf( "\r%s",
                RIB_type_string( ASI_stream_attribute( stream, PERCENT_DONE ),
                                          RIB_PERCENT ) );
      }
    }
  }

  //
  // close the file
  //

  if (file != 0)
  {
    fclose( file );
  }

  //
  // finished - display the total time
  //

  timer = AIL_ms_count()-timer;

  printf( "\n\nDone.  Total time converting (in milliseconds): %d.\n",timer );

  //
  // Show properties
  //

  Enum_properties( ASI, ASI_stream_attribute, stream, RIB_ATTRIBUTE );

  //
  // Exit w/success
  //

  ASI_stream_close( stream );

  if ( ASI_error() != NULL )
  {
    printf( "Error: %s\n", ASI_error() );
  }

  exit( 0 );
}
