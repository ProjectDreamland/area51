//==============================================================================
//==============================================================================
//  stringTool
//==============================================================================
//==============================================================================
//
//  String Table conversion tool
//
//
//
//==============================================================================
//==============================================================================

#include "x_files.hpp"
#include "x_bytestream.hpp"
#include "Auxiliary/CommandLine/CommandLine.hpp"

//==============================================================================
//  Defines
//==============================================================================

#define MAX_COLUMNS     3                   // Number of columns we are interested in

//==============================================================================
//  Display Help
//==============================================================================

void DisplayHelp( void )
{
    x_printf( "\n" );
    x_printf( "stringTool (c)2001 Inevitable Entertainment Inc.\n" );
    x_printf( "\n" );
    x_printf( "  usage:\n" );
    x_printf( "         stringTool [-opt [param]] [txtfile|binfile]\n" );
    x_printf( "\n" );
    x_printf( "options:\n" );
    x_printf( "         -output  <folder>  - Set output folder for writing\n" );
    x_printf( "         -c                 - Convert unicode txt to bin & header\n" );
    x_printf( "         -p [<prefix>]      - Prefix all identifiers with prefix, default \"IDS_\"\n" );
    x_printf( "         -overwrite         - Overwrite output files if they exist\n" );
    x_printf( "\n" );
}

//==============================================================================
//  main
//==============================================================================

int main( int argc, char** argv )
{
    command_line    CommandLine;
    xbool           NeedHelp;
    s32             iOpt;
    xstring         OutputFolder;
    xstring         Prefix;
    xbool           OutputFolderSet = FALSE;
    xbool           DoConvert       = FALSE;
    xbool           DoPrefix        = FALSE;
    xbool           Overwrite       = FALSE;
    s32             i;
    s32             iDefine = 0;

    // Setup recognized command line options
    CommandLine.AddOptionDef( "OVERWRITE" );
    CommandLine.AddOptionDef( "C" );
    CommandLine.AddOptionDef( "P", command_line::STRING );
    CommandLine.AddOptionDef( "OUTPUT", command_line::STRING );

    // Parse command line
    NeedHelp = CommandLine.Parse( argc, argv );
    if( NeedHelp || (CommandLine.GetNumArguments() == 0) )
    {
        DisplayHelp();
        return 10;
    }

    // Check output folder option
    iOpt = CommandLine.FindOption( xstring("OUTPUT") );
    if( iOpt != -1 )
    {
        OutputFolder    = CommandLine.GetOptionString( iOpt );
        OutputFolderSet = TRUE;

        s32 StringLen = OutputFolder.GetLength();
        if( (StringLen > 0) && ((OutputFolder[StringLen-1] != '\\') && (OutputFolder[StringLen-1] != '/')) )
            OutputFolder += '\\';
    }

    // Check prefix option
    iOpt = CommandLine.FindOption( xstring("P") );
    if( iOpt != -1 )
    {
        DoPrefix = TRUE;
        Prefix = CommandLine.GetOptionString( iOpt );
    }

    // Check convert option
    DoConvert = (CommandLine.FindOption( xstring("C") ) != -1);

    // Check overwrite option
    Overwrite = (CommandLine.FindOption( xstring("OVERWRITE") ) != -1);

    // Loop through all the files
    for( i=0 ; i<CommandLine.GetNumArguments() ; i++ )
    {
        // Get Pathname of file
        const xstring& TextName = CommandLine.GetArgument( i );

        // Should I convert it?
        if( DoConvert )
        {
            xstring     BinName     = CommandLine.ChangeExtension( TextName, "bin" );
            xstring     HeaderName  = CommandLine.ChangeExtension( TextName, "h" );
            xstring     Path;
            xstring     File;
            xwstring    Text;
            xstring     Header;
            xwstring    Line;
            s32         Index       = 0;
            s32         nDefine     = 0;
            s32         iIndex;
            s32         nEntries    = 0;
            s32         nColumns;
            s32         Column[MAX_COLUMNS];

            xbytestream Binary;
            xbytestream IndexTable;

            // Change Path if output folder set
            if( OutputFolderSet )
            {
                CommandLine.SplitPath( BinName, Path, File );
                BinName = CommandLine.JoinPath( OutputFolder, File );
                CommandLine.SplitPath( HeaderName, Path, File );
                HeaderName = CommandLine.JoinPath( OutputFolder, File );
            }

            // Load the string
            VERIFY( Text.LoadFile( TextName ) );

            // Write banner at top of header file
            Header += "//------------------------------------------------------------------------------\n";
            Header += "//  DO NOT EDIT - automatically generated by 'StringTool'\n";
            Header += "//------------------------------------------------------------------------------\n";
            Header += "\n";

            // Skip blank lines at beginning
            while( (Index < Text.GetLength()) && ((Text[Index] == 0x0d) || (Text[Index] == 0x0a)) ) 
                Index++;

            // Compile to Binary and Header
            do
            {
                // Read line from Text File
                nColumns  = 0;
                Column[0] = Index;
                while( (Index < Text.GetLength()) && (Text[Index] != 0x00) )
                {
                    // Check for new column
                    if( Text[Index] == 0x09 )
                    {
                        Text[Index] = 0;
                        Index++;
                        nColumns++;
                        if( nColumns < MAX_COLUMNS )
                            Column[nColumns] = Index;
                    }

                    // Check for end of line
                    else if( (Text[Index] == 0x0d) )
                    {
                        Text[Index] = 0;
                        Index++;
                        if( Text[Index] == 0x0a )
                        {
                            Text[Index] = 0;
                            Index++;
                        }
                        nColumns++;
                        if( nColumns < MAX_COLUMNS )
                            Column[nColumns] = Index;

                        // Exit enclosing while
                        break;
                    }

                    // Just advance to next character
                    else
                        Index++;
                }

                // Is it a valid line?
                if( (nColumns > 0) && (x_wstrlen( &Text[Column[0]] ) > 0) && (x_wstrlen( &Text[Column[1]] ) > 0) )
                {
                    // Write to Header String
                    Header.AddFormat( "#define %s%-50s %5d\n", Prefix, xstring(&Text[Column[0]]), nDefine );
                    nDefine++;

                    // Write Offset into index table
                    iIndex = Binary.GetLength();
                    IndexTable += (byte)((iIndex>> 0)&0xff);
                    IndexTable += (byte)((iIndex>> 8)&0xff);
                    IndexTable += (byte)((iIndex>>16)&0xff);
                    IndexTable += (byte)((iIndex>>24)&0xff);

                    // Write wide string into Binary
                    if( nColumns > 1 )
                    {
                        s32 Length = x_wstrlen( &Text[Column[1]] );

                        // Adjust string to remove enclosing quotes (automatically placed by excel around
                        // lines that have an embedded newline)
                        if( Length > 1 )
                        {
                            if( (Text[Column[1]] == '"') && (Text[Column[1]+Length-1] == '"') )
                            {
                                Column[1]++;
                                Length -= 2;
                            }
                        }
                        Binary.Append( (byte*)&Text[Column[1]], Length*2 );
                    }
                    Binary += 0;
                    Binary += 0;

                    // Increment number of entries
                    nEntries++;
                }
                /*
                else
                {
                    // No - display error
                    x_printf( " Error - Invalid line\n" );
                }
                */

                // Advance to beginning of next line
                while( (Index < Text.GetLength()) && ((Text[Index] == 0x0d) || (Text[Index] == 0x0a)) ) 
                    Index++;

            } while( Index < Text.GetLength() );

            // Write Header
            if( Overwrite || !CommandLine.FileExists( HeaderName ) )
            {
                // Save the file
                if( !Header.SaveFile( HeaderName ) )
                {
                    x_printf( "Error - Saving Header \"%s\"\n", HeaderName );
                }
                else
                {
                    x_printf( "        Saving Header \"%s\"\n", HeaderName );
                }
            }
            else
            {
                // Display error
                x_printf( "Error - File \"%s\" already exists\n", HeaderName );
            }

            // Write Binary
            IndexTable.Insert( 0, (byte)((nEntries>>24)&0xFF) );
            IndexTable.Insert( 0, (byte)((nEntries>>16)&0xFF) );
            IndexTable.Insert( 0, (byte)((nEntries>> 8)&0xFF) );
            IndexTable.Insert( 0, (byte)((nEntries>> 0)&0xFF) );
            Binary.Insert( 0, IndexTable );
            if( Overwrite || !CommandLine.FileExists( BinName ) )
            {
                // Save the file
                if( !Binary.SaveFile( BinName ) )
                {
                    x_printf( "Error - Saving Binary \"%s\"\n", BinName );
                }
                else
                {
                    x_printf( "        Saving Binary \"%s\"\n", BinName );
                }
            }
            else
            {
                // Display error
                x_printf( "Error - File \"%s\" already exists\n", BinName );
            }
        }
    }

    // Return success
    return 0;
}