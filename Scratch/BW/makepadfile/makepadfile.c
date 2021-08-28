// makepadfile.c - writes out a file that is blank. Used for padding PS2 dvd images.
//

#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "memory.h"

#define ONE_MEGABYTE 1048576

int main(int argc, char* argv[])
{
    int     Size;
    FILE*   fp;
    void*   pBuffer;
    int     i;
    size_t  Written;

    if( argc != 3 )
    {
        printf( "Usage <size-in-megabytes> <target-filename>\n");
        exit( -1 );
    }

    Size = atoi( argv[1] );
    if( (Size <= 0) || (Size > 2048) )
    {
        printf( "Error: Size parameter needs to be between 1 and 2048 (2G)\n");
        exit( -1 );
    }

    fp = fopen( argv[2], "wb" );
    if( fp==NULL )
    {
        printf( "Error: Unable to open '%s' for output.\n", argv[2] );
        exit( -1 );
    }

    pBuffer = malloc( ONE_MEGABYTE );
    assert( pBuffer );

    memset( pBuffer, 0, ONE_MEGABYTE );

    for( i=0; i<Size; i++ )
    {
        Written = fwrite( pBuffer, ONE_MEGABYTE, 1, fp );
        if( Written != 1 )
        {
            printf( "Error: Unable to write data to '%s\n", argv[2] );
            exit( -1 );
        }
    }

    fclose( fp );
	return 0;
}
