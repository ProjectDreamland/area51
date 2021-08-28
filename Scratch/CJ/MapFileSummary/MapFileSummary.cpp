//
// MapFileSummary.cpp
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include "windows.h"

#include "x_files.hpp"
#include "Auxiliary/CommandLine/CommandLine.hpp"

#include <stdio.h>

//==============================================================================
//  Symbols
//==============================================================================

struct section
{
    const char* pName;              // Name of the section
    u32         Address;            // Address of the section
    u32         Size;               // Size of the section in bytes
    u32         Hits;               // Number of hits in this seciton (that missed object files)
};

struct object
{
    s32         iSection;           // Index of the section this object file belongs to
    const char* pName;              // Name of the object file
    u32         Address;            // Address of the object file
    u32         Size;               // Size of the object file in bytes
    u32         Hits;               // Number of hits in this object file (that missed functions)
};

struct symbol
{
    s32         iSection;           // Index of the Section this function belongs to
    s32         iObject;            // Index of the Object File this function belongs to
    const char* pName;              // Name of the function
    u32         Address;            // Address of the function
    u32         Size;               // Size of the function in bytes
    s32         Hits;               // Number of samples in this function
};

xarray<section> s_Sections;
xarray<object>  s_Objects;
xarray<symbol>  s_Symbols;

xstring         MapFile;

#define PRIME_SECTION   100
#define PRIME_OBJECT    1000
#define PRIME_SYMBOL    10000

//==============================================================================
//  Utility
//==============================================================================

void endianswap( u32& v )
{
    v = ( ((v >> 24) & 0x000000ff) |
          ((v >>  8) & 0x0000ff00) |
          ((v <<  8) & 0x00ff0000) |
          ((v << 24) & 0xff000000) );
}

//==============================================================================
//  Display Help
//==============================================================================

void DisplayHelp( void )
{
    x_printf( "\n" );
    x_printf( "MapFileSummary (c)2003 Inevitable Entertainment Inc.\n" );
    x_printf( "\n" );
    x_printf( "  usage:\n" );
    x_printf( "         MapFileSummary [-opt [param]] <logs>\n" );
    x_printf( "\n" );
    x_printf( "options:\n" );
    x_printf( "         -m <mapfile>  : mapfile to use\n" );
    x_printf( "         -v            : verbose\n" );
    x_printf( "\n" );
}

//==============================================================================
//  Parsing Help
//==============================================================================

xbool ishex( char c )
{
    c = toupper(c);
    return( ((c>='0') && (c<='9')) || ((c>='A') && (c<='F')) );
}

s32 tohex( char c )
{
    c = toupper(c);
    if( c > '9' )
        return c-'A'+10;
    else
        return c-'0';
}

u32 ReadHex( xstring& s, s32& i )
{
    s32 v = 0;
    while( (i < s.GetLength()) && (ishex(s[i])) )
    {
        v <<= 4;
        v += tohex(s[i]);
        i++;
    }
    return v;
}

void SkipSpaces( xstring&s, s32& i )
{
    while( (i < s.GetLength()) && (isspace(s[i])) )
        i++;
}

void SkipField( xstring& s, s32& i )
{
    SkipSpaces( s, i );
    while( (i < s.GetLength()) && !(isspace(s[i])) )
        i++;
}

void SkipToEOL( xstring& s, s32& i )
{
    while( (i < s.GetLength()) && (s[i] != 0x0a) )
    {
        if( s[i] == 0x0d ) s[i] = 0;
        i++;
    }
    if( i < s.GetLength() )
        s[i++] = 0;
}

//==============================================================================
//  LoadMapFile
//==============================================================================

int fn_symbol_address_sort( const void* a1, const void* a2 )
{
    symbol* p1 = (symbol*)a1;
    symbol* p2 = (symbol*)a2;

    if( p1->Address < p2->Address )
        return -1;
    else if( p1->Address > p2->Address )
        return 1;
    else
        return 0;
}

//==============================================================================

int fn_symbol_size_sort( const void* a1, const void* a2 )
{
    symbol* p1 = (symbol*)a1;
    symbol* p2 = (symbol*)a2;

    if( p1->Size < p2->Size )
        return -1;
    else if( p1->Size > p2->Size )
        return 1;
    else
        return 0;
}

//==============================================================================

void SortSymbolsByAddress( void )
{
    x_qsort( &s_Symbols[0], s_Symbols.GetCount(), sizeof(symbol), fn_symbol_address_sort );
}

//==============================================================================

void SortSymbolsBySize( void )
{
    x_qsort( &s_Symbols[0], s_Symbols.GetCount(), sizeof(symbol), fn_symbol_size_sort );
}

//==============================================================================

xbool LoadMapFile( const char* pFileName )
{
    xbool   Success         = TRUE;
    s32     iCurrentSection = 0;
    s32     iCurrentObject  = 0;

    // Prime array
    s_Sections.SetCapacity( PRIME_SECTION );
    s_Objects .SetCapacity( PRIME_OBJECT  );
    s_Symbols .SetCapacity( PRIME_SYMBOL  );

    // Add undefined section
    {
        section& s  = s_Sections.Append();
        s.pName     = "<undefined>";
        s.Address   = 0;
        s.Size      = 0;
        s.Hits      = 0;
    }

    // Add undefined object
    {
        object& s   = s_Objects.Append();
        s.pName     = "<undefined>";
        s.Address   = 0;
        s.Size      = 0;
        s.Hits      = 0;
    }

    // Add undefined symbol
    {
        symbol& s   = s_Symbols.Append();
        s.pName     = "<undefined>";
        s.Address   = 0;
        s.Size      = 0;
        s.Hits      = 0;
        s.iSection  = iCurrentSection;
        s.iObject   = iCurrentObject;
    }

    // Load mapfile
    if( MapFile.LoadFile( pFileName ) )
    {
        s32         LineIndex   = 0;
        s32         Index       = 0;

        // Skip to first data line
        while( (Index < MapFile.GetLength()) && (MapFile[Index] != '0') )
            Index++;

        while( Index < MapFile.GetLength() )
        {

            LineIndex = Index;

            u32 Address = ReadHex    ( MapFile, Index );
                          SkipSpaces ( MapFile, Index );
            u32 Size    = ReadHex    ( MapFile, Index );
                          SkipField  ( MapFile, Index );
                          SkipSpaces ( MapFile, Index );

            // Section?
            if( ((Index-LineIndex) == 24) )
            {
                section& s  = s_Sections.Append();
                s.pName     = &MapFile[Index];
                s.Address   = Address;
                s.Size      = Size;
                s.Hits      = 0;

                iCurrentSection = s_Sections.GetCount()-1;
            }

            // Object?
            if( ((Index-LineIndex) == 40) )
            {
                object& s   = s_Objects.Append();
                s.pName     = &MapFile[Index];
                s.Address   = Address;
                s.Size      = Size;
                s.Hits      = 0;

                iCurrentObject = s_Objects.GetCount()-1;
            }

            // Symbol?
            if( ((Index-LineIndex) == 48) && (Size > 0) )
            {
                symbol& s  = s_Symbols.Append();
                s.pName    = &MapFile[Index];
                s.Address  = Address;
                s.Size     = Size;
                s.Hits     = 0;
                s.iSection = iCurrentSection;
                s.iObject  = iCurrentObject;
            }

            // Skip to next line
            SkipToEOL( MapFile, Index );
        }
    }
    else
    {
        x_printf( "Error - Failed to load map file '%s'\n", pFileName );
        Success = FALSE;
    }

    // Sort the symbols into address order
    SortSymbolsByAddress();

    return Success;
}

//==============================================================================
//  ApplySample
//==============================================================================

void ApplySample( u32 Address )
{
    s32 imin = 0;
    s32 imax = s_Symbols.GetCount()-1;
    s32 i;

    // Binary search of the symbols
    while( imax >= imin )
    {
        s32 i = (imin+imax)/2;

        // Check for hit
        if( (Address - s_Symbols[i].Address) < s_Symbols[i].Size )
        {
            // Hit symbol, increment counter and bail
            s_Symbols[i].Hits++;
            return;
        }
        else
        {
            // Refine the search
            if( s_Symbols[i].Address < Address )
                imin = i+1;
            else
                imax = i-1;
        }
    }

    // Search the object files for a hit
    for( i=0 ; i<s_Objects.GetCount() ; i++ )
    {
        // Check for hit
        if( (Address - s_Objects[i].Address) < s_Objects[i].Size )
        {
            // Hit symbol, increment counter and bail
            s_Objects[i].Hits++;
            return;
        }
    }

    // Increment <undefined> symbol
    s_Symbols[0].Hits++;
}

//==============================================================================
//  GetFunctionName
//==============================================================================

xbool GetFunctionName( u32 Address, const char*& pName )
{
    xbool   Found = FALSE;
    s32     imin = 0;
    s32     imax = s_Symbols.GetCount()-1;
    s32     i;

    // Binary search of the symbols
    while( imax >= imin )
    {
        s32 i = (imin+imax)/2;

        // Check for hit
        if( (Address - s_Symbols[i].Address) < s_Symbols[i].Size )
        {
            // Hit symbol, increment counter and bail
            pName = s_Symbols[i].pName;
            return TRUE;
        }
        else
        {
            // Refine the search
            if( s_Symbols[i].Address < Address )
                imin = i+1;
            else
                imax = i-1;
        }
    }

    // Search the object files for a hit
    for( i=0 ; i<s_Objects.GetCount() ; i++ )
    {
        // Check for hit
        if( (Address - s_Objects[i].Address) < s_Objects[i].Size )
        {
            // Hit symbol, increment counter and bail
            pName = s_Objects[i].pName;
            return TRUE;
        }
    }

    // Increment <undefined> symbol
    pName = "<unknown>";
    return FALSE;
}

//==============================================================================
//  DumpMapFile
//==============================================================================

void DumpMapFile( void )
{
    s32     i;

#if 1
    x_printf( "Sections\n"
              "--------\n" );
    for( i=0 ; i<s_Sections.GetCount() ; i++ )
    {
        section& s = s_Sections[i];
        x_printf( "\"%08x\"\t\"%08x\"\t\"%s\"\n", s.Address, s.Size, s.pName );
    }
    x_printf( "\n" );
#endif

#if 1
    x_printf( "Symbols\n"
        "-------\n" );
    for( i=0 ; i<s_Objects.GetCount() ; i++ )
    {
        if( s_Objects[i].Hits > 0 )
        {
            object& s = s_Objects[i];
            x_printf( "\"%08x\"\t\"%08x\"\t%d\t\"%s\"\t\"%s\"\n", s.Address, s.Size, s.Hits, s_Sections[s.iSection].pName, s.pName );
        }
    }

    for( i=0 ; i<s_Symbols.GetCount() ; i++ )
    {
        symbol& s = s_Symbols[i];
        x_printf( "\"%08x\"\t\"%08x\"\t%d\t\"%s\"\t\"%s\"\n", s.Address, s.Size, s.Hits, s_Sections[s.iSection].pName, s.pName );
    }
#endif
}

//==============================================================================

xbool FindAddressRange( const char* pSectionName, u32& StartAddress, u32& EndAddress )
{
    s32 i;

    StartAddress = 0xFFFFFFFF;
    EndAddress = 0x00000000;

    for( i=0 ; i<s_Symbols.GetCount() ; i++ )
    {
        symbol& s = s_Symbols[i];
        if( x_strcmp(s_Sections[s.iSection].pName,pSectionName)==0 )
        {
            StartAddress = MIN( StartAddress, s.Address );
            EndAddress   = MAX( EndAddress, s.Address );
        }
    }

    return ( StartAddress != 0xFFFFFFFF );
}

//==============================================================================

void DumpLargestSymbols( const char* pSectionName, s32 nSymbolsToDump )
{
    s32 i=0;
    s32 N=0;
    u32 SizeTotal=0;

    x_printf("------------------------- %s ----------------------------\n",pSectionName );
    //x_printf("\tIndex\tAddress\tSize\tSize\tSection\tSymbol\n");
    for( i=s_Symbols.GetCount()-1; i>=0; i-- )
    {
        symbol& s = s_Symbols[i];
        if( x_strcmp(s_Sections[s.iSection].pName,pSectionName)==0 )
        {
            SizeTotal += s.Size;
            x_printf( "\t%5d\t%08x\t%8d\t%8d\t%s\t%s\n", N, s.Address, s.Size, SizeTotal,s_Sections[s.iSection].pName, s.pName );
            N++;
            if( N==nSymbolsToDump )
                break;
        }
    }
}

//==============================================================================

void DumpSummary( void )
{
    u32 MinA,MaxA;

    // Find size of .text
    if( FindAddressRange(".text",MinA,MaxA) )
    {
        x_printf(".TEXT     %08X <-> %08X %10d bytes\n", MinA, MaxA, MaxA-MinA );
    }
    
    // Find size of .data
    if( FindAddressRange(".data",MinA,MaxA) )
    {
        x_printf(".DATA     %08X <-> %08X %10d bytes\n", MinA, MaxA, MaxA-MinA );
    }

    // Find size of .rodata
    if( FindAddressRange(".rodata",MinA,MaxA) )
    {
        x_printf(".RODATA   %08X <-> %08X %10d bytes\n", MinA, MaxA, MaxA-MinA );
    }

    // Find size of .text
    if( FindAddressRange(".bss",MinA,MaxA) )
    {
        x_printf(".BSS      %08X <-> %08X %10d bytes\n", MinA, MaxA, MaxA-MinA );
    }

    // SortSymbolsBySize
    SortSymbolsBySize();

    DumpLargestSymbols( ".text", 40 );
    DumpLargestSymbols( ".data", 40 );
    DumpLargestSymbols( ".rodata", 40 );
    DumpLargestSymbols( ".bss", 40 );
}

//==============================================================================
//  main
//==============================================================================

void SkipWhitespace( char*& p )
{
    while( x_isspace( *p ) )
        p++;
}

xbool ParseInt( char*& p, s32& v )
{
    SkipWhitespace( p );

    v = 0;
    s32 c = 0;
    while( x_isdigit( *p ) )
    {
        c++;
        v *= 10;
        v += *p++ - '0';
    }

    return c > 0;
}

xbool ParseHex( char*& p, u32& v )
{
    SkipWhitespace( p );

    v = 0;
    s32 c = 0;
    while( ishex( *p ) )
    {
        c++;
        v *= 16;
        v += tohex( *p++ );
    }

    return c > 0;
}

void PrintName( const char* pName )
{
    s32 Len = x_strlen( pName );
    s32 i = Len-1;

    while( (i >= 0) && (pName[i] != '\\') )
    {
        i--;
    }
    i++;
    
    printf( &pName[i] );
}

int main(int argc, char* argv[])
{
    command_line    CommandLine;
    s32             i;
    xstring         MapFileName;
    bool            Verbose = false;

    // Setup recognized command line options
    CommandLine.AddOptionDef( "V" );
    CommandLine.AddOptionDef( "M", command_line::STRING );

    // Parse command line
    xbool NeedHelp = CommandLine.Parse( argc, argv );
    if( NeedHelp || (CommandLine.GetNumOptions() == 0) )
    {
        DisplayHelp();
        return 10;
    }

    // Process options
    for( i=0 ; i<CommandLine.GetNumOptions() ; i++ )
    {
        if( CommandLine.GetOptionName(i) == "M" )
        {
            MapFileName = CommandLine.GetOptionString(i);
        }

        if( CommandLine.GetOptionName(i) == "V" )
        {
            Verbose = true;
        }
    }

    // Load the mapfile
    LoadMapFile( MapFileName );

#if 0
    // Loop through all the files
    for( i=0 ; i<CommandLine.GetNumArguments() ; i++ )
    {
        // Get Pathname of file
        const xstring& FileName = CommandLine.GetArgument( i );

        // Load and process the file
        xstring s;
        if( s.LoadFile( FileName ) )
        {
            char* p = (char*)&s[0];
            s32 Sequence;
            while( ParseInt( p, Sequence ) )
            {
                printf( "%5d: ", Sequence );

                if( (*p != 0) && (*p++ == ':') )
                {
                    u32 Addr;
                    if( ParseHex( p, Addr ) )
                    {
                        const char* pName;
                        if( GetFunctionName( Addr, pName ) )
                            PrintName( pName );
                        else
                            printf( "%08X", Addr );

                        while( *p++ == '\\' )
                        {
                            printf( " - " );

                            if( ParseHex( p, Addr ) )
                            {
                                if( GetFunctionName( Addr, pName ) )
                                    PrintName( pName );
                                else
                                    printf( "%08X", Addr );
                            }
                            else
                                break;
                        }

                        printf( "\n" );
                    }
                }
            }
        }
    }
#endif

    // Dump the mapfile
    if( Verbose )
    {
        DumpMapFile();
    }
    else
    {
        DumpSummary();
    }

    // Exit
    return 0;
}

