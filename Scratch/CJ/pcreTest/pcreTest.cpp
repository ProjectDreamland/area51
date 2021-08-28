#include "x_files.hpp"
#include "Auxiliary/PCRE/regex.hpp"

void main( int argc, char** argv )
{
    if( argc < 3 )
    {
        x_printf( "Error" );
        exit( 0 );
    }

    regex re( argv[1] );

    re.Match( argv[2] );

    int n = re.GetNumSubstrings();
    x_printf( "Num = %d\n", n );

    for( int i=0 ; i<n ; i++ )
    {
        x_printf( "Str = %.*s\n", re.GetSubstringLen(i), re.GetSubstringPtr(i) );
    }
}
