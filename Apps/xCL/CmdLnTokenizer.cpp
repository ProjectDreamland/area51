//==============================================================================
//
//  CmdLnTokenizer.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "CmdLnTokenizer.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

cmd_line_tokenizer::cmd_line_tokenizer( void )
{   
    m_Buffer[0]   = 0;
    m_BufDataSize = 0;
    m_BufCursor   = 0;

    m_pToken = NULL;

    m_InFile = false;
    m_pRFile = NULL;

    m_Arg  = 0;
    m_argc = 0;
    m_argv = (char**)1;
}

//==============================================================================

cmd_line_tokenizer::~cmd_line_tokenizer()
{
    if( m_InFile )
    {
        fclose( m_pRFile );
    }
}

//==============================================================================

void cmd_line_tokenizer::Activate( int argc, char** argv )
{
    // Make sure we don't get activated twice.
    assert( m_Arg == 0 );

    m_Arg  = 1;
    m_argc = argc;
    m_argv = argv;
}

//==============================================================================

char* cmd_line_tokenizer::GetFileToken( void )
{
    bool InQuotes = false;

    //
    // We are reading tokens from a response file.  We may need to recurse a few 
    // times before we get a result.
    //

    // Get some data in the buffer.
    while( m_BufCursor >= m_BufDataSize )
    {
        // Pull one line from the file.
        if( fgets( m_Buffer, TOKEN_BUFFER_SIZE-2, m_pRFile ) == NULL )
        {
            // We got back a NULL.  Must be the end of the file.
            // Close the file, and resume parsing the command line.
            fclose( m_pRFile );
            m_InFile       = false;
            m_RFileName[0] = '\0';
            return( NextToken() );
        }
        else
        {
            // We got something in the buffer.
            m_BufCursor   = 0;
            m_BufDataSize = strlen( m_Buffer );
        }
    }

    // Now we have data in the buffer.
    // See if we can find a token within it.

    // Skip leading whitespace.  Recurse if we run out of buffer.
    while( (isspace( m_Buffer[m_BufCursor] )) ||
           (m_Buffer[m_BufCursor] == NULL) )
    {
        m_BufCursor += 1;
        if( m_BufCursor >= m_BufDataSize )
            return( NextToken() );            
    }

    // Remember where the token starts.
    m_pToken = &m_Buffer[m_BufCursor];

    //
    // Search to the end of the token, and drop a NULL.
    //
    // Whitespace delimits tokens.  However, quotes can be used to force the 
    // inclusion of whitespace in a token.
    //
    // This loop is carefully rigged to accept tokens like:
    //      /L:"abc 123"
    //

    do
    {
        // Did we hit a quote?
        if( m_Buffer[m_BufCursor] == '\"' )
            InQuotes = !InQuotes;
        m_BufCursor += 1;
    } 
    while( m_Buffer[m_BufCursor] && 
           (InQuotes || !isspace( m_Buffer[m_BufCursor] )) );

    m_Buffer[m_BufCursor] = '\0';

    //
    // Remove surrounding double quotes.
    // (It looks like BoundsChecker places double quotes around everything
    // in command line response files.  Go figure!)
    //

    if( (*m_pToken == '"') &&
        (m_Buffer[m_BufCursor-1] == '"') &&
        (strlen(m_pToken) > 1) )
    {
        m_pToken++;
        m_Buffer[m_BufCursor-1] = '\0';
    }

    // We got our token!
    return( m_pToken );       
}

//==============================================================================

char* cmd_line_tokenizer::NextToken( void )
{
    // Make sure Activate has been called.
    assert( m_Arg > 0 );

    // Are we reading from a response file?
    if( m_InFile )
    {
        return( GetFileToken() );
    }
    
    // No more command line args?
    if( m_Arg == m_argc )
    {
        m_pToken = NULL;
        return( m_pToken );
    }

    // Check for response file.
    if( m_argv[m_Arg][0] == '@' )
    {
        // Open the response file, then recurse to get token.
        m_InFile = true;
        m_pRFile = fopen( m_argv[m_Arg]+1, "rt" );
        assert( m_pRFile );
        strcpy( m_RFileName, m_argv[m_Arg] );
        m_Arg += 1;
        return( NextToken() );
    }

    // Just return the next command line arg.
    strcpy( m_Buffer, m_argv[m_Arg] );
    m_pToken = m_Buffer;
    m_Arg   += 1;
    return( m_pToken );
}

//==============================================================================

char* cmd_line_tokenizer::CurrentToken( void )
{
    return( m_pToken );
}

//==============================================================================

void cmd_line_tokenizer::Reset( void )
{   
    // To make life easier, require that all tokens have been pulled before
    // a Reset is allowed.
    assert( !m_InFile );
    assert( m_Arg == m_argc );

    m_Buffer[0]   = 0;
    m_BufDataSize = 0;
    m_BufCursor   = 0;

    m_Arg = 1;
}

//==============================================================================

char* cmd_line_tokenizer::GetResponseFile( void )
{
    if( m_InFile )  return( m_RFileName );
    else            return( NULL );
}

//==============================================================================
