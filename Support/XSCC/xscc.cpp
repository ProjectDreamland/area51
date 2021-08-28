//==============================================================================
//==============================================================================
//  xscc - xScript Compiler
//==============================================================================
//==============================================================================

#include "x_files.hpp"
#include "../xcore/Auxiliary/CommandLine/CommandLine.hpp"

#include "ScriptCompiler/xsc_compiler.hpp"
#include "ScriptVM/xsc_vm_core.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  Display Help
//==============================================================================

void DisplayHelp( void )
{
    x_printf( 
"\n"
"xScript Compiler (c)2001 Inevitable Entertainment Inc.\n"
"\n"
"  usage:\n"
"         xscc [-opt [param]] [filenames]\n"
"\n"
"options:\n"
"   PC         - Export for PC in the specify path. If the path is empty no\n" 
"                file is exported.\n"
"   PS2        - Export for PS2 in the specify path. If the path is empty no\n"
"                file is exported.\n"
"   NGC        - Export for Nintendo in the specify path. If the path is empty\n"
"                no file is exported.\n"
"   XBOX       - Export for XBox in the specify path. If the path is empty\n"
"                no file is exported.\n"
"   PROGRESS   - Print out the progress of the compiler\n"
"   DEP        - Print out external resources\n"
"   LOG        - Print out debug info while compiling\n"
"   L          - Output listing\n"
"   T          - Show compile times\n"
"\n" );
}

static void testprints( s32 v )
{
    x_printf( "%d\n", v );
}

static void testprintf( f32 v )
{
    x_printf( "%f\n", v );
}

//==============================================================================
//  main
//==============================================================================
enum ePlatforms
{
    kPLATFORM_PC    =0,
    kPLATFORM_PS2   =1,
    kPLATFORM_NGC   =2,
    kPLATFORM_XBOX  =3,
    kPLATFORM_COUNT =4    
};

int main( int argc, char** argv )
{
    s32             i;
    command_line    CommandLine;
    xbool           NeedHelp        = FALSE;
    xbool           ShowTimes       = FALSE;
    xbool           OutputListing   = FALSE;
    xarray<xstring> SourceFiles;

    // Setup recognized command line options
    CommandLine.AddOptionDef( "T"                             );
    CommandLine.AddOptionDef( "L"                             );
    CommandLine.AddOptionDef( "DEP"                           );
    CommandLine.AddOptionDef( "LOG"                           );
    CommandLine.AddOptionDef( "PC",      command_line::STRING );
    CommandLine.AddOptionDef( "PS2",     command_line::STRING );
    CommandLine.AddOptionDef( "NGC",     command_line::STRING );
    CommandLine.AddOptionDef( "XBOX",    command_line::STRING );

    // Parse command line
    NeedHelp = CommandLine.Parse( argc, argv );
    if( NeedHelp || (CommandLine.GetNumArguments() == 0) )
    {
        DisplayHelp();
        return 10;
    }

    // Check options
    ShowTimes     = (CommandLine.FindOption( xstring("T") ) != -1);
    OutputListing = (CommandLine.FindOption( xstring("L") ) != -1);

    s32 whichOption = -1;
    xstring platformPaths[kPLATFORM_COUNT];

    xbool   printExternal = false,
            outputLog     = false;

    printExternal   = (CommandLine.FindOption( xstring("DEP") ) != -1 );
    outputLog       = (CommandLine.FindOption( xstring("LOG") ) != -1 );

    whichOption = CommandLine.FindOption( xstring("PC") );
    if(whichOption != -1 )
    {
        platformPaths[kPLATFORM_PC] = CommandLine.GetOptionString(whichOption );
    }

    whichOption = CommandLine.FindOption( xstring("PS2") );
    if(whichOption != -1 )
    {
        platformPaths[kPLATFORM_PS2] = CommandLine.GetOptionString(whichOption );
    }

    whichOption = CommandLine.FindOption( xstring("NGC") ) ;
    if(whichOption != -1 )
    {
        platformPaths[kPLATFORM_NGC] = CommandLine.GetOptionString(whichOption );
    }

    whichOption = CommandLine.FindOption( xstring("XBOX") ) ;
    if(whichOption != -1 )
    {
        platformPaths[kPLATFORM_XBOX] = CommandLine.GetOptionString(whichOption );
    }



    // Build list of files to process
    for( i=0 ; i<CommandLine.GetNumArguments() ; i++ )
    {
        // Glob each pattern to build a list of files to process
        SourceFiles.Append() = CommandLine.GetArgument( i );
    }

    // Print status
    x_printf( "Compiling...\n" );

    // Call the compiler on each source file in turn
    for( i=0 ; i<SourceFiles.GetCount() ; i++ )
    {
        s32 j;
        for( j = 0; j < kPLATFORM_COUNT; j++)
        {
            if(platformPaths[j].GetLength() > 1 )
            {
            

                // Get input and output names
                const xstring&  InputFileName   = SourceFiles[i];
            
//                xstring         OutputFileName  = CommandLine.ChangeExtension( InputFileName, xstring("") );

                xstring         fileName, pathName;
                CommandLine.SplitPath ( InputFileName, pathName, fileName );
                fileName = CommandLine.ChangeExtension(fileName, xstring("") );
                xstring OutputFileName = platformPaths[j] + "\\" + fileName;
	        
                // Start Timer for compile
                xtimer Timer;
                Timer.Start();

                // Instantiate Compiler & load source
                xsc_compiler Compiler;
                if( Compiler.Init( InputFileName ) )
                {
                    // Print status
                    x_printf( "%s", (const char*)InputFileName );

                    // Compile
                    Compiler.Compile( OutputFileName );
                    {
                        // Dump the contents of the module
                        if( OutputListing  || outputLog )
                        {
                            // Dump Errors
                            xstring Dump;
                            Dump = Compiler.DumpErrors();
                            Dump += "\n";

                            // Create VM and load compiled binary back
                            xsc_vm_core VM;
                            if( VM.LoadModule( OutputFileName+".xsc" ) )
                            {
                                // Write Dump from VM
                                Dump += VM.Dump( );
                                Dump += "\n";
                                Dump += Compiler.DumpAST();
                                Dump += "\n";
                                Dump += Compiler.DumpSymbolTable();
                            }

                            // Save LST
                            Dump.SaveFile( (const char*)CommandLine.ChangeExtension( InputFileName, "lst" ) );
                        }
                    }

                    // Stop Timer
                    Timer.Stop();

                    // Create Summary
                    xstring Dump;
                    if( ShowTimes )
                        Dump.AddFormat( " in %.2fms", Timer.ReadMs() );
                    else
                        Dump += "\n";
                    Dump += Compiler.DumpErrors( );

                    // Print Summary
                    while( Dump.GetLength() > 0 )
                    {
                        xstring     Line;
                        s32         p = Dump.Find( '\n' );
                        if( p != -1 )
                        {
                            Line = Dump.Left( p );
                            Dump = Dump.Right( Dump.GetLength() - (p+1) );
                        }
                        else
                        {
                            Line = Dump;
                            Dump.Clear();
                        }
                        x_printf( "%s\n", (const char*)Line );
                    }
                }
                else
                {
                    // Couldn't load source
                    x_printf( "Error loading soure '%s'\n", (const char*)InputFileName );
                }
            }
        }
    }

    // Return Success
    return 0;
}
