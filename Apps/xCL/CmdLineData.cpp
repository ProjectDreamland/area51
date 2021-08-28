//==============================================================================
//
//  CmdLineData.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "xCL.hpp"
#include "CmdLineData.hpp"
#include "CmdLnTokenizer.hpp"

//==============================================================================
//  VARIABLES
//==============================================================================

static cmd_line_tokenizer  T;
static cmd_line_data&      C = g_CmdLineData;

//==============================================================================
//  TYPES
//==============================================================================

typedef void (*option_handler_fnptr)( void );

typedef struct
{
    char*                  Option;
    option_handler_fnptr   Handler;
} option_handler_entry;

//==============================================================================
//  FUNCTIONS
//==============================================================================

//==============================================================================
//  Option Handler Functions
//------------------------------------------------------------------------------
//
//  The following functions are used to process individual options encountered
//  in the command line.  All of the functions are expected to use the local
//  cmd_line_data& C.
//
//==============================================================================

void IgnoreOption0( void )
{
}

//==============================================================================

void IgnoreOption1( void )
{
    T.NextToken();
}

//==============================================================================

void RecordDefine( void )
{
    C.m_Define.Add( T.NextToken() );

    // Check for target definition string.
    {
        int i = C.m_Define.GetCount()-1;
        if( strncmp( "TARGET_", C.m_Define[i], 7 ) == 0 )
            strcpy( C.m_TargetString, C.m_Define[i] );
    }

    // Check for vendor definition string.
    {
        int i = C.m_Define.GetCount()-1;
        if( strncmp( "VENDOR_", C.m_Define[i], 7 ) == 0 )
            strcpy( C.m_VendorString, C.m_Define[i] );
    }
}

//==============================================================================

void RecordIncludePath( void )
{
    C.m_IncludeDir.Add( T.NextToken() );
}

//==============================================================================

void RecordLibraryPath( void )
{
    C.m_LibraryDir.Add( &T.CurrentToken()[9] );
}

//==============================================================================

void RecordOptimization( void )
{
    switch( T.CurrentToken()[2] )
    {
        case 'd':   
        case '1':   
        case 't':   
        case '2':   
            C.m_Optimization = T.CurrentToken()[2];
            break;
        default:
            break;
    }
}

//==============================================================================

void RecordDebugMode( void )
{
    C.m_DebugMode = true;
}

//==============================================================================

void RecordPreprocess( void )
{
    C.m_Preprocess = true;
}                 

//==============================================================================

void RecordForceC( void )
{
    C.m_ForceC = true;
}                 

//==============================================================================

void RecordForceCPP( void )
{
    C.m_ForceCPP = true;
}                 

//==============================================================================

void RecordLibraryMode( void )
{
    C.m_Tool = TOOL_LIBRARIAN;
}

//==============================================================================

void RecordPrecompiled( void )
{
    // The PCH may or may not be present.  And it may or may not be 
    // enclosed in quotes.

    if( T.CurrentToken()[3] == '\0' )
        return;

    if( T.CurrentToken()[3] == '"' )
    {
        strcpy( C.m_PCHFile, &T.CurrentToken()[4] );
        int i = strlen(C.m_PCHFile) - 1;
        if( C.m_PCHFile[ i ] == '"' )
            C.m_PCHFile[ i ] = '\0';
    }
    else
    {
        strcpy( C.m_PCHFile, &T.CurrentToken()[3] );
    }
}

//==============================================================================

void RecordLinkDefine( void )
{
    C.m_Define.Add( &T.CurrentToken()[3] );

    // Check for target definition string.
    {
        int i = C.m_Define.GetCount()-1;
        if( strncmp( "TARGET_", C.m_Define[i], 7 ) == 0 )
            strcpy( C.m_TargetString, C.m_Define[i] );
    }

    // Check for vendor definition string.
    {
        int i = C.m_Define.GetCount()-1;
        if( strncmp( "VENDOR_", C.m_Define[i], 7 ) == 0 )
            strcpy( C.m_VendorString, C.m_Define[i] );
    }
}

//==============================================================================

void RecordLinkOptions( void )
{
    // Given a token of /L:"<stuff>", record <stuff> as additional link options.
    // Make sure to remove the quotes.

    strcpy( C.m_LinkOptions, &T.CurrentToken()[4] );
    C.m_LinkOptions[ strlen(C.m_LinkOptions)-1 ] = '\0';    // Kill close "
}

//==============================================================================

void RecordOutputFile( void )
{
    // Output file specification may or may not be enclosed in quotes.

    if( T.CurrentToken()[5] == '"' )
    {
        strcpy( C.m_OutputFile, &T.CurrentToken()[6] );
        int i = strlen(C.m_OutputFile) - 1;
        if( C.m_OutputFile[ i ] == '"' )
            C.m_OutputFile[ i ] = '\0';
    }
    else
    {
        strcpy( C.m_OutputFile, &T.CurrentToken()[5] );
    }
}

//==============================================================================

void RecordOutputPath( void )
{
    // Output path specification may or may not be enclosed in quotes.

    if( T.CurrentToken()[3] == '"' )
    {
        strcpy( C.m_OutputPath, &T.CurrentToken()[4] );
        int i = strlen(C.m_OutputPath) - 1;
        if( C.m_OutputPath[ i ] == '"' )
            C.m_OutputPath[ i ] = '\0';
    }
    else
    {
        strcpy( C.m_OutputPath, &T.CurrentToken()[3] );
    }

    // Snip off trailing '/'.
    {
        int Len = strlen( C.m_OutputPath );
        if( C.m_OutputPath[Len-1] == '/' )
            C.m_OutputPath[Len-1] =  '\0';
    }
}

//==============================================================================

void RecordSBRPath( void )
{
    // SBR path specification may or may not be enclosed in quotes.

    if( T.CurrentToken()[3] == '"' )
    {
        strcpy( C.m_SBRPath, &T.CurrentToken()[4] );
        int i = strlen(C.m_SBRPath) - 1;
        if( C.m_SBRPath[ i ] == '"' )
            C.m_SBRPath[ i ] = '\0';
    }
    else
    {
        strcpy( C.m_SBRPath, &T.CurrentToken()[3] );
    }

    // Snip off trailing '/'.
    {
        int Len = strlen( C.m_SBRPath );
        if( C.m_SBRPath[Len-1] == '/' )
            C.m_SBRPath[Len-1] =  '\0';
    }
}

//==============================================================================

void RecordPDBPath( void )
{
    // PDB path specification may or may not be enclosed in quotes.

    if( T.CurrentToken()[3] == '"' )
    {
        strcpy( C.m_PDBPath, &T.CurrentToken()[4] );
        int i = strlen(C.m_PDBPath) - 1;
        if( C.m_PDBPath[ i ] == '"' )
            C.m_PDBPath[ i ] = '\0';
    }
    else
    {
        strcpy( C.m_PDBPath, &T.CurrentToken()[3] );
    }

    // Snip off trailing '/'.
    {
        int Len = strlen( C.m_PDBPath );
        if( C.m_PDBPath[Len-1] == '/' )
            C.m_PDBPath[Len-1] =  '\0';
    }
}

//==============================================================================

void RecordSubSystem( void )
{
    char* p = &(T.CurrentToken()[11]);

    if( stricmp( p, "windows" ) == 0 )   C.m_SubSystem = SUBSYS_WINDOWS;
    if( stricmp( p, "console" ) == 0 )   C.m_SubSystem = SUBSYS_WINCON;
}

//==============================================================================
//  Option Handler Table
//------------------------------------------------------------------------------
//  
//  This table associates the Option Handler Functions with strings that signal
//  the option.
//  
//  NOTE:  The order of the strings in the list is relevant!  Longer strings
//  should preceed shorter strings to avoid errors.  For example, "Ob" must
//  occur before "O".
//  
//==============================================================================

static option_handler_entry  OptionHandlerList[] = {

    // --- Compiler ('C') or Linker ('L') -----.
    //                                         |
    { "incrementa",  IgnoreOption0      },  // L:enable incremental link
    { "subsystem:", RecordSubSystem     },  // L:target subsystem
    { "libpath:",   RecordLibraryPath   },  // L:
    { "pdbtype:",    IgnoreOption0      },  // L:PDB file type
    { "machine:",    IgnoreOption0      },  // L:target CPU
    { "nologo",      IgnoreOption0      },  // C:disable compiler banner
    { "NOLOGO",      IgnoreOption0      },  // C:disable compiler banner
    { "LTCG",        IgnoreOption0      },  // C:
    { "entry:",      IgnoreOption0      },  // L:entry point
    { "debug",      RecordDebugMode     },  // L:
    { "base:",       IgnoreOption0      },  // L:base address
    { "map:",        IgnoreOption0      },  // L:map file
    { "out:",       RecordOutputFile    },  // L:
    { "OUT:",       RecordOutputFile    },  // L:
    { "pdb:",        IgnoreOption0      },  // L:PDB file
    { "lib",        RecordLibraryMode   },  // L:
    { "Ob0",         IgnoreOption0      },  // C:in-line function expansion
    { "Ob1",         IgnoreOption0      },  // C:in-line function expansion
    { "Ob2",         IgnoreOption0      },  // C:in-line function expansion
    { "EHsc",        IgnoreOption0      },  // C:
    { "EP",          IgnoreOption0      },  // C:preprocess to stdout
    { "FD",          IgnoreOption0      },  // C:generate file dependancies
    { "Fp",          IgnoreOption0      },  // C:PCH file
    { "Fr",          IgnoreOption0      },  // C:
    { "Fo",         RecordOutputPath    },  // C:
    { "Fd",         RecordPDBPath       },  // C:
    { "FR",         RecordSBRPath       },  // C:SBR file
    { "G3",          IgnoreOption0      },  // C:code generation, target '386
    { "G4",          IgnoreOption0      },  // C:code generation, target '486
    { "G5",          IgnoreOption0      },  // C:code generation, target Pentium
    { "G6",          IgnoreOption0      },  // C:code generation, target Pentium Pro
    { "G7",          IgnoreOption0      },  // C:code generation, target Pentium II
    { "Gm",          IgnoreOption0      },  // C:enable minimal rebuild
    { "GS",          IgnoreOption0      },  // C:
    { "GX",          IgnoreOption0      },  // C:enable exception handling
    { "Gy",          IgnoreOption0      },  // C:
    { "GZ",          IgnoreOption0      },  // C:enable runtime error checks
    { "ML",          IgnoreOption0      },  // C:link with LIBC.LIB
    { "MT",          IgnoreOption0      },  // C:multi-threaded
    { "RTC1",        IgnoreOption0      },  // C:
    { "RTCc",        IgnoreOption0      },  // C:
    { "TC",         RecordForceC        },  // C:force C   compilation
    { "TP",         RecordForceCPP      },  // C:force C++ compilation
    { "Zi",         RecordDebugMode     },  // C:
    { "Yc",          IgnoreOption0      },  // C:create precompiled header file......RecordPrecompiled?
    { "Yu",          IgnoreOption0      },  // C:use precompiled header file.........RecordPrecompiled?
    { "YX",          IgnoreOption0      },  // C:automatic PCH ......................RecordPrecompiled?
    { "ZI",         RecordDebugMode     },  // C:
    { "Z7",         RecordDebugMode     },  // C:
    { "D:",         RecordLinkDefine    },  // L:Not a standard link option
    { "L:",         RecordLinkOptions   },  // L:Not a standard link option
    { "c",           IgnoreOption0      },  // C:compile only
    { "C",           IgnoreOption0      },  // C:preserve comments in pre-processor
    { "D",          RecordDefine        },  // C:
    { "I",          RecordIncludePath   },  // C:
    { "l",           IgnoreOption1      },  // C:locality specifier
    { "O",          RecordOptimization  },  // C:
    { "P",          RecordPreprocess    },  // C:Preprocess to file
    { "W",           IgnoreOption0      },  // C:set warning level
};

//==============================================================================
//  Member functions for cmd_line_data.
//==============================================================================

void cmd_line_data::ProcessOption( char* pToken )
{
    int i;
    int ListLength  = sizeof( OptionHandlerList ) /
                      sizeof( option_handler_entry );

    assert( (pToken[0] == '/') || (pToken[0] == '-') );

    // Search the list of options for a handler.
    for( i = 0; i < ListLength; i++ )
    {
        if( strncmp( pToken+1, 
                     OptionHandlerList[i].Option, 
                     strlen( OptionHandlerList[i].Option ) ) == 0 )
            break;
    }

    // Did we find a match?
    if( i == ListLength )
    {
        // Record the option as an unknown option.
        m_UnknownOption.Add( pToken );
    }
    else
    {
        // Process the option using the listed function.
        OptionHandlerList[i].Handler();
    }
}

///////////////////////////////////////////////////////////////////////////

void cmd_line_data::ProcessFile( char* pToken )
{
    char  Ext[ _MAX_EXT ];

    // Strange thing here.  If the path or filename has a space in it, then
    // the system will surround the path with double quotes.  Before we 
    // break the extension out, we need to see if we have quotes around the
    // file name, and if so, remove them.

    if( pToken[0] == '"' )
    {
        int x;
        pToken += 1;
        x = strlen(pToken)-1;
        pToken[ x ] = '\0';
    }

    _splitpath( pToken, NULL, NULL, NULL, Ext );

    if     ( stricmp( Ext, ".lib" ) == 0 )  m_LibraryFile.  Add( pToken ); 
    else if( stricmp( Ext, ".a"   ) == 0 )  m_LibraryFile.  Add( pToken ); 
    else if( stricmp( Ext, ".ilb" ) == 0 )  m_LibraryFile.  Add( pToken ); 
    else if( stricmp( Ext, ".obj" ) == 0 )  m_ObjectFile.   Add( pToken ); 
    else if( stricmp( Ext, ".o"   ) == 0 )  m_ObjectFile.   Add( pToken ); 
    else if( stricmp( Ext, ".s"   ) == 0 )  m_SourceOther.  Add( pToken ); 
    else if( stricmp( Ext, ".vu"  ) == 0 )  m_SourceOther.  Add( pToken ); 
    else                                    m_SourceCode.   Add( pToken );
}

///////////////////////////////////////////////////////////////////////////

cmd_line_data::cmd_line_data( void )
{
    // Set initial values to member fields.
    m_Tool            = TOOL_UNKNOWN;
    m_Target          = BUILD_TARGET_UNKNOWN;
    m_Vendor          = VENDOR_UNKNOWN;
    m_SubSystem       = SUBSYS_DEFAULT;
    m_DebugMode       = false;
    m_Preprocess      = false;
    m_ForceC          = false;
    m_ForceCPP        = false;
    m_Optimization    = 't';
    m_TargetString[0] = '\0';
    m_VendorString[0] = '\0';
    m_LinkOptions[0]  = '\0';
    m_OutputFile[0]   = '\0';
    m_OutputPath[0]   = '\0';
    m_SBRPath[0]      = '\0';
    m_PDBPath[0]      = '\0';

    m_argc            = 0;
    m_argv            = NULL;
}

///////////////////////////////////////////////////////////////////////////

cmd_line_data::~cmd_line_data()
{
}
                                                                        
///////////////////////////////////////////////////////////////////////////

void cmd_line_data::Activate( int argc, char** argv )
{   
    char* pToken;
    bool  RFile = false;

    // Retain the original command line information.
    m_argc = argc;
    m_argv = argv;                         

    // Activate the tokenizer.
    T.Activate( argc, argv );

    // Display all tokens found.
    if( g_Verbose )
    {
        int i = 0;
        OpenSection( "Input Token Stream" );
        while( (pToken = T.NextToken()) )
        {
            if( !RFile && T.GetResponseFile() )
            {
                printf( "           %s\n", T.GetResponseFile() );
                RFile = true;
            }
            if( RFile && !T.GetResponseFile() )
            {
                RFile = false;
            }

            printf( "    [%03d]  ", i++ );
            if( RFile )    printf( "    " );
            printf( "%s\n", pToken );
        }
        CloseSection();
        T.Reset();
    }

    // Make intial decision regarding the tool.
    {
        char FName[ _MAX_FNAME ];
        char Ext  [ _MAX_EXT   ];
        _splitpath( argv[0], NULL, NULL, FName, Ext );
        if( stricmp( Ext, ".exe" ) == 0 )
        {
            if( stricmp( FName, "cl" ) == 0  )
                m_Tool = TOOL_COMPILER;
            if( stricmp( FName, "link" ) == 0  )
                m_Tool = TOOL_LINKER;
            if( stricmp( FName, "lib" ) == 0  )
                m_Tool = TOOL_LIBRARIAN;
        }
    }

    // Process input tokens.
    while( (pToken = T.NextToken()) )
    {
        if( (pToken[0] == '/') || (pToken[0] == '-') )
            ProcessOption( pToken );
        else
            ProcessFile( pToken );
    }

    //    
    // Decide on target.  (Careful with order of PS2 and PS2_IOP.)
    //

    if     ( m_TargetString[0]                          == 0 )      m_Target = BUILD_TARGET_UNKNOWN;
    else if( strncmp( m_TargetString+6, "_PC",      3 ) == 0 )      m_Target = BUILD_TARGET_PC;
    else if( strncmp( m_TargetString+6, "_PS2_IOP", 8 ) == 0 )      m_Target = BUILD_TARGET_PS2_IOP;
    else if( strncmp( m_TargetString+6, "_PS2_DLL", 8 ) == 0 )      m_Target = BUILD_TARGET_PS2_DLL;
    else if( strncmp( m_TargetString+6, "_PS2",     4 ) == 0 )      m_Target = BUILD_TARGET_PS2;
    else if( strncmp( m_TargetString+6, "_XBOX",    5 ) == 0 )      m_Target = BUILD_TARGET_XBOX;
    else if( strncmp( m_TargetString+6, "_GCN",     4 ) == 0 )      m_Target = BUILD_TARGET_GCN;
                                                                 
    //
    // Decide on vendor.
    //
                                                                 
    if     ( m_VendorString[0]                     == 0 )  m_Vendor = VENDOR_UNKNOWN;
    else if( strncmp( m_VendorString+6, "_MS", 3 ) == 0 )  m_Vendor = VENDOR_MS;
    else if( strncmp( m_VendorString+6, "_SN", 3 ) == 0 )  m_Vendor = VENDOR_SN;
    else if( strncmp( m_VendorString+6, "_MW", 3 ) == 0 )  m_Vendor = VENDOR_MW;

    //
    // If the TARGET is XBOX then the vendor is always Microsoft
    //
    if( m_Target == BUILD_TARGET_XBOX )
    {
        m_Vendor = VENDOR_MS;
    }

    //
    // If there is no TARGET and no VENDOR, then default to PC and Microsoft.
    //

    if( (m_Target == BUILD_TARGET_UNKNOWN) && (m_Vendor == VENDOR_UNKNOWN) )
    {
        m_Target = BUILD_TARGET_PC;
        m_Vendor = VENDOR_MS;
    }

    // Display argument state.
    if( g_Verbose )
        Dump();
}

//==============================================================================

void cmd_line_data::Dump( void )
{
    char* pToolName[]   = { "<unknown>", "Compiler", "Linker", "Librarian" };
    char* pTargetName[] = { "<unknown>", "PC", "PS2", "PS2 IOP", "X-Box", "GameCube" };
    char* pVendorName[] = { "<unknown>", "Microsoft", "SN Systems", "Metrowerks" };
    char* pSubSysName[] = { "<default>", "Windows", "WinCon" };

    OpenSection( "Processed Command Line State" );

    printf( "Tool...........%s\n", pToolName[m_Tool] );
    printf( "Optimization...%c\n", m_Optimization );
    printf( "Debug..........%s\n", m_DebugMode  ? "Yes" : "No" );
    printf( "Target.........%s (%s)\n", pTargetName[m_Target], m_TargetString );
    printf( "Vendor.........%s (%s)\n", pVendorName[m_Vendor], m_VendorString );
    printf( "SubSystem......%s\n",      pSubSysName[m_SubSystem] );
    printf( "Output path....%s\n", m_OutputPath );
    printf( "Output file....%s\n", m_OutputFile );
    printf( "SBR path.......%s\n", m_SBRPath );
    printf( "PDB path.......%s\n", m_PDBPath );
    printf( "PCH file.......%s\n", m_PCHFile );
    printf( "Link Options...%s\n", m_LinkOptions );

    printf( "Miscellaneous.." );
    if( m_Preprocess )  printf( "[Preprocess to file]  "    );
    if( m_ForceC     )  printf( "[Force C compilation]  "   );
    if( m_ForceCPP   )  printf( "[Force C++ compilation]  " );
    printf( "\n" );

    DivideSection();

    m_Define.       Dump( "Defines"             );
    m_IncludeDir.   Dump( "Include path"        );
    m_LibraryDir.   Dump( "Library path"        );
    m_SourceCode.   Dump( "Source list: C/C++"  );
    m_SourceOther.  Dump( "Source list: Other"  );
    m_ObjectFile.   Dump( "Object list"         );
    m_LibraryFile.  Dump( "Library list"        );
    m_UnknownOption.Dump( "Unknown options"     );

    CloseSection();
}

//==============================================================================
