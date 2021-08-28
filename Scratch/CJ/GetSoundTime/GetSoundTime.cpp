#include "Auxiliary/CommandLine/CommandLine.hpp"
#include "x_files.hpp"
#include "aiff_file.hpp"
#include "stdlib.h"

void main( int argc, char** argv )
{
    command_line    CmdLine;
    if( CmdLine.Parse( argc, argv ) || (CmdLine.GetNumArguments() == 0) )
    {
        x_printf( "Usage: GetSoundTime <aiff file>\n" );
        exit( 0 );
    }
    
    for( s32 i=0; i<CmdLine.GetNumArguments(); i++ )
    {
        const xstring& Arg = CmdLine.GetArgument( i );
        //x_printf( "%s\n", (const char*)Arg );

        aiff_file Aiff;
        if( Aiff.Open( Arg ) )
        {
            s32 SampleRate = Aiff.GetSampleRate();
            s32 nSamples = Aiff.GetNumSamples();
            f32 Time = ( (f32)nSamples / (f32)SampleRate );
            x_printf( "%.3f\n", Time );
        }
        else
        {
            x_printf( "Error\n" );
        }
    }
}
