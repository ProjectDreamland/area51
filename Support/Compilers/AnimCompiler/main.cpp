
#include "x_files.hpp"
#include "CommandLine.hpp"
#include "animcompiler.hpp"
#include "x_plus.hpp"

#ifdef TARGET_PC
#include "Windows.h"
#endif


//=========================================================================
// DATA
//=========================================================================

xbool g_Verbose     = FALSE;

//=========================================================================
//=========================================================================

struct export_info
{
    platform    Platform;
    xstring     OutName;
};

//=========================================================================

void ExecuteScript( command_line& CommandLine )
{
    export_info     ExpotName[32];
    s32             nExportNames=0;
    anim_compiler   Compiler;
    s32             i;

    // SB - Please leave this code here!, it's useful for debugging crashes
    /*
    xstring Info ;
    Info.Clear() ;
    for( i=0 ; i<CommandLine.GetNumOptions() ; i++ )
    {
        // Get option name and string
        Info += "-" + CommandLine.GetOptionName( i );
        Info += " " ;
        Info += CommandLine.GetOptionString( i ) ;
        Info += " " ;
    }
    ::MessageBox(NULL, Info, "CommandLine", MB_OK) ; 
    */

    //
    // Parse all the options
    //

    // Constructor sets up default parameters - see AnimCompiler.hpp
    compiler_anim::params   Params ;
    xbool   bKeepBind   =  FALSE;

    x_try;

    for( i=0 ; i<CommandLine.GetNumOptions() ; i++ )
    {
        // Get option name and string
        xstring OptName   = CommandLine.GetOptionName( i );
        xstring OptString = CommandLine.GetOptionString( i );

        if( OptName == xstring( "LOG" ) )
        {
            g_Verbose = TRUE;
        }
        else if( OptName == xstring( "BINDPOSE" ) )
        {
            static xbool bAlreadySet = FALSE;
            if( bAlreadySet == TRUE )
                x_throw( "There is more than one bind pose. Make sure that there is only one bind pose." );

            bAlreadySet = TRUE;
            Compiler.SetBindPose( OptString );
        }
        else if( OptName == xstring( "KEEPBIND" ) )
        {
            bKeepBind = x_atoi( OptString );
        }
        else if( OptName == xstring( "LOOP" ) )
        {
            Params.bLooping = x_atoi( OptString );
        }
        else if( OptName == xstring( "LOOP_FRAME" ) )
        {
            Params.iLoopFrame = x_atoi( OptString );
        }
        else if( OptName == xstring( "END_FRAME_OFFSET" ) )
        {
            Params.EndFrameOffset = x_atoi( OptString );
        }
        else if( OptName == xstring( "ACCUM_HORIZ_MOTION" ) )
        {
            Params.bAccumHorizMotion = x_atoi( OptString );
        }
        else if( OptName == xstring( "ACCUM_VERT_MOTION" ) )
        {
            Params.bAccumVertMotion = x_atoi( OptString );
        }
        else if( OptName == xstring( "ACCUM_YAW_MOTION" ) )
        {
            Params.bAccumYawMotion = x_atoi( OptString );
        }
        else if( OptName == xstring( "GRAVITY" ) )
        {
            Params.bGravity = x_atoi( OptString );
        }
        else if( OptName == xstring( "WORLD_COLLISION" ) )
        {
            Params.bWorldCollision = x_atoi( OptString );
        }
        else if( OptName == xstring( "FPS" ) )
        {
            Params.FPS = x_atoi( OptString );
        }
        else if( OptName == xstring( "DOWNSAMPLE" ) )
        {
            Params.Downsample = x_atof( OptString );
        }
        else if( OptName == xstring( "HANDLE" ) )
        {
            Params.HandleAngle = DEG_TO_RAD( x_atoi( OptString ) );
        }
        else if( OptName == xstring( "NAME" ) )
        {
            Params.Name = OptString;
            if( Params.Name.Find( " " ) != -1 )
                x_throw( xstring( "There are spaces in the name [" + Params.Name + "]. They need to be removed!" ) );
        }
        else if( OptName == xstring( "ANIM" ) )
        {
            Params.FileName = OptString ;

            Compiler.AddAnimation( Params ) ;
        }
        else if( OptName == xstring( "XBOX" ) )
        {
            ExpotName[nExportNames].OutName  = OptString;
            ExpotName[nExportNames].Platform = PLATFORM_XBOX;
            nExportNames++;
        }
        else if( OptName == xstring( "PS2" ) )
        {
            ExpotName[nExportNames].OutName  = OptString;
            ExpotName[nExportNames].Platform = PLATFORM_PS2;
            nExportNames++;
        }
        else if( OptName == xstring( "PC" ) )
        {
            ExpotName[nExportNames].OutName  = OptString;
            ExpotName[nExportNames].Platform = PLATFORM_PC;
            Compiler.SetOutputName(ExpotName[nExportNames].OutName);
            nExportNames++;
        }
        else if( OptName == xstring( "WEIGHT" ) )
        {
            Params.Weight = x_atof( OptString );
        }
        else if( OptName == xstring( "BLEND_TIME" ) )
        {
            Params.BlendTime = x_atof( OptString );
        }
        else if( OptName == xstring( "CHAIN_ANIM" ) )
        {
            Params.ChainAnim = OptString ;
        }
        else if( OptName == xstring( "CHAIN_FRAME" ) )
        {
            Params.iChainFrame = x_atoi( OptString );
        }
        else if( OptName == xstring( "CHAIN_CYCLES_MIN" ) )
        {
            Params.ChainCyclesMin = x_atof( OptString );
        }
        else if( OptName == xstring( "CHAIN_CYCLES_MAX" ) )
        {
            Params.ChainCyclesMax = x_atof( OptString );
        }
        else if( OptName == xstring( "CHAIN_CYCLES_INTEGER" ) )
        {
            Params.bChainCyclesInteger = x_atoi( OptString );
        }
        else if( OptName == xstring( "BLEND_FRAMES" ) )
        {
            Params.bBlendFrames = x_atoi( OptString );
        }
        else if( OptName == xstring( "BLEND_LOOP" ) )
        {
            Params.bBlendLoop = x_atoi( OptString );
        }
    }

    if( nExportNames == 0 )
        x_throw( "Not platform expecified" );

    //
    // Execute the Commands
    //
    anim_group AnimGroup;
    Compiler.Compile( AnimGroup, bKeepBind );

    for( s32 i=0; i<nExportNames; i++ )
    {
        AnimGroup.Save( ExpotName[i].OutName );
    }

    x_catch_begin;
    #ifdef X_EXCEPTIONS
        x_printf( "Error: %s\n", xExceptionGetErrorString() );
    #endif
    x_catch_end;
}

//=========================================================================

void PrintHelp( void )
{
    x_printf( "Error: Compiling\n" );
    x_printf( ":Platform to compile for: -XBOX c:/A51/Level1/XBOX/FileName  -PS2 c:/A51/Level1/PS2/FileName  -PC c:/A51/Level1/PC/FileName  \n" );   
    x_printf( ":FileNames:               -FILE input.txt                               \n" );
}

//=========================================================================
void main( s32 argc, char* argv[] )
{
    // Init static anim event stuff
    anim_event::Init();

    command_line CommandLine;
    
    // Expecify all the options
    CommandLine.AddOptionDef( "LOG" );
    CommandLine.AddOptionDef( "XBOX",                   command_line::STRING );
    CommandLine.AddOptionDef( "PS2",                    command_line::STRING );
    CommandLine.AddOptionDef( "PC",                     command_line::STRING );
    CommandLine.AddOptionDef( "BINDPOSE",               command_line::STRING );
    CommandLine.AddOptionDef( "ANIM",                   command_line::STRING );
    CommandLine.AddOptionDef( "LOOP",                   command_line::STRING );
    CommandLine.AddOptionDef( "LOOP_FRAME",             command_line::STRING );
    CommandLine.AddOptionDef( "END_FRAME_OFFSET",       command_line::STRING );
    CommandLine.AddOptionDef( "ACCUM_HORIZ_MOTION",     command_line::STRING );
    CommandLine.AddOptionDef( "ACCUM_VERT_MOTION",      command_line::STRING );
    CommandLine.AddOptionDef( "ACCUM_YAW_MOTION",       command_line::STRING );
    CommandLine.AddOptionDef( "GRAVITY",                command_line::STRING );
    CommandLine.AddOptionDef( "WORLD_COLLISION",        command_line::STRING );
    CommandLine.AddOptionDef( "KEEPBIND",               command_line::STRING );    
    CommandLine.AddOptionDef( "FPS",                    command_line::STRING );
    CommandLine.AddOptionDef( "DOWNSAMPLE",             command_line::STRING );
    CommandLine.AddOptionDef( "HANDLE",                 command_line::STRING );
    CommandLine.AddOptionDef( "NAME",                   command_line::STRING );
    CommandLine.AddOptionDef( "WEIGHT",                 command_line::STRING );
    CommandLine.AddOptionDef( "BLEND_TIME",             command_line::STRING );
    CommandLine.AddOptionDef( "CHAIN_ANIM",             command_line::STRING );
    CommandLine.AddOptionDef( "CHAIN_FRAME",            command_line::STRING );
    CommandLine.AddOptionDef( "CHAIN_CYCLES_MIN",       command_line::STRING );
    CommandLine.AddOptionDef( "CHAIN_CYCLES_MAX",       command_line::STRING );
    CommandLine.AddOptionDef( "CHAIN_CYCLES_INTEGER",   command_line::STRING );
    CommandLine.AddOptionDef( "BLEND_FRAMES",           command_line::STRING );
    CommandLine.AddOptionDef( "BLEND_LOOP",             command_line::STRING );

#if 0
    // SB - Please leave this code here!, it's useful for debugging crashes
    const char* pInputLogFile = "C://GameData//A51//Apps//Temp//npc_act_mil_01_levelblue_sl0-5.anim.1.txt" ;

    // Parse command line from editor output file - VERY USEFUL FOR DEBUGGIN CRASHES!!!
    token_stream TOK ;
    if ( (pInputLogFile) && (TOK.OpenFile(pInputLogFile)) )
    {
        // Scan for args
        s32             Args = 0 ;
        xarray<char*>   pArgs ;
        xarray<xstring> Strings ;
        
        // Make sure we never re-allocate otherwise our ptrs screw up
        pArgs.SetCapacity(10000);
        Strings.SetCapacity(10000);
        
        // Add .exe as parameter0 (this parameter is not included in the .txt file)
        {
            xstring& String = Strings.Append() ;
            String = "AnimCompiler.exe" ;
            
            const char* pString = (const char*)String ;
            pArgs.Append((char*)pString) ;
        }

        while(!TOK.IsEOF())
        {
            TOK.ReadSymbol() ;
            if (TOK.String()[0] == '-')
            {
                // Add -Command
                {
                    xstring& String = Strings.Append() ;
                    String = TOK.String() ;

                    const char* pString = (const char*)String ;
                    pArgs.Append((char*)pString) ;
                }

                // Add param...
                TOK.Read() ;
                {
                    xstring& String = Strings.Append() ;
                    String = TOK.String() ;

                    const char* pString = (const char*)String ;
                    pArgs.Append((char*)pString) ;
                }

            }
        }

        // Parse the command line
        if( CommandLine.Parse( pArgs.GetCount(), pArgs.GetPtr() ) )
        {
            PrintHelp();
            return;
        }

        // Do the script
        ExecuteScript( CommandLine );
        return ;
    }
#endif

    // Parse the command line
    if( CommandLine.Parse( argc, argv ) )
    {
        PrintHelp();
        return;
    }

    // Do the script
    ExecuteScript( CommandLine );

    LOG_FLUSH();
}



