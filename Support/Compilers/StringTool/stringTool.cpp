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
#include "Parsing/textout.hpp"

//==============================================================================
//  Defines
//==============================================================================

#define MAX_COLUMNS     5                   // Number of columns we are interested in
#define VERSION         "v1.5b"
#define MAX_PLATFORMS   3
#define PLATFORM_PC     0
#define PLATFORM_PS2    1
#define PLATFORM_XBOX   2

//==============================================================================
//  Display Help
//==============================================================================

void DisplayHelp( void )
{
    x_printf( "\n" );
    x_printf( "stringTool (c)2001-2003 Inevitable Entertainment Inc.\n" );
    x_printf( xfs("Version %s\n",VERSION) );
    x_printf( "\n" );
    x_printf( "  usage:\n" );
    x_printf( "         stringTool [-opt [param]] [txtfile|binfile]\n" );
    x_printf( "\n" );
    x_printf( "options:\n" );
    //x_printf( "         -output <folder>  - Set output folder for writing\n" );
    //x_printf( "         -p <prefix>       - Prefix all identifiers with prefix\n" );
    //x_printf( "         -overwrite        - Overwrite output files if they exist\n" );
    //x_printf( "         -gcn              - Bit Swap for GameCube\n" );
    //x_printf( "         -info             - Output tOut file that has the IDS\n" );
    x_printf( "         -debug            - Output DEBUG ID names (col 1)\n" );
    x_printf( "         -pc               - Output destination for PC platform\n" );
    x_printf( "         -ps2              - Output destination for PS2 platform\n" );
    x_printf( "         -xbox             - Output destination for XBox platform\n" );


    x_printf( "File Structure\n" );
    x_printf( "nStrings(32)                 // number of strings\n" );
    x_printf( "nStringOffsets(32)           // offsets to StringIDs\n" );
    x_printf( "StringID(null)String(null)   // String ID followed by Text String\n" );
    x_printf( "...                          // \n" );                   
    x_printf( "StringID(null)String(null)   // Repeated nStrings times\n" );
    x_printf( "\n" );
}

//==============================================================================
//  HexValue
//==============================================================================

s32 HexValue( xwchar H )
{
    s32 Value;
    if( IN_RANGE( '0', H, '9' ) )   Value = H - '0';
    if( IN_RANGE( 'A', H, 'F' ) )   Value = H - 'A' + 10;
    if( IN_RANGE( 'a', H, 'f' ) )   Value = H - 'a' + 10;
    return( Value );
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
    //xbool           OutputFolderSet = FALSE;
    xbool           DoPrefix        = FALSE;
    xbool           Overwrite       = TRUE;
	xbool			GCNOutput		= FALSE;
	xbool			Info			= FALSE;
	xbool			SubTitleMode    = TRUE;
    xbool           DebugOutput     = FALSE;
    xstring         BinName[ MAX_PLATFORMS ];
    s32             i;
    s32             iDefine = 0;
    s32             NextColOffset = 0;

    //-- Init x_Files and Memory
    x_Init( argc, argv );
    //x_MemInit();

    // Setup recognized command line options
    //CommandLine.AddOptionDef( "OVERWRITE" );
    //CommandLine.AddOptionDef( "P", command_line::STRING );
    //CommandLine.AddOptionDef( "OUTPUT", command_line::STRING );
	//CommandLine.AddOptionDef( "GCN" );
	//CommandLine.AddOptionDef( "INFO" );
	//CommandLine.AddOptionDef( "SUBTITLE" );
	CommandLine.AddOptionDef( "DEBUG" );
    CommandLine.AddOptionDef( "PC", command_line::STRING );
    CommandLine.AddOptionDef( "PS2", command_line::STRING );
    CommandLine.AddOptionDef( "XBOX", command_line::STRING );

    // Parse command line
    NeedHelp = CommandLine.Parse( argc, argv );
    
    if( NeedHelp || (CommandLine.GetNumArguments() == 0) )
    {
        DisplayHelp();
        return 10;
    }

    // Check output folder option
    iOpt = CommandLine.FindOption( xstring("PC") );
    if( iOpt != -1 )
    {
        BinName[PLATFORM_PC] = CommandLine.GetOptionString( iOpt );
    }
    else
    {
        BinName[PLATFORM_PC].Clear();
    }

    // Check output folder option
    iOpt = CommandLine.FindOption( xstring("PS2") );
    if( iOpt != -1 )
    {
        BinName[PLATFORM_PS2] = CommandLine.GetOptionString( iOpt );
    }
    else
    {
        BinName[PLATFORM_PS2].Clear();
    }

    // Check output folder option
    iOpt = CommandLine.FindOption( xstring("XBOX") );
    if( iOpt != -1 )
    {
        BinName[PLATFORM_XBOX] = CommandLine.GetOptionString( iOpt );
    }
    else
    {
        BinName[PLATFORM_XBOX].Clear();
    }
/*
    // Check prefix option
    iOpt = CommandLine.FindOption( xstring("P") );
    if( iOpt != -1 )
    {
        DoPrefix = TRUE;
        Prefix = CommandLine.GetOptionString( iOpt );
    }

    // Check overwrite option
    Overwrite = (CommandLine.FindOption( xstring("OVERWRITE") ) != -1);

	// Check for GCN option
	GCNOutput = (CommandLine.FindOption( xstring("GCN") ) != -1 );

	// Check for GCN option
	Info = (CommandLine.FindOption( xstring("INFO") ) != -1 );

//    SubTitleMode = (CommandLine.FindOption( xstring("SUBTITLE") ) != -1 );
*/
    DebugOutput = (CommandLine.FindOption( xstring("DEBUG") ) != -1 );

    x_printf(xfs("\nStringTool %s started on (%s) file.\n",VERSION,CommandLine.GetArgument( 0 )));

    // Loop through all the files
    for( i=0 ; i<CommandLine.GetNumArguments() ; i++ )
    {
        // Get Pathname of file
        const xstring& TextName = CommandLine.GetArgument( i );

        //     = CommandLine.ChangeExtension( TextName, "stringbin" );
        xstring     CodeName	= CommandLine.ChangeExtension( TextName, "cpp" );
        xstring     OutName	    = CommandLine.ChangeExtension( TextName, "info" );
		text_out	tOut;

        xstring     Path;
        xstring     File;
        xwstring    Text;
        xarray<xwstring>    SortedStrings;
        xarray<xstring>     StringsID;
        xwstring    Line;
        s32         Index       = 0;
        s32         nDefine     = 0;
        s32         iIndex;
        s32         nEntries    = 0;
        s32         nColumns;
        s32         Column[MAX_COLUMNS];
        const xwchar term = '\0';

        xbytestream Binary;
        xbytestream IndexTable;

        // Change Path if output folder set
/*        if( OutputFolderSet )
        {
            CommandLine.SplitPath( BinName, Path, File );
            BinName = CommandLine.JoinPath( OutputFolder, File );
       
            CommandLine.SplitPath( CodeName, Path, File );
            CodeName = CommandLine.JoinPath( OutputFolder, File );

            CommandLine.SplitPath( OutName, Path, File );
            OutName = CommandLine.JoinPath( OutputFolder, File );
		}
*/
		if( Info )
		{
			x_try;
            
                tOut.OpenFile( OutName );
            
            x_catch_begin;
            
				//xExceptionDisplay();
                x_printf( "Error Opening Info FILE\n" );
				return 0;

            x_catch_end;
		}


        // Load the string
        if( Text.LoadFile( TextName ) == FALSE )
            x_printf( "Error Loading File - \"%s\" \n", TextName );

		StringsID.Clear();

        // Skip blank lines at beginning
        while( (Index < Text.GetLength()) && ((Text[Index] == 0x0d) || (Text[Index] == 0x0a)) ) 
            Index++;

        // Binary sort the strings.
        SortedStrings.Clear();
        xbool IsInserted = FALSE;
        s32 i;
        xbool IDFound = FALSE;
        while( (Index < Text.GetLength()) && (Text[Index] != 0x00) )
        {
            if( Text[Index] == 0x09 )
                IDFound = TRUE;

            if( (Text[Index] == 0x0d) )
            {
                Line += Text[Index];
                Index++;
                
                if( Text[Index] == 0x0a )
                {
                    Line += Text[Index];
                    Index++;
                }

                //Line += term;

                for( i = 0 ; i < SortedStrings.GetCount() ; i++ )
                {

                    s32 retval = x_wstrcmp( Line, SortedStrings[i] );

                    if( retval < 0 )
                    {
                        SortedStrings.Insert( i, Line );
                        IsInserted = TRUE;
                        break;
                    }
                }

                if( !IsInserted )
                    SortedStrings.Append( Line );
               
                IsInserted = FALSE;
                IDFound = FALSE;
                Line.Clear();
            }
            else
            {
                if( !IDFound )
                    Line += x_toupper( (const char)Text[Index] );
                else
                    Line += Text[Index];

                Index++;
            }
        }

        Text.Clear();
        for( i = 0 ; i < SortedStrings.GetCount() ; i ++ )
            Text += SortedStrings[i];

        // Skip blank lines at beginning
        Index = 0;
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
            NextColOffset=0;

            if( (nColumns > 0) && (x_wstrlen( &Text[Column[0]] ) > 0) /*&& (x_wstrlen( &Text[Column[1]] ) > 0)*/ )
            {
                if( DebugOutput )
                {
                    x_printf("%s\n",xfs("<%04i> %s",nEntries ,(const char*)xstring(&Text[Column[0]])));
                    x_printf("%s\n",xfs("<p>%s", (const char*)xstring(&Text[Column[1]])));
                    x_printf("%s\n",xfs("<p>%s", (const char*)xstring(&Text[Column[2]])));
                }

                // Process text to remove bogus characters that excel kindly places in there
                {
                    s32 End = Column[1] + x_wstrlen( &Text[Column[1]] );
                        
                    for( s32 i=Column[1]; i<End; i++ )
                    {
                        // Extra hack to handle playstation 2 action cluster buttons

                        // Following 5 are removed to support extened char set - JHOWA
//                        if( Text[i] == 0x00C7 ) Text[i] = 128;
//                        if( Text[i] == 0x00FC ) Text[i] = 129;
//                        if( Text[i] == 0x00E9 ) Text[i] = 130;
//                        if( Text[i] == 0x00E2 ) Text[i] = 131;
//                        if (Text[i] == 0x0092 ) Text[i] = '\'';

                        if( Text[i] == 0x2013 ) Text[i] = '!';
                        if( Text[i] == 0x2018 ) Text[i] = '\'';
                        if( Text[i] == 0x2019 ) Text[i] = '\'';
                        if( Text[i] == 0x201C ) Text[i] = '\"';
                        if( Text[i] == 0x201D ) Text[i] = '\"';
                        if( Text[i] == 0x2122 ) Text[i] = 0x0012;   // tm
                        if( Text[i] == 0x201E ) Text[i] = 0x0011;   // German leading Quote
                        if( Text[i] == 0x2026 || Text[i] == 0x0085)
                        {
                            Text[i] = '.';
                            Text.Insert( i, xwstring(".") );
                            Text.Insert( i, xwstring(".") );
                            End += 2;
                            NextColOffset+=2;
                        }
                        if( Text[i] > 0x00FF )
                        {
                            Text[i] = '?';
                            //x_printf( "Warning: 0x%04X\n", Text[i] );
                        }

                        if( Text[i] == 0x0092 )
                            Text[i] = '\'';
                    }
                }


                //-- IDS for Info File.
				StringsID.Append( xstring( xfs("%s", (const char*)xstring(&Text[Column[0]]))) );

                nDefine++;

                // Write Offset into index table
                iIndex = Binary.GetLength();
                if (DebugOutput)
                    x_printf("index = %d\n", iIndex);

                IndexTable += (byte)((iIndex>> 0)&0xff);
                IndexTable += (byte)((iIndex>> 8)&0xff);
                IndexTable += (byte)((iIndex>>16)&0xff);
                IndexTable += (byte)((iIndex>>24)&0xff);

                // Write wide string into Binary
                if( nColumns > 1 )
                {
                    s32     Length = x_wstrlen( &Text[Column[1]] );
                    xwchar* pSearch;

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

                    xwchar* pQuote;
                    pSearch = &Text[Column[1]];

                    while( (pQuote = x_wstrstr( pSearch, xwstring( "\"\"" ) )) )
                    {
                        s32 Position = pQuote - (const xwchar*)Text;
                        Text.Delete( Position );
                        Index  -= 1;
                        Length -= 1;
                        pSearch = pQuote + 1;
                    }

                    // Look for embedded color codes.
                    // Form is "~RRGGBB~" with the RGB values in hex.
                    // Output is 0xFFRR, 0xGGBB where RGB are non-zero byte values.

                    xwchar* pColor;
                    pSearch = &Text[Column[1]];

                    while( (pColor = x_wstrchr( pSearch, '~' )) )
                    {
                        s32 Position = pColor - (const xwchar*)Text;

                        if( Text[Position+7] != '~' )
                        {
                            pSearch = pColor + 1;
                            continue;
                        }

                        xwchar  P = 0;
                        xwchar  Q = 0;
                        u8      R, G, B;

                        R   = HexValue( Text[Position+1] );
                        R <<= 4;
                        R  += HexValue( Text[Position+2] );

                        G   = HexValue( Text[Position+3] );
                        G <<= 4;
                        G  += HexValue( Text[Position+4] );

                        B   = HexValue( Text[Position+5] );
                        B <<= 4;
                        B  += HexValue( Text[Position+6] );

                        P   = 0xFF;
                        P <<= 8;
                        P  |= R;

                        Q   = G;
                        Q <<= 8;
                        Q  |= B;

                        if( Q == 0 )    Q = 1;  // Don't let a color make a NULL.

                        Text.Delete( Position+1, 6 );
                        Text[Position+0] = P;
                        Text[Position+1] = Q;

                        Length -= 6;
                        Index  -= 6;

                        pSearch = pColor + 1;
                    }

                    
                    //-- Append Title
                    Binary.Append( (byte*)&Text[Column[0]], x_wstrlen( &Text[Column[0]] )*2 );
                    Binary.Append((byte*)&term,sizeof(xwchar));

                    //-- Append String
                    Binary.Append( (byte*)&Text[Column[1]], Length*2 );
                    Binary.Append((byte*)&term,sizeof(xwchar));

                    //-- If SubtitleMode then output the name of the speaker from Column 3
                    if( SubTitleMode && nColumns > 2)
                    {
                        if( x_wstrlen( &Text[Column[2]] ) > 0 )
                        {
                            Binary.Append( (byte*)&Text[Column[2]+NextColOffset], (x_wstrlen( &Text[Column[2]+NextColOffset] )*2)  );
                            Binary.Append((byte*)&term,sizeof(xwchar));
                        }
                        else
                            Binary.Append((byte*)&term,sizeof(xwchar));
                    }
                    else if( SubTitleMode )
                        Binary.Append((byte*)&term,sizeof(xwchar));

                    if( nColumns > 3 )
                    {
                        if( x_wstrlen( &Text[Column[3]] ) > 0 )
                        {
                            Binary.Append( (byte*)&Text[Column[3]+NextColOffset], (x_wstrlen( &Text[Column[3]+NextColOffset] )*2)  );
                            Binary.Append((byte*)&term,sizeof(xwchar));
                        }
                        else
                        {
                            Binary.Append( (byte*)&term, sizeof(xwchar));
                        }
                    }
                    else
                    {
                        Binary.Append( (byte*)&term, sizeof(xwchar));
                    }
                }
                else
                {
                    Binary.Append((byte*)&term,sizeof(xwchar));
                }

//                x_printf("%s\n",xfs("%s", (const char*)xstring(&Text[Column[0]])));
//                x_printf("%s\n",xfs("%s", (const char*)xstring(&Text[Column[1]])));
//                x_printf("%s\n",xfs("%s", (const char*)xstring(&Text[Column[2]+NextColOffset])));
//                x_printf("%s\n",xfs("%s", (const char*)xstring(&Text[Column[3]+NextColOffset])));
//                x_printf("%i\n",NextColOffset);

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
            while( (Index < Text.GetLength()) && 
                   ((Text[Index] == 0x00) || 
                    (Text[Index] == 0x0d) || 
                    (Text[Index] == 0x0a)) ) 
                Index++;

        } while( Index < Text.GetLength() );

		//-- Info Output does not need a header.. .info file is a loaded header file so we dont have to recompile the game to add new
		//-- lines of text to the Excel strings file.
		if( Info )
		{
	        x_printf( "Saveing  \"%s\"\n", OutName );			
			tOut.AddHeader( "Strings", StringsID.GetCount());
			for( s32 index = 0 ; index < StringsID.GetCount(); index++ )
			{
				tOut.AddString( "StringID",	(const char *)StringsID[index] );
				tOut.AddEndLine();
			}
			tOut.CloseFile();
		}

        // Write Binary
		IndexTable.Insert( 0, (byte)((nEntries>>24)&0xFF) );
		IndexTable.Insert( 0, (byte)((nEntries>>16)&0xFF) );
		IndexTable.Insert( 0, (byte)((nEntries>> 8)&0xFF) );
		IndexTable.Insert( 0, (byte)((nEntries>> 0)&0xFF) );

		//-- If we are gamecube then swap the bits..
		if( GCNOutput )
		{
			byte*	pindexData = IndexTable.GetBuffer();
			u32		ndata32;
			u32*	temp32;

			//-- Swap the IndexTable
   			for( s32 index = 0 ; index < IndexTable.GetLength()/4 ; index ++ )
			{
				temp32 = (u32*)pindexData;
				ndata32 = *temp32;
				*temp32 = ENDIAN_SWAP_32(ndata32);
				pindexData = (byte*)temp32;
				pindexData+=4;
			}

			byte*	pdata = Binary.GetBuffer();
			u16		ndata16;
			u16*	temp16;

			//-- Swap the Strings
   			for( index = 0 ; index < Binary.GetLength()/2 ; index ++ )
			{
				temp16 = (u16*)pdata;
				ndata16 = *temp16;
				*temp16 = ENDIAN_SWAP_16(ndata16);
				pdata = (byte*)temp16;
				pdata+=2;
			}
		}

        Binary.Insert( 0, IndexTable );

	    for( s32 iPlatform = PLATFORM_PC; iPlatform < MAX_PLATFORMS; iPlatform++ )
        {	
            if( BinName[iPlatform].GetLength() == 0 )
                continue;

            if( Overwrite || !CommandLine.FileExists( BinName[iPlatform] ) )
		    {
			    // Save the file
			    if( !Binary.SaveFile( BinName[iPlatform] ) )
			    {
				    x_printf( "Error - Saving Binary \"%s\"\n", BinName[iPlatform] );
			    }
			    else
			    {
				    x_printf( "Saving Binary \"%s\"\n", BinName[iPlatform] );
			    }
		    }
		    else
		    {
			    // Display error
			    x_printf( "Error - File \"%s\" already exists\n", BinName[iPlatform] );
		    }
        }
    }
    

    // Return success
    //x_MemKill();
    x_Kill();

    return 0;
}