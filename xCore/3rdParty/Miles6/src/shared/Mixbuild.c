#include <stdlib.h>
#include <stdio.h>

#include <mss.h>

#define M_DEST_STEREO   1
#define M_SRC_16        2
#define M_FILTER        4
#define M_SRC_STEREO    8
#define M_VOL_SCALING   16
#define M_RESAMPLE      32
#define M_ORDER         64
#define M_UP_FILTER     128

FILE* file;

static char* Describe_operation( int op )
{
  static char output[256];
  sprintf( output,
           "%s%s%s%s%s",
           ( op & M_DEST_STEREO ) ? "_DestStereo" : "_DestMono",
           ( op & M_SRC_STEREO  ) ? ( ( op & M_ORDER ) ? "_SrcFlipped" : "_SrcStereo" ) : "_SrcMono",
           ( op & M_SRC_16      ) ? "_Src16" : "_Src8",
           ( op & M_VOL_SCALING ) ? "_Volume" : "_NoVolume",
           ( op & M_RESAMPLE    ) ? ( ( op & M_FILTER ) ? ( ( op & M_UP_FILTER ) ? "_UpFiltered" : "_DownFiltered" ) : "_Resample" ) : "_NoResample" );

  return( output );
}

static void Start_routine( int op )
{
  fprintf( file,
           "Merge%s PROC ; %i\n", Describe_operation( op ), op);

  if ( ( ! ( ( op & M_FILTER ) && ( ! ( op & M_UP_FILTER ) ) ) ) && ( op & M_RESAMPLE ) )
  {
    fprintf( file,
             "\n  ; adjust fractional\n"
             "  shl   edx, 16\n" );
  }

  if ( ! ( op & M_FILTER ) || ( op & M_UP_FILTER ) )
  {
    if ( ( ! ( op & M_SRC_16 ) ) && ( op & M_VOL_SCALING ) )
    {

      fprintf( file,
               "\n  ; incorporate 8 to 16 upscale into volume\n"
               "  shl  [scale_left], 8\n" );

      if ( ( op & M_DEST_STEREO ) || ( op & M_SRC_STEREO ) )
      {
        fprintf( file,
                 "  shl   [scale_right], 8\n" );
      }
    }
    
    if ( ( op & M_FILTER ) && ( op & M_SRC_STEREO )  && ( ( op & M_DEST_STEREO ) || ( op & M_VOL_SCALING ) ) )
    {
      fprintf( file,
               "\n  ; Save registers\n"
               "  push  ebp\n" );
    }
  
  }
  else
  {
    fprintf( file,
             "\n  ; check to see if we have to call the upsampling version\n"
             "  cmp   [playback_ratio], 10000h\n"
             "  jle   Merge%s\n", Describe_operation( op | M_UP_FILTER ) );

    if ( ( op & M_FILTER ) &&
         ( ! ( op & M_UP_FILTER ) ) &&
         (
           ( ( op & M_SRC_STEREO )  &&
             (
               ( op & M_DEST_STEREO ) || ( op & M_VOL_SCALING )
             )
           )
           ||
           ( ( op & M_DEST_STEREO ) && ( op & M_VOL_SCALING ) )
         )
       )
    {
      fprintf( file,
               "\n  ; build average dividers\n"
               "  mov   ecx, edx\n"
               "  mov   ebx, [playback_ratio]\n");

      if ( op & M_VOL_SCALING )
      {
        if ( ! ( op & M_SRC_16 ) )
        {
          fprintf( file,
                   "  xor   edx, edx\n"
                   "  mov   eax, 65536*256  ; 100000000f/2048/32*256\n"
                   "  imul  [scale_left]\n" );
        }
        else
        {
          fprintf( file,
                   "  xor   edx, edx\n"
                   "  mov   eax, 65536  ; 100000000f/2048/32\n"
                   "  imul  [scale_left]\n" );
        }
      }
      else
      {
        if ( ! ( op & M_SRC_16 ) )
        {
          fprintf( file,
                   "  mov   edx, 8\n"
                   "  xor   eax, eax  ; 100000000f/32*256\n" );
        }
        else
        {
          fprintf( file,
                   "  xor   edx, edx\n"
                   "  mov   eax, 08000000h  ; 100000000f/32\n" );
        }
      }

      fprintf( file,
               "  div   ebx\n"
               "  mov   [divider_l], eax\n" );

      if ( op & M_VOL_SCALING )
      {
        if ( ! ( op & M_SRC_16 ) )
        {
          fprintf( file,
                   "  xor   edx, edx\n"
                   "  mov   eax, 65536*256  ; 100000000f/2048/32*256\n"
                   "  imul  [scale_right]\n" );
        }
        else
        {
          fprintf( file,
                   "  xor   edx, edx\n"
                   "  mov   eax, 65536  ; 100000000f/2048/32\n"
                   "  imul  [scale_right]\n" );
        }
      }
      else
      {
        if ( ! ( op & M_SRC_16 ) )
        {
          fprintf( file,
                   "  mov   edx, 8\n"
                   "  xor   eax, eax  ; 100000000f/32*256\n" );
        }
        else
        {
          fprintf( file,
                   "  xor   edx, edx\n"
                   "  mov   eax, 08000000h  ; 100000000f/32\n" );
        }
      }

      fprintf( file,
               "  div   ebx\n"
               "  mov   [divider_r], eax\n" );
    }
    else
    {
      fprintf( file,
               "\n  ; build average dividers\n"
               "  mov   ecx, edx\n"
               "  mov   ebx, [playback_ratio]\n" );

      if ( op & M_VOL_SCALING )
      {
        if ( ! ( op & M_SRC_16 ) )
        {
          fprintf( file,
                   "  xor   edx, edx\n"
                   "  mov   eax, 65536*256  ; 100000000f/2048/32*256\n"
                   "  imul  [scale_left]\n" );
        }
        else
        {
          fprintf( file,
                   "  xor   edx, edx\n"
                   "  mov   eax, 65536  ; 100000000f/2048/32\n"
                   "  imul  [scale_left]\n" );
        }
      }
      else
      {
        if ( ! ( op & M_SRC_16 ) )
        {
          fprintf( file,
                   "  mov   edx, 8\n"
                   "  xor   eax, eax  ; 100000000f/32*256\n" );
        }
        else
        {
          fprintf( file,
                   "  xor   edx, edx\n"
                   "  mov   eax, 08000000h  ; 100000000f/32\n" );
        }
      }

      fprintf( file,
               "  div   ebx\n"
               "  mov   [divider_l], eax\n" );
    }

    fprintf( file,
             "  mov   edx, ecx\n" );

    fprintf( file,
             "\n  ; load initial sample\n"
             "  mov   ecx, [cur_l]\n" );

    if ( ( op & M_FILTER ) && ( op & M_SRC_STEREO )  && ( ( op & M_DEST_STEREO ) || ( op & M_VOL_SCALING ) ) )
    {
      fprintf( file,
               "\n  ; Save registers\n"
               "  push  ebp\n" );
    }

    fprintf( file,
             "\n  ; handle start up loop management\n"
             "  mov  eax, edx\n"
             "  and  edx, 03fffffffh\n"
             "  test eax, 080000000h\n"
             "  jnz  whole_continue\n"
             "  test eax, 040000000h\n"
             "  jnz  last_continue\n" );
  }

}

static void Load_sample( int op )
{
  fprintf( file,
           "\n  ; Load sample data\n" );

  if ( op & M_SRC_STEREO )
  {
    if ( op & M_SRC_16 )
    {
      if ( op & M_ORDER )
      {
        fprintf( file,
                 "  mov   eax, dword ptr [esi]\n"
                 "  movsx ebx, ax\n"
                 "  sar   eax, 16\n" );
      }
      else
      {
        fprintf( file,
                 "  mov   ebx, dword ptr [esi]\n"
                 "  movsx eax, bx\n"
                 "  sar   ebx, 16\n" );
      }
    }
    else
    {
      if ( op & M_ORDER )
      {
        fprintf( file, 
                 "  xor   eax, eax\n"
                 "  xor   ebx, ebx\n"
                 "  mov   bx, word ptr [esi]\n"
                 "  mov   al, bh\n"
                 "  and   ebx, 0ffh\n"
                 "  sub   eax, 080h\n"
                 "  sub   ebx, 080h\n" );
      }
      else
      {
        fprintf( file,
                 "  xor   ebx, ebx\n"
                 "  xor   eax, eax\n"
                 "  mov   ax, word ptr [esi]\n"
                 "  mov   bl, ah\n"
                 "  and   eax, 0ffh\n"
                 "  sub   ebx, 080h\n"
                 "  sub   eax, 080h\n" );
      }

    }
  }
  else
  {
    if ( op & M_SRC_16 )
    {
      fprintf( file, 
               "  movsx eax, word ptr [esi]\n" );
    }
    else
    {
      fprintf( file, 
               "  xor   eax, eax\n"
               "  mov   al, byte ptr [esi]\n"
               "  sub   eax, 080h\n" );
    }
  }

  // if dest mono and src stereo, then tie the channels together
  if ( ( ! ( op & M_DEST_STEREO ) ) && ( op & M_SRC_STEREO ) && ( ! ( op & M_VOL_SCALING ) ) )
  {
    fprintf( file,
             "\n  ; Merge left and right channels for mono dest\n"
             "  add   eax, ebx\n" );
    
    op &= ~M_SRC_STEREO;
  }

  if ( op & M_UP_FILTER )
  {
    if ( ( op & M_SRC_STEREO ) && ( ( op & M_DEST_STEREO ) || ( op & M_VOL_SCALING ) ) )
    {
      fprintf( file,
               "\n  ; rotate filter values\n"
               "  mov   ecx, [cur_l]\n"
               "  mov   [cur_l], eax\n"
               "  mov   eax, [cur_r]\n"
               "  mov   [cur_r], ebx\n"
               "  mov   [cur_r2], eax\n" );
    }
    else
    {
      fprintf( file, 
               "\n  ; rotate filter values\n"
               "  mov   ecx, [cur_l]\n"
               "  mov   [cur_l], eax\n" );
    }
  }
}


static void Apply_volume( int op )
{
  // filtering does the volume in the filter, so don't do it again
  if ( ! ( op & M_FILTER ) )
  {

    if ( ( ! ( op & M_DEST_STEREO ) ) && ( op & M_SRC_STEREO ) && ( ! ( op & M_VOL_SCALING ) ) )
    {
      op &= ~M_SRC_STEREO;
    }

    if ( ( op & M_DEST_STEREO ) && ( ! ( op & M_SRC_STEREO ) ) && ( op & M_VOL_SCALING ) )
    {
      fprintf( file, 
               "\n  ; Duplicate the left channel into the right\n"
               "  mov   ebx, eax\n" );
      
      op |= M_SRC_STEREO;
    }

    fprintf( file, 
             "\n  ; Apply volume\n");

    if ( op & M_VOL_SCALING )
    {
      fprintf( file, 
               "  imul  eax, [scale_left]\n");

      if ( op & M_SRC_STEREO )
      {
        fprintf( file, 
                 "  imul  ebx, [scale_right]\n");
      }
    }
    else
    {
      fprintf( file, 
               "  shl   eax, %i\n", ( op & M_SRC_16 ) ? 11 : 19 );

      if ( op & M_SRC_STEREO )
      {
        fprintf( file,
                 "  shl   ebx, %i\n", ( op & M_SRC_16 ) ? 11 : 19 );
      }
    }

    // if dest mono and src stereo, then tie the channels together
    if ( ( ! ( op & M_DEST_STEREO ) ) && ( op & M_SRC_STEREO ) )
    {
      fprintf( file,
               "\n  ; Merge left and right channels for mono dest\n"
               "  add   eax, ebx\n" );
    }
  }

}

static void Output_add( char * reg, int size )
{
  if ( size < 0 )
  {
    if ( size == -1 )
    {
      fprintf( file, 
               "  dec   %s\n", reg );
    }
    else
    {
      fprintf( file, 
               "  sub   %s, %i\n", reg, -size );
    }
  }
  else
  {
    if ( size == 1 )
    {
      fprintf( file, 
               "  inc   %s\n", reg );
    }
    else
    {
      fprintf( file, 
               "  add   %s, %i\n", reg, size );
    }
  }
}


static void Start_loop( int op )
{
  int size = ( ( op & M_SRC_16 ) ? 2 : 1 ) * ( ( op & M_SRC_STEREO ) ? 2 : 1 );

  fprintf( file, 
           "\n  ; Merge sample data loop\n"
           "  ALIGN 16\n"
           "  merge_loop:\n" );
}


static void Filter_with_volume( int op )
{
  if ( op & M_UP_FILTER )
  {
    fprintf( file, 
             "\n  ; Upsample the data points\n" );

    if ( ( op & M_SRC_STEREO ) && ( ( op & M_DEST_STEREO ) || ( op & M_VOL_SCALING ) ) )
    {
      fprintf( file, 
               "  mov   ebp, edx\n"
               "  mov   eax, [cur_l]\n"
               "  mov   ebx, [cur_r]\n"
               "  shr   ebp, 16\n"
               "  sub   eax, ecx\n"
               "  sub   ebx, [cur_r2]\n"
               "  imul  eax, ebp\n"
               "  imul  ebx, ebp\n"
               "  sar   eax, 16\n"
               "  sar   ebx, 16\n"
               "  add   eax, ecx\n"
               "  add   ebx, [cur_r2]\n" );
    }
    else
    {
      fprintf( file, 
               "  mov   ebx, edx\n"
               "  mov   eax, [cur_l]\n"
               "  shr   ebx, 16\n"
               "  sub   eax, ecx\n"
               "  imul  eax, ebx\n"
               "  sar   eax, 16\n"
               "  add   eax, ecx\n" );
    }

    Apply_volume( op & ~ M_FILTER );

  }
  else if ( op & M_FILTER )
  {
    int size = ( ( op & M_SRC_16 ) ? 2 : 1 ) * ( ( op & M_SRC_STEREO ) ? 2 : 1 );

    if ( ( op & M_SRC_STEREO ) && ( ( op & M_DEST_STEREO ) || ( op & M_VOL_SCALING ) ) )
    {
      fprintf( file, 
               "  mov   ebx, 65536\n"
               "  sub   ebx, edx\n"
               "  add   edx, [playback_ratio]\n"
               "\n  ; weight the initial sample\n"
               "  mov   eax, [cur_r]\n"
               "  imul  ecx, ebx\n"
               "  imul  eax, ebx\n"
               "  sar   ecx, 16\n"
               "  sar   eax, 16\n"
               "  cmp   edx, 65536*2\n"
               "  mov   [cur_r], eax\n"
               "  jl    skip_loop\n"
               "\n  ; loop to load all of the full sample points\n"
               "  whole_loop:\n"
               "  cmp   esi, [src_end]\n"
               "  jae   src_whole_exit\n"
               "  whole_continue:\n" );

      Load_sample( op );

      fprintf( file,
               "  add   ecx, eax\n"
               "  add   [cur_r], ebx\n"
               "  sub   edx, 65536\n" );

      Output_add( "esi", size );

      fprintf( file, 
               "  cmp   edx, 65536*2\n"
               "  jae   whole_loop\n"
               "\n  skip_loop:\n"
               "  and   edx, 0ffffh\n"
               "  cmp   esi, [src_end]\n"
               "  jae   src_last_exit\n"
               "  last_continue:\n" );

      Load_sample( op );

      Output_add( "esi", size );

      fprintf( file, 
               "  mov   ebp, ecx\n"
               "  mov   ecx, eax\n"
               "\n  ; weight the final sample\n"
               "  imul  eax, edx\n"
               "  sar   eax, 16\n"
               "  add   eax, ebp\n"
               "  mov   ebp, [cur_r]\n"
               "  mov   [cur_r], ebx\n"
               "  imul  ebx, edx\n"
               "  sar   ebx, 16\n"
               "  add   ebx, ebp\n"
               "  imul  eax, [divider_l]\n"
               "  imul  ebx, [divider_r]\n" );

      // if dest mono and src stereo, then tie the channels together
      if ( ( ! ( op & M_DEST_STEREO ) ) && ( op & M_SRC_STEREO ) )
      {
        fprintf( file, 
                 "\n  ; Merge left and right channels for mono dest\n"
                 "  add   eax, ebx\n" );
      }
    }
    else
    {
      fprintf( file, 
               "  mov   ebx, 65536\n"
               "  sub   ebx, edx\n"
               "  add   edx, [playback_ratio]\n"
               "\n  ; weight the initial sample\n"
               "  imul  ecx, ebx\n"
               "  sar   ecx, 16\n"
               "  cmp   edx, 65536*2\n"
               "  jl    skip_loop\n"
               "\n  ; loop to load all of the full sample points\n"
               "  whole_loop:\n"
               "  cmp   esi, [src_end]\n"
               "  jae   src_whole_exit\n"
               "  whole_continue:\n" );

      Load_sample( op );

      fprintf( file,
               "  add   ecx, eax\n"
               "  sub   edx, 65536\n" );

      Output_add( "esi", size );

      fprintf( file, 
               "  cmp   edx, 65536*2\n"
               "  jae   whole_loop\n"
               "\n  skip_loop:\n"
               "  and   edx, 0ffffh\n"
               "  last_continue:\n"
               "  cmp   esi, [src_end]\n"
               "  jae   src_last_exit\n" );

      Load_sample( op );

      Output_add( "esi", size );

      fprintf( file, 
               "  mov   ebx, ecx\n"
               "  mov   ecx, eax\n"
               "\n  ; weight the final sample\n"
               "  imul  eax, edx\n"
               "  sar   eax, 16\n"
               "  add   eax, ebx\n" );

      if ( ( op & M_DEST_STEREO ) && ( ! ( op & M_SRC_STEREO ) ) && ( op & M_VOL_SCALING ) )
      {
        fprintf( file, 
                 "\n  ; Duplicate the left channel into the right\n"
                 "  mov   ebx, eax\n"
                 "  imul  eax, [divider_l]\n"
                 "  imul  ebx, [divider_r]\n" );
      }
      else
      {
        fprintf( file, 
                 "  imul  eax, [divider_l]\n" );
      }
    }

  }
}

static void Low_merge_sample( int op )
{
  fprintf( file, 
           "\n  ; Merge sample data into output buffer\n"
           "  add   [edi], eax\n" );

  if ( op & M_DEST_STEREO )
  {
    if ( ( op & M_SRC_STEREO ) || ( ( op & M_DEST_STEREO ) && ( op & M_VOL_SCALING ) ) )
    {
      fprintf( file, 
               "  add   [edi+4], ebx\n" );
    }
    else
    {
      fprintf( file, 
               "  add   [edi+4], eax\n" );
    }
    fprintf( file, 
             "  add   edi, 8\n" );
  }
  else
  {
    fprintf( file, 
             "  add   edi, 4\n" );
  }
}

static void Merge_sample( int op )
{
  Low_merge_sample( op );

  fprintf( file, 
           "  cmp   edi, dest_end\n" );

  if ( ( op & M_FILTER) && ( ! ( op & M_UP_FILTER ) ) )
  {
    fprintf( file, 
             "  jb    merge_loop\n" );
  }
  else
  {
    fprintf( file, 
             "  jae   dest_end_exit\n" );
  }
}


static void Add_source( int op )
{

  int size = ( ( op & M_SRC_16 ) ? 2 : 1 ) * ( ( op & M_SRC_STEREO ) ? 2 : 1 );

  if ( op & M_FILTER )
  {
    if ( op & M_UP_FILTER )
    {
      fprintf( file, 
               "\n  ; Add to accumulator and advance the source correctly\n"
               "  add   edx, [step_fract]\n"
               "  jnc   merge_loop\n" );

      goto move_source;

    }
  }
  else
  {
    if ( op & M_RESAMPLE )
    {
      fprintf( file, 
               "\n  ; Add to accumulator and advance the source correctly\n"
               "  add   edx, [step_fract]\n"
               "  sbb   eax, eax\n"
               "  add   esi, step_whole[4+eax*4]\n" );
    }
    else
    {
     move_source:

      fprintf( file, 
               "\n  ; Move the source pointer\n");

      Output_add( "esi", size );
    }

    fprintf( file, 
             "  cmp   esi, src_end\n"
             "  jae   src_end_exit\n" );
  }

}

static void End_loop( int op )
{
  int size = ( ( op & M_SRC_16 ) ? 2 : 1 ) * ( ( op & M_SRC_STEREO ) ? 2 : 1 );

  if ( ( ! ( op & M_FILTER ) ) || ( op & M_UP_FILTER ) )
  {
    fprintf( file, 
             "\n  ; End loop\n"
             "  jmp   merge_loop\n" );
  }

  fprintf( file, 
           "\n  ; Jump out point if end of dest is reached\n"
           "  dest_end_exit:\n" );

  if ( op & M_RESAMPLE )
  {
    if ( op & M_UP_FILTER )
    {
      Output_add( "esi", size );

      fprintf( file, 
               "  add   edx, [step_fract]\n"
               "  jc    skip_filter_adjust\n" );

      if ( ( op & M_SRC_STEREO ) && ( ( op & M_DEST_STEREO ) || ( op & M_VOL_SCALING ) ) )
      {
        fprintf( file, 
                 "\n  ; rotate filter values\n"
                 "  mov   [cur_l], ecx\n"
                 "  mov   ecx, [cur_r2]\n"
                 "  mov   [cur_r], ecx\n" );
      }
      else
      {
        fprintf( file,
                 "\n  ; rotate filter values\n"
                 "  mov   [cur_l], ecx\n" );
      }

      fprintf( file,
               "\n  ; un-increment the source to skip the early source adjustment\n" );

      Output_add( "esi", -size );

      fprintf( file,
               "\n  skip_filter_adjust:\n" );
    }
    else
    {
      if ( op & M_FILTER )
      {
        fprintf( file, 
                 "  jmp   src_save_value\n"
                 "\n  ; jump out when src is exceed, but save our current loop position\n"
                 "  src_whole_exit:\n"
                 "  or   edx, 080000000h\n"
                 "  src_last_exit:\n"
                 "  or   edx, 040000000h\n"
                 "  src_save_value:\n"
                 "  mov  [cur_l], ecx\n" );
      }
      else
      {
        fprintf( file, 
                 "\n  ; Add to accumulator and advance the source correctly\n"
                 "  add   edx, [step_fract]\n"
                 "  sbb   eax, eax\n"
                 "  add   esi, step_whole[4+eax*4]\n" );
      }
    }
  }
  else
  {
    Output_add( "esi", size );
  }

  if ( ( ! ( op & M_FILTER ) ) || ( op & M_UP_FILTER ) )
  {
    fprintf( file, 
             "\n  ; Jump out point if end of src is reached\n"
             "  src_end_exit:\n" );
  }

}

static void End_routine( int op )
{
  if ( op & M_FILTER )
  {
    if ( ( op & M_SRC_STEREO ) && ( ( op & M_DEST_STEREO ) || ( op & M_VOL_SCALING ) ) )
    {
      fprintf( file, 
               "\n  ; Restore registers\n"
               "  pop   ebp\n" );
    }
  }

  if ( ( ! ( ( op & M_FILTER ) && ( ! ( op & M_UP_FILTER ) ) ) ) && ( op & M_RESAMPLE ) )
  {
    fprintf( file,
             "\n  ; adjust fractional\n"
             "  shr   edx, 16\n" );
  }

  fprintf( file, 
           "\n  ; Routine end\n"
           "  ret\n\n" );

  fprintf( file, 
           "Merge%s ENDP\n\n\n", Describe_operation( op ) );
}


void MSS_MAIN_DEF main( int argc, char** argv )
{
  int op;

  if ( argc != 2 )
  {
    file = stdout;
  }
  else
  {
    file = fopen( argv[ 1 ], "wt" );
    if ( file == 0 )
    {
      fprintf( stderr, "Error creating %s.\n", argv[ 1 ] );
      exit( 1 );
    }
  }

  for( op = 0 ; op < 256 ; op++ )
  {

    if ( op & M_ORDER )  // don't do flipped unless the src is stereo
    {
      if ( ! ( op & M_SRC_STEREO ) )
      {
        continue;
      }
    }

    if ( op & M_FILTER )  // don't do filtering on non-resampled data
    {
      if ( ! ( op & M_RESAMPLE ) )
      {
        continue;
      }
    }

    if ( op & M_UP_FILTER )  // only do upsample versions on when the filter bit is set
    {
      if ( ! ( op & M_FILTER ) )
      {
        continue;
      }
    }

    Start_routine( op );

    if ( ( ! (op & M_FILTER ) ) || ( op & M_UP_FILTER ) )
    {
      Load_sample( op );

      Apply_volume( op );
    }

    Start_loop( op );

      Filter_with_volume( op ); // if necessary

      Merge_sample( op );

      Add_source( op );

      if ( ( ! (op & M_FILTER ) ) || ( op & M_UP_FILTER ) )
      {
        Load_sample( op );

        Apply_volume( op );

      }

    End_loop( op );
    
    End_routine( op );
  }

  // do function table

  fprintf( file,
           "vector_table    LABEL DWORD\n" );

  for( op = 0 ; op < 128 ; op++ )
  {

    if ( op & M_ORDER )  // don't do flipped unless the src is stereo
    {
      if ( ! ( op & M_SRC_STEREO ) )
      {
        fprintf( file,
                 "  dd Merge%s\n", Describe_operation( op & ~M_ORDER ) );
        continue;
      }
    }

    if ( op & M_FILTER )  // don't do filtering on non-resampled data
    {
      if ( ! ( op & M_RESAMPLE ) )
      {
        fprintf( file,
                 "  dd Merge%s\n", Describe_operation( op & ~M_FILTER ) );
        continue;
      }
    }

    fprintf( file,
             "  dd Merge%s\n", Describe_operation( op ) );
  }
}
