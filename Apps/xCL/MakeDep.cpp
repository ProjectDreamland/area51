#include "x_files.hpp"
#include "Auxiliary/CommandLine/CommandLine.hpp"
#include "Auxiliary/pcre/regex.hpp"
#include "md5.h"
#include <windows.h>
#include "MakeDep.hpp"

//=============================================================================

struct node
{
    xstring         m_Key;
    xstring         m_PathName;
    xarray<s32>     m_Includes;
    u8              m_MD5Digest[16];
};

//=============================================================================

xstring             g_SourceFileName;

xarray<xstring>     g_Files;

xarray<xstring>     g_MissingFiles;

xarray<xstring>     g_IncludePaths;
xarray<dep_node>    g_DepNodes;

xarray<s32>         g_TopLevelNodes;

s32                 g_FilesOpened   = 0;
s32                 g_CacheHits     = 0;

//=============================================================================

s32 FindIncludeNode( const xstring& IncludeFileName )
{
    // Search to find an existing node given the key
    for( s32 i=0 ; i<g_DepNodes.GetCount() ; i++ )
    {
        dep_node& n = g_DepNodes[i];
        if( n.m_Key == IncludeFileName )
        {
            g_CacheHits++;
            return i;
        }
    }

    // Failed
    return -1;
}

//=============================================================================

void AddIncludeChain( s32 Index, s32 Target )
{
    ASSERT( Index != -1 );

    // Append this if it doesn't already exist
    for( s32 i=0 ; i<g_DepNodes[Target].m_Includes.GetCount() ; i++ )
    {
        if( g_DepNodes[Target].m_Includes[i] == Index )
            return;
    }
    g_DepNodes[Target].m_Includes.Append( Index );

    // Append everything this depends on
    for( i=0 ; i<g_DepNodes[Index].m_Includes.GetCount() ; i++ )
        AddIncludeChain( g_DepNodes[Index].m_Includes[i], Target );
}

//=============================================================================

void ProcessInclude( const xstring& IncludeFileName, s32 Target )
{
    // Check for a cached version of the include file
    s32 iCached = FindIncludeNode( IncludeFileName );
    if( iCached != -1 )
    {
        AddIncludeChain( iCached, Target );
        return;
    }

    // TODO: Run through all the paths to locate the include file
    for( s32 i=0 ; i<g_IncludePaths.GetCount() ; i++ )
    {
        // Make the path
        xstring Path = g_IncludePaths[i];
        Path += IncludeFileName;

        // Process it
        s32 Index = dep_ProcessFullyQualifiedFile( Path, IncludeFileName );
        if( Index != -1 )
        {
            // Found it, add to the includes
            AddIncludeChain( Index, Target );
            return;
        }
    }

    // Couldn't find this include, list it as missing!
    for( i=0 ; i<g_MissingFiles.GetCount() ; i++ )
    {
        if( g_MissingFiles[i] == IncludeFileName )
            return;
    }
    g_MissingFiles.Append( IncludeFileName );
//    x_printf( "  %s\n", IncludeFileName );
}

//=============================================================================

s32 dep_ProcessFullyQualifiedFile( const xstring& FileName, const xstring& Key )
{
    // Load the file
    g_FilesOpened++;
    xstring f;
    if( f.LoadFile( FileName ) )
    {
        // Create a node for this file
        s32         Index   = g_DepNodes.GetCount();
        dep_node&   n       = g_DepNodes.Append();

        // Setup the node
        n.m_PathName = FileName;
        n.m_Key = Key;

        // Compute checksum
        MD5Context md5;
        MD5Init     ( &md5 );
        MD5Update   ( &md5, (u8*)&f[0], f.GetLength() );
        MD5Final    ( &md5, n.m_MD5Digest );

        char OldDirectory[X_MAX_PATH];
        GetCurrentDirectory( X_MAX_PATH, OldDirectory );
        char Drive[X_MAX_PATH];
        char Dir[X_MAX_PATH];
        x_splitpath( FileName, Drive, Dir, NULL, NULL );
        xstring ChangeDir = Drive;
        ChangeDir += Dir;
        SetCurrentDirectory( ChangeDir );
//        x_printf( "Changed Dir To: %s - %s - %s - %s\n", ChangeDir, Drive, Dir, FileName );

        // Process for #include directives
        s32 StartIndex = 0;
        s32 FoundIndex = -1;
        while( (FoundIndex = f.Find( xstring("#include"), StartIndex )) != -1 )
        {
            xstring s;

            // Backtrack to ensure it's a directive at the start of a line
            s32 i = FoundIndex - 1;
            while( i >= 0 )
            {
                if( f[i] == '\n' )
                    break;

                if( !x_isspace(f[i--]) )
                    goto Skip;
            }

            // Skip #include and space to get to pathname
            i = FoundIndex + 8;
            while( x_isspace(f[i]) )
                i++;

            // Read pathname
            if( (f[i] == '"') || (f[i] == '<') )
            {
                i++;
                while( (f[i] != '"') && (f[i] != '>') && (f[i] != '\n') )
                    s += f[i++];
            }

            // Create node for this include directive
            ProcessInclude( s, Index );

Skip:
            StartIndex = FoundIndex + 1;
        }

        SetCurrentDirectory( OldDirectory );

        // Return the node index
        return Index;
    }
    else
    {
        // Failed
        return -1;
    }
}

//=============================================================================

void dep_AddIncludePath( const xstring& Path )
{
    if( Path.GetLength() == 0 )
        return;

    xstring& p = g_IncludePaths.Append();
    p = Path;

    // Ensure a trailing /
    char LastChar = p[p.GetLength()-1];
    if( (LastChar != '\\') && (LastChar != '/') )
        p += '/';
}

//=============================================================================
// main
//=============================================================================

/*

int main( int argc, char** argv )
{
    command_line CmdLine;

    CmdLine.AddOptionDef( "I", command_line::STRING );

    // Parse the command line
    if( CmdLine.Parse( argc, argv ) )
    {
        x_printf( "Usage: Makedep -i <include_path> file" );
        return 15;
    }

    // Get the include paths
    AddIncludePath( xstring(".") );

    for( s32 i=0 ; i<CmdLine.GetNumOptions() ; i++ )
    {
        if( CmdLine.GetOptionName( i ) == "I" )
        {
            AddIncludePath( CmdLine.GetOptionString( i ) );
        }
    }

    // Add Include paths from Include environment variable
    char IncludeVar[4096];
    GetEnvironmentVariable( "Include", IncludeVar, 4096 );
    const char* p = strtok( IncludeVar, ";" );
    while( p )
    {
        x_printf( "%s\n", p );
        AddIncludePath( xstring(p) );
        p = strtok( NULL, ";" );
    }

    // Read the files and resolve the includes
    for( s32 i=0 ; i<CmdLine.GetNumArguments() ; i++ )
    {
        {
            g_SourceFileName = CmdLine.GetArgument(i);

//            x_printf( "%s\n", g_SourceFileName );

            s32 Index = ProcessFullyQualifiedFile( g_SourceFileName, g_SourceFileName );
            g_TopLevelNodes.Append( Index );

            // Get MD5's for source files including all dependencies
            MD5Context md5;
            MD5Init( &md5 );
            MD5Update( &md5, g_DepNodes[Index].m_MD5Digest, 16 );
            for( s32 j=0 ; j<g_DepNodes[Index].m_Includes.GetCount() ; j++ )
            {
                MD5Update( &md5, g_DepNodes[g_DepNodes[Index].m_Includes[j]].m_MD5Digest, 16 );
            }
            MD5Final( &md5, g_DepNodes[Index].m_MD5Digest );

            {
                for( s32 k=0 ; k<16 ; k++ )
                {
                    x_printf( "%02X", g_DepNodes[Index].m_MD5Digest[k] );
                }
                x_printf( "  %s\n", g_SourceFileName );
#if 0
                for( s32 j=0 ; j<g_DepNodes[Index].m_Includes.GetCount() ; j++ )
                {
                    x_printf( "  " );
                    for( s32 k=0 ; k<16 ; k++ )
                    {
                        x_printf( "%02X", g_DepNodes[g_DepNodes[Index].m_Includes[j]].m_MD5Digest[k] );
                    }
                    x_printf( "  %s\n", g_DepNodes[g_DepNodes[Index].m_Includes[j]].m_PathName );
                }
#endif
            }
        }
    }

    x_printf( "Stats: Source = %d, Cached = %d, Opened = %d, Nodes = %d\n", CmdLine.GetNumArguments(), g_CacheHits, g_FilesOpened, g_DepNodes.GetCount() );

    x_printf( "\nMissing Files\n" );
    for( s32 i=0 ; i<g_MissingFiles.GetCount() ; i++ )
    {
        x_printf( "%s\n", g_MissingFiles[i] );
    }

    // Success
    return 0;
}

*/

//=============================================================================
