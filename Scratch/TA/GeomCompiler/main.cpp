
#include "x_files.hpp"
#include "Rawmesh.hpp"
#include "CommandLine.hpp"
#include "GeomCompiler.hpp"

static const s32    s_MaxLODs = 8;
static xstring      s_LODName     [ s_MaxLODs ];
static f32          s_LODDistance [ s_MaxLODs ];
static xstring      s_ExpotName;

//=========================================================================

void ExecuteScript( command_line& CommandLine )
{
    geom_compiler   Compiler;
    s32             MaxLOD=1;
    s32             i;
    xbool           bPlatfrom = FALSE;

    //
    // Parse all the options
    //
    for( i=0 ; i<CommandLine.GetNumOptions() ; i++ )
    {
        // Get option name and string
        xstring OptName   = CommandLine.GetOptionName( i );
        xstring OptString = CommandLine.GetOptionString( i );
        xbool   bOption   = FALSE;

        // Check for all the posible lods
        for( s32 i=0; i<s_MaxLODs; i++ )
        {
            if( OptName == xstring( xfs("F%d",i) ) )
            {
                s_LODName[i] = OptString;
                bOption = TRUE;
                if( MaxLOD < i ) MaxLOD = i+1; 
                break;
            }

            if( OptName == xstring( xfs("D%d",i) ) )
            {
                s_LODDistance[i] = x_atof( OptString ) ;
                bOption          = TRUE;
                if( MaxLOD < i ) MaxLOD = i+1;
                break;
            } 
        }

        if( OptName == xstring( "EXPORT" ) )
            s_ExpotName = OptString;

        if( OptName == xstring( "PLATFORM_PC" ) )
        {
            Compiler.AddPlatform( PLATFORM_PC, OptString );
            bPlatfrom = TRUE;
        }

        if( OptName == xstring( "PLATFORM_PS2" ) )
        {
            Compiler.AddPlatform( PLATFORM_PS2, OptString );
            bPlatfrom = TRUE;
        }
    }

    if( bPlatfrom == FALSE )
    {
        x_printf( "Error: Not platform expecified\n" );
        return;
    }

    //
    // Check integrity
    //
    for( i=0; i<MaxLOD; i++ )
    {
        if( s_LODName[i] == "" )
        {
            x_printf("Error: There was not an LOD #%d \n", i );
            return;
        }
    }

    //
    // Execute the Commands
    //
    e_begin;

        for( i=0; i<MaxLOD; i++ )
        {
            Compiler.AddLOD( s_LODName[i], s_LODDistance[i], i==0 );
        }

        Compiler.Export( s_ExpotName );

    e_block;
        x_printf( "Error: %s\n", xExceptionGetErrorString() );
    e_block_end;
}

//=========================================================================
void PrintHelp( void )
{
    x_printf( "Error: Compiling\n" );
    x_printf( "Platform to compile for: -PLATFORM_PS2 c:/A51/Level1/PC  -PLATFORM_PC c:/A51/Level1/PS2  \n" );   
    x_printf( "Type of geom:            -RIGID_WALL  (For Wall pices in grid)                           \n" );
    x_printf( "FileNames:               -F0 LOD0_RawMesh.matx -F1 LOD1_RawMesh.matx ... etc             \n" );
    x_printf( "MinDistances:            -D0 0                 -D1 10000             ... etc         \n" );
}

//=========================================================================
void main( s32 argc, char* argv[] )
{
    x_Init();

    command_line CommandLine;
    
    // Expecify all the options
    CommandLine.AddOptionDef( "F0", command_line::STRING );
    CommandLine.AddOptionDef( "F1", command_line::STRING );
    CommandLine.AddOptionDef( "F2", command_line::STRING );
    CommandLine.AddOptionDef( "F3", command_line::STRING );
    CommandLine.AddOptionDef( "F4", command_line::STRING );
    CommandLine.AddOptionDef( "F5", command_line::STRING );

    CommandLine.AddOptionDef( "D0", command_line::NUMBER );
    CommandLine.AddOptionDef( "D1", command_line::NUMBER );
    CommandLine.AddOptionDef( "D2", command_line::NUMBER );
    CommandLine.AddOptionDef( "D3", command_line::NUMBER );
    CommandLine.AddOptionDef( "D4", command_line::NUMBER );
    CommandLine.AddOptionDef( "D5", command_line::NUMBER );

    CommandLine.AddOptionDef( "EXPORT", command_line::STRING );
    CommandLine.AddOptionDef( "RIGID_WALL" );

    CommandLine.AddOptionDef( "PLATFORM_PS2", command_line::STRING );
    CommandLine.AddOptionDef( "PLATFORM_PC",  command_line::STRING );

    // Parse the command line
    if( CommandLine.Parse( argc, argv ) )
    {
        PrintHelp();
        return;
    }

    if( CommandLine.FindOption( xstring("EXPORT") ) == -1 )
    {
        PrintHelp();
        return;
    }
    
    // Do the script
    ExecuteScript( CommandLine );
}